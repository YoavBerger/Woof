#include "HandleUrl.h"
#include <boost/algorithm/string/find.hpp>
#include <iostream>

std::string HandleUrl::getBaseUrl(const boost::beast::string_view& url)
{
    size_t startPos = url.find("//", 0) + 2;
	size_t endPos = url.find('/', startPos); //if a regular url looks like this: http://example.com/page.php it will get the post of the third '/' which is where we want to cut it
	return std::string(url.data()).substr(startPos, url.length() - (startPos) - (url.length() - endPos));
}

std::string HandleUrl::urlDecode(const std::string& packet)
{
    std::string decoded;
    for (size_t i = 0; i < packet.length(); i++) {
        if (packet[i] == '%' && i + 2 < packet.length()) {
            int value;
            std::istringstream(packet.substr(i + 1, 2)) >> std::hex >> value;
            decoded += static_cast<char>(value);
            i += 2;
        }
        else {
            decoded += packet[i];
        }
    }
    return decoded;
}
