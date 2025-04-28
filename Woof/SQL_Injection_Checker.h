#pragma once
#include "pch.h"
#include "SecurityChecker.h"
#include <unordered_set>
#include <regex>

class SQL_Injection_Checker : public SecurityChecker
{
public:
	//SQL_Injection_Checker(const SQL_Injection_Checker&) = delete;
	//SQL_Injection_Checker& operator=(const SQL_Injection_Checker&) = delete;
	//
	//static std::shared_ptr<SQL_Injection_Checker> getInstance();

	virtual SecurityCheckResult check(const boost::beast::http::request<boost::beast::http::string_body>& request,const boost::asio::ip::address& clientAddress) override;

	SQL_Injection_Checker();

private:
	bool checkHeader(const std::string& headerValue) const;
	bool checkBody(const std::string& body) const;
	
	std::regex _blacklist;
	std::vector<std::string> _high_risk_headers;
};