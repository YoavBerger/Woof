/*
 * SecurityChecker.h
 *
 *  Created on: Dec 2, 2024
 *      Author: kali
 */

#ifndef HEADERS_SECURITYCHECKER_H_
#define HEADERS_SECURITYCHECKER_H_
#include <string>
using std::string;

enum errors
{
	SQLI,
	XSS,
	CSRF,
	RFI,
	LFI,
	Command_Injection,
	Directory_Traversal,
	Malware_Upload,
	DDOS
} typedef Error;

struct VerifyData
{
	bool toTrust;
	Error err;
} typedef VerifyData;


class SecurityCHecker
{
public:
	virtual VerifyData check(const string& packet) const = 0;
protected:
	string _subdomain;
};


#endif /* HEADERS_SECURITYCHECKER_H_ */
