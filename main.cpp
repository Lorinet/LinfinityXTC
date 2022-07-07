#include <iostream>
#include <string>

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "xtc.h"
#include "server.h"
#include "client.h"

int main(int argc, char* argv[])
{
    if(argc > 1)
    {
        if(strcmp(argv[1], "server") == 0)
        {
            run_server();
        }
        else if(strcmp(argv[1], "client") == 0)
        {
            std::vector<std::string> args;
            for(int i = 5; i < argc; i++) args.push_back(std::string(argv[i]));
            run_client(argv[2], argv[3], argv[4], args);
        }
        else if(strcmp(argv[1], "passwd") == 0)
        {
            std::cout << "Enter username: ";
            std::string user;
            std::getline(std::cin, user);
            termios oldt;
            tcgetattr(STDIN_FILENO, &oldt);
            termios newt = oldt;
            newt.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            std::cout << "Enter password: ";
            std::string pass;
            std::getline(std::cin, pass);
            std::cout << std::endl;
            std::cout << "Verify password: ";
            std::string conpass;
            std::getline(std::cin, conpass);
            std::cout << std::endl;
            if(pass == conpass)
            {
                xtc_create_password_hash(user, pass);
                std::cout << "Password hash created" << std::endl;
            }
            else std::cout << "Passwords do not match" << std::endl;
        }
    }
    else
    {
        std::cout << "Usage:\nxtc passwd - create password hash file 'xtc_authkey'\nxtc server - start XTC server\nxtc client <ip> <user> <passwd> <service> <request ...> - connect to XTC server and send request" << std::endl;
    }
    return 0;
}