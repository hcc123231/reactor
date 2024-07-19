#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/epoll.h>
#define PORT_COUNT 20
#define PORT 2000
#define ADDR "192.168.117.128"
#define CONNECTIONS 1000000
#define EVENT_COUNT 1024
#define BUF_SIZE 1024
int connections;
typedef void (*func)(int fd);
struct conn
{
    int fd;
    func recvf_cb;
    func send_cb;
    char rbuf[BUF_SIZE];
    char wbuf[BUF_SIZE];
};
struct conn conn_list[CONNECTIONS];
int epfd;
void accept_callback(int fd);
void recv_callback(int fd);
void send_callback(int fd);
void registe(int fd);
void recv_callback(int fd)
{
    int ret=recv(fd,conn_list[fd].rbuf,BUF_SIZE,0);
    if(ret==-1)
    {
        perror("failed to recv");
        exit(1);
    }
    else if(ret==0)
    {
        printf("fd:%d had disconnect!\n",fd);
        conn_list[fd].fd=0;
        memset(conn_list[fd].rbuf,0,BUF_SIZE);
        memset(conn_list[fd].wbuf,0,BUF_SIZE);
        conn_list[fd].recvf_cb=NULL;
        conn_list[fd].send_cb=NULL;
        epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
        close(fd);
        return;
    }
    memset(conn_list[fd].rbuf,0,BUF_SIZE);
    struct epoll_event ev;
    ev.data.fd=fd;
    ev.events=EPOLLOUT;
    epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
    return;
}
void accept_callback(int fd)
{
    struct sockaddr_in cliaddr;
    int len=sizeof(cliaddr);
    int acfd=accept(fd,(struct sockaddr*)&cliaddr,&len);
    if(acfd==-1)
    {
        perror("failed to accept");
        exit(1);
    }
    connections++;
    if(connections%10000==0)
    {
        printf("connections:%d\n",connections);
    }
    conn_list[acfd].fd=acfd;
    conn_list[acfd].recvf_cb=recv_callback;
    conn_list[acfd].send_cb=send_callback;
    registe(acfd);
    return;
}
void send_callback(int fd)
{
    strcpy(conn_list[fd].wbuf,"response\n");
    if(send(fd,conn_list[fd].wbuf,strlen(conn_list[fd].wbuf),0)==-1)
    {
        perror("failed to send");
        exit(1);
    }
    struct epoll_event ev;
    ev.data.fd=fd;
    ev.events=EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
    memset(conn_list[fd].wbuf,0,BUF_SIZE);
    return;
}
void registe(int fd)
{
    conn_list[fd].fd=fd;
    conn_list[fd].recvf_cb=recv_callback;
    conn_list[fd].send_cb=send_callback;
    struct epoll_event ev;
    ev.data.fd=fd;
    ev.events=EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
}
void init_server(int port)
{
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1)
    {
        perror("failed to create socket");
        exit(1);
    }
    struct sockaddr_in serveraddr;
    serveraddr.sin_addr.s_addr=inet_addr(ADDR);
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    if(bind(lfd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr_in))==-1)
    {
        perror("failed to bind");
        exit(1);
    }
    if(listen(lfd,10)==-1)
    {
        perror("failed to listen!");
        exit(1);
    }
    conn_list[lfd].fd=lfd;
    conn_list[lfd].recvf_cb=accept_callback;
    conn_list[lfd].send_cb=send_callback;
    struct epoll_event ev;
    ev.data.fd=lfd;
    ev.events=EPOLLIN;
    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    
}
int main()
{
    epfd=epoll_create(1);
    for(int i=0;i<PORT_COUNT;i++)
    {
        init_server(i+PORT);
    }
    
    struct epoll_event evs[EVENT_COUNT];
    while(1)
    {
        int num=epoll_wait(epfd,evs,EVENT_COUNT,-1);
        for(int i=0;i<num;i++)
        {
            if(evs[i].events&EPOLLIN)
            {
                conn_list[evs[i].data.fd].recvf_cb(evs[i].data.fd);
            }
            else if(evs[i].events&EPOLLOUT)
            {
                conn_list[evs[i].data.fd].send_cb(evs[i].data.fd);
            }
        }
    }
    

    return 0;
}