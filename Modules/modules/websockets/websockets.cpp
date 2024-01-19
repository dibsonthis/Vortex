#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS
#ifdef _WIN32 || _WIN64
// #define WIN32_LEAN_AND_MEAN
#define _WEBSOCKETPP_MINGW_THREAD_
#endif
// #include "Vortex/Node/Node.hpp"
#include <functional>
#include "Vortex/Lexer/Lexer.hpp"
#include "Vortex/Parser/Parser.hpp"
#include "Vortex/Bytecode/Bytecode.hpp"
#include "Vortex/Bytecode/Generator.hpp"
#include "Vortex/VirtualMachine/VirtualMachine.hpp"
#include "include/websocketpp/asio.hpp"

// TLS or no TLS
#include "include/websocketpp/config/asio_client.hpp" // TLS
#include "include/websocketpp/config/asio.hpp"
// #include "include/websocketpp/config/asio_no_tls_client.hpp"	// no TLS

#include "include/websocketpp/client.hpp"
#include "include/websocketpp/server.hpp"
#include "include/websocketpp/common/thread.hpp"
#include "include/websocketpp/common/memory.hpp"

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::server<websocketpp::config::asio_tls> server;

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

using context_ptr = std::shared_ptr<asio::ssl::context>;

static context_ptr on_tls_init()
{
    context_ptr ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);

    try
    {
        ctx->set_options(
            asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::no_sslv3 | asio::ssl::context::single_dh_use);

        ctx->set_verify_mode(asio::ssl::verify_none);
    }
    catch (std::exception &e)
    {
        std::cout << "Error in context pointer: " << e.what() << std::endl;
    }
    return ctx;
}

extern "C" Value _client_init(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'init_client' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    client *c = new client();

    c->clear_access_channels(websocketpp::log::alevel::frame_header);
    c->clear_access_channels(websocketpp::log::alevel::frame_payload);
    // c->set_error_channels(websocketpp::log::elevel::none);

    // Initialize ASIO
    c->init_asio();

    c->set_tls_init_handler(websocketpp::lib::bind(&on_tls_init));

    Value client_ptr = pointer_val();
    client_ptr.get_pointer()->value = (void *)c;

    return client_ptr;
}

extern "C" Value _client_connect(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'client_connect' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value url = args[1];
    Value headers = args[2];

    if (!client_ptr.is_pointer())
    {
        error("Function 'client_connect' expects argument 'client' to be a pointer");
    }

    if (!url.is_string())
    {
        error("Function 'client_connect' expects argument 'url' to be a string");
    }

    if (!headers.is_object())
    {
        error("Function 'client_connect' expects argument 'headers' to be a object");
    }

    client *c = (client *)client_ptr.get_pointer()->value;

    // Create a connection to the given URI and queue it for connection once
    // the event loop starts
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c->get_connection(url.get_string(), ec);

    if (ec)
    {
        error("Could not create connection: " + ec.message());
    }

    auto headers_object = headers.get_object();

    for (auto &x : headers_object->values)
    {
        if (!x.second.is_string())
        {
            std::cout << "Header warning: ignoring '" + x.first + "' - value must be a string" << std::endl;
        }
        else
        {
            (con)->append_header(x.first, x.second.get_string());
        }
    }

    (con)->append_header("access-control-allow-origin", "*");
    c->connect(con);

    Value con_ptr = pointer_val();
    con_ptr.get_pointer()->value = (void *)con.get();

    return con_ptr;
}

extern "C" Value _client_run(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'client_connect' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];

    if (!client_ptr.is_pointer())
    {
        error("Function 'client_connect' expects argument 'client' to be a pointer");
    }

    client *c = (client *)client_ptr.get_pointer()->value;

    c->run();
    c->reset();

    return none_val();
}

extern "C" Value _client_on_message(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_message' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_message' expects argument 'function' to be a Function");
    }

    if (!client_ptr.is_pointer())
    {
        error("Function 'on_message' expects argument 'client' to be a pointer");
    }

    if (func.get_function()->arity != 1)
    {
        error("Function 'on_message' expects argument 'function' to be a Function with 1 parameter");
    }

    client *c = (client *)client_ptr.get_pointer()->value;

    auto on_message_func = [func](websocketpp::connection_hdl hdl, message_ptr msg)
    {
        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_constant(main->chunk, string_val(msg->get_payload()));
        add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 1, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    c->set_message_handler(on_message_func);

    return none_val();
}

