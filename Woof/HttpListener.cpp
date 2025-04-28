#include "HttpListener.h"
HttpListener::HttpListener(asio::io_context& ioc, tcp::endpoint listen_endpoint, tcp::endpoint server_endpoint)
    : ioc_(ioc), acceptor_(asio::make_strand(ioc)), _server_endp(server_endpoint)

{
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(listen_endpoint.protocol(), ec);
    if (ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    if (ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(listen_endpoint, ec);
    if (ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        fail(ec, "listen");
        return;
    }
}

// Start accepting incoming connections
void HttpListener::run()
{
    do_accept();
}


void HttpListener::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(
        boost::asio::make_strand(ioc_),
        beast::bind_front_handler(
            &HttpListener::on_accept,
            shared_from_this()));
}

void HttpListener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        fail(ec, "accept");
        return; // To avoid infinite loop
    }
    else
    {
        std::shared_ptr<HttpProxy> proxy= std::make_shared<HttpProxy>(std::move(socket), ioc_, _server_endp);

        proxy->run();
    }

    // Accept another connection
    do_accept();
}