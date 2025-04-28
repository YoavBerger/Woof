# WOOF: Web Application Firewall

WOOF is a powerful Web Application Firewall (WAF) designed to protect web applications from various security threats including SQL Injection, XSS attacks, CSRF, and DDoS attacks.

## Features

WOOF includes protection against:

- **SQL Injection (SQLi)**: Detects and blocks malicious SQL queries attempting to manipulate your database
- **Cross-Site Scripting (XSS)**: Prevents attackers from injecting client-side scripts into web pages
- **Cross-Site Request Forgery (CSRF)**: Validates request origins to protect against cross-site attacks
- **Distributed Denial of Service (DDoS)**: Limits request rates to prevent service overload
- **Honeypot Protection**: Implements honeypot techniques to detect and block malicious activity

## Architecture

WOOF acts as a reverse proxy that sits between clients and your web application:

1. WOOF listens for incoming HTTP requests on a configured port
2. Each request is processed through multiple security checkers
3. If the request passes all checks, it's forwarded to your application server
4. Responses from the application are sent back to the client

## Configuration

### Main Configuration

The WAF is configured through `Settings.json`:

```json
{
   "client_port": 80,           // Port where the WAF listens for incoming requests
   "server_ip": "127.0.0.1",    // IP address of your web application server
   "server_port": 3000,         // Port of your web application server
   "request_limit": 20,         // Maximum number of requests allowed within the time limit
   "seconds_per_limit": 60,     // Time period (in seconds) for the request limit
   "block_time": 3600,          // Duration (in seconds) to block IPs that exceed limits
   "max_active_connections_per_ip": 20  // Maximum concurrent connections allowed per IP
}
```

### Subdirectory Configuration

URL path-specific protections are configured in `SubSettings.json`:

```json
{
   "/forum": "(1.2.3.4)",      // Enable protections 1, 2, 3, and 4 for /forum path
   "/register": "(1.2.3)",     // Enable protections 1, 2, and 3 for /register path
   "/login": "(1.2)",          // Enable protections 1 and 2 for /login path
   "/search": "(1.2.3)",       // Enable protections 1, 2, and 3 for /search path
   "/": "(1)"                  // Enable protection 1 for the root path
}
```

The numbers in parentheses correspond to different protection types:
1. Basic protection
2. SQL Injection protection
3. XSS protection
4. CSRF protection

### Additional Configuration Files

- `csrfAllowedReferers.json`: List of allowed referrers for CSRF protection


## Installation and Setup

### Prerequisites

- C++ compiler with C++17 support
- Boost libraries (Beast, ASIO)
- nlohmann JSON for Modern C++

### Running the WAF

1. Compile the WOOF project
2. Configure your settings in `Settings.json`
3. Start your web application (e.g., `victimwebsite.py`)
4. Run the WOOF executable

The WAF will start listening on the configured port and protecting your application.

## Testing

You can use the included `POC_tester.py` script to test the WAF's protection capabilities:

```bash
python POC_tester.py
```

This script tests SQL injection, XSS, and DDoS protections by sending malicious payloads to both the protected endpoint (through WAF) and directly to the vulnerable application.

## Performance Considerations

WOOF uses a dynamic thread pool that automatically adjusts to the load:

- Maximum threads: 3 (configurable in `main.cpp`)
- Thread creation threshold: 0.1 seconds (if a job waits longer than this)
- Thread removal time: 0.01 seconds (how quickly unused threads are removed)
- Load check interval: 3 seconds (how often the pool size is evaluated)

### Dynamic Thread Pool

The dynamic thread pool is an advanced feature that allows WOOF to efficiently handle varying loads:

- Automatically creates new threads when request queuing exceeds the threshold
- Removes unused threads to conserve system resources during low-traffic periods
- Adjusts to traffic patterns without manual intervention
- Prevents thread starvation under heavy load while minimizing resource usage

### Dynamic Rate Limiting

WOOF implements dynamic rate limiting as part of its DDoS protection:

- Tracks request frequency from each IP address
- Compares against the configured thresholds (`request_limit` and `seconds_per_limit`)
- Automatically blocks IPs that exceed the allowed request rate
- Maintains a blocklist with timeout mechanism (`block_time`)
- Prevents both simple and distributed denial of service attacks

## Customizing Protection Rules

To modify or extend protection rules, you can edit the corresponding checker classes:

- `SQL_Injection_Checker.cpp`: SQL injection detection rules
- `XSS_Checker.cpp`: Cross-site scripting detection
- `CSRF_Checker.cpp`: Cross-site request forgery protection
- `DDOS_Checker.cpp`: Denial of service attack prevention

## Limitations

- The current implementation focuses on HTTP (not HTTPS)
- Some protection mechanisms use pattern matching and may need refinement for specific applications
- Advanced evasion techniques might require additional customization

## Work in Progress

WOOF is under active development with plans for several enhancements:

### Future Improvements

1. **Enhanced Dynamic Rate Limiting**: The current dynamic rate limiting implementation may require additional tweaking to balance security and performance optimally. Further adjustments to the rate limiting formula are outside the scope of the current project as they primarily involve mathematical tuning rather than architectural changes.

2. **Additional Protection Mechanisms**: The following protections will be added in future versions:
   - **Remote File Inclusion (RFI)**: Building on existing validation frameworks to prevent inclusion of malicious remote files
   - **Local File Inclusion (LFI)**: Protecting against directory traversal and unauthorized file access
   - **Command Injection**: Preventing execution of arbitrary system commands through web applications

These planned features leverage the existing architecture and validation mechanisms already implemented in WOOF, thereby they are straigh forward to implement.
