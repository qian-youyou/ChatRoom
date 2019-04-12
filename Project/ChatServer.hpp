#pragma once

#include <iostream>
#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "Log.hpp"
#include "UserManager.hpp"
#include "DataPool.hpp"
#include "Message.hpp"
#include <unistd.h>

class ChatServer;

class Param{
public:
    ChatServer *sp;
    int sock;

    std::string ip;
    int port;

    Param(ChatServer *_sp, int &_sock, const std::string &_ip,\
          const int &_port)
        :sp(_sp)
         ,sock(_sock)
         ,ip(_ip)
         ,port(_port)
    {}

    ~Param()
    {}
};

class ChatServer{

private:
    int tcp_regist_sock; // 注册/登录 
    int tcp_port;        // TCP端口号

    int udp_trans_sock;   // 传输信息
    int udp_port;        // UDP端口号

    UserManager um;
    DataPool pool;
public:
    ChatServer(int tcp_port_ = 8080, int udp_port_ = 8088)
            :tcp_port(tcp_port_)
             ,tcp_regist_sock(-1)
             ,udp_port(udp_port_)
             ,udp_trans_sock(-1)
    {}

    void InitServer(){

        tcp_regist_sock = SocketApi::Socket(SOCK_STREAM);
        SocketApi::Bind(tcp_regist_sock, tcp_port);
        SocketApi::Listen(tcp_regist_sock);
       
        udp_trans_sock = SocketApi::Socket(SOCK_DGRAM);
        SocketApi::Bind(udp_trans_sock, udp_port);
    }
   
    unsigned int RegisterUser(const std::string &name, const std::string &school,\
                              const std::string &passwd){
        return um.Insert(name, school, passwd);
    }
    
    unsigned int LoginUser(const unsigned int &id, const std::string &passwd ,\
                           std::string &ip, int port){

        return um.Check(id, passwd);
    }

    void Product(){
        std::string message;
        struct sockaddr_in peer;
        Util::RecvMessage(udp_trans_sock, message, peer);
        std::cout << "Debug: recv message:" << message << std::endl;
        if(!message.empty()){
            Message m;
            m.ToRecvValue(message);
            if(m.Type() == LOGIN_TYPE){
                um.AddOnlineUser(m.Id(), peer);
                std::string name_;
                std::string school_;
                um.GetUserInfo(m.Id(), name_, school_);
                Message new_msg(name_, school_, m.Text(), m.Id(), m.Type());
                new_msg.ToSendString(message);
            }
            pool.PutMessage(message);
        }
    }

    void Consume(){
        std::string message;
        pool.GetMessage(message);
        std::cout << "Debug: send message:" << message << std::endl;
        auto online = um.OnlineUser();
        for(auto it = online.begin(); it != online.end(); it++){
            Util::SendMessage(udp_trans_sock, message, it->second);
        }
    }

    static void *HandlerRequest(void *arg){
        // 读到sock，通过协议分析，再处理，再响应
        
        Param *p = (Param*)arg;
        int sock = p->sock;
        ChatServer *sp = p->sp;
        std::string ip = p->ip;
        int port = p->port;
        delete p;
        pthread_detach(pthread_self());

        Request rq;
        Util::RecvRequest(sock, rq); 
        Json::Value root;
        LOG(rq.text, NORMAL);
        Util::UnSeralizer(rq.text, root);
        
        if(rq.method == "REGISTER"){

            std::string name = root["name"].asString();
            std::string school = root["school"].asString();
            std::string passwd = root["passwd"].asString();
            unsigned int id = sp->RegisterUser(name, school, passwd);
            send(sock, &id, sizeof(id), 0);
        }
        else if(rq.method == "LOGIN"){
            unsigned int id = root["id"].asInt();
            std::string passwd = root["passwd"].asString();
            unsigned int result = sp->LoginUser(id, passwd, ip, port);
            send(sock, &result, sizeof(result), 0);

        }
        else{
            // LogOut
        }

        close(sock);
    }


    void Start(){
   
        std::string ip; // 获取新链接
        int port;
        // 主线程：用户管理---TCP
        for(;;){
            int sock = SocketApi::Accept(tcp_regist_sock, ip, port);
            if(sock > 0){
                
                std::cout << "get a new client" << ip << ":" << port << std::endl;
                
                Param *p = new Param(this, sock, ip, port);
                pthread_t tid;
                pthread_create(&tid, NULL, HandlerRequest, p);
            }
        }
    }   

    ~ChatServer()
    {}   
};
