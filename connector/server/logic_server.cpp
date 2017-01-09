/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "logic_server.h"

Server::Logic_server::Logic_server(){
    this->signal_break = false;
    clientManager = ClientManager();
    this->accountMessage.init_mysql();
}

Server::Logic_server::Logic_server(const char *ip , int32_t port , int32_t maxsize , int32_t listenq , int32_t fdsize , int32_t epollevents){
    memset(this->IPADDRESS , 0 , sizeof(this->IPADDRESS));
    strcpy(this->IPADDRESS , ip);
    this->PORT = port;
    this->MAXSIZE = maxsize;
    this->LISTENQ = listenq;
    this->FDSIZE = fdsize;
    this->EPOLLEVENTS = epollevents;
    
    this->signal_break = false;
    clientManager = ClientManager();
}

Server::Logic_server::~Logic_server(){

}

void Server::Logic_server::init_config(const char *path){
    Config::Configure configure = Config::Configure(path);
    configure.analyse();
    memset(this->IPADDRESS , 0 , sizeof(this->IPADDRESS));
    strcpy(this->IPADDRESS , configure["IPADDRESS"].c_str());
    this->PORT = atoi(configure["PORT"].c_str());
    this->MAXSIZE = atoi(configure["MAXSIZE"].c_str());
    this->LISTENQ = atoi(configure["LISTENQ"].c_str());
    this->FDSIZE = atoi(configure["FDSIZE"].c_str());
    this->EPOLLEVENTS = atoi(configure["EPOLLEVENTS"].c_str());
    printf("Print gateway_ini: %s %d %d %d %d %d\n" , this->IPADDRESS , this->PORT , this->MAXSIZE , this->LISTENQ , this->FDSIZE , this->EPOLLEVENTS);
    configure.close();
}

int32_t Server::Logic_server::socket_bind(const char* ip,int32_t port)
{
    int32_t  listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        perror("socket error:");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
    {
        perror("bind error: ");
        exit(1);
    }
    return listenfd;
}

void Server::Logic_server::do_epoll(int32_t listenfd)
{
    int32_t epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int32_t ret;
    char buf[MAXSIZE];
    memset(buf,0,MAXSIZE);
    //创建一个描述符
    epollfd = epoll_create(FDSIZE);
    //添加监听描述符事件
    add_event(epollfd,listenfd,EPOLLIN);
    while(this->signal_break == false)
    {
        //获取已经准备好的描述符事件
        ret = epoll_wait(epollfd,events,EPOLLEVENTS,-1);
        handle_events(epollfd,events,ret,listenfd,buf);
    }
    close(epollfd);
}

void Server::Logic_server::handle_events(int32_t epollfd,struct epoll_event *events,int32_t num,int32_t listenfd,char *buf)
{
    int32_t i;
    int32_t fd;
    //进行选好遍历
    for (i = 0;i < num;i++)
    {
        fd = events[i].data.fd;
        //根据描述符的类型和事件类型进行处理
        if ((fd == listenfd) &&(events[i].events & EPOLLIN))
            handle_accpet(epollfd,listenfd);
        else if (events[i].events & EPOLLIN)
            do_read(epollfd,fd,buf);
    }
}
void Server::Logic_server::handle_accpet(int32_t epollfd,int32_t listenfd)
{
    int32_t clifd;
    struct sockaddr_in cliaddr;
    socklen_t  cliaddrlen;
    clifd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen);
    //新连接了一个client进入了服务器，将其添加进去clientManager
    ClientSocketInfo clientSocketInfo = ClientSocketInfo();
    clientSocketInfo.setSocketFd(clifd);
    clientSocketInfo.setLocalPort(ntohs(cliaddr.sin_port));
    clientSocketInfo.setUin(clifd); //加密形成一个uin,need update
 //   clientSocketInfo.setPeerIP(inet_ntoa(cliaddr.sin_addr));
    clientSocketInfo.setPeerIP(inet_ntoa(cliaddr.sin_addr));
    clientSocketInfo.setUsername("");
    
    clientManager.addClientSocketInfo(clientSocketInfo);
  //  this->
    printf("Log Debug-------------------handle accept client: %s %d %d %d\n" , clientSocketInfo.peerIP,
            clientSocketInfo.localPort , clientSocketInfo.socketFd , clientSocketInfo.uin);
    
    if (-1 == clifd)
        perror("accpet error:");
    else
    {
        printf("accept a new client: %s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
        //添加一个客户描述符和事件
        add_event(epollfd,clifd,EPOLLIN);
    }
}

