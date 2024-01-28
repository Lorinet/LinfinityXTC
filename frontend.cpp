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
#include "frontend.h"

int frontend_request(nlohmann::json in, nlohmann::json& out)
{
    std::cout << "FRONTEND REQUEST" << std::endl;
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("ERROR opening socket");
        return -1;
    }

    server = gethostbyname("127.0.0.1");

    if (server == NULL)
    {
        printf("ERROR, no such host\n");
        return -1;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(XTC_FRONTEND_PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting");
        return -1;
    }
    std::cout << "FRONTEND REQUEST SEND JSON" << std::endl;
    int rval = xtc_send_json(sockfd, in);
    std::cout << rval << std::endl;
    if(rval < 0) return rval;
    rval = xtc_send_string(sockfd, "\n");
    std::cout << rval << std::endl;
    if(rval < 0) return rval;
    std::cout << "FRONTEND REQUEST RECV JSON" << std::endl;
    return xtc_recv_json(sockfd, out);
}