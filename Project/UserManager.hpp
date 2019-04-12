#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <pthread.h>


class User{
private:
    std::string nick_name;
    std::string school;
    std::string pwd;

public:
    User()
    {}

    User(const std::string &_nick_name, const std::string &_school,\
         const std::string _pwd)
        :nick_name(_nick_name)
        ,school(_school)
        ,pwd(_pwd)
    {}

    bool IsPasswdOk(const std::string &_passwd){
        return pwd == _passwd ? true:false;
    }

    std::string &GetNickName(){
        return nick_name;
    }

    std::string &GetSchool(){
        return school;
    }

    ~User()
    {}
};

class UserManager{
private:
    // id与用户的映射表
    unsigned int assign_id;
    std::unordered_map<unsigned int, User> users;
    std::unordered_map<unsigned int, struct sockaddr_in> online_users; // 从套接字获取客户端ip 端口
    pthread_mutex_t lock;

    void Lock(){
        pthread_mutex_lock(&lock);
    }

    void UnLock(){
        pthread_mutex_unlock(&lock);
    }

public:
    UserManager():assign_id(10000)
    {
        pthread_mutex_init(&lock, NULL);
    }
    
    unsigned int Insert(const std::string &_nick_name, const std::string &_school,\
                        const std::string &_passwd){
        Lock();
        unsigned int id = assign_id++;
        User u(_nick_name, _school, _passwd);
        if(users.find(id) == users.end()){
            //user.insert(make_pair(id, u));
            users.insert({id, u});
            UnLock();
            return id;
        }
        UnLock();
        return 1;
    }

    unsigned int Check(const int &id, const std::string &passwd){
        Lock();
        auto it = users.find(id);
        if(it != users.end()){
            User &u = it->second;
           if(u.IsPasswdOk(passwd)){
               UnLock(); 
               return id; 
           }
        }
        UnLock();
        return 2;
    }

    void AddOnlineUser(unsigned int id, struct sockaddr_in &peer){
        Lock();
        auto it = online_users.find(id);
        if(it == online_users.end()){
            online_users.insert({id, peer});
        }
        UnLock();
    }

    std::unordered_map<unsigned int, struct sockaddr_in> OnlineUser(){
        Lock();
        auto online = online_users; 
        UnLock();
        return online;
    }

    void GetUserInfo(const unsigned int &id, std::string &name, std::string &school){
        Lock();
        name = users[id].GetNickName(); 
        school = users[id].GetSchool();
        UnLock();

    }

    ~UserManager()
    {
        pthread_mutex_destroy(&lock);
    }
};