void Server::Logic_server::do_read(int32_t epollfd,int32_t fd,char *buf)
{
    int32_t nread;
  //  nread = read(fd,buf,MAXSIZE);
//    clientManager.clientManager[fd].resetReceiveMessage();
    bool flag = clientManager.clientManager[fd].readMessage(nread);
    
    
    printf("Debug len gateway_server do_read: %d\toption: %d\tread message: %s\nnext all: %s\n" , 
            *((int *)clientManager.clientManager[fd].receiveBuffer) , 
            *((int*)(clientManager.clientManager[fd].receiveBuffer+4)) , 
            clientManager.clientManager[fd].receiveBuffer+8 , 
            clientManager.clientManager[fd].receiveBuffer);
    if (-1 == nread)
    {
        perror("read error:");
        close(fd);
        delete_event(epollfd,fd,EPOLLIN);
    }
    else if (0 == nread)
    {
        fprintf(stderr,"client close.\n");
        //关闭这个fd，就将这个客户端连接从连接管理中删除
        clientManager.eraseClientSocketInfo(fd);
        close(fd);
        delete_event(epollfd,fd,EPOLLIN);
    }
    else
    {
        if(flag){

        
            printf("read message is : %s\n",clientManager.clientManager[fd].receiveBuffer);
            do_write(epollfd , fd , clientManager.clientManager[fd].receiveBuffer , clientManager.clientManager[fd].sendBuffer);
        }
    }
}

void Server::Logic_server::do_write(int32_t epollfd,int32_t fd,char *buf , char *sendMessage)
{
    int32_t nwrite;
    printf("Debug logic_server.cpp do_write: %d %d\n" , *(int *)(buf+4) , (int)Login);
    //除去账户的登录和注册，其他消息都是交给逻辑服务器处理，
    //这里是网关服务器发给逻辑服务器的消息buf，协议是包长度(int32),操作选项(int32),uin(int32),处理信息(char *)
      //  char * receFromLogic = sendMessageToLogic(fd , buf);
    int uin = *(int *)(buf+8);

    //更新分数操作
    if(*(int *)(buf+4) == (int32_t)UpdateScore){
        printf("operation 4---------fd: %d\n" , uin);
        int score = atoi(buf+12);
        char val[20]; 
        memset(val , 0 , sizeof(val));
        int curbest_score = accountMessage.getBestScore(uin);
        score = score>curbest_score?score:curbest_score;
        sprintf(val , "bestscore: %d" , score);
        accountMessage.updateScore(uin , score);
        memcpy(sendMessage , buf , 12);
        memcpy(sendMessage+12 , val , sizeof(val));
        nwrite = write(fd , sendMessage , sizeof(val)+12);
    }
    //告诉服务器鸟死了
    else if(*(int *)(buf+4) == (int32_t)BirdDie){
        memcpy(sendMessage , buf , 12);
        char val[20] = "game_over";
        memcpy(sendMessage , buf , 12);
        memcpy(sendMessage+12 , val , sizeof(val));
        nwrite = write(fd , sendMessage , sizeof(val)+12);
    }
    //点击鼠标使鸟飞一次
    else if(*(int *)(buf+4) == (int32_t)ClickBirdFly){
        memcpy(sendMessage , buf , 12);
        char val[20] = "my_bird_fly";
        memcpy(sendMessage , buf , 12);
        memcpy(sendMessage+12 , val , sizeof(val));
        nwrite = write(fd , sendMessage , sizeof(val)+12);
    }
    //点击开始游戏
    else if(*(int *)(buf+4) == (int32_t)TapToStart){
        memcpy(sendMessage , buf , 12);
        char val[20] = "wait_another";
        memcpy(sendMessage , buf , 12);
        memcpy(sendMessage+12 , val , sizeof(val));
        nwrite = write(fd , sendMessage , sizeof(val)+12);
    }
   // nwrite = write(fd,buf,strlen(buf));
    if (-1 == nwrite)
    {
        perror("write error:");
        close(fd);
        delete_event(epollfd,fd,EPOLLOUT);
    }
    else
        modify_event(epollfd,fd,EPOLLIN);
    
    clientManager.clientManager[fd].resetReceiveMessage();

    memset(buf,0,MAXSIZE);
}

void Server::Logic_server::add_event(int32_t epollfd,int32_t fd,int32_t state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void Server::Logic_server::delete_event(int32_t epollfd,int32_t fd,int32_t state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

void Server::Logic_server::modify_event(int32_t epollfd,int32_t fd,int32_t state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

void Server::Logic_server::sigroutine()
{
    this->signal_break = true;
}

//char * Server::sendMessageToLogic(int32_t fd , char* buf)
//{
//    return NULL;
//}

int main(int argc,char *argv[])
{
    int32_t  listenfd;
 
    Server::Logic_server server = Server::Logic_server();

    server.init_config("../config.ini/logic_server.ini");
    //下方的参数全是依靠配置文件自己配置好的
    listenfd = server.socket_bind(server.IPADDRESS,server.PORT);
    listen(listenfd,server.LISTENQ);
    
//    signal(SIGINT , server.sigroutine);
//    signal(SIGQUIT , server.sigroutine);
    server.do_epoll(listenfd);
    server.accountMessage.close_mysql();
    return 0;
}
