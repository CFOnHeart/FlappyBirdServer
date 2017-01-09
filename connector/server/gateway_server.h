#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H



#include  "../connect_manage/client_connect.h"
#include "../message_solve/account_message_solve.h"

namespace Server{
const char *gateway_path = "gateway_path.ini";
class Gateway_server{
public:
    char IPADDRESS[17];
    int32_t PORT;
    int32_t MAXSIZE;
    int32_t LISTENQ;
    int32_t FDSIZE;
    int32_t EPOLLEVENTS;
    
    bool signal_break; //中断处理信号
    //处于../connect_manage/client_connect.h中 用于管理客户端的连接，
    //这是个map，key值是socketfd,value是ClientSocketInfo
    ClientManager clientManager;
    message_solve::AccountMessage accountMessage;

    int logicServerFd;//连接逻辑服务器的fd

public:
    Gateway_server();
    Gateway_server(const char *ip , int32_t port , int32_t maxsize , int32_t listenq , int32_t fdsize , int32_t epollevents);
    ~Gateway_server();
    
    //将上方的成员变量用配置文件path来读取它
    void init_config(const char *path);

    //函数声明
    //创建套接字并进行绑定
    int socket_bind(const char* ip,int32_t port);
    //IO多路复用epoll
    void do_epoll(int32_t listenfd);
    //事件处理函数
    void handle_events(int32_t epollfd,struct epoll_event *events,int32_t num,int32_t listenfd,char *buf);
    //处理接收到的连接
    void handle_accpet(int32_t epollfd,int32_t listenfd);
    //读处理
    void do_read(int32_t epollfd,int32_t fd,char *buf);
    //写处理
    void do_write(int32_t epollfd,int32_t fd,char *buf,char *sendMessage);
    //添加事件
    void add_event(int32_t epollfd,int32_t fd,int32_t state);
    //修改事件
    void modify_event(int32_t epollfd,int32_t fd,int32_t state);
    //删除事件
    void delete_event(int32_t epollfd,int32_t fd,int32_t state);
    //处理中断信号事件
    void sigroutine();
    //连接逻辑服务器
    void connectLogicServer();
    //根据username来hash计算一个唯一的uin
    int32_t hashForUin(const char *username);
    //消息发送给逻辑服务器处理
   // char * sendMessageToLogic(int32_t fd , char* buf);
};
};
#endif
