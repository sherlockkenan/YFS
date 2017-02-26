// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
    mutex=PTHREAD_MUTEX_INITIALIZER;
    cond=PTHREAD_COND_INITIALIZER;
    lock_table.clear();
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
    lock_protocol::status ret = lock_protocol::OK;
    
    pthread_mutex_lock(&mutex);
    std::map<lock_protocol::lockid_t,lock_stat>::iterator it=lock_table.find(lid);
    //find
    if(it!=lock_table.end()){

        while (it->second==LOCK){
            pthread_cond_wait(&cond, &mutex);
        }
        lock_table[lid]=LOCK;
        r=lid;
        nacquire++;

    }
    //not find new lock
    else{
        lock_table.insert(std::pair<lock_protocol::lockid_t,lock_stat>(lid,LOCK));
        r=lid;
        nacquire++;
    }


    pthread_mutex_unlock(&mutex);
    return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
    pthread_mutex_lock(&mutex);
    std::map<lock_protocol::lockid_t,lock_stat>::iterator it=lock_table.find(lid);
    if(it==lock_table.end()){
        ret=lock_protocol::NOENT;
        
    }else{
        if(lock_table[lid]==LOCK){
            lock_table[lid]=FREE;
	        pthread_cond_signal(&cond);
            nacquire--;
        }
        else{
            ret=lock_protocol::RETRY;
        }


    }
    pthread_mutex_unlock(&mutex);
        
  return ret;
}
