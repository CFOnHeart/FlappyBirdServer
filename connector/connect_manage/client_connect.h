/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   client_connect.h
 * Author: ganjungan
 *
 * Created on 2017年1月5日, 下午7:46
 */

#ifndef CLIENT_CONNECT_H
#define CLIENT_CONNECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdint.h>
#include <vector>
#include <map>

enum Protocal{
    Login=1 , //登录操作
    UpdateScore , //上传分数
    Register , //注册
    TapToStart , //点击准备启动游戏
    BirdDie , //鸟死了
    ClickBirdFly , //点击鼠标使鸟飞一次
    LogicServerFlag , //表示这是逻辑服务器传回来的消息
};

class ClientSocketInfo{
public:
    const static int USERNAME_SIZE = 20;
    const static int MAX_IP_LENGTH = 20;
    const static int MAX_CLIENT_SIZE = 1024; //在socket连接中读写消息缓存的大小
    int32_t socketFd;
    int32_t socketType;
    int32_t localPort;
    
    int32_t uin; //与应用层领域相关联的IO，用于唯一标识
    //暂时认定客户端刚连进来的时候如果没有用户名密码就认为是空，不然直接将登陆或者刚注册成功的账号放在其中
    char username[USERNAME_SIZE]; 
    
    char peerIP[MAX_IP_LENGTH];
    
    int32_t receiveBytes; //用来表示当前socketFd下已经缓存的数据长度
    char receiveBuffer[MAX_CLIENT_SIZE]; //将读到的数据保存到这个缓冲区下
    char sendBuffer[MAX_CLIENT_SIZE]; //将发送返回的数据保存到这个缓冲区下
    
    ClientSocketInfo(){
        memset(receiveBuffer , 0 , sizeof(receiveBuffer));
        memset(sendBuffer , 0 , sizeof(sendBuffer));
        receiveBytes = 0;
    }
    ~ClientSocketInfo(){
        
    }
    void setSocketFd(int32_t socketFd){this->socketFd = socketFd;}
    void setSocketType(int32_t socketType){this->socketType = socketType;}
    void setLocalPort(int32_t localPort){this->localPort = localPort;}
    void setUin(int32_t uin){this->uin = uin;}
    void setUsername(const char *username){
        strncpy(this->username , username , USERNAME_SIZE);
    }
    void setPeerIP(const char *peerIP){
        strncpy(this->peerIP , peerIP , MAX_IP_LENGTH);
    }
    void setReceiveBytes(int32_t receiveBytes){this->receiveBytes = receiveBytes;}
    void setReceiveBuffer(const char *receiveBuffer){
        strncpy(this->receiveBuffer , receiveBuffer , MAX_CLIENT_SIZE);
    }
    void setSendBuffer(const char *sendBuffer){
        strncpy(this->sendBuffer , sendBuffer , MAX_CLIENT_SIZE);
    }
    
    //下方的几个函数处理粘包和拆包的问题，将数据缓存到receiveBuffer中
    int32_t receiveMessageHead(){
        while(this->receiveBytes < 8){ //我自定义了包头保存了长度4字节，以及操作选项4字节
            int32_t nread = read(socketFd , receiveBuffer+receiveBytes , MAX_CLIENT_SIZE);
            if(nread<=0){
                
                return nread;
            }
            receiveBytes += nread;
            printf("Debug client_connect.h receiveMessageHead: %d %d %s\n" , *((int*)(this->receiveBuffer)) , *((int*)(this->receiveBuffer+4)) , receiveBuffer+8);
        }
        
        return this->receiveBytes;
    }
    
    int32_t receiveMessage(){
        int32_t val = this->receiveMessageHead();
        if(val<=0) return val;
        int32_t len = *((int*)(this->receiveBuffer));
        printf("Debug client_connect.h receiveMessage: %d %d\n",len,*((int*)(this->receiveBuffer+4)));
        while(len > this->receiveBytes){
            int32_t nread = read(socketFd , receiveBuffer+receiveBytes , MAX_CLIENT_SIZE);
            if(nread<=0) return nread;
            receiveBytes += nread;
        }
        return len;
    }
    //从缓冲区读入一次数据，当读入缓存中存的数据长度以及大于等于包长时就说明取到了完整的数据包
    bool readMessage(int32_t &nread){
        nread = read(socketFd , receiveBuffer+receiveBytes , MAX_CLIENT_SIZE);
        if(nread>0) receiveBytes += nread;
        if(receiveBytes<4) return false;
        int len = *(int *)receiveBuffer;
        return receiveBytes >= len;
    }
    
    bool resetReceiveMessage(){
        if(this->receiveBytes<8) return false;
        int32_t len = *((int*)(this->receiveBuffer));
        if(len > this->receiveBytes) return false;
        memcpy(this->sendBuffer , this->receiveBuffer , receiveBytes);
        memcpy(this->receiveBuffer , this->sendBuffer+len , receiveBytes-len);
        receiveBytes -= len;
        return true;
    }
    //往sendMessage中写入数据
//    bool writeSendMessage(){
//        if(this->receiveBytes<8) return false;
//        int32_t len = *((int*)(this->receiveBuffer));
//        if(len > this->receiveBytes) return false;
//        strncpy(this->sendBuffer , this->receiveBuffer , len);
//        return true;
//    }
};

class ClientManager{
public:
    std::map<int32_t,ClientSocketInfo> clientManager;
    std::map<int32_t,ClientSocketInfo>::iterator it;
    
    ClientManager();
    ~ClientManager();
    void init();
    void addClientSocketInfo(ClientSocketInfo client);
  //  void eraseClientSocketInfo(ClientSocketInfo client);
    void eraseClientSocketInfo(int32_t socketFd);
  //  void eraseClientSocketInfo(int32_t uin);
    bool updateUserName(const char *name , int32_t uin);
    //根据fd找到对应的ClientSocketInfo
    ClientSocketInfo* searchClientSocketInfo(int32_t socketFd);
    //根据uin找到对应的client的fd
    int32_t searchClientSocketFd(int32_t uin);
};

#endif /* CLIENT_CONNECT_H */

