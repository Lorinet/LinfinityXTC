#pragma once
#include "json.hpp"
#include <string>
#include <vector>
#include <map>

class xtc_api;

extern std::map<std::string, xtc_api*> xtc_services;

void xtc_register_services();
int xtc_dispatch_request(int sockfd, nlohmann::json request);

class xtc_api
{
public:
    virtual int request(int sockfd, std::vector<std::string> args) = 0;
    virtual int action(int sockfd, nlohmann::json request) = 0;
};

class xtc_notifications : public xtc_api
{
public:
    static void initialize()
    {
        xtc_services.emplace("xtc_notifications", new xtc_notifications());
    }
    int request(int sockfd, std::vector<std::string> args);
    int action(int sockfd, nlohmann::json request);
};

class xtc_airshare : public xtc_api
{
public:
    static void initialize();
    static std::string home_dir;
    int request(int sockfd, std::vector<std::string> args);
    int action(int sockfd, nlohmann::json request);
};