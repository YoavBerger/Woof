#pragma once
#include "pch.h"
#include "SecurityChecker.h"
#include <unordered_set>
#include <regex>
#include "HandleUrl.h"

class XSS_Checker : public SecurityChecker
{
public:
	virtual SecurityCheckResult check(const boost::beast::http::request<boost::beast::http::string_body>& request, const boost::asio::ip::address& clientAddress) override;

	XSS_Checker();

private:	
	std::regex _blacklist;
};