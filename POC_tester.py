import requests
import threading
import time

SQLI_PAYLOADS = [
    "' OR 1=1 --",
    "' UNION SELECT null, username, password FROM users --",
    "'; DROP TABLE users --"
]

XSS_PAYLOADS = [
    '<script>alert("XSS")</script>',
    '<img src="x" onerror="alert(\'XSS\')">',
    '<script>document.body.innerHTML = "<h1>XSS Exploit</h1>";</script>'
]

DOS_REQUESTS = 22

def test_sql_injection_login(base_url):
    for payload in SQLI_PAYLOADS:
        data = {
            'username': payload,
            'password': payload
        }
        response = requests.post(f'{base_url}/login', data=data)
        print(f'[SQLi Test] Attempt with payload {payload} -> Status: {response.status_code}')


def test_sql_injection_register(base_url):
    for payload in SQLI_PAYLOADS:
        data = {
            'username': payload,
            'password': payload
        }
        response = requests.post(f'{base_url}/register', data=data)
        print(f'[SQLi Test] Attempt with payload {payload} -> Status: {response.status_code}')


def test_xss_forum(base_url):
    for payload in XSS_PAYLOADS:
        data = {
            'message': payload
        }
        response = requests.post(f'{base_url}/forum', data=data)
        print(f'[XSS Test] Attempt with payload {payload} -> Status: {response.status_code}')


def dos_attack(base_url):
    for _ in range(DOS_REQUESTS):
        response = requests.get(f'{base_url}/forum')
        print(f'[DDOS Test] Request sent to /forum -> Status: {response.status_code}')


def main():
    print("Starting SQL Injection Test on WAF...")
    test_sql_injection_login('http://localhost:80')

    print("\nStarting SQL Injection Test on WAF...")
    test_sql_injection_register('http://localhost:80')

    print("\nStarting XSS Test on WAF...")
    test_xss_forum('http://localhost:80')

    print("\nStarting DDOS Test on WAF...")
    dos_attack('http://localhost:80')

    print("Starting SQL Injection Test on vulnerable server...")
    test_sql_injection_login('http://localhost:3000')

    print("\nStarting SQL Injection Test on WAF...")
    test_sql_injection_register('http://localhost:3000')

    print("\nStarting XSS Test on WAF...")
    test_xss_forum('http://localhost:3000')

    print("\nStarting DDOS Test on WAF...")
    dos_attack('http://localhost:3000')


if __name__ == '__main__':
    main()