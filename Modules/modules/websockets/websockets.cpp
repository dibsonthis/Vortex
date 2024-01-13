#define ASIO_STANDALONE
#include <functional>
#include "include/websocketpp/asio.hpp"
// #include "include/Vortex.hpp"
#include "Vortex/Node/Node.hpp"
#include "Vortex/Lexer/Lexer.hpp"
#include "Vortex/Parser/Parser.hpp"
#include "Vortex/Bytecode/Bytecode.hpp"
#include "Vortex/Bytecode/Generator.hpp"
#include "Vortex/VirtualMachine/VirtualMachine.hpp"

// TLS or no TLS
#include "include/websocketpp/config/asio_client.hpp" // TLS
// #include "include/websocketpp/config/asio_no_tls_client.hpp"	// no TLS

#include "include/websocketpp/client.hpp"
#include "include/websocketpp/common/thread.hpp"
#include "include/websocketpp/common/memory.hpp"

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

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
        error("Function 'op_open' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];
    Value func = args[1];

    if (!func.is_function())
    {
        error("Function 'op_open' expects argument 'function' to be a Function");
    }

    if (!socket_object.is_object())
    {
        error("Function 'op_open' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'op_open' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'op_open' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "connection_ptr") == socket->keys.end())
    {
        error("Function 'op_open' expects argument 'client' to be a valid client object");
    }

    if (func.get_function()->arity != 0)
    {
        error("Function 'op_open' expects argument 'function' to be a Function with 0 parameter");
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