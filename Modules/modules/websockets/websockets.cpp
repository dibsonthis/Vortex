#define ASIO_STANDALONE
#include <functional>
#include "include/websocketpp/asio.hpp"
#include "Vortex/Node/Node.hpp"
#include "Vortex/Lexer/Lexer.hpp"
#include "Vortex/Parser/Parser.hpp"
#include "Vortex/Bytecode/Bytecode.hpp"
#include "Vortex/Bytecode/Generator.hpp"
#include "Vortex/VirtualMachine/VirtualMachine.hpp"

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

extern "C" Value _client(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'client' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value url = args[0];

    if (!url.is_string())
    {
        error("Function 'client' expects argument 'url' to be a string");
    }

    client *c = new client();

    std::string uri = url.get_string();

    c->clear_access_channels(websocketpp::log::alevel::frame_header);
    c->clear_access_channels(websocketpp::log::alevel::frame_payload);
    // c->set_error_channels(websocketpp::log::elevel::none);

    // Initialize ASIO
    c->init_asio();

    c->set_tls_init_handler(websocketpp::lib::bind(&on_tls_init));

    // Create a connection to the given URI and queue it for connection once
    // the event loop starts
    websocketpp::lib::error_code ec;
    client::connection_ptr *con = new client::connection_ptr(c->get_connection(uri, ec));
    (*con)->append_header("access-control-allow-origin", "*");
    c->connect(*con);

    Value socket_object = object_val();
    socket_object.get_object()->keys = {"websocket_ptr", "connection_ptr", "websocket_hdl", "running"};

    Value client_ptr = pointer_val();
    client_ptr.get_pointer()->value = (void *)c;

    Value websocket_hdl = pointer_val();
    websocketpp::connection_hdl *hdl = new websocketpp::connection_hdl((*con)->get_handle());
    websocket_hdl.get_pointer()->value = hdl;

    Value connection_ptr = pointer_val();
    connection_ptr.get_pointer()->value = (void *)con;

    socket_object.get_object()->values["websocket_ptr"] = client_ptr;
    socket_object.get_object()->values["websocket_hdl"] = websocket_hdl;
    socket_object.get_object()->values["connection_ptr"] = connection_ptr;
    socket_object.get_object()->values["running"] = boolean_val(false);

    return socket_object;
}

extern "C" Value _run(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'run' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];

    if (!socket_object.is_object())
    {
        error("Function 'run' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'run' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'run' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'run' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'run' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'run' expects argument 'client' to be a valid client object");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    websocketpp::connection_hdl *hdl = (websocketpp::connection_hdl *)socket->values["websocket_hdl"].get_pointer()->value;

    socket->values["running"] = boolean_val(true);

    c->run();

    return none_val();
}

extern "C" Value _close(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'close' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];

    if (!socket_object.is_object())
    {
        error("Function 'close' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'close' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'close' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'close' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'close' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'close' expects argument 'client' to be a valid client object");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    websocketpp::connection_hdl *hdl = (websocketpp::connection_hdl *)socket->values["websocket_hdl"].get_pointer()->value;

    if (c->stopped())
    {
        std::cout << "Connection is not running - cannot close" << std::endl;
        return none_val();
    }

    c->close(*hdl, websocketpp::close::status::normal, "");

    socket->values["running"] = boolean_val(false);

    return none_val();
}

extern "C" Value _send(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'send' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];
    Value message = args[1];

    if (!socket_object.is_object())
    {
        error("Function 'send' expects argument 'client' to be an object");
    }

    if (!message.is_string())
    {
        error("Function 'send' expects argument 'message' to be a string");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'send' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'send' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'send' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'send' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'send' expects argument 'client' to be a valid client object");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    websocketpp::connection_hdl *hdl = (websocketpp::connection_hdl *)socket->values["websocket_hdl"].get_pointer()->value;

    if (c->stopped())
    {
        std::cout << "Connection is not running - cannot send" << std::endl;
        return none_val();
    }

    c->send(*hdl, message.get_string(), websocketpp::frame::opcode::text);

    return none_val();
}

extern "C" Value _on_open(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_open' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_open' expects argument 'function' to be a Function");
    }

    if (!socket_object.is_object())
    {
        error("Function 'on_open' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'on_open' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'on_open' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'on_open' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'on_open' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'on_open' expects argument 'client' to be a valid client object");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_open' expects argument 'function' to be a Function with 0 parameter");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    client::connection_ptr *con = (client::connection_ptr *)socket->values["connection_ptr"].get_pointer()->value;

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

    (*con)->set_open_handler(on_open_func);

    return none_val();
}

