/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "client_connect.h"


ClientManager::ClientManager(){
    clientManager.clear();
}

ClientManager::~ClientManager(){
}

void ClientManager::init(){
    clientManager.clear();
}

void ClientManager::addClientSocketInfo(ClientSocketInfo client){
    clientManager[client.socketFd] = client;
}

void ClientManager::eraseClientSocketInfo(int32_t socketFd){
    if((it=clientManager.find(socketFd)) != clientManager.end())
        clientManager.erase(it);
}

bool ClientManager::updateUserName(const char* name , int32_t uin){
    for(it=clientManager.begin() ; it!=clientManager.end() ; it++){
        if((it->second).uin == uin){
            (it->second).setUsername(name);
            return true;
        }
    }
    return false;
}

ClientSocketInfo* ClientManager::searchClientSocketInfo(int32_t socketFd){
    if((it=clientManager.find(socketFd)) != clientManager.end()) return &(it->second);
    else return NULL;
}

int32_t ClientManager::searchClientSocketFd(int32_t uin){
    for(it=clientManager.begin() ; it!=clientManager.end() ; it++){
        if((it->second).uin == uin) return it->first;
    }
    return -1;
}
