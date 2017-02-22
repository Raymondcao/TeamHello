#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include "webserver.h"
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <utility>
#include "request_handler.h"
#include "request_handler_echo.h"
#include "request_handler_static.h"

using namespace boost;
using namespace boost::system;
using namespace boost::asio;


/// The common session for all incoming requests.

class session
{
public:
   
   static void read_body(std::shared_ptr<session> pThis);
   
   static void read_next_line(std::shared_ptr<session> pThis);
   
   static void read_first_line(std::shared_ptr<session> pThis);
   
   std::string read_first_line_text(std::istream & stream, std::string & line, std::string & ignore);

   std::string read_next_line_text(std::istream & stream, std::string & line, std::string & ignore);

   std::string ss = "";
   
   configArguments configContent;

   ip::tcp::socket socket;
   
   session(io_service& io_service, const configArguments& configContent)
      :socket(io_service)
   {
       this->configContent = configContent;
   }
   
   static void read_request(std::shared_ptr<session> pThis);
private:
    asio::streambuf buff;
};


#endif // HTTP_REQUEST_HANDLER_H
