#ifndef HEADERS_SECURITYCHECKER_H_
#define HEADERS_SECURITYCHECKER_H_

#include "pch.h"
#include <string>
#include <memory>

enum class Error
{
	NONE = 0,
	DDOS,
	SQLI,
	XSS,
	CSRF	
};

struct SecurityCheckResult
{
	bool isAllowed;
	Error error;
	std::string message; 

	SecurityCheckResult(bool allowed, Error err, const std::string& msg)
		: isAllowed(allowed), error(err), message(msg) {}
};

class SecurityChecker
{
public:
	virtual ~SecurityChecker() = default;

	virtual SecurityCheckResult check(
		const boost::beast::http::request<boost::beast::http::string_body>& request,
		const boost::asio::ip::address& clientAddress
	) = 0;

protected:
	std::string _subdomain;
};
#endif
