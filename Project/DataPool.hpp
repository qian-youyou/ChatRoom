#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <semaphore.h>

using namespace std;

class DataPool{
private:
    vector<string> pool;
    int cap;
    sem_t data_sem;
    sem_t blank_sem;
    int product_step;
    int consume_step;

public:
    DataPool(int _cap = 512)
        :cap(_cap)
        ,pool(_cap)
    {
        sem_init(&data_sem, 0, 0);
        sem_init(&blank_sem, 0, cap);
        product_step = 0;
        consume_step = 0;
    }

    void GetMessage(string &msg){
        sem_wait(&data_sem);
        msg = pool[consume_step];
        consume_step++;
        consume_step %= cap;
        sem_post(&blank_sem);
    }

    void PutMessage(const string &msg){
        sem_wait(&blank_sem);
        pool[product_step] = msg;
        product_step++;
        product_step %= cap;
        sem_post(&data_sem);
        
    }

    ~DataPool(){
        sem_destroy(&data_sem);
        sem_destroy(&blank_sem);
    }
};
