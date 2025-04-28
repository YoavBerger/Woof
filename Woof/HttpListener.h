#pragma once
#include "pch.h"
#include "HttpProxy.h"

class HttpListener : public std::enable_shared_from_this<HttpListener>
{
public:
    HttpListener(asio::io_context& ioc, tcp::endpoint listen_endpoint, tcp::endpoint server_endpoint);

    void run();
private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);

    asio::io_context& ioc_;
    tcp::acceptor acceptor_;

    tcp::endpoint _server_endp;
};

