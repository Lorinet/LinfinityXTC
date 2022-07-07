#pragma once
#include <string>

#include <stdint.h>

#include "json.hpp"

void xtc_create_password_hash(std::string user, std::string passwd);
void xtc_authenticate(int sockfd, std::string user, std::string passwd);
bool xtc_validate_client(int sockfd);

int xtc_send_data(int sockfd, const char* buf, int len);
int xtc_recv_data(int sockfd, char*& buf, int& len);

int xtc_send_string(int sockfd, std::string str);
int xtc_recv_string(int sockfd, std::string& str);

int xtc_send_json(int sockfd, nlohmann::json dat);
int xtc_recv_json(int sockfd, nlohmann::json& dat);

int xtc_send_long(int sockfd, int64_t num);
int xtc_recv_long(int sockfd, int64_t& num);

int xtc_send_file(int sockfd, const char* path);
int xtc_recv_file(int sockfd, const char* path);

void xtc_error(int err, std::string what);