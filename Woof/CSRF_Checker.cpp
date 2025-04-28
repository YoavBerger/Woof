#include "CSRF_Checker.h"
#include "HandleUrl.h"
#include "CheckCaller.h"

CSRF_Checker::CSRF_Checker(const std::unordered_map<std::string, std::vector<std::string>>& whitelist)
	: _referer_whitelist(whitelist)
{
}

bool CSRF_Checker::isRefererAllowed(const std::string& host, const std::string& referer) const {
	if (referer.empty()) {
		return true; // Allow requests with no referer
	}
	
	std::string baseRef = HandleUrl::getBaseUrl(referer);
	std::vector<std::string> whitelist = CheckCaller::findBestMatch<std::vector<std::string>>(baseRef, _referer_whitelist);
	
	// Check if the referer is in the whitelist
	for (const auto& allowedReferer : whitelist) {
		if (allowedReferer == baseRef) {
			return true;
		}
	}
	
	return false;
}

SecurityCheckResult CSRF_Checker::check(
	const boost::beast::http::request<boost::beast::http::string_body>& request,
	const boost::asio::ip::address& clientAddress) 
{
	// Get the referer and origin headers from the request
	std::string referer = std::string(request.base()[boost::beast::http::field::referer]);
	std::string origin = std::string(request.base()[boost::beast::http::field::origin]);
	std::string host = std::string(request.base()[boost::beast::http::field::host]);
	
	// If the request method is "GET" or "HEAD", CSRF is less concerning
	std::string method = std::string(request.method_string());
	if (method == "GET" || method == "HEAD") {
		return SecurityCheckResult(true, Error::NONE, "");
	}
	
	// Check the referer header
	if (!referer.empty() && !isRefererAllowed(host, referer)) {
		return SecurityCheckResult(
			false, 
			Error::CSRF,
			"Invalid referer detected: " + referer
		);
	}
	
	// Check the origin header
	if (!origin.empty() && !isRefererAllowed(host, origin)) {
		return SecurityCheckResult(
			false, 
			Error::CSRF,
			"Invalid origin detected: " + origin
		);
	}
	
	return SecurityCheckResult(true, Error::NONE, "");
}
