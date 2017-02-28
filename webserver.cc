#include "webserver.h"
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <utility>
#include <fstream>
#include <algorithm>
#include "session.h"
#include "proxy_handler.h"

using namespace boost;
using namespace boost::system;
using namespace boost::asio;

Server *Server::serverInstance = nullptr;

Server* Server::serverBuilder(const NginxConfig& config_out)
{
    if (serverInstance != nullptr)
    {
        return serverInstance;
    }
    configArguments configContents;
    std::map<std::string, std::vector<std::string> > uri_prefix2request_handler_name;
    int configParsingErrorCode = parseConfig(config_out, configContents, uri_prefix2request_handler_name);
    if (configParsingErrorCode != 0)
    {
        return nullptr;
    }
    serverInstance = new Server(configContents, uri_prefix2request_handler_name);
    return serverInstance;
}

Server::Server(configArguments configArgs, std::map<std::string, std::vector<std::string> > uri_prefix2request_handler_name)
: io_service()
, acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), configArgs.port))
, configContent(configArgs)
, uri_prefix2request_handler_name(uri_prefix2request_handler_name)
, log("")
, tmp_log("")
, acceptor_proxy(io_service, ip::tcp::endpoint(ip::tcp::v4(), configArgs.port_proxy))
{
    (this->acceptor).listen();

    if (configArgs.proxy_on)
    {
        this->proxy_switch=true;
        printf("proxy port: %d\n", configArgs.port_proxy);
        (this->acceptor_proxy).listen();
    }

    doAccept();
}

void Server::doAccept()
{
    log += tmp_log;
    std::shared_ptr<session> sesh = std::make_shared<session>(io_service, configContent.handlerMapping, configContent.defaultHandler, &tmp_log);
    acceptor.async_accept(sesh->socket, [sesh, this](const error_code& accept_error)
    {
        if(!accept_error)
        {
            session::read_request(sesh);
        }
        doAccept();
    });

    if (this->proxy_switch)
    {
        acceptor_proxy.async_accept(sesh->socket, [sesh, this](const error_code& accept_error)
        {
            if(!accept_error)
            {
                session::read_request(sesh);
            }
            doAccept();
        });    
    }
}

void Server::run()
{
    std::cout<<"Server is running!\n";
    io_service.run();
}

std::string Server::constructLogMsg(std::string url, int response_code)
{
    return "New request:\nurl requested:" + url + "\nResponse code:" + std::to_string(response_code) + "\n";
}

std::map<std::string, int> Server::getUrlRequestedCount()
{
    std::map<std::string, int> counter;
    std::istringstream f(log);
    std::string line;
    while (std::getline(f, line))
    {
        if (line.find(":") != std::string::npos && line.substr(0, line.find(":")) == "url requested" && line.find(":") < line.length() - 1)
        {
            std::string url = line.substr(line.find(":") + 1);
            counter[url]++;
        }
    }
    return counter;
}
    
std::map<std::string, int> Server::getResponseCodeCount()
{
    std::map<std::string, int> counter;
    std::istringstream f(log);
    std::string line;
    while (std::getline(f, line))
    {
        std::cout << line << "\n";
        if (line.find(":") != std::string::npos && line.substr(0, line.find(":")) == "Response code" && line.find(":") < line.length() - 1)
        {
            std::string code = line.substr(line.find(":") + 1);
            counter[code]++;
        }
    }
    std::cout << "counter size: " << counter.size() << "\n";
    return counter;
}

int Server::parseConfig(const NginxConfig& config_out, configArguments& configArgs, std::map<std::string, std::vector<std::string> >& uri_prefix2request_handler_name)
{
    // Return error if there is repetitive uri_prefix
    std::vector<std::string> checkRepetition;
    for (int i = 0; i < config_out.statements_.size(); i++)
    {
        std::string header = config_out.statements_[i]->tokens_[0];
        if (header == "path" && config_out.statements_[i]->tokens_.size() == 3)
        {
            checkRepetition.push_back(config_out.statements_[i]->tokens_[1]);
        }
    }
    std::sort(checkRepetition.begin(), checkRepetition.end());
    for (int i = 0; i < checkRepetition.size() - 1; i++)
    {
        if (checkRepetition[i] == checkRepetition[i + 1])
        {
            std::cerr << "Error: The url prefix " << checkRepetition[i] << " in config file is repetitive.\n";
            return 9;
        }
    }
    
    // Parse port and default request handler
    for (int i = 0; i < config_out.statements_.size(); i++)
    {
        std::string header = config_out.statements_[i]->tokens_[0];
        if (header == "port")
        {
            if (config_out.statements_[i]->tokens_.size() == 2 && std::all_of(config_out.statements_[i]->tokens_[1].begin(), config_out.statements_[i]->tokens_[1].end(), ::isdigit))
            {
                unsigned int tmpPort = std::stoi(config_out.statements_[i]->tokens_[1]);
                if (tmpPort > 65535 || tmpPort < 0)
                {
                    std::cerr << "The port number " << tmpPort << " in config file is invalid.\n";
                    return 5;
                }
                configArgs.port = (short unsigned int)tmpPort;
            }
            else
            {
                std::cerr << "Please specify a port number.\n";
                return 6;
            }
        }
	    else if (header == "path")
	    {
	        if (config_out.statements_[i]->tokens_.size() == 3)
            {
	            std::string handler_name_ = config_out.statements_[i]->tokens_[2];
                auto handler = RequestHandler::CreateByName(handler_name_.c_str());
                RequestHandler::Status s = handler->Init(config_out.statements_[i]->tokens_[1], *(config_out.statements_[i]->child_block_.get()));
                if (s != RequestHandler::OK)
                {
                    std::cerr << "Error: failed to initialize request handler " << config_out.statements_[i]->tokens_[2] << " for " << config_out.statements_[i]->tokens_[1] << ".\n";
                    return 3;
                }
                (configArgs.handlerMapping)[config_out.statements_[i]->tokens_[1]] = handler;
                uri_prefix2request_handler_name[handler_name_].push_back(config_out.statements_[i]->tokens_[1]);

                if (handler_name_=="ProxyHandler")
                {
                    configArgs.port_proxy = ((ProxyHandler *) handler)->getPort();
                    configArgs.proxy_on = true;
                }
            }
            else
            {
                std::cerr << "Error: argument is lacking or too much in statement " << i << "\n";
                return 4;
            }
	    }
        else if (header == "default")
        {
            if (config_out.statements_[i]->tokens_.size() == 2)
            {
                std::string handler_name_ = config_out.statements_[i]->tokens_[1];
                auto handler = RequestHandler::CreateByName(handler_name_.c_str());
                RequestHandler::Status s = handler->Init("", *(config_out.statements_[i]->child_block_.get()));
                if (s != RequestHandler::Status::OK)
                {
                    std::cerr << "Error: failed to initialize the default hanlder " << config_out.statements_[i]->tokens_[1] << ".\n";
                    return 7;
                }
                configArgs.defaultHandler = handler;
            }
            else
            {
                std::cerr << "Error: wrong format for default request handler.\n";
                return 8;
            }
        }
        else
        {
            std::cerr << "Error: unrecognized header: " << header << ".\n";
            return 9;
        }
    }
    return 0;
}
