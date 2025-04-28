#pragma once
#include "pch.h"
#include <fstream>
#include <format>
#include <iostream>
#include <array>
#include <queue>

#define API_FORMAT "%12s.%s.%s.%s.%s.dnsbl.httpbl.org"
#define HONEYPOT_KEY_FILE_LOCATION "../honeypot_key"
#define KEY_LEN 12

#define DNS_PORT "53"

//please reffer to project honeypot documentation for further clarifications
#define SUSPICIOUS_TYPE 1

//out of 255
#define MAX_THREAT_SCORE_ALLOWED 15 

#define BLOCKED_IP LONG_MAX 
#define VALID_IP 0

typedef struct Client_Data
{
    std::queue<std::time_t> timestamps;
    int currentConnections;
    std::time_t blockedUntil;
} IP_Data;

class HoneypotAPIFormat
{
protected:
    static std::string format_API_Request(const boost::asio::ip::address& IP);
    static std::array<std::string, 4> parseIP(const std::string& IP);

private:
	static std::string get_API_Key();
};

class ProjectHoneypotQuery : public HoneypotAPIFormat
{
public:
    
    ProjectHoneypotQuery(const ProjectHoneypotQuery&) = delete;
    ProjectHoneypotQuery& operator=(const ProjectHoneypotQuery&) = delete;

    static ProjectHoneypotQuery& getInstance(boost::asio::io_context& io_context, std::unordered_map<boost::asio::ip::address, IP_Data*>& _IP_Tracker, std::mutex& trackerMtx);
    void updateTrackerInfo(const boost::asio::ip::address& IP);

private:
    ProjectHoneypotQuery(boost::asio::io_context& io_context, std::unordered_map<boost::asio::ip::address, IP_Data*>& _IP_Tracker, std::mutex& trackerMtx);

    void handle_resolve(const boost::system::error_code& err, const boost::asio::ip::tcp::resolver::results_type& results, const boost::asio::ip::address& IP);

    boost::asio::ip::tcp::resolver _resolver;
    boost::asio::io_context& _io_context;

    std::mutex& _trackerMtx;
    std::unordered_map<boost::asio::ip::address, IP_Data*>& _IP_Tracker;
};