#include "CheckCaller.h"
#include "SQL_Injection_Checker.h"
#include "XSS_Checker.h"
#include "CSRF_Checker.h"
#include "DDOS_Checker.h"
#include <sstream>
#include <algorithm>
#include "JsonDecoder.h"

std::vector<std::shared_ptr<SecurityChecker>> CheckCaller::getCheckers(const std::vector<Error>& errorTypes, boost::asio::io_context& io) 
{
    auto settings = JsonDecoder::getInstance()->getSettings();

    std::vector<std::shared_ptr<SecurityChecker>> checkers;
    
    for (const auto& errorType : errorTypes) {
        switch (errorType) {
            case Error::SQLI:
                checkers.push_back(SQL_Injection_Checker::getInstance());
                break;
            case Error::XSS:
                checkers.push_back(XSS_Checker::getInstance());
                break;
            case Error::CSRF:
                checkers.push_back(CSRF_Checker::getInstance(JsonDecoder::jsonArraysToMapOfVectors(CSRF_ALLOWED_REFERERS_PATH)));
                break;
            case Error::DDOS:
                checkers.push_back(DDOS_Checker::getInstance(std::atoi(settings.at("max_active_connections_per_ip").c_str()), std::atoi(settings.at("request_limit").c_str()), std::atoi(settings.at("block_time").c_str()), io, std::atoi(settings.at("seconds_per_limit").c_str())));
                break;
            // Add other checkers as needed
            default:
                // Skip unknown checker types
                break;
        }
    }
    
    return checkers;
}

SecurityCheckResult CheckCaller::runSecurityChecks(
    const boost::beast::http::request<boost::beast::http::string_body>& request,
    const boost::asio::ip::address& clientAddress,
    const std::vector<std::shared_ptr<SecurityChecker>>& checkers) {
    
    
    for (const auto& checker : checkers) {
        if (checker) {
            auto result = checker->check(request, clientAddress);
            
            if (!result.isAllowed) {
                SecurityCheckResult violation = SecurityCheckResult(false, result.error, result.message);
                return violation;
            }
        }
    }
    
    return SecurityCheckResult(true, Error::NONE, "");
}

