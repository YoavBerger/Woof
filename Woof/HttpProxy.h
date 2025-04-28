#pragma once
#include "pch.h"
#include "SecurityChecker.h"

class HttpProxy : public std::enable_shared_from_this<HttpProxy>
{
public:
    HttpProxy(tcp::socket&& socket, asio::io_context& ioc, tcp::endpoint realServerEndpoint);
    void run();

private:
    //server
    beast::tcp_stream _client_stream;
    beast::flat_buffer _recv_buffer;
    
    http::request<http::string_body> _req;
    asio::io_context& _ioc;

    tcp::endpoint _realServerEndp;

    void on_read_from_client(beast::error_code ec, std::size_t bytes_transferred);
    void on_write_to_client(std::shared_ptr<http::response<http::string_body>> response, bool keep_alive, beast::error_code ec, std::size_t bytes_transferred);

    void do_read_from_client();
    void send_response_to_client(http::response<http::string_body> msg);
    void do_close();
    

    //client
    tcp::resolver _resolver;
    beast::tcp_stream _server_stream;
    beast::flat_buffer _sending_buffer; // (Must persist between reads)

    http::response<http::string_body> _res;


    void on_resolve_server(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect_server(beast::error_code ec, tcp::resolver::results_type::endpoint_type type);
    void on_write_to_server(beast::error_code ec, size_t bytes_transferred);
    void on_read_from_server(beast::error_code ec, size_t bytes_transferred);
    
    void send_to_server();

    void send_security_violation_response(const SecurityCheckResult& violation);
};
