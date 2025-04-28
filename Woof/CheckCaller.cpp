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
    JsonDecoder decoder;
    static auto settings = decoder.getSettings();

    static auto ddosChecker = std::make_shared<DDOS_Checker>(std::atoi(settings.at("max_active_connections_per_ip").c_str()), std::atoi(settings.at("request_limit").c_str()), std::atoi(settings.at("block_time").c_str()), io, std::atoi(settings.at("seconds_per_limit").c_str()));
    static auto csrfChecker = std::make_shared<CSRF_Checker>(CSRF_Checker(JsonDecoder::jsonArraysToMapOfVectors(CSRF_ALLOWED_REFERERS_PATH)));
    static auto xssChecker = std::make_shared<XSS_Checker>(XSS_Checker());
    static auto sqliChecker = std::make_shared<SQL_Injection_Checker>(SQL_Injection_Checker());

    std::vector<std::shared_ptr<SecurityChecker>> checkers;
    
    for (const auto& errorType : errorTypes) {
        switch (errorType) {
            case Error::DDOS:
                checkers.insert(checkers.begin(), ddosChecker);
                //inserting at the start of the vector since DDOS checking needs to perform first to not overload WAF
                break;
            case Error::SQLI:
                checkers.push_back(sqliChecker);
                break;
            case Error::XSS:
                checkers.push_back(xssChecker);
                break;
            case Error::CSRF:
                checkers.push_back(csrfChecker);
                break;
            default:
                break;
        }
    }
    
    return checkers;
}

inline bool CheckCaller::isValidRegex(const std::string& pattern)
{
    return std::regex(pattern).mark_count() >= 0;
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

