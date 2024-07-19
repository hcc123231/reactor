#include<poll.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#define N 10
#define M 1024
int conn=0;
int main(int argc,char* const argv[])
{
    if(argc<3){
        printf("[ address ] [ port ]");
        exit(1);
    }
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1)
    {
        perror("failed to create socket!");
        exit(1);
    }
    struct sockaddr_in severaddr;
    severaddr.sin_addr.s_addr=inet_addr(argv[1]);
    severaddr.sin_family=AF_INET;
    severaddr.sin_port=htons(atoi(argv[2]));
    if(bind(sockfd,(struct sockaddr*)&severaddr,sizeof(severaddr))==-1)
    {
        perror("failed to bind!");
        exit(1);
    }
    if(listen(sockfd,N)==-1)
    {
        perror("failed to listen!");
        exit(1);
    }
    struct pollfd myfds[1024];//这个1024只是随意指定的数组大小，实际无限制
    for(int i=0;i<1024;i++)
    {
        myfds[i].fd=-1;//由于没有文件描述符的值为负，所以初始化用负数是可行的，并且用负数来标识未被使用
        myfds[i].events=POLLIN;
    }
    myfds[0].fd=sockfd;
    int maxfd=sockfd;
    while(1)
    {
        if(poll(myfds,maxfd+1,-1)==-1)
        {
            perror("failed to create poll!");
            exit(1);
        }
        if(myfds[0].revents&POLLIN)
        {
            struct sockaddr_in clientaddr;
            int clilen=sizeof(clientaddr);
            int connfd=accept(sockfd,(struct sockaddr*)&clientaddr,&clilen);
            if(connfd==-1)
            {
                perror("failed to connect!");
                exit(1);
            }
            conn++;
            if(conn%1000==0){
                printf("connections:%d\n",conn);
            }
            for(int i=0;i<=maxfd;i++)
            {
                if(myfds[i].fd==-1)
                {
                    myfds[i].fd=connfd;
                    break;
                }
            }
            maxfd=maxfd>connfd?maxfd:connfd;
        }
        for(int i=1;i<=maxfd;i++)
        {
            if(myfds[i].revents&POLLIN)
            {
                char buf[M];
                int re=recv(myfds[i].fd,&buf,M,0);
                if(re==-1)
                {
                    perror("failed to recv!");
                    exit(1);
                }
                if(re==0)
                {
                    printf("已断开连接\n");
                    myfds[i].fd=-1;
                    close(myfds[i].fd);
                    continue;
                }
                //printf("%s\n",buf);
            }
        }
    }
    close(sockfd);
    return 0;
}