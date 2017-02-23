//Inspired by minimal server design by phillipvoyle
//https://www.codeproject.com/Articles/1007262/A-minimal-http-web-server-using-boost-asio#_comments

#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <utility>
#include "status_handler.h"
#include "webserver.h"

using namespace boost;
using namespace boost::system;
using namespace boost::asio;

class mime_types;

RequestHandler::Status StatusHandler::Init(const std::string& uri_prefix,
                      const NginxConfig& config)
{
    return RequestHandler::Status::OK;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request& request,
                               Response* response)
{
    std::string body = "";
    Server* server = Server::getServerInstance();
    if (!server)
    {
        return RequestHandler::FAILED;
    }
    std::map<std::string, int> url_requested_count = server->getUrlRequestedCount();
    std::map<RequestHandler::Status, int> response_code_count = server->getResponseCodeCount();
    std::map<std::string, std::vector<std::string> > uri_prefix2request_handler_name = server->getUriPrefixRequestHandlerMap();
    body += "Some statistics about this server: \n";
    body += "url requested count: \n";
    for (std::map<std::string, int>::iterator it = url_requested_count.begin(); it != url_requested_count.end(); ++it)
    {
        body = body + it->first + ": " + std::to_string(it->second) + "\n";
    }
    body += "response code count: \n";
    for (std::map<RequestHandler::Status, int>::iterator it = response_code_count.begin(); it != response_code_count.end(); ++it)
    {
        int status = (int)it->first;
        body = body + std::to_string(status) + ": " + std::to_string(it->second) + "\n";
    }
    body += "\nThe request handlers exist: \n";
    for (std::map<std::string, std::vector<std::string> >::iterator it = uri_prefix2request_handler_name.begin(); it != uri_prefix2request_handler_name.end(); ++it)
    {
        body = body + it->first + " has uri prefixes " + it->second[0];
        for (int i = 1; i < it->second.size(); i++)
        {
            body = body + ", " + it->second[i];
        }
        body += ".\n";
    }
    
    response->SetStatus(Response::ok);
    response->AddHeader("content-type", "text/plain");
    response->AddHeader("content-length", std::to_string(body.size()));
    response->SetBody(body);
    
    return RequestHandler::OK;
}