extern "C" Value _on_message(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_message' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_message' expects argument 'function' to be a Function");
    }

    if (!socket_object.is_object())
    {
        error("Function 'on_message' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'on_message' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'on_message' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'on_message' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'on_message' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'on_message' expects argument 'client' to be a valid client object");
    }

    if (func.get_function()->arity != 1)
    {
        error("Function 'on_message' expects argument 'function' to be a Function with 1 parameter");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    client::connection_ptr *con = (client::connection_ptr *)socket->values["connection_ptr"].get_pointer()->value;

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

    (*con)->set_message_handler(on_message_func);

    return none_val();
}

extern "C" Value _on_close(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_close' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_close' expects argument 'function' to be a Function");
    }

    if (!socket_object.is_object())
    {
        error("Function 'on_close' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'on_close' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'on_close' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'on_close' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'on_close' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'on_close' expects argument 'client' to be a valid client object");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_close' expects argument 'function' to be a Function with 0 parameter");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    client::connection_ptr *con = (client::connection_ptr *)socket->values["connection_ptr"].get_pointer()->value;

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

    (*con)->set_close_handler(on_close_func);

    return none_val();
}

extern "C" Value _on_fail(std::vector<Value> &args)
{
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'on_fail' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'on_fail' expects argument 'function' to be a Function");
    }

    if (!socket_object.is_object())
    {
        error("Function 'on_fail' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'on_fail' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'on_fail' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'on_fail' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_ptr"].is_pointer())
    {
        error("Function 'on_fail' expects argument 'client' to be a valid client object");
    }

    if (!socket->values["websocket_hdl"].is_pointer())
    {
        error("Function 'on_fail' expects argument 'client' to be a valid client object");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'on_fail' expects argument 'function' to be a Function with 0 parameter");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;
    client::connection_ptr *con = (client::connection_ptr *)socket->values["connection_ptr"].get_pointer()->value;

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

    (*con)->set_fail_handler(on_fail_func);

    return none_val();
}

/* Server */

struct connection_data
{
    int sessionId;
    std::string name;
};

typedef std::map<websocketpp::connection_hdl, connection_data, std::owner_less<websocketpp::connection_hdl>> con_list;

con_list m_connections;
int m_next_sessionid;

enum tls_mode
{
    MOZILLA_INTERMEDIATE = 1,
    MOZILLA_MODERN = 2
};

connection_data &get_data_from_hdl(websocketpp::connection_hdl hdl)
{
    auto it = m_connections.find(hdl);

    if (it == m_connections.end())
    {
        // this connection is not in the list. This really shouldn't happen
        // and probably means something else is wrong.
        throw std::invalid_argument("No data available for session");
    }

    return it->second;
}

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

    Value server_object = object_val();
    server_object.get_object()->keys = {"server_ptr", "running"};

    Value server_ptr = pointer_val();
    server_ptr.get_pointer()->value = (void *)s;

    server_object.get_object()->values["server_ptr"] = server_ptr;
    server_object.get_object()->values["running"] = boolean_val(false);

    return server_object;
}

extern "C" Value _listen(std::vector<Value> &args)
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

extern "C" Value _stop(std::vector<Value> &args)
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

    if (std::find(_server->keys.begin(), _server->keys.end(), "server_hdl") == _server->keys.end())
    {
        error("Function 'stop' expects argument 'server' to be a valid server object");
    }

    if (std::find(_server->keys.begin(), _server->keys.end(), "connection_ptr") == _server->keys.end())
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
    int num_required_args = 2;

    if (args.size() != num_required_args)
    {
        error("Function 'send' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value server_object = args[0];
    Value message = args[1];

    if (!server_object.is_object())
    {
        error("Function 'send' expects argument 'server' to be an object");
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

    for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
    {
        s->send(it->first, message.get_string(), websocketpp::frame::opcode::text);
    }

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
        std::string path = con->get_resource();

        data.name = path;

        m_connections[hdl] = data;

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

    auto on_message_func = [func](websocketpp::connection_hdl hdl, message_ptr msg)
    {
        connection_data &data = get_data_from_hdl(hdl);

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

    auto on_close_func = [func](websocketpp::connection_hdl hdl)
    {
        connection_data &data = get_data_from_hdl(hdl);

        std::cout << "Closing connection " << data.name
                  << " with sessionid " << data.sessionId << std::endl;

        m_connections.erase(hdl);

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