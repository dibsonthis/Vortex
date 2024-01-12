#define ASIO_STANDALONE
#include "include/websocketpp/asio.hpp"
#include "include/Vortex.hpp"

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

std::map<client *, std::string> _responses;

// Handlers
void on_open(client *c, websocketpp::connection_hdl hdl)
{
    // std::string msg = "Hello";
    // c->send(hdl, msg, websocketpp::frame::opcode::text);
    // c->get_alog().write(websocketpp::log::alevel::app, "Sent Message: " + msg);
}

void on_fail(client *c, websocketpp::connection_hdl hdl)
{
    c->get_alog().write(websocketpp::log::alevel::app, "Connection Failed");
}

void on_message(client *c, websocketpp::connection_hdl hdl, message_ptr msg)
{
    _responses[c] = msg->get_payload();
    // c->get_alog().write(websocketpp::log::alevel::app, "Received Reply: " + msg->get_payload());
    // c->close(hdl, websocketpp::close::status::normal, "");
}

void on_close(client *c, websocketpp::connection_hdl hdl)
{
    _responses.erase(c);
    c->get_alog().write(websocketpp::log::alevel::app, "Connection Closed");
}

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

    // Register our handlers
    c->set_open_handler(bind(&on_open, c, ::_1));
    c->set_fail_handler(bind(&on_fail, c, ::_1));
    c->set_message_handler(bind(&on_message, c, ::_1, ::_2));
    c->set_close_handler(bind(&on_close, c, ::_1));
    c->set_tls_init_handler(websocketpp::lib::bind(&on_tls_init));

    // Create a connection to the given URI and queue it for connection once
    // the event loop starts
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c->get_connection(uri, ec);
    con->append_header("access-control-allow-origin", "*");
    c->connect(con);

    Value socket_object = object_val();
    socket_object.get_object()->keys = {"websocket_ptr", "websocket_hdl", "running"};

    Value client_ptr = pointer_val();
    client_ptr.get_pointer()->value = (void *)c;

    Value websocket_hdl = pointer_val();
    websocketpp::connection_hdl *hdl = new websocketpp::connection_hdl(con->get_handle());
    websocket_hdl.get_pointer()->value = hdl;

    socket_object.get_object()->values["websocket_ptr"] = client_ptr;
    socket_object.get_object()->values["websocket_hdl"] = websocket_hdl;
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

extern "C" Value _get_response(std::vector<Value> &args)
{
    int num_required_args = 1;

    if (args.size() != num_required_args)
    {
        error("Function 'get_response' expects " + std::to_string(num_required_args) + " argument(s)");
    }

    Value socket_object = args[0];

    if (!socket_object.is_object())
    {
        error("Function 'get_response' expects argument 'client' to be an object");
    }

    auto &socket = socket_object.get_object();

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_ptr") == socket->keys.end())
    {
        error("Function 'get_message' expects argument 'client' to be a valid client object");
    }

    if (std::find(socket->keys.begin(), socket->keys.end(), "websocket_hdl") == socket->keys.end())
    {
        error("Function 'get_message' expects argument 'client' to be a valid client object");
    }

    client *c = (client *)socket->values["websocket_ptr"].get_pointer()->value;

    if (c->stopped())
    {
        std::cout << "Connection is not running - cannot get response" << std::endl;
        return none_val();
    }

    if (_responses.count(c))
    {
        return string_val(_responses[c]);
    }

    return none_val();
}