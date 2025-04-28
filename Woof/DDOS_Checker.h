#pragma once
#include "pch.h"
#include "SecurityChecker.h"
#include "HoneypotAPIFormat.h"

#define ZSCORE_WINDOW_SIZE 100

#define ALPHA 0.1
#define BETA 0.9
#define MAX_SCALE_RLIMIT 1.5
#define MIN_SCALE_RLIMIT 0.5

class DDOS_Checker : public SecurityChecker
{
public:
	virtual SecurityCheckResult check(const boost::beast::http::request<boost::beast::http::string_body>& request, const boost::asio::ip::address& clientAddress) override;

	void removeExpiredConnection(const boost::asio::ip::address& clientAddress);

	//DDOS_Checker(const DDOS_Checker&) = delete;
	//DDOS_Checker& operator=(const DDOS_Checker&) = delete;

	//static std::shared_ptr<DDOS_Checker> getInstance(unsigned int activeConnectionsPerIP, unsigned int requestLimiter, const std::time_t ipBlockTime, boost::asio::io_context& io, const std::time_t timer_interval)
	//{
	//	static std::shared_ptr<DDOS_Checker> instance = 
	//		std::make_shared<DDOS_Checker>(activeConnectionsPerIP, requestLimiter, ipBlockTime, io, timer_interval);
	//	
	//	return instance;
	//}

	DDOS_Checker(const unsigned int activeConnectionsPerIP, const unsigned int requestLimiter, const std::time_t ipBlockTime, boost::asio::io_context& io, const std::time_t timer_interval);

	~DDOS_Checker();

private:
	boost::asio::steady_timer _update_timer;
	boost::asio::chrono::seconds _functions_run_interval;


	double calcZScore();
	void modifyRateLimit();

	void schedule_update_traffic();
	void update_total_traffic(const boost::system::error_code& ec);

	const std::time_t _ipBlockTime;

	unsigned int _currentRequestLimiter;
	const unsigned int _defaultRequestLimiter;
	
	std::mutex _trackerMtx;
	std::unordered_map<boost::asio::ip::address, IP_Data*> _IP_Tracker;

	unsigned int _activeConnectionsPerIP;

	const std::time_t _timer_interval;
	
	std::mutex _totalRequestsMtx;
	unsigned int requestsAcquiredInTime;
	
	//no need for mutex since this is only accessed by the timer running every _timePerRequests seconds
	std::vector<unsigned int> _packetsPerTimePeriod;
	
	ProjectHoneypotQuery& _queryAPI;	
};