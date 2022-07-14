#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "values.h"
#include "xtc.h"
#include "api.h"
#include "client.h"

void run_client(const char* ip, const char* user, const char* passwd, std::vector<std::string> args)
{
    xtc_register_services();

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(ip);

    if (server == NULL)
    {
        printf("ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(XTC_PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting");
        exit(1);
    }

    xtc_authenticate(sockfd, std::string(user), std::string(passwd));
    std::string service = args[0];
    args.erase(args.begin());
    if(xtc_services.find(service) != xtc_services.end())
    {
        xtc_services[service]->request(sockfd, args);
    }
    else std::cout << "Invalid service " << service << std::endl;
}