#include <iostream>
#include <string>
#include <fstream>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "values.h"
#include "discovery.h"
#include "json.hpp"

void *run_discovery_server(void *null)
{
    int sock;
    int yes = 1;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    int count;
    int ret;
    fd_set readfd;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        printf("sock error\n");
    }

    addr_len = sizeof(struct sockaddr_in);

    memset((void *)&server_addr, 0, addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(XTC_DISCOVERY_PORT);

    ret = bind(sock, (struct sockaddr *)&server_addr, addr_len);
    if (ret < 0)
    {
        printf("bind error");
    }
    while (true)
    {
        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);

        ret = select(sock + 1, &readfd, NULL, NULL, 0);
        if (ret > 0)
        {
            if (FD_ISSET(sock, &readfd))
            {
                count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addr_len);
                if (strstr(buffer, "xtc_discovery_found"))
                {
                    std::ifstream hashfs("xtc_authkey");
                    std::string user;
                    hashfs >> user;
                    hashfs.close();
                    char hname[256];
                    gethostname(hname, 256);
                    std::string devname;
                    std::ifstream dnf("xtc_devname");
                    if(!dnf.fail())
                    {
                        std::getline(dnf, devname);
                        dnf.close();
                    }
                    else devname = std::string(hname);
                    nlohmann::json js = {
                        {"message", "xtc_discovery_ack"},
                        {"username", user},
                        {"device_name", devname}
                    };
                    std::string jss = js.dump();
                    count = sendto(sock, jss.c_str(), jss.size(), 0, (struct sockaddr *)&client_addr, addr_len);
                }
            }
        }
    }
}

void broadcast_discovery()
{
    int sock;
    int yes = 1;
    struct sockaddr_in broadcast_addr;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    int count;
    int ret;
    fd_set readfd;
    char buffer[16192];
    int i;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        printf("sock error");
    }
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
    if (ret == -1)
    {
        printf("setsockopt error");
    }

    addr_len = sizeof(struct sockaddr_in);

    memset((void *)&broadcast_addr, 0, addr_len);
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(XTC_DISCOVERY_PORT);

    ret = sendto(sock, "xtc_discovery_found", strlen("xtc_discovery_found"), 0, (struct sockaddr *)&broadcast_addr, addr_len);

    FD_ZERO(&readfd);
    FD_SET(sock, &readfd);

    ret = select(sock + 1, &readfd, NULL, NULL, NULL);

    if (ret > 0)
    {
        if (FD_ISSET(sock, &readfd))
        {
            count = recvfrom(sock, buffer, 16192, 0, (struct sockaddr *)&server_addr, &addr_len);
            try
            {
                nlohmann::json js = nlohmann::json::parse(std::string(buffer));
                if(js["message"] == "xtc_discovery_ack")
                {
                    js["ip"] = std::string(inet_ntoa(server_addr.sin_addr));
                    js.erase("message");
                    std::cout << js << std::endl;
                }
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << '\n';
            }
        }
    }
}