#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "SecurityChecker.h"
#include <boost/beast/http.hpp>
#include <boost/asio/ip/address.hpp>
#include "SettingsLoadHelper.h"

#define CSRF_ALLOWED_REFERERS_PATH "csrfAllowedReferers.json"

class CheckCaller : protected SettingsLoadHelper
{
public:
    static std::vector<std::shared_ptr<SecurityChecker>> getCheckers(const std::vector<Error>& errorTypes, boost::asio::io_context& io);
   
    template <class T>
    static T findBestMatch(const std::string& url, const std::map<std::string, T>& subDirectories)
    {
        std::string decodedUrl = HandleUrl::urlDecode(url);
        // First try exact match
        auto it = subDirectories.find(decodedUrl);
        if (it != subDirectories.end()) {
            return it->second;
        }

        std::string bestMatch;
        int maxLength = 0;

        for (const auto& item : subDirectories) {
            if (!isValidRegex(item.first)) continue; // Skip invalid patterns

            std::regex regexPattern(item.first);
            if (std::regex_search(url, regexPattern) && item.first.length() > maxLength) {
                maxLength = item.first.length();
                bestMatch = item.first;
            }
        }

        if (!bestMatch.empty()) {
            return subDirectories.at(bestMatch);
        }

        return T();
    }
    
    static SecurityCheckResult runSecurityChecks(
        const boost::beast::http::request<boost::beast::http::string_body>& request,
        const boost::asio::ip::address& clientAddress,
        const std::vector<std::shared_ptr<SecurityChecker>>& checkers);
};

