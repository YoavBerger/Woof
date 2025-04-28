#include "HoneypotAPIFormat.h"

std::string HoneypotAPIFormat::get_API_Key()
{
	static std::string api_key;
	if (api_key == "")
	{
		std::ifstream keyfile(HONEYPOT_KEY_FILE_LOCATION);
		if (!keyfile)
		{
			throw std::invalid_argument("error accessing honeypot key!");
		}

		if (!std::getline(keyfile, api_key))
		{
			throw std::invalid_argument("honeypot key does not exist in honeypot_key file");
		}
	}
	
	return api_key;
}

std::string HoneypotAPIFormat::format_API_Request(const boost::asio::ip::address& IP)
{
	std::array<std::string, 4> ipArr = HoneypotAPIFormat::parseIP(IP.to_string());

	//std::string request = std::format(API_FORMAT, _honeypot_api_key, splitted[3], splitted[2], splitted[1], splitted[0]);
	//doesn't work because dependencies require c++17 and not c++20 :(
	std::string request = get_API_Key() + '.' + ipArr[3] + '.' + ipArr[2] + '.' + ipArr[1] + '.' + ipArr[0] + ".dnsbl.httpbl.org";
	return request;
}

std::array<std::string, 4> HoneypotAPIFormat::parseIP(const std::string& IP)
{
	std::array<std::string, 4> ipArr;
	std::string splitPart;
	std::istringstream stream(IP);

	int i = 0;
	while (std::getline(stream, splitPart, '.'))
	{
		ipArr[i++] = splitPart.data();
	}

	if (i != 4)
	{
		throw std::invalid_argument("Invalid IP to parse: " + IP);
	}

	return ipArr;
}

ProjectHoneypotQuery::ProjectHoneypotQuery(boost::asio::io_context& io_context, std::unordered_map<boost::asio::ip::address, IP_Data*>& _IP_Tracker, std::mutex& trackerMtx) 
	: _resolver(io_context), _io_context(io_context), _IP_Tracker(_IP_Tracker), _trackerMtx(trackerMtx) {
}

ProjectHoneypotQuery& ProjectHoneypotQuery::getInstance(boost::asio::io_context& io_context, std::unordered_map<boost::asio::ip::address, IP_Data*>& _IP_Tracker, std::mutex& trackerMtx)
{
	static ProjectHoneypotQuery instance(io_context, _IP_Tracker, trackerMtx);
	return instance;
}

void ProjectHoneypotQuery::updateTrackerInfo(const boost::asio::ip::address& IP)
{
	if (!IP.is_v4())
	{
		// project honeyport API does not support IPv6
		return;
	}
	std::string query = HoneypotAPIFormat::format_API_Request(IP);

	_resolver.async_resolve(query, DNS_PORT,
		[IP, query, this](const boost::system::error_code& err, const boost::asio::ip::tcp::resolver::results_type& res) {
			handle_resolve(err, res, IP);
		});
}

void ProjectHoneypotQuery::handle_resolve(const boost::system::error_code& err, const boost::asio::ip::tcp::resolver::results_type& results, const boost::asio::ip::address& IP)
{
    if (err)
    {
		//std::cout << err << std::endl;
		IP_Data* IPdata = new IP_Data({ std::queue<std::time_t>(), 1, VALID_IP });
		std::lock_guard mtx(_trackerMtx);
		_IP_Tracker[IP] = IPdata;
		return;
    }
	std::cout << "ENDPOINT: " << results.begin()->endpoint() << std::endl;
	
	std::string ip = results.begin()->endpoint().address().to_string();
	std::array<std::string, 4> parsedRes = HoneypotAPIFormat::parseIP(ip);

	if (std::stoi(parsedRes[0]) == 127) {
		int days = std::stoi(parsedRes[1]);
		int threat_score = std::stoi(parsedRes[2]);
		int type = std::stoi(parsedRes[3]);

		std::cout << "Type: " << type << "\nIP: " << results.begin()->host_name() << std::endl;

		std::lock_guard<std::mutex> lockTracker(_trackerMtx);
		if (type > SUSPICIOUS_TYPE || threat_score >= MAX_THREAT_SCORE_ALLOWED)
		{
			IP_Data* IPdata = new IP_Data({ std::queue<std::time_t>(), 1, BLOCKED_IP });
			std::lock_guard mtx(_trackerMtx);
			_IP_Tracker[results.begin()->endpoint().address()] = IPdata;
			return;
		}

		IP_Data* IPdata = new IP_Data({ std::queue<std::time_t>(), 1, VALID_IP });
		std::lock_guard mtx(_trackerMtx);
		_IP_Tracker[results.begin()->endpoint().address()] = IPdata;
	}
	else {
		IP_Data* IPdata = new IP_Data({ std::queue<std::time_t>(), 1, VALID_IP });
		std::lock_guard mtx(_trackerMtx);
		_IP_Tracker[results.begin()->endpoint().address()] = IPdata;
		std::cout << "IP is not listed in boost::beast::http:BL." << std::endl;
	}
}
