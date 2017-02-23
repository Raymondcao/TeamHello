#include "webserver.h"
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include "session.h"

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
    serverStatistics serverStats;
    int configParsingErrorCode = parseConfig(config_out, configContents, serverStats);
    if (configParsingErrorCode != 0)
    {
        return nullptr;
    }
    serverInstance = new Server(configContents, serverStats);
    return serverInstance;
}

Server::Server(configArguments configArgs, serverStatistics serverStats)
: io_service()
, acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), configArgs.port))
, configContent(configArgs)
, serverStats(serverStats)
{
    (this->acceptor).listen();
    doAccept();
}

void Server::doAccept()
{
    std::shared_ptr<session> sesh = std::make_shared<session>(io_service, configContent.handlerMapping, configContent.defaultHandler);
    acceptor.async_accept(sesh->socket, [sesh, this](const error_code& accept_error)
    {
        if(!accept_error)
        {
            session::read_request(sesh);
        }
        doAccept();
    });
}

void Server::run()
{
    std::cout<<"Server is running!\n";
    io_service.run();
}

int Server::parseConfig(const NginxConfig& config_out, configArguments& configArgs, serverStatistics& serverStats)
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
                serverStats.uri_prefix2request_handler_name[handler_name_].push_back(config_out.statements_[i]->tokens_[1]);
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
        else if (header != "path")
        {
            std::cerr << "Error: unrecognized header: " << header << ".\n";
            return 9;
        }
    }
    return 0;
}
