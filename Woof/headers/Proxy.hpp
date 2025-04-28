#ifndef HEADERS_PROXY_HPP_
#define HEADERS_PROXY_HPP_
#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <vector>

using std::string;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class Proxy
{
public:
	Proxy(const string& addr, const short ip);

private:
	asio::io_context _context;
    tcp::acceptor _acceptor;
	asio::io_context::work _work;
	tcp::socket _serverSock;
	std::vector<tcp::socket> _clientSocks;
};



#endif /* HEADERS_PROXY_HPP_ */
