#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define N 128
int main(int argc,char const* argv[])
{
    if(argc<3)
    {
        fprintf(stderr,"you need %s ip port",argv[0]);
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
    int addrlen=sizeof(severaddr);
    if(connect(sockfd,(struct sockaddr*)&severaddr,addrlen)==-1)
    {
        perror("failed to make a connect!");
        exit(1);
    }
    char buf[N]="";
    int m=3;
    while(m--)
    {
        fgets(buf,N,stdin);
        buf[strlen(buf)-1]='\0';
        if(send(sockfd,buf,N,0)==-1)
        {
            perror("failed  to send anything!");
            exit(1);
        }
        char rec[N]="";
        if(recv(sockfd,rec,N,0)==-1)
        {
            perror("failed to recv anything!");
            exit(1);
        }
        printf("%s\n",rec);
    }
    
    
    
    close(sockfd);
    return 0;
}