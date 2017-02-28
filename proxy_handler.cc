#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <boost/algorithm/string.hpp>

#include "request_handler.h"
#include "proxy_handler.h"

RequestHandler::Status ProxyHandler::Init(const std::string& uri_prefix, 
              const NginxConfig& config)
{
    if (config.statements_[0]->tokens_[0] == "proxy" && config.statements_[0]->tokens_.size() == 2)
    {
        this->redir_addr = config.statements_[0]->tokens_[1];
        this->prefix = uri_prefix;

        if (config.statements_[1]->tokens_[0] == "port" && config.statements_[1]->tokens_.size() == 2)
        {
            unsigned int tmpPort = std::stoi(config.statements_[1]->tokens_[1]);
            if (tmpPort > 65535 || tmpPort < 0)
            {
                std::cerr << "The port number " << tmpPort << " in config file is invalid.\n";
                return RequestHandler::Status::FAILED;
            }
            this->port = (short unsigned int)tmpPort;
            return RequestHandler::Status::OK;
        }
    }
    return RequestHandler::Status::FAILED;
}

// Handles an HTTP request. Writes the response to the HttpResponse object,
// and returns true if the request was handled successfully. If the return value
// is false, then the HttpResponse object is invalid and should not be used.
//
// The dispatch mechanism is implemented in the main server code.
RequestHandler::Status ProxyHandler::HandleRequest(const Request& request,
                               Response* response)
{
	using namespace std;
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;

	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(this->redir_addr, to_string(80));
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	// Try each endpoint until we successfully establish a connection.
	tcp::socket socket(io_service);
	boost::asio::connect(socket, endpoint_iterator);
	std::string requestpath = request.uri();
	if (requestpath.length() > 0 && requestpath[0] != '/')
	requestpath = "/" + requestpath;
	std::string requestURL = this->redir_addr;

	// Modify the raw request to reflect the modified URI
	std::string rawrequest = request.raw_request();
	size_t firstNewline = rawrequest.find("\r\n");
	if (firstNewline == std::string::npos) return RequestHandler::Status::FAILED; // request was malformed

	// We need to add a slash to the beginning of requestpath to standardize.
	if (requestpath.length() == 0 || requestpath[0] != '/') {
	requestpath = "/" + requestpath;
	}

	// Take out the first line of the request then add back in the modified URI
	std::string restrequest = rawrequest.substr(firstNewline, rawrequest.length() - firstNewline);
	std::string newFirstLine = 	request.method() + " / " +  + " HTTP/1.1";

	// Take out the second line of the request then add back in the modified host
	size_t secondNewline = restrequest.find("\r\n", 1);
	if (secondNewline == std::string::npos) return RequestHandler::Status::FAILED; // request was malformed
	restrequest = restrequest.substr(
	secondNewline, restrequest.length() - secondNewline);
	std::string newSecondLine = "Host: " + this->redir_addr;

	boost::trim(newFirstLine);
	boost::trim(newSecondLine);
	if (restrequest[0] != '\r') return RequestHandler::Status::FAILED;

	rawrequest = newFirstLine + "\r\n" + newSecondLine + "\r\n" +
	"Accept: */*\r\n" + "Connection: close\r\n\r\n";
	cout << rawrequest << endl;
	// Send request to proxy server.
	boost::system::error_code wr_error;
	boost::asio::write(socket, boost::asio::buffer(rawrequest));

	// Get the response
    boost::asio::streambuf respbuf;
    boost::asio::read_until(socket, respbuf, "\r\n");
    std::istream respStream(&respbuf);

    // Read in the first line
    std::string http_version_, reason_phrase_;
    Response::ResponseCode status_code_;
    int status_code_tmp;
    respStream >> http_version_;
    respStream >> status_code_tmp;
    if (status_code_tmp == 200){
      status_code_=Response::ResponseCode::moved_temporarily;
    }else{
      status_code_ = static_cast<Response::ResponseCode>(status_code_tmp);
    }
    response->SetStatus(status_code_);
    std::getline(respStream, reason_phrase_);
    // boost::trim(reason_phrase_);

    // Read all the headers
    boost::asio::read_until(socket, respbuf, "\r\n\r\n");

    std::string header;
    while (std::getline(respStream, header) && header != "\r") {
      size_t splitLoc = header.find(":");
      if (splitLoc == std::string::npos) return RequestHandler::Status::FAILED; // header was malformed

      std::string key = header.substr(0, splitLoc);
      std::string val = header.substr(splitLoc+2, header.length() - splitLoc);

      boost::trim(key);
      boost::trim(val);

      if (key == "Connection") {
        val = "keep-alive";
      }
      response->AddHeader(key,val);
    }

    // Read in whatever is left of the buffer
    std::stringstream stringRead;
    stringRead << &respbuf;

    std::string body = stringRead.str();

    boost::system::error_code ec;
    while (boost::asio::read(socket, respbuf, boost::asio::transfer_at_least(1), ec)) {
      if (ec) break;

      // Clear the stringstream
      stringRead.str("");

      stringRead << &respbuf;
      body.append(stringRead.str());
    }

    if (ec != boost::asio::error::eof) return RequestHandler::Status::FAILED;

    // Sometimes we get random numbers before the first HTML tag, we don't know
    // where they're coming from, so we're just going to parse them out for now.
    size_t firstTag = body.find("<");
    if (firstTag != std::string::npos) {
      body = body.substr(firstTag, body.length() - firstTag);
    }

    size_t lastTag = body.find_last_of(">");
    if (lastTag != std::string::npos) {
      body = body.substr(0, lastTag+1);
    }

    response->SetBody(body);


  return RequestHandler::Status::OK;
};
