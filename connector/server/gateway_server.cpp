#include <unistd.h>
#include <fcntl.h>
#include "gateway_server.h"

Server::Gateway_server::Gateway_server(){
    this->signal_break = false;
    clientManager = ClientManager();
    this->accountMessage.init_mysql();
}

Server::Gateway_server::Gateway_server(const char *ip , int32_t port , int32_t maxsize , int32_t listenq , int32_t fdsize , int32_t epollevents){
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

Server::Gateway_server::~Gateway_server(){

}

void Server::Gateway_server::init_config(const char *path){
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

int32_t Server::Gateway_server::socket_bind(const char* ip,int32_t port)
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

void Server::Gateway_server::do_epoll(int32_t listenfd)
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

void Server::Gateway_server::handle_events(int32_t epollfd,struct epoll_event *events,int32_t num,int32_t listenfd,char *buf)
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
void Server::Gateway_server::handle_accpet(int32_t epollfd,int32_t listenfd)
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

void Server::Gateway_server::do_read(int32_t epollfd,int32_t fd,char *buf)
{
    int32_t nread;
  //  nread = read(fd,buf,MAXSIZE);
   // clientManager.clientManager[fd].resetReceiveMessage();
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
       //用于处理好后的消息写入
            do_write(epollfd , fd , clientManager.clientManager[fd].receiveBuffer , clientManager.clientManager[fd].sendBuffer); 
       
        }
    }
}

void Server::Gateway_server::do_write(int32_t epollfd,int32_t fd,char *buf,char *sendMessage)
{
    int32_t nwrite;
    printf("Debug gateway_server.cpp do_write: %d %d\n" , *(int32_t *)(buf+4) , (int32_t)Login);
    //暂时将登录和注册的处理
    if(*(int32_t *)(buf+4) == (int32_t)Login){
        accountMessage.analyseMessage(buf+8);
        int score = accountMessage.getBestScore(accountMessage.username , accountMessage.password);
        char val[10] = {0};
        sprintf(val , "%d" , score);

        printf("Debug gateway_server do_write to client: %s\n" , val);
        //登录后需要修改这个client连接中的uin和username
        int32_t uin = hashForUin(accountMessage.username);
        clientManager.clientManager[fd].setUin(uin);
        clientManager.clientManager[fd].setUsername(accountMessage.username);
        
        nwrite = write(fd,val,strlen(val));
    }
    else if(*(int32_t *)(buf+4) == (int32_t)Register){
        accountMessage.analyseMessage(buf+8);
        int32_t score = accountMessage.getBestScore(accountMessage.username);
        if(score < 0) nwrite = write(fd,"failed",strlen("failed"));
        else{
            //获取一个uin
            int32_t uin = hashForUin(accountMessage.username);
            //注册后需要修改这个client连接中的uin和username
            clientManager.clientManager[fd].setUin(uin);
            clientManager.clientManager[fd].setUsername(accountMessage.username);
            accountMessage.addUserInfo(uin , accountMessage.username , accountMessage.password , 0);
            nwrite = write(fd,"success",strlen("success"));
        }
    }
    else if(*(int32_t *)(buf+4) == (int32_t)LogicServerFlag){
        //从逻辑服务器传回来的消息协议就是包长度(int32),类型(int32),uin(int32),传递的信息(char *)
        printf("Debug gateway_server do_write 逻辑服务器返回来的消息是 %d %d %d %s\n" ,
        *(int32_t *)buf , *(int32_t *)(buf+4) , *(int32_t *)(buf+8) , buf+12
         );
        int32_t uin = *(int32_t *)(buf+8);
        int32_t socketFd = clientManager.searchClientSocketFd(uin);
        if(socketFd > 0) nwrite = write(socketFd , buf , 12+strlen(buf+12));
        else{
            perror("传回来的uin无法找到对应的socketfd\n");
        }
    }
    else{
        //除去账户的登录和注册，其他消息都是交给逻辑服务器处理，
        //这里返回给客户端的也是逻辑服务器处理好后返回来的消息
      //  char * receFromLogic = sendMessageToLogic(fd , buf);
        int32_t len = *(int *)buf; 
        len += 4;
        int32_t uin = clientManager.clientManager[fd].uin;
        memcpy(sendMessage , (void *)(&len) , 4); //设定包长度
        memcpy(sendMessage+4 , buf+4 , 4); //设定操作的选项
        memcpy(sendMessage+8 , (void *)(&len) , 4); //将网关验证身份后的uin传到逻辑服务器
        memcpy(sendMessage+12 , buf+8 , len-12); //将所要传的消息传给逻辑服务器 
        //网关传给逻辑服务器的协议就是
        //包长(int32),操作类型(int32),uin(int32),处理数据(char*)
        nwrite = write(logicServerFd,buf,strlen(buf));
    }
    clientManager.clientManager[fd].resetReceiveMessage();
    memset(buf,0,MAXSIZE);
}

void Server::Gateway_server::add_event(int32_t epollfd,int32_t fd,int32_t state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void Server::Gateway_server::delete_event(int32_t epollfd,int32_t fd,int32_t state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

void Server::Gateway_server::modify_event(int32_t epollfd,int32_t fd,int32_t state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

void Server::Gateway_server::sigroutine()
{
    this->signal_break = true;
}

void Server::Gateway_server::connectLogicServer()
{
   Config::Configure configure = Config::Configure("../config.ini/logic_server.ini");
   configure.analyse();;
   char ip[20]; 
   int port , maxsize , listenq , fdsize , epollevents;
   memset(ip , 0 , sizeof(ip));
   strcpy(ip , configure["IPADDRESS"].c_str());
   port = atoi(configure["PORT"].c_str()); 
   maxsize = atoi(configure["MAXSIZE"].c_str());
   listenq = atoi(configure["LISTENQ"].c_str());
   fdsize = atoi(configure["FDSIZE"].c_str());
   epollevents = atoi(configure["EPOLLEVENTS"].c_str());

   printf("ip: %s\nport: %d\n" , ip , port);
   configure.close();
   struct sockaddr_in pin;
   
   bzero(&pin,sizeof(pin));
   pin.sin_family=AF_INET;                 //AF_INET表示使用IPv4
   pin.sin_addr.s_addr=inet_addr(ip);
   pin.sin_port=htons(port);
   logicServerFd = socket(AF_INET , SOCK_STREAM , 0);
   if (connect(logicServerFd,(struct sockaddr*)&pin,sizeof(pin))==-1){
         printf("Connect Error!\n");
   }
}
//char * Server::sendMessageToLogic(int32_t fd , char* buf)
//{
//    return NULL;
//}
int32_t Server::Gateway_server::hashForUin(const char *username)
{
    int32_t uin = 0;
    int32_t len = strlen(username);
    for(int i=0 ; i<len ; i++){
        uin ^= ((uin << 5) + username[i] + (uin >> 2));
    }
    return uin;
}
int main(int argc,char *argv[])
{
    int32_t  listenfd;
 
    Server::Gateway_server server = Server::Gateway_server();
    
    server.connectLogicServer();//连接logic服务器

    server.init_config("../config.ini/gateway_server.ini");
    listenfd = server.socket_bind(server.IPADDRESS,server.PORT);
    //设置服务器非阻塞响应
    fcntl(listenfd , F_SETFL , fcntl(listenfd,F_GETFD , 0)|O_NONBLOCK);

    listen(listenfd,server.LISTENQ);
    
//    signal(SIGINT , server.sigroutine);
//    signal(SIGQUIT , server.sigroutine);
    server.do_epoll(listenfd);
    server.accountMessage.close_mysql();
    return 0;
}
