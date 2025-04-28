#include "pch.h"
#include "HttpListener.h"
#include "DynamicThreadPool.h"
#include "JsonDecoder.h"
#include "CheckCaller.h"

#define LISTEN_ADDR "0.0.0.0"



#define MAX_THREADS 3
#define SECONDS_TO_CREATE_THREAD 0.1
#define SECONDS_TO_REMOVE_THREAD 0.01
#define SECONDS_PER_LOAD_CHECK 3

void initServer(asio::io_context& ioc);

int main()
{
    //int const threads = std::max<int>(1, THREADS);
    asio::io_context ioc;
    initServer(ioc);

    /*// Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();*/

    std::vector<boost::thread> threads;

    DynamicThreadPool checker(ioc, threads,
        MAX_THREADS - 1,             // Max pool size.
        SECONDS_TO_CREATE_THREAD,         // Create thread if job waits for 2 sec.
        SECONDS_TO_REMOVE_THREAD,
        SECONDS_PER_LOAD_CHECK); // Check if pool needs to grow every 3 sec.
    ioc.run();

    for (int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

    return EXIT_SUCCESS;
}

void initServer(asio::io_context& ioc)
{
    auto settings = JsonDecoder::getInstance()->getSettings();
    const int LISTEN_PORT = std::stoi(settings["client_port"]);
    const std::string SERVER_ADDR = settings["server_ip"].substr(1, settings["server_ip"].length() - 2);
    const int SERVER_PORT = std::stoi(settings["server_port"]);

    auto const listen_port = asio::ip::port_type(LISTEN_PORT);
    auto const server_port = asio::ip::port_type(SERVER_PORT);
    auto const listen_address = asio::ip::make_address(LISTEN_ADDR);
    auto const server_address = asio::ip::make_address(SERVER_ADDR);
    
    //need to init DDOS checker for edge cases since it's required when a connection closes
    CheckCaller::getCheckers(std::vector<Error>({ Error::DDOS }), ioc);

    // Create and launch a listening port
    std::shared_ptr<HttpListener> listener = std::make_shared<HttpListener>(ioc, tcp::endpoint{ listen_address, listen_port }, tcp::endpoint{ server_address, server_port });

    listener->run();
}
