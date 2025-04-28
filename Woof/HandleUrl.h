#pragma once
#include "pch.h"

class HandleUrl
{
public:
	static std::string getBaseUrl(const boost::beast::string_view& url);
	static std::string urlDecode(const std::string& packet);
};
