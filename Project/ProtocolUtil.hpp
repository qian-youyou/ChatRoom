#pragma once

#include <iostream>
#include "Log.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include "UserManager.hpp"
#include "json/json.h"
#include <sstream>

#define BACKLOG 5
#define MESSAGE_SIZE 512

class Request{
public:
    std::string method;
    std::string content_length; // 正文长度
    std::string blank;
    std::string text;

Request()
        :blank("\n")
    {}

    ~Request()
    {}
};

class Util{
public:
    static bool RegisterEnter(std::string &nick_name, std::string &school,\
                              std::string &passwd){
        std::cout << "Please Enter Nick Name : ";
        std::cin >> nick_name;
        std::cout << "Please Enter School : ";
        std::cin >> school;
        std::cout << "Please Enter Passwd: ";
        std::cin >> passwd;
        std::string again;
        std::cout << "Please Enter Passwd Again: ";
        std::cin >> again;
        if(passwd == again){
            return true;
        }
        return false;
    }

    static bool LoginEnter(unsigned int &id, std::string &passwd){
        std::cout << "Please Enter Your ID:  ";
        std::cin >> id;
        std::cout << "Please Enter Your Passwd:  ";
        std::cin >> passwd;
        return true;
    }

    static void Seralizer(Json::Value &root, std::string outString){
        Json::FastWriter w;
        outString = w.write(root);
    }

    static void UnSeralizer(std::string &inString, Json::Value &root){
        Json::Reader r;
        r.parse(inString, root, false);
    }

    static std::string IntToString(int x){
        std::stringstream ss;
        ss << x;
        return ss.str();
    }

    static int StringToInt(std::string &str){
        int x;
        std::stringstream ss(str);
        ss >> x;
        return x;

    }

    static void RecvOneLine(int sock, std::string &outString){

        char c ='x';
        while(c != '\n'){
            ssize_t s = recv(sock, &c, 1, 0);
            
            if(s >0){
                if(c == '\n'){
                    break;
                }
                outString.push_back(c);
            }
            else{
                break;
            }
        }
    }

    static void RecvRequest(int sock, Request &rq){
        RecvOneLine(sock, rq.method);
        RecvOneLine(sock, rq.content_length);
        RecvOneLine(sock, rq.blank);

        std::string &cl = rq.content_length;

        // RecvOneLine(sock, rq.text);
        std::size_t pos = cl.find(":");
        if(std::string::npos == pos){
            return ;
        }
        std::string sub = cl.substr(pos+2);
        int size = StringToInt(sub);
        char c;
        for(auto i = 0; i < size; i++){
            recv(sock, &c, 1, 0);
            (rq.text).push_back(c);
        }
    }

    static void SendRequest(int sock, Request &rq){
            std::string &_m = rq.method;
            std::string &_cl = rq.content_length;
            std::string &_b = rq.blank;
            std::string &_t = rq.text;
            send(sock, _m.c_str(), _m.size(), 0);
            send(sock, _cl.c_str(), _cl.size(), 0);
            send(sock, _b.c_str(), _b.size(), 0);
            send(sock, _t.c_str(), _t.size(), 0);
    }

    static void RecvMessage(int sock, std::string &message,\
                            struct sockaddr_in &peer){
        char msg[MESSAGE_SIZE];
        socklen_t len = sizeof(peer);
        ssize_t s =recvfrom(sock, msg, sizeof(msg)-1, 0,\
                            (struct sockaddr*)&peer, &len);
        if(s < 0){
            LOG("recvfrom message error", WARING);
        }
        else{
            message = msg;
        }
    }   

    static void SendMessage(int sock, const std::string &message,\
                            struct sockaddr_in &peer){
        sendto(sock, message.c_str(), message.size(), 0,\
               (struct sockaddr*)&peer, sizeof(peer));
    }

    static void addUser(std::vector<std::string> &online, std::string &f){
        for(auto it = online.begin(); it != online.end(); it++){
            if(*it == f){
                return;
            }
        }
        online.push_back(f);    
    }
};

class SocketApi{

    public:
        static int Socket(int type){

            int sock = socket(AF_INET, type, 0);
            if(sock < 0){
                LOG("Socket error!", ERROR);
                exit(2);
            }
            return sock;
        }

        static void Bind(int sock, int port){

            struct sockaddr_in local;
            local.sin_family = AF_INET;
            local.sin_addr.s_addr = htonl(INADDR_ANY);
            local.sin_port = htons(port);

            if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
                LOG("Bind error", ERROR);
                exit(3);
            }
        }

        static void Listen(int sock){

            if(listen(sock, BACKLOG) < 0){
                LOG("Listen error", ERROR);
                exit(4);
            }
        }

        static int Accept(int listen_sock, std::string &net_ip, int &net_port){

            struct sockaddr_in peer; // 远端信息,客户端通过accept后两个参数传信息
            socklen_t len = sizeof(peer);
            int sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
            if(sock < 0){

                LOG("Accept error", WARING);
                return -1;
            }
            net_ip = inet_ntoa(peer.sin_addr); 
            net_port = htons(peer.sin_port);
            return sock;
        }

        static bool Connect(const int &sock, std::string peer_ip,const int &port){

            struct sockaddr_in peer;
            peer.sin_family = AF_INET;
            peer.sin_addr.s_addr = inet_addr(peer_ip.c_str());
            peer.sin_port = htons(port);

            if(connect(sock, (struct sockaddr*)&peer, sizeof(peer)) < 0){
                LOG("Connect error!", WARING);
                return false;
            }
            return true;
        }
};

