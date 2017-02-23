#include "webserver.h"
#include <string>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <utility>

int main(int argc, const char * argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserver <configFileName>\n";
        return 1;
    }
    NginxConfigParser config_parser;
    NginxConfig config_out;
    if (!config_parser.Parse(argv[1], &config_out))
    {
        std::cerr << "Error: failed to parse the config file.\n";
        return 2;
    }
    auto server = std::auto_ptr<Server>(Server::serverBuilder(config_out));
    if (server.get() == nullptr)
    {
        std::cerr << "Error: cannot create server due to wrong config file format.\n";
        return 3;
    }
    (server.get())->run();
    return 0;
}
