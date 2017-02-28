#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <map>
#include <vector>
#include <boost/asio.hpp>
#include "config_parser.h"
#include "request_handler.h"
#include <fstream>
#include <sstream>

#define LOG_FILE_NAME "log.txt"

struct configArguments
{
    short unsigned int port;
    short unsigned int port_proxy=0;
    // Key is uri prefix. Each prefix is mapped to a handler
    std::map<std::string, RequestHandler*> handlerMapping;
    RequestHandler* defaultHandler;
};

class Server
{
public:
    static Server *serverBuilder(const NginxConfig& config_out);
    void run();
    std::map<std::string, int> getUrlRequestedCount();
    std::map<std::string, int> getResponseCodeCount();
    std::map<std::string, std::vector<std::string> > getUriPrefixRequestHandlerMap(){return this->uri_prefix2request_handler_name;}
    static Server *getServerInstance(){return serverInstance;}
    static int parseConfig(const NginxConfig& config_out, configArguments& configArgs, std::map<std::string, std::vector<std::string> >& uri_prefix2request_handler_name);
    static std::string constructLogMsg(std::string url, int response_code);
	
private:
    Server(configArguments configArgs, std::map<std::string, std::vector<std::string> > uri_prefix2request_handler_name);
    void doAccept();

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::acceptor acceptor_proxy;
    configArguments configContent;
    std::map<std::string, std::vector<std::string> > uri_prefix2request_handler_name;
    std::string log;
    std::string tmp_log;
    
    static Server *serverInstance;
};

#endif //WEBSERVER_H