extern "C" Value _client_on_open(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_open' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_open' expects argument 'function' to be a Function");
    }

    if (!client_ptr.is_pointer())
    {
        error("Function 'on_open' expects argument 'client' to be a pointer");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_open' expects argument 'function' to be a Function with 0 parameters");
    }

    client *c = (client *)client_ptr.get_pointer()->value;

    auto on_open_func = [func](websocketpp::connection_hdl hdl)
    {
        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    c->set_open_handler(on_open_func);

    return none_val();
}

extern "C" Value _client_on_close(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_close' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_close' expects argument 'function' to be a Function");
    }

    if (!client_ptr.is_pointer())
    {
        error("Function 'on_close' expects argument 'client' to be a pointer");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_close' expects argument 'function' to be a Function with 0 parameters");
    }

    client *c = (client *)client_ptr.get_pointer()->value;

    auto on_close_func = [func](websocketpp::connection_hdl hdl)
    {
        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    c->set_close_handler(on_close_func);

    return none_val();
}

extern "C" Value _client_on_fail(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_fail' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_fail' expects argument 'function' to be a Function");
    }

    if (!client_ptr.is_pointer())
    {
        error("Function 'on_fail' expects argument 'client' to be a pointer");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_fail' expects argument 'function' to be a Function with 0 parameters");
    }

    client *c = (client *)client_ptr.get_pointer()->value;

    auto on_fail_func = [func](websocketpp::connection_hdl hdl)
    {
        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    c->set_fail_handler(on_fail_func);

    return none_val();
}

extern "C" Value _client_send(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'client_send' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value con_ptr = args[1];
    Value message = args[2];

    if (!client_ptr.is_pointer())
    {
        error("Function 'client_send' expects argument 'client' to be a pointer");
    }

    if (!con_ptr.is_pointer())
    {
        error("Function 'client_send' expects argument 'con_ptr' to be a pointer");
    }

    if (!message.is_string())
    {
        error("Function 'client_send' expects argument 'message' to be a string");
    }

    client *c = (client *)client_ptr.get_pointer()->value;
    client::connection_type *con = (client::connection_type *)con_ptr.get_pointer()->value;

    if (c->stopped())
    {
        std::cout << "Connection is not running - cannot send" << std::endl;
        return none_val();
    }

    c->send((*con).get_handle(), message.get_string(), websocketpp::frame::opcode::text);

    return none_val();
}

