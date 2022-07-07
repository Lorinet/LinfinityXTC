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
#include "server.h"

void run_server()
{
    printf("Linfinity XTC [Version 0.1 fehu]\n");
    xtc_register_services();
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    int en = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(XTC_PORT);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd;
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            return;
        }

        pid = fork();

        if (pid < 0)
        {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0)
        {
            close(sockfd);
            process_client(newsockfd);
            exit(0);
        }
        else
        {
            close(newsockfd);
        }
    }
}

void process_client(int sockfd)
{
    if(xtc_validate_client(sockfd))
    {
        std::cout << "Authentication successful" << std::endl;
        nlohmann::json js;
        int errcode = xtc_recv_json(sockfd, js);
        if(errcode <= 0)
        {
            std::cout << "Communication error" << std::endl;
            return;
        }
        std::cout << js.dump() << std::endl;
        xtc_dispatch_request(sockfd, js);
    }
    else
    {
        std::cout << "Access denied" << std::endl;
    }
}