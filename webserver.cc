#include "webserver.h"
#include "session.h"
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <utility>
#include <algorithm>

using namespace boost;
using namespace boost::system;
using namespace boost::asio;

class session;

Server* Server::serverBuilder(const NginxConfig& config_out)
{
    configArguments configContents;
    int configParsingErrorCode = Server::parseConfig(config_out, configContents);
    if (configParsingErrorCode != 0)
    {
        return nullptr;
    }
    return new Server(configContents);
}

Server::Server(const configArguments& configArgs)
: io_service()
, acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), configArgs.port))
, configContent(configArgs)
, totalRequestCount(0)
{
    (this->acceptor).listen();
    doAccept();
}

void Server::doAccept()
{
    totalRequestCount++;
    std::shared_ptr<session> sesh = std::make_shared<session>(io_service, configContent.map_path_rootdir, configContent.echo_path, configContent.static_path);
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

int Server::parseConfig(const NginxConfig& config_out, configArguments& configArgs)
{
    // Use longest prefix matching to parse each statement whose header
    // is "path"
    // TODO: modify the algorithm so that it's more efficient
    std::map<std::string, std::vector<std::string> > handlerPathMap;
    std::map<std::string, std::vector<std::string> > eliminate;
    for (int i = 0; i < config_out.statements_.size(); i++)
    {
        std::string header = config_out.statements_[i]->tokens_[0];
        if (header == "path")
        {
            if (config_out.statements_[i]->tokens_.size() != 3)
            {
                std::cerr << "Error: argument is lacking or too much in statement " << i << "\n";
                return 2;
            }
            else
            {
                std::string uri_prefix = config_out.statements_[i]->tokens_[1];
                std::string requestHandlerName = config_out.statements_[i]->tokens_[2];
                handlerPathMap[requestHandlerName].push_back(uri_prefix);
            }
        }
    }
    for (std::map<std::string, std::vector<std::string> >::iterator it = handlerPathMap.begin(); it != handlerPathMap.end(); ++it)
    {
        std::sort(it->second.begin(), it->second.end());
        for (int i = 0; i < it->second.size() - 1; i++)
        {
            if ((it->second)[i] == (it->second)[i + 1])
            {
                std::cerr << "Error: there is a repetitive path for request handler " << it->first << ".\n";
                return 3;
            }
            if (it->second[i].size() < it->second[i + 1].size() && it->second[i] == it->second[i + 1].substr(0, it->second[i].size()))
            {
                eliminate[it->first].push_back((it->second)[i]);
            }
        }
    }
    for (std::map<std::string, std::vector<std::string> >::iterator it = handlerPathMap.begin(); it != handlerPathMap.end(); ++it)
    {
        for (int i = 0; i < config_out.statements_.size(); i++)
        {
            std::string header = config_out.statements_[i]->tokens_[0];
            if (header == "path" && config_out.statements_[i]->tokens_[2] == it->first)
            {
                if (std::find(eliminate[it->first].begin(), eliminate[it->first].end(), config_out.statements_[i]->tokens_[1]) == eliminate[it->first].end())
                {
                    auto handler = RequestHandler::CreateByName(it->first);
                    Status s = handler->Init(config_out.statements_[i]->tokens_[1], *(config_out.statements_[i]->child_block.get()));
                    if (s != RequestHandler::Status::OK)
                    {
                        std::cerr << "Error: failed to initialize request handler " << it->first << " for " << config_out.statements_[i]->tokens_[1] << ".\n";
                        return 4;
                    }
                    (configArgs.handlerMapping)[config_out.statements_[i]->tokens_[1]] = handler;
                }
            }
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
        else if (header == "default")
        {
            if (config_out.statements_[i]->tokens_.size() == 2)
            {
                auto handler = RequestHandler::CreateByName(config_out.statements_[i]->tokens_[1]);
                Status s = handler->Init("", *(config_out.statements_[i]->child_block.get()));
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
