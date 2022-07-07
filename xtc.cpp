#include <iostream>
#include <fstream>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>

#include "xtc.h"
#include "sha256.h"

void xtc_create_password_hash(std::string user, std::string passwd)
{
    std::ofstream hashfs("xtc_authkey");
    hashfs << user << std::endl << sha256(passwd);
    hashfs.close();
}

void xtc_authenticate(int sockfd, std::string user, std::string passwd)
{
    xtc_send_string(sockfd, user);
    xtc_send_string(sockfd, passwd);
}

bool xtc_validate_client(int sockfd)
{
    std::ifstream hashfs("xtc_authkey");
    std::string user;
    std::string hash;
    hashfs >> user >> hash;
    hashfs.close();
    std::string inus;
    if(xtc_recv_string(sockfd, inus) >= 0)
    {
        if(user != inus) return false;
    }
    std::string inpw;
    if(xtc_recv_string(sockfd, inpw) >= 0)
    {
        if(sha256(inpw) != hash) return false;
    }
    return true;
}

int xtc_send_string(int sockfd, std::string str)
{
    return xtc_send_data(sockfd, str.c_str(), str.size());
}

int xtc_recv_string(int sockfd, std::string& str)
{
    char* charbuf = NULL;
    int len;
    int retv = xtc_recv_data(sockfd, charbuf, len);
    if(retv >= 0) str = std::string(charbuf, len);
    else xtc_error(retv, "xtc_recv_string");
    return retv;
}

int xtc_send_data(int sockfd, const char *buf, int len)
{
    int rval = write(sockfd, buf, len);
    if(rval >= 0)
    {
        char* buf = new char[1];
        rval = read(sockfd, buf, 1);
    }
    else xtc_error(rval, "xtc_send_data");
    return rval;
}

int xtc_recv_data(int sockfd, char*& buf, int& len)
{
    int toby = 0;
    int retc = 0;
    char* finbuf = new char[0];
    char* tembuf;
    while(true)
    {
        tembuf = new char[2048];
        retc = read(sockfd, tembuf, 2048);
        if(retc < 0) return retc;
        int old = toby;
        toby += retc;
        char* aux = new char[toby];
        memcpy(aux, finbuf, old);
        memcpy(&aux[old], tembuf, retc);
        delete[] finbuf;
        finbuf = aux;
        if(retc < 2048) break;
    }
    buf = finbuf;
    len = toby;
    char ack[] = {'#'};
    int rval = write(sockfd, ack, 1);
    if(rval < 0)
    {
        toby = rval;
        xtc_error(rval, "xtc_recv_data");
    }
    return toby;
}

int xtc_send_json(int sockfd, nlohmann::json dat)
{
    return xtc_send_string(sockfd, dat.dump());
}

int xtc_recv_json(int sockfd, nlohmann::json& dat)
{
    std::string str;
    int rval = xtc_recv_string(sockfd, str);
    std::cout << str << std::endl;
    if(rval >= 0) dat = nlohmann::json::parse(str);
    else xtc_error(rval, "xtc_recv_json");
    return rval;
}

int xtc_send_long(int sockfd, int64_t num)
{
    uint32_t upper_be = htonl(num >> 32);
    uint32_t lower_be = htonl((uint32_t) num);
    int retv = xtc_send_data(sockfd, (char*)&upper_be, sizeof(uint32_t));
    if(retv < 0) 
    {
        xtc_error(retv, "xtc_send_long");
        return retv;
    }
    return xtc_send_data(sockfd, (char*)&lower_be, sizeof(uint32_t));
}

int xtc_recv_long(int sockfd, int64_t& num)
{
    char* ubu;
    char* lobu;
    int ln;
    int retv = xtc_recv_data(sockfd, ubu, ln);
    if(retv < 0)
    {
        xtc_error(retv, "xtc_recv_long ubu");
        return retv;
    }
    retv = xtc_recv_data(sockfd, lobu, ln);
    if(retv < 0)
    {
        xtc_error(retv, "xtc_recv_long lobu");
        return retv;
    }
    uint32_t upper_be = ntohl(*(uint32_t*)ubu);
    uint32_t lower_be = ntohl(*(uint32_t*)lobu);
    int64_t va = 0ll;
    va |= (int64_t)upper_be << 32;
    va |= (int64_t)lower_be;
    num = va;
    return 0;
}

int xtc_send_file(int sockfd, const char* path)
{
    struct stat st;
    stat(path, &st);
    int64_t flen = st.st_size;
    int rval = xtc_send_long(sockfd, flen);
    if(rval < 0)
    {
        xtc_error(rval, "xtc_send_file xtc_send_long");
        return rval;
    }
    int fd = open(path, O_RDONLY | O_LARGEFILE);
    loff_t bywri = 0;
    rval = sendfile64(sockfd, fd, &bywri, flen);
    char* buf = new char[1];
    if(rval < 0)
    {
        close(fd);
        xtc_error(rval, "xtc_send_file sendfile64");
        return rval;
    }
    rval = read(sockfd, buf, 1);
    if(rval < 0)
    {
        close(fd);
        xtc_error(rval, "xtc_send_file read");
        return rval;
    }
}

int xtc_recv_file(int sockfd, const char* path)
{
    std::cout << "Recv file " << path << std::endl;
    int64_t flen;
    int rval = xtc_recv_long(sockfd, flen);
    if(rval < 0)
    {
        xtc_error(rval, "xtc_recv_file xtc_recv_long");
        return rval;
    }
    int fd = open(path, O_WRONLY | O_CREAT, 0666);
    char* buf = new char[16384];
    int toby = 0;
    while(toby < flen)
    {
        rval = read(sockfd, buf, 16384);
        if(rval < 0)
        {
            close(fd);
            xtc_error(rval, "xtc_recv_file read");
            return rval;
        }
        rval = write(fd, buf, rval);
        if(rval < 0)
        {
            close(fd);
            xtc_error(rval, "xtc_recv_file write");
            return rval;
        }
        toby += rval;
    }
    close(fd);
    std::cout << "File closed" << std::endl;
    char ack[] = {'#'};
    rval = write(sockfd, ack, 1);
    if(rval < 0) 
    {
        xtc_error(rval, "xtc_recv_file ack");
        return rval;
    }
    return toby;
}

void xtc_error(int err, std::string what)
{
    std::cout << "ERROR: " << what << " failed: " << err << std::endl;
}