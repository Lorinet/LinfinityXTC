#include <iostream>
#include <sstream>

#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include "api.h"
#include "xtc.h"
#include "values.h"

std::map<std::string, xtc_api*> xtc_services;

void xtc_register_services()
{
    xtc_notifications::initialize();
    xtc_airshare::initialize();
}

int xtc_dispatch_request(int sockfd, nlohmann::json request)
{
    if(request.find("service") == request.end()) return XTSTATUS_INVALID_SYNTAX;
    return xtc_services[request["service"]]->action(sockfd, request);
}

int xtc_notifications::request(int sockfd, std::vector<std::string> args)
{
    return XTSTATUS_OK;
}

int xtc_notifications::action(int sockfd, nlohmann::json request)
{
    if(request.find("action") == request.end()) return XTSTATUS_INVALID_SYNTAX;
    std::string act = request["action"];
    if(act == "broadcast")
    {

    }
    else if(act == "get_notifications")
    {
        nlohmann::json j = 
        {
            {
                {"app", "Messages"},
                {"title", "Random guy"},
                {"body", "Hey"},
                {"actions", {"Reply", "Mute"}}
            },
            {
                {"app", "Super Media Player"},
                {"title", "Now playing"},
                {"body", "Now playing: now playing"},
                {"actions", {"Pause", "Previous", "Next"}}
            },
            {
                {"app", "System"},
                {"title", "Test Notification X"},
                {"body", "This is a test notification"},
                {"actions", {}}
            }
        };
        xtc_send_json(sockfd, j);
    }
    return XTSTATUS_OK;
}

std::string xtc_airshare::home_dir;

void xtc_airshare::initialize()
{
    home_dir = std::string(getenv("HOME"));
    xtc_services.emplace("xtc_airshare", new xtc_airshare());
}

int xtc_airshare::request(int sockfd, std::vector<std::string> args)
{
    std::cout << "Hey" << std::endl;
    if(args.size() < 2) return XTSTATUS_INVALID_SYNTAX;
    std::string act = args[0];
    nlohmann::json js;
    if(act == "push_file")
    {
        std::stringstream ss(args[1]);
        std::string seg;
        std::vector<std::string> segl;
        while(std::getline(ss, seg, '/')) segl.push_back(seg);
        js =
        {
            {"service", "xtc_airshare"},
            {"action", "push_file"},
            {"name", segl.back()}
        };
        xtc_send_json(sockfd, js);
        xtc_send_file(sockfd, args[1].c_str());
    }
    else if(act == "pull_file")
    {
        std::stringstream ss(args[1]);
        std::string seg;
        std::vector<std::string> segl;
        while(std::getline(ss, seg, '/')) segl.push_back(seg);
        js =
        {
            {"service", "xtc_airshare"},
            {"action", "pull_file"},
            {"path", args[1]}
        };
        xtc_send_json(sockfd, js);
        xtc_recv_file(sockfd, (home_dir + "/XTCAirShare/" + segl.back()).c_str());
    }
    else if(act == "list_directory")
    {
        js =
        {
            {"service", "xtc_airshare"},
            {"action", "list_directory"},
            {"path", args[1]}
        };
        xtc_send_json(sockfd, js);
        xtc_recv_json(sockfd, js);
        std::cout << js << std::endl;
    }
    return XTSTATUS_OK;
}

int xtc_airshare::action(int sockfd, nlohmann::json request)
{
    if(request.find("action") == request.end()) return XTSTATUS_INVALID_SYNTAX;
    std::string act = request["action"];
    if(act == "list_directory")
    {
        if(request.find("path") == request.end()) return XTSTATUS_INVALID_SYNTAX;
        nlohmann::json resp;
        DIR* dir;
        dirent* ent;
        if((dir = opendir(std::string(request["path"]).c_str())) != NULL)
        {
            while((ent = readdir(dir)) != NULL)
            {
                resp.push_back(std::string(ent->d_name));
            }
            closedir(dir);
        }
        else return XTSTATUS_FAILED;
        xtc_send_json(sockfd, resp);
    }
    else if(act == "pull_file")
    {
        if(request.find("path") == request.end()) return XTSTATUS_INVALID_SYNTAX;
        xtc_send_file(sockfd, std::string(request["path"]).c_str());
    }
    else if(act == "push_file")
    {
        std::cout << "Alo" << std::endl;
        if(request.find("name") == request.end()) return XTSTATUS_INVALID_SYNTAX;
        xtc_recv_file(sockfd, (home_dir + "/XTCAirShare/" + std::string(request["name"])).c_str());
    }
    return XTSTATUS_OK;
}