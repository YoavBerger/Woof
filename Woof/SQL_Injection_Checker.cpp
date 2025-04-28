#include "SQL_Injection_Checker.h"
#include "HandleUrl.h"
//
//std::shared_ptr<SQL_Injection_Checker> SQL_Injection_Checker::getInstance() {
//	static std::shared_ptr<SQL_Injection_Checker> instance = std::make_shared<SQL_Injection_Checker>();
//	return instance;
//}

SQL_Injection_Checker::SQL_Injection_Checker()
	: _blacklist(R"(\b(select|union|insert|delete|update|drop|alter|exec|into|outfile|load_file|information_schema|benchmark)\b|(--|#|/\*|\*/|['"`]))", std::regex_constants::icase) {
	_high_risk_headers = {
		"User-Agent", "Referer", "Cookie", "X-Forwarded-For",
		"Authorization", "Content-Type"
	};
}

bool SQL_Injection_Checker::checkHeader(const std::string& headerValue) const {
	std::string decodedValue = HandleUrl::urlDecode(headerValue);
	return std::regex_search(decodedValue, _blacklist);
}

bool SQL_Injection_Checker::checkBody(const std::string& body) const {
	std::string decodedBody = HandleUrl::urlDecode(body);
	return std::regex_search(decodedBody, _blacklist);
}

SecurityCheckResult SQL_Injection_Checker::check(
	const boost::beast::http::request<boost::beast::http::string_body>& request,
	const boost::asio::ip::address& clientAddress) {
	
	// Check high-risk headers
	for (const auto& header : _high_risk_headers) {
		auto it = request.find(header);
		if (it != request.end()) {
			if (checkHeader(std::string(it->value()))) {
				return SecurityCheckResult(
					false,
					Error::SQLI,
					"SQL Injection pattern detected in header: " + header
				);
			}
		}
	}
	
	// Check request body
	if (!request.body().empty()) {
		if (checkBody(request.body())) {
			return SecurityCheckResult(
				false,
				Error::SQLI,
				"SQL Injection pattern detected in request body"
			);
		}
	}
	
	// Check URL parameters in target
	std::string target = std::string(request.target());
	if (checkHeader(target)) {
		return SecurityCheckResult(
			false,
			Error::SQLI,
			"SQL Injection pattern detected in URL"
		);
	}

	return SecurityCheckResult(true, Error::NONE, "");
}