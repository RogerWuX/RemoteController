#include <getopt.h>
#include <exception>
#include <iostream>
#include <asio.hpp>
#include "Server.h"

#define DEFAULT_SERVER_ADDRESS "0.0.0.0"
#define DEFAULT_SERVER_PORT 7777
#define DEFAULT_CERT_FILE "cert.pem"
#define DEFAULT_KEY_FILE "key.pem"

int main(int argc, char *argv[])
{
    option kLongOptTable[] =
    {
        {"addr",     required_argument, NULL, 'a'},
        {"port",     required_argument, NULL, 'p'},
        {"certfile", required_argument, NULL, 'c'},
        {"keyfile",  required_argument, NULL, 'k'},
    };

    std::string kServerAddress = DEFAULT_SERVER_ADDRESS;
    unsigned int nServerPort = DEFAULT_SERVER_PORT;
    std::string kCertFilePath = DEFAULT_CERT_FILE;
    std::string kKeyFilePath = DEFAULT_KEY_FILE;

    int nOpt;
    while(true) {
        nOpt = getopt_long(argc, argv, "apck::", kLongOptTable, 0);
        if(nOpt == -1)
            break;

        switch(nOpt){
        case 'a':{
            kServerAddress = (const char *)optarg;
            break;
        }
        case 'p':{
            nServerPort = atoi(optarg);
            if(nServerPort == 0){
                std::cerr<<"invalid server port"<<std::endl;
                return 1;
            }
            break;
        }
        case 'c':{
            kCertFilePath = (const char *)optarg;
            break;
        }
        case 'k':{
            kKeyFilePath = (const char *)optarg;
            break;
        }
        default:
            break;
        }
    }

    try{
        asio::io_context kIoContext;
        asio::ip::address kListenAddress = asio::ip::make_address(kServerAddress);

        Server kServer(kIoContext, kListenAddress, nServerPort);
        kServer.SetCertInfo(kCertFilePath, kKeyFilePath);
        kServer.StartAccept();
        kIoContext.run();
    }
    catch (std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return 0;
}
