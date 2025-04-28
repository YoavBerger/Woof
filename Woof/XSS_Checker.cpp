#include "XSS_Checker.h"
#include <regex>
#include <sstream>

XSS_Checker::XSS_Checker()
    : _blacklist(R"(<.*?(script|alert|eval|onerror|onload|document\.cookie|window\.location).*?>|javascript:)", std::regex_constants::icase) {
}

SecurityCheckResult XSS_Checker::check(
    const boost::beast::http::request<boost::beast::http::string_body>& request,
    const boost::asio::ip::address& clientAddress) {
    
    std::ostringstream oss;
    oss << request;
    std::string full_request = HandleUrl::urlDecode(oss.str());

    if (std::regex_search(full_request, _blacklist)) {
        return SecurityCheckResult(
            false,
            Error::XSS,
            "Cross-Site Scripting (XSS) pattern detected in request"
        );
    }
    
    return SecurityCheckResult(true, Error::NONE, "");
}

