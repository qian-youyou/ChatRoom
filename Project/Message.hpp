#pragma once

#include <iostream>
#include <string>
#include "json/json.h"
#include "ProtocolUtil.hpp"

using namespace std;

#define NORMAL_TYPE 0
#define LOGIN_TYPE 1

class Message{
public:
    string nick_name;
    string school;
    string text;
    unsigned int id;
    unsigned int type;

public:

    Message()
    {}

    Message(const string &_nick_name, const string &_school, const string _text,\
            const unsigned int &_id, const unsigned int &_type = NORMAL_TYPE)
        :nick_name(_nick_name)
        ,school(_school)
        ,text(_text)
        ,id(_id)
        ,type(_type)
    {}

    void ToSendString(std::string& sendString){
        Json::Value root;
        root["name"] = nick_name;
        root["school"] = school;
        root["text"] = text;
        root["id"] = id;
        root["type"] = type;
        Util::Seralizer(root, sendString);
    }

    void ToRecvValue(string &recvString){
        Json::Value root;
        Util::UnSeralizer(recvString, root);
        nick_name = root["name"].asString();
        school = root["school"].asString();
        text = root["text"].asString();
        id = root["id"].asInt();
        type = root["type"].asInt();
    }

    const string &NickName(){
        return nick_name;
    }

    const string &School(){
        return school;
    }

    const string &Text(){
        return text;
    }

    const unsigned int &Id(){
        return id;
    }
    
    const unsigned int &Type(){
        return type;
    }

    ~Message()
    {}
};
