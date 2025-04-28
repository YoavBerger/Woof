#include "DDOS_Checker.h"
#include <ctime>

std::string timeToDate(const std::time_t time);

DDOS_Checker::~DDOS_Checker()
{
	/*delete checker;
	checker = nullptr;*/
	std::lock_guard<std::mutex> trackerMtx(_trackerMtx);
	for (auto it : _IP_Tracker)
	{
		delete it.second;
		_IP_Tracker.erase(it.first);
	}
}

DDOS_Checker::DDOS_Checker(const unsigned int activeConnectionsPerIP, const unsigned int requestLimiter, const std::time_t IpBlockTime, boost::asio::io_context& io, const std::time_t timer_interval) : _IP_Tracker(), _timer_interval(timer_interval), _defaultRequestLimiter(requestLimiter), _ipBlockTime(IpBlockTime), _functions_run_interval(boost::asio::chrono::seconds(timer_interval)), _update_timer(io, boost::asio::chrono::seconds(timer_interval)), _queryAPI(ProjectHoneypotQuery::getInstance(io, _IP_Tracker, _trackerMtx))
{
	_currentRequestLimiter = requestLimiter;
	_activeConnectionsPerIP = activeConnectionsPerIP;

	_update_timer = boost::asio::steady_timer(io, boost::asio::chrono::seconds(timer_interval));

	schedule_update_traffic();
}

SecurityCheckResult DDOS_Checker::check(const boost::beast::http::request<boost::beast::http::string_body>& packet, const boost::asio::ip::address& addr)
{
	auto it = _IP_Tracker.find(addr);
	IP_Data* data;
	
	std::unique_lock<std::mutex> reqLock(_totalRequestsMtx);
	requestsAcquiredInTime++;
	reqLock.unlock();

	if (it != _IP_Tracker.end()) 
	{
		data = it->second;
	}
	else 
	{
		_queryAPI.updateTrackerInfo(addr);
		return SecurityCheckResult(true, Error::NONE, "");
	}

	if (data->blockedUntil > std::time_t(VALID_IP))
	{
		return SecurityCheckResult(false, Error::DDOS, "Too many request initiated. IP blocked until: " + timeToDate(data->blockedUntil));
	}

	//remove unwanted requests timestamps from the IP
	std::unique_lock<std::mutex> queueLock(_trackerMtx);
	while (!data->timestamps.empty() && std::time(0) - data->timestamps.front() < std::time_t(_timer_interval))
	{
		data->timestamps.pop();
	}
	queueLock.unlock();

	//add new request timestamp
	queueLock.lock();
	data->timestamps.emplace(std::time_t(0));
	data->currentConnections += 1;
	queueLock.unlock();

	

	if (data->timestamps.size() >= _currentRequestLimiter || data->currentConnections >= _activeConnectionsPerIP)
	{
		//block the IP for the defined time
		data->blockedUntil = std::time(0) + _ipBlockTime;
		return SecurityCheckResult(false, Error::DDOS, "Too many request initiated. IP blocked until: " + timeToDate(data->blockedUntil));
	}

	return SecurityCheckResult(true, Error::NONE, "");
}

std::string timeToDate(const std::time_t time)
{
	char buffer[26];
	std::tm time_info;

	if (gmtime_s(&time_info, &time) != 0) {
		return "Error formatting time";
	}

	asctime_s(buffer, sizeof(buffer), &time_info);

	return std::string(buffer);
}

void DDOS_Checker::schedule_update_traffic()
{
	_update_timer.expires_after(_functions_run_interval);
	_update_timer.async_wait(boost::bind(&DDOS_Checker::update_total_traffic, this, boost::asio::placeholders::error));
}

void DDOS_Checker::update_total_traffic(const boost::system::error_code& ec)
{
	if (!ec) 
	{
		std::unique_lock<std::mutex> reqLock(_totalRequestsMtx);
		_packetsPerTimePeriod.push_back(requestsAcquiredInTime);
		requestsAcquiredInTime = 0;
		reqLock.unlock();

		if (_packetsPerTimePeriod.size() >= ZSCORE_WINDOW_SIZE)
		{
			_packetsPerTimePeriod.erase(_packetsPerTimePeriod.begin());
		}

		modifyRateLimit();

		schedule_update_traffic();
	}
	else
	{
		fail(ec, "DDOS traffic update");
	}

}

double DDOS_Checker::calcZScore()
{
	double zscore = 0;

	size_t n = _packetsPerTimePeriod.size();
	if (n >= 2) {
		double sum = 0.0, sum_squares = 0.0;
		for (auto count : _packetsPerTimePeriod) {
			sum += count;
			sum_squares += static_cast<double>(count) * count;
		}
		double mean = sum / n;
		double variance = (sum_squares + mean * (- 2 * sum + mean)) / (n - 1);
		double stddev = sqrt(std::max(variance, 1e-9)); // Avoid division by zero

		double latest = _packetsPerTimePeriod.back();
		zscore = (latest - mean) / stddev;
	}

	return zscore;
}

void DDOS_Checker::modifyRateLimit()
{
	const double zscore = calcZScore();

	//scale function for the rate limit based on zscore - if it's negative the rate limit scales up and if it's positive it scales down
	double targetScale = 1 - ALPHA * zscore;
	//smoothing the scaling based on the current scaling for the rate limit so the changes would be gradual
	double smoothedScaling = BETA * (_currentRequestLimiter / _defaultRequestLimiter) + (1 - BETA) * targetScale;

	_currentRequestLimiter = _defaultRequestLimiter * std::clamp(smoothedScaling, MIN_SCALE_RLIMIT, MAX_SCALE_RLIMIT);
}

void DDOS_Checker::removeExpiredConnection(const boost::asio::ip::address& clientAddress)
{
	if (_IP_Tracker.find(clientAddress) != _IP_Tracker.end())
	{
		_IP_Tracker[clientAddress]->currentConnections -= 1;
	}
}