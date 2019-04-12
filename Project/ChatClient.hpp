#pragma once

#include <iostream>
#include <string>
#include "ProtocolUtil.hpp"
#include <sstream>
#include <vector>
#include <pthread.h>
#include "Message.hpp"
#include "Window.hpp"

using namespace std;

#define TCP_PORT 8080
#define UDP_PORT 8888

class ChatClient;

struct ParamPair{
     Window *wp;
     ChatClient *cp;
    
};

class ChatClient{
private:
    int tcp_sock;
    int udp_sock;
    string peer_ip;
    string passwd;

public:
    unsigned int id;
    string nick_name;
    string school;
    struct sockaddr_in server;

public:
    ChatClient(string _ip)
        :peer_ip(_ip)
    {
        tcp_sock = -1;
        udp_sock = -1;
        server.sin_family = AF_INET;
        server.sin_port = htons(UDP_PORT);
        server.sin_addr.s_addr = inet_addr(peer_ip.c_str());
    }

    void InitClient()
    {
        // tcp_sock = SocketApi::Socket(SOCK_STREAM);
        udp_sock = SocketApi::Socket(SOCK_DGRAM);
    }

    bool ConnectServer()
    {
        tcp_sock = SocketApi::Socket(SOCK_STREAM);
        return SocketApi::Connect(tcp_sock, peer_ip, TCP_PORT);
    }

    bool Register()
    {
        if(Util::RegisterEnter(nick_name, school, passwd) && ConnectServer()){
            // 接收用户请求
            Request rq;
            // 为了让接收方知道调用方法和正文长度，所以方法 长度不进行序列化
            rq.method = "REGISTER\n";
            Json::Value root;
            root["name"] = nick_name;
            root["school"] = school;
            root["passwd"] = passwd;

            // 对用户信息序列化
            Util::Seralizer(root, rq.text);
            // 记录正文长度
            rq.content_length = "Content_Length: ";
            // stringstream
            rq.content_length += Util::IntToString((rq.text).size());
            rq.content_length += "\n";

            Util::SendRequest(tcp_sock, rq);
            recv(tcp_sock, &id, sizeof(id), 0);
            
            bool res = false;
            if(id >= 10000){
                cout << "Register Success! You Login ID Is:" << id << endl;
                res = true;
            }
            else{
                cout << "Register Failed! Code Is:" << id << endl;
            }

            close(tcp_sock);
            return res;
        }
    }
    
    bool Login()
    {
        if(Util::LoginEnter(id, passwd) && ConnectServer()){
            Request rq;
            rq.method = "LOGIN\n";
            Json::Value root;
            root["id"] = id;
            root["passwd"] = passwd;

            Util::Seralizer(root, rq.text);
            rq.content_length = "Content_Length: ";
            // stringstream
            rq.content_length += Util::IntToString((rq.text).size());
            rq.content_length += "\n";

            Util::SendRequest(tcp_sock, rq);
            unsigned int result = 0;
            recv(tcp_sock, &result, sizeof(result), 0);
     
            bool res = false;
            if(id >= 10000){
                string name = "None";
                string school = "None";
                string text = "Join ChatRoom !";
                unsigned int id = result;
                unsigned int type_ = LOGIN_TYPE;

                Message m(name, school, text, id);
                string sendString;
                m.ToSendString(sendString);
                UdpSend(sendString);
                cout << "Login Success!"  << endl;
                res = true;
            }
            else{
                cout << "Login Failed! Code Is:" << result << endl;
            }

            close(tcp_sock);
            return res;
        }
    }
    
    void UdpRecv(string &msg){
        struct sockaddr_in peer;
        Util::RecvMessage(udp_sock, msg, peer);
    }

    void UdpSend(const string &msg){
        /*  
            struct sockaddr_in peer;
            peer.sin_family = AF_INET;
            peer.sin_port = htons(UDP_PORt);
            peer.sin_addr.s_addr = peer_ip;
         */  
        Util::SendMessage(udp_sock, msg, server);
    }

    static void *Welcome(void *arg){
        pthread_detach(pthread_self());
        Window *wp = (Window*)arg;
        wp->Welcome();
    }

    static void *Input(void *arg){
        pthread_detach(pthread_self());
        struct ParamPair *pptr = (struct ParamPair*)arg;
        Window *wp = pptr->wp;
        ChatClient *cp = pptr->cp;
        
        wp->DrawInput();
        string text;
        while(1){
            wp->GetStringFromInput(text);
            Message msg(cp->nick_name, cp->school, text, cp->id);
            string sendString;
            msg.ToSendString(sendString);
            cp->UdpSend(sendString);
        }

    }

    void Chat()
    {
        
       /* string n;
        string s;
        string t;
        cout << "Please Enter Your　Name:";
        cin >> n;

        cout << "Please Enter Your School:";
        cin >> s;
        Message msg;
        while(1){
            string t;
            cout << "Please Enter:";
            cin >> t;
            Message msg(n, s, t, id);
            string sendSteing;
            msg.ToSendString(sendString);
            Util::SendMessage(udp_sock, sendString, server);

            string recvString;
            struct sockaddr_in peer;
            Util::RecvMessage(udp_sock, recvString, peer);
            cout << "Debug:" << recvString << endl;

            msg.ToRecvValue(recvString);

        }*/

        Window w;
        pthread_t h,l;
        struct ParamPair pp = {&w, this};
        pthread_create(&h, NULL, Welcome, &w);
        pthread_create(&l, NULL, Input, &pp);

        string recvString;
        string showString;
        vector<string> online;

        w.DrawOutput();
        w.DrawOnline();
        while(1){
            Message msg;
            UdpRecv(recvString);
            msg.ToRecvValue(recvString);
            cout << "name:" << msg.NickName() << endl;
            cout << "school:" << msg.School() << endl;
            cout << "Text:" << msg.Text() << endl;
            cout << "ID:" << msg.Id() << endl;

            if(msg.Id() == id && msg.Type() == LOGIN_TYPE){
                nick_name = msg.NickName();
                school = msg.School();
            }

            showString = msg.NickName();
            showString += "-";
            showString = msg.School();

            string f = showString;
            Util::addUser(online, f);
            showString += "#";
            showString += msg.Text();
            w.PutMessageToOutput(showString);

            w.PutUsersToOnline(online);
        }
    }


    bool Logout()
    {

    }

    ~ChatClient()
    {}
};
