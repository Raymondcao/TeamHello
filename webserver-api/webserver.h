#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <map>
#include <vector>
#include <boost/asio.hpp>
#include "config_parser.h"
#include "request_handler.h"

struct configArguments
{
    short unsigned int port;
    // Key is uri prefix. Each prefix is mapped to a handler
    std::map<std::string, RequestHandler*> handlerMapping;
    RequestHandler* defaultHandler;
};

struct serverStatistics
{
    std::map<std::string, int> url_requested_count;
    std::map<RequestHandler::Status, int> response_code_count;
    std::map<std::string, std::vector<std::string> > uri_prefix2request_handler_name;
};

class Server
{
public:
    static Server *serverBuilder(const NginxConfig& config_out);
    void run();
    std::map<std::string, int> getUrlRequestedCount(){return serverStats.url_requested_count;}
    std::map<RequestHandler::Status, int> getResponseCodeCount(){return serverStats.response_code_count;}
    std::map<std::string, std::vector<std::string> > getUriPrefixRequestHandlerMap(){return serverStats.uri_prefix2request_handler_name;}
    static Server *getServerInstance(){return serverInstance;}
    static int parseConfig(const NginxConfig& config_out, configArguments& configArgs, serverStatistics& serverStats);
	
private:
    Server(configArguments configArgs, serverStatistics serverStats);
    void doAccept();

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    configArguments configContent;
    serverStatistics serverStats;
    static Server *serverInstance;
};

#endif //WEBSERVER_H