extern "C" Value _client_close(std::vector<Value> &args)
{
    int num_required_args = 4;

    if (args.size() != num_required_args)
    {
        error("Function 'client_close' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value client_ptr = args[0];
    Value con_ptr = args[1];
    Value code = args[2];
    Value reason = args[3];

    if (!client_ptr.is_pointer())
    {
        error("Function 'client_close' expects argument 'client' to be a pointer");
    }

    if (!con_ptr.is_pointer())
    {
        error("Function 'client_close' expects argument 'con_ptr' to be a pointer");
    }

    if (!code.is_number())
    {
        error("Function 'client_close' expects argument 'code' to be a number");
    }

    if (!reason.is_string())
    {
        error("Function 'client_close' expects argument 'reason' to be a string");
    }

    client *c = (client *)client_ptr.get_pointer()->value;
    client::connection_type *con = (client::connection_type *)con_ptr.get_pointer()->value;

    if (c->stopped())
    {
        std::cout << "Connection is not running - cannot close" << std::endl;
        return none_val();
    }

    c->close((*con).get_handle(), code.get_number(), reason.get_string());

    return none_val();
}

/* Server */

struct connection_data
{
    int sessionId;
    std::string name;
};

typedef std::map<websocketpp::connection_hdl, connection_data, std::owner_less<websocketpp::connection_hdl>> con_list;

std::map<server *, con_list> m_servers;

int m_next_sessionid;

enum tls_mode
{
    MOZILLA_INTERMEDIATE = 1,
    MOZILLA_MODERN = 2
};

context_ptr server_on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl)
{
    namespace asio = websocketpp::lib::asio;

    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
    std::cout << "using TLS mode: " << (mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate") << std::endl;

    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);

    try
    {
        if (mode == MOZILLA_MODERN)
        {
            // Modern disables TLSv1
            ctx->set_options(asio::ssl::context::default_workarounds |
                             asio::ssl::context::no_sslv2 |
                             asio::ssl::context::no_sslv3 |
                             asio::ssl::context::no_tlsv1 |
                             asio::ssl::context::single_dh_use);
        }
        else
        {
            ctx->set_options(asio::ssl::context::default_workarounds |
                             asio::ssl::context::no_sslv2 |
                             asio::ssl::context::no_sslv3 |
                             asio::ssl::context::single_dh_use);
        }
        // ctx->set_password_callback(bind(&get_password));
        ctx->use_certificate_chain_file("server.pem");
        ctx->use_private_key_file("server.pem", asio::ssl::context::pem);

        // Example method of generating this file:
        // `openssl dhparam -out dh.pem 2048`
        // Mozilla Intermediate suggests 1024 as the minimum size to use
        // Mozilla Modern suggests 2048 as the minimum size to use.
        ctx->use_tmp_dh_file("dh.pem");

        std::string ciphers;

        if (mode == MOZILLA_MODERN)
        {
            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
        }
        else
        {
            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
        }

        if (SSL_CTX_set_cipher_list(ctx->native_handle(), ciphers.c_str()) != 1)
        {
            std::cout << "Error setting cipher list" << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return ctx;
}

extern "C" Value _server(std::vector<Value> &args)
{
    int num_required_args = 0;

    if (args.size() != num_required_args)
    {
        error("Function 'server' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    server *s = new server();

    s->clear_access_channels(websocketpp::log::alevel::frame_header);
    s->clear_access_channels(websocketpp::log::alevel::frame_payload);
    // s->set_error_channels(websocketpp::log::elevel::none);

    s->set_tls_init_handler(websocketpp::lib::bind(&server_on_tls_init, MOZILLA_INTERMEDIATE, ::_1));

    // Initialize ASIO
    s->init_asio();

    m_servers[s] = con_list();

    Value server_object = object_val();
    server_object.get_object()->keys = {"server_ptr", "running"};

    Value server_ptr = pointer_val();
    server_ptr.get_pointer()->value = (void *)s;

    server_object.get_object()->values["server_ptr"] = server_ptr;
    server_object.get_object()->values["running"] = boolean_val(false);

    return server_object;
}

extern "C" Value _server_listen(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'listen' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value port = args[1];

    if (!server_object.is_object())
    {
        error("Function 'listen' expects argument 'server' to be an object");
    }

    if (!port.is_number())
    {
        error("Function 'listen' expects argument 'port' to be a number");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'listen' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'listen' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    _server->values["running"] = boolean_val(true);

    // Listen on port
    s->listen((int)port.get_number());

    // Start the server accept loop
    s->start_accept();

    // Start the ASIO io_service run loop
    s->run();

    return none_val();
}

extern "C" Value _server_stop(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'stop' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];

    if (!server_object.is_object())
    {
        error("Function 'stop' expects argument 'server' to be an object");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'stop' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'stop' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    _server->values["running"] = boolean_val(false);

    s->stop();

    return none_val();
}

extern "C" Value _server_send(std::vector<Value> &args)
{
    int num_required_args = 3;

    if (args.size() != num_required_args)
    {
        error("Function 'send' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value id = args[1];
    Value message = args[2];

    if (!server_object.is_object())
    {
        error("Function 'send' expects argument 'server' to be an object");
    }

    if (!id.is_number())
    {
        error("Function 'send' expects argument 'id' to be a number");
    }

    if (!message.is_string())
    {
        error("Function 'send' expects argument 'message' to be a string");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'send' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'send' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    if (s->stopped())
    {
        std::cout << "Connection is not running - cannot send" << std::endl;
        return none_val();
    }

    for (auto it = m_servers[s].begin(); it != m_servers[s].end(); ++it)
    {
        if ((it->second).sessionId == id.get_number())
        {
            try
            {
                auto con = s->get_con_from_hdl(it->first);
                if (con->get_state() == websocketpp::session::state::open)
                {
                    s->send(it->first, message.get_string(), websocketpp::frame::opcode::text);
                }
                break;
            }
            catch (...)
            {
                std::cout << "Error sending to client: " << (it->second).sessionId;
            }
        }
    }

    return none_val();
}

extern "C" Value _server_broadcast(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'broadcast' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value message = args[1];

    if (!server_object.is_object())
    {
        error("Function 'broadcast' expects argument 'server' to be an object");
    }

    if (!message.is_string())
    {
        error("Function 'broadcast' expects argument 'message' to be a string");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'broadcast' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'broadcast' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    if (s->stopped())
    {
        std::cout << "Connection is not running - cannot broadcast" << std::endl;
        return none_val();
    }

    for (auto it = m_servers[s].begin(); it != m_servers[s].end(); ++it)
    {
        try
        {
            auto con = s->get_con_from_hdl(it->first);
            if (con->get_state() == websocketpp::session::state::open)
            {
                s->send(it->first, message.get_string(), websocketpp::frame::opcode::text);
            }
            break;
        }
        catch (...)
        {
            std::cout << "Error sending to client: " << (it->second).sessionId;
        }
    }

    return none_val();
}

extern "C" Value _server_on_validate(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_validate' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value func = args[1];

    if (!server_object.is_object())
    {
        error("Function 'on_validate' expects argument 'server' to be an object");
    }

    if (!func.is_function())
    {
        error("Function 'on_validate' expects argument 'function' to be a Function");
    }

    if (func.get_function()->arity != 1)
    {
        error("Function 'on_validate' expects argument 'function' to be a Function with 1 parameter");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'on_validate' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'on_validate' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    auto on_validate_func = [func, s](websocketpp::connection_hdl hdl)
    {
        server::connection_ptr con = s->get_con_from_hdl(hdl);
        auto &req = con->get_request();
        auto &headers = req.get_headers();

        Value header_object = object_val();
        auto &header_internal_object = header_object.get_object();

        for (const auto &header : headers)
        {
            header_internal_object->keys.push_back(header.first);
            header_internal_object->values[header.first] = string_val(header.second);
        }

        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_constant(main->chunk, header_object);
        add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 1, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);

        if (func_vm.stack.size() == 0)
        {
            return false;
        }

        if (!func_vm.stack.back().is_boolean())
        {
            return false;
        }

        return func_vm.stack.back().get_boolean();
    };

    s->set_validate_handler(on_validate_func);

    return none_val();
}

extern "C" Value _server_on_open(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_open' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value func = args[1];

    if (!server_object.is_object())
    {
        error("Function 'on_open' expects argument 'server' to be an object");
    }

    if (!func.is_function())
    {
        error("Function 'on_open' expects argument 'function' to be a Function");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_open' expects argument 'function' to be a Function with 0 parameters");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'on_open' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'on_open' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    auto on_open_func = [func, s](websocketpp::connection_hdl hdl)
    {
        connection_data data;

        data.sessionId = m_next_sessionid++;

        server::connection_ptr con = s->get_con_from_hdl(hdl);
        std::string endpoint = con->get_remote_endpoint();

        data.name = endpoint;

        m_servers[s][hdl] = data;

        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    s->set_open_handler(on_open_func);

    return none_val();
}

extern "C" Value _server_on_message(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_message' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_message' expects argument 'function' to be a Function");
    }

    if (!server_object.is_object())
    {
        error("Function 'on_message' expects argument 'server' to be an object");
    }

    if (func.get_function()->arity != 1)
    {
        error("Function 'on_message' expects argument 'function' to be a Function with 1 parameter");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'on_message' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'on_message' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    auto on_message_func = [s, func](websocketpp::connection_hdl hdl, message_ptr msg)
    {
        connection_data &data = m_servers[s][hdl];

        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        Value payload = object_val();
        payload.get_object()->keys = {"id", "name", "data"};
        payload.get_object()->values["id"] = number_val(data.sessionId);
        payload.get_object()->values["name"] = string_val(data.name);
        payload.get_object()->values["data"] = string_val(msg->get_payload());

        add_constant(main->chunk, func);
        add_constant(main->chunk, payload);
        add_opcode(main->chunk, OP_LOAD_CONST, 1, 0);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 1, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    s->set_message_handler(on_message_func);

    return none_val();
}

extern "C" Value _server_on_fail(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_fail' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value func = args[1];

    if (!server_object.is_object())
    {
        error("Function 'on_fail' expects argument 'server' to be an object");
    }

    if (!func.is_function())
    {
        error("Function 'on_fail' expects argument 'function' to be a Function");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_fail' expects argument 'function' to be a Function with 0 parameters");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'on_fail' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'on_fail' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    auto on_fail_func = [func](websocketpp::connection_hdl hdl)
    {
        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    s->set_fail_handler(on_fail_func);

    return none_val();
}

extern "C" Value _server_on_close(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_close' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value func = args[1];

    if (!server_object.is_object())
    {
        error("Function 'on_close' expects argument 'server' to be an object");
    }

    if (!func.is_function())
    {
        error("Function 'on_close' expects argument 'function' to be a Function");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_close' expects argument 'function' to be a Function with 0 parameters");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'on_close' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'on_close' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    auto on_close_func = [s, func](websocketpp::connection_hdl hdl)
    {
        connection_data &data = m_servers[s][hdl];

        std::cout << "Closing connection " << data.name
                  << " with id " << data.sessionId << std::endl;

        m_servers[s].erase(hdl);

        VM func_vm;
        std::shared_ptr<FunctionObj> main = std::make_shared<FunctionObj>();
        main->name = "";
        main->arity = 0;
        main->chunk = Chunk();
        CallFrame main_frame;
        main_frame.function = main;
        main_frame.sp = 0;
        main_frame.ip = main->chunk.code.data();
        main_frame.frame_start = 0;
        func_vm.frames.push_back(main_frame);

        add_constant(main->chunk, func);
        add_opcode(main->chunk, OP_LOAD_CONST, 0, 0);
        add_opcode(main->chunk, OP_CALL, 0, 0);

        add_code(main->chunk, OP_EXIT, 0);

        auto offsets = instruction_offsets(main_frame.function->chunk);
        main_frame.function->instruction_offsets = offsets;

        evaluate(func_vm);
    };

    s->set_close_handler(on_close_func);

    return none_val();
}

extern "C" Value _server_get_clients(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'get_clients' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];

    if (!server_object.is_object())
    {
        error("Function 'get_clients' expects argument 'server' to be an object");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'get_clients' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'get_clients' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    Value clients_list = list_val();

    for (auto it = m_servers[s].begin(); it != m_servers[s].end(); ++it)
    {
        Value client_object = object_val();
        client_object.get_object()->keys = {"id", "name"};
        client_object.get_object()->values["id"] = number_val((it->second).sessionId);
        client_object.get_object()->values["name"] = string_val((it->second).name);
        clients_list.get_list()->push_back(client_object);
    }

    return clients_list;
}

extern "C" Value _server_close_connection(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'close_connection' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value id = args[1];

    if (!server_object.is_object())
    {
        error("Function 'close_connection' expects argument 'server' to be an object");
    }

    if (!id.is_number())
    {
        error("Function 'close_connection' expects argument 'id' to be a number");
    }

    auto &_server = server_object.get_object();

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_ptr") == _server->keys.end())
    {
        error("Function 'close_connection' expects argument 'server' to be a valid server object");
    }

    if (!_server->values["server_ptr"].is_pointer())
    {
        error("Function 'close_connection' expects argument 'server' to be a valid server object");
    }

    server *s = (server *)_server->values["server_ptr"].get_pointer()->value;

    if (s->stopped())
    {
        std::cout << "Connection is not running - cannot close connection" << std::endl;
        return none_val();
    }

    for (auto it = m_servers[s].begin(); it != m_servers[s].end(); ++it)
    {
        if ((it->second).sessionId == id.get_number())
        {
            websocketpp::lib::error_code ec;
            auto con = s->get_con_from_hdl(it->first);
            if (con->get_state() == websocketpp::session::state::open)
            {
                s->close(it->first, websocketpp::close::status::going_away, "", ec);
            }
            if (ec)
            {
                std::cout << "> Error closing connection " << (it->second).sessionId << ": "
                          << ec.message() << std::endl;
            }
            m_servers[s].erase(it->first);
            break;
        }
    }

    return none_val();
}