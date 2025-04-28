#include "HttpProxy.h"
#include "JsonDecoder.h"
#include "CheckCaller.h"
#include "DDOS_Checker.h"

HttpProxy::HttpProxy(tcp::socket&& socket, asio::io_context& ioc, tcp::endpoint realServerEndpoint)
    : _client_stream(std::move(socket)), _ioc(ioc), _resolver(asio::make_strand(ioc)), _server_stream(asio::make_strand(ioc))
{
    _realServerEndp = realServerEndpoint;
}

void HttpProxy::send_to_server()
{
    _resolver.async_resolve(
        _realServerEndp,
        beast::bind_front_handler(
            &HttpProxy::on_resolve_server,
            shared_from_this()));
}

void HttpProxy::on_resolve_server(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return fail(ec, "resolve");

    _server_stream.expires_after(std::chrono::seconds(30));

    _server_stream.async_connect(
        results,
        beast::bind_front_handler(
            &HttpProxy::on_connect_server,
            shared_from_this()));
}

void HttpProxy::on_connect_server(beast::error_code ec, tcp::resolver::results_type::endpoint_type type)
{
    if (ec)
        return fail(ec, "connect");

    _server_stream.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(_server_stream, _req,
        beast::bind_front_handler(
            &HttpProxy::on_write_to_server,
            shared_from_this()));
}

void HttpProxy::on_write_to_server(beast::error_code ec, size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    _res = {};
    // Receive the HTTP response
    http::async_read(_server_stream, _sending_buffer, _res,
        beast::bind_front_handler(
            &HttpProxy::on_read_from_server,
            shared_from_this()));
}

void HttpProxy::on_read_from_server(beast::error_code ec, size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "read1");

    // Write the message from server back to the client
    send_response_to_client(_res);

    _server_stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    if (ec && ec != beast::errc::not_connected)
        return fail(ec, "shutdown");

}

void HttpProxy::run()
{
    asio::dispatch(_client_stream.get_executor(),
        beast::bind_front_handler(
            &HttpProxy::do_read_from_client,
            shared_from_this()));
}

void HttpProxy::do_read_from_client()
{
    _req = {};

    //no need for dynamic timeout calculations since we set a hard limit on active connections in DDOS_Checker. however, a such feature would be easy to implement if needed in the future
    _client_stream.expires_after(std::chrono::seconds(30));

    http::async_read(_client_stream, _recv_buffer, _req,
        beast::bind_front_handler(
            &HttpProxy::on_read_from_client,
            shared_from_this()));
}

void HttpProxy::on_read_from_client(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
        return do_close();

    if (ec)
        return fail(ec, "read2");

    // Get security checks for this URL path
    auto securityChecksByPath = JsonDecoder::getInstance()->getSubDirectoriesSettings();
    auto errorTypesToCheck = CheckCaller::findBestMatch<std::vector<Error>>(
        std::string(_req.target()), 
        securityChecksByPath);

    auto securityCheckers = CheckCaller::getCheckers(errorTypesToCheck, _ioc);
    
    // Run security checks
    SecurityCheckResult securityResults = CheckCaller::runSecurityChecks(
        _req, 
        _client_stream.socket().remote_endpoint().address(),
        securityCheckers);


    if (!securityResults.isAllowed) {                
        send_security_violation_response(securityResults); 
    }
    else {
        // Forward the request to the server if successful
        send_to_server();
    }
}

void HttpProxy::send_response_to_client(http::response<http::string_body> msg)
{
    auto response = std::make_shared<http::response<http::string_body>>(std::move(msg));

    bool keep_alive = response->keep_alive();

    http::async_write(
        _client_stream,
        *response,
        beast::bind_front_handler(
            &HttpProxy::on_write_to_client, shared_from_this(), response, keep_alive));
}

void HttpProxy::on_write_to_client(std::shared_ptr<http::response<http::string_body>> response, bool keep_alive, beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    boost::ignore_unused(response);


    if (ec)
        return fail(ec, "write");


    if (!keep_alive)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Read another request if connection is keep-alive
    do_read_from_client();
}

void HttpProxy::do_close()
{
    //getting ddos checker object 
    //since this is the only time this is done, i decided against changing CheckCaller just so this could potentially look less messy
    DDOS_Checker* ddosChecker = static_cast<DDOS_Checker*>(CheckCaller::getCheckers(std::vector({ Error::DDOS }), _ioc)[0].get());
    ddosChecker->removeExpiredConnection(_client_stream.socket().remote_endpoint().address());

    beast::error_code ec;
    _client_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
}

void HttpProxy::send_security_violation_response(const SecurityCheckResult& violation) {
    http::response<http::string_body> res{http::status::forbidden, _req.version()};
    
    res.set(http::field::server, "Woof WAF");
    res.set(http::field::content_type, "text/html");
    res.keep_alive(_req.keep_alive());
    
    res.set("X-Content-Type-Options", "nosniff");
    res.set(http::field::x_frame_options, "DENY");
    res.set("Content-Security-Policy", "default-src 'self'");
    
    std::string securityErrorName;
    
    switch (violation.error) {
        case Error::SQLI:
            securityErrorName = "SQL Injection Attempt";
            break;
        case Error::XSS:
            securityErrorName = "Cross-Site Scripting (XSS) Attempt";
            break;
        case Error::CSRF:
            securityErrorName = "Cross-Site Request Forgery (CSRF) Attempt";
            break;
        case Error::DDOS:
            securityErrorName = "Potential DoS Attack";
            break;
        default:
            securityErrorName = "Security Violation";
    }
    
    res.body() = "<!DOCTYPE html>\n"
                "<html>\n"
                "<head><title>403 Forbidden</title></head>\n"
                "<body>\n"
                "<h1>Access Denied</h1>\n"
                "<p>The request was blocked due to a security violation.</p>\n"
                "<p>Violation Type: " + securityErrorName + "</p>\n"
                "<p>Details: " + violation.message + "</p>\n"
                "</body>\n"
                "</html>\n";
    
    res.prepare_payload();
    send_response_to_client(res);
}