#pragma once
#include "pch.h"
#include "SecurityChecker.h"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

class CSRF_Checker : public SecurityChecker
{
public:
	// Singleton factory method
	static std::shared_ptr<CSRF_Checker> getInstance(const std::unordered_map<std::string, std::vector<std::string>>& whitelist);
		
	CSRF_Checker(const CSRF_Checker&) = delete;
	CSRF_Checker& operator=(const CSRF_Checker&) = delete;

	virtual SecurityCheckResult check(const boost::beast::http::request<boost::beast::http::string_body>& request, const boost::asio::ip::address& clientAddress) override;
	
	CSRF_Checker(const std::unordered_map<std::string, std::vector<std::string>>& whitelist);

private:
	bool isRefererAllowed(const std::string& host, const std::string& referer) const;
	
	const std::unordered_map<std::string, std::vector<std::string>> _referer_whitelist;
};