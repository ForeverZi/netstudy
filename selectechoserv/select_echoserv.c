#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include "error_handling.h"

#define BUF_SIZE 30

int selectPoll(fd_set *toRead, struct timeval timeout, int *fd_max, char *buf, int serv_sock);
int prepareServSock(char *port);
void echoClient(int clnt_sock, fd_set *readSet, char *buf);
int handleAccept(int serv_sock, fd_set *readSet);


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }
    int serv_sock = prepareServSock(argv[1]);
    fd_set toRead;
    char buf[BUF_SIZE];
    FD_ZERO(&toRead);
    FD_SET(serv_sock, &toRead);
    struct timeval timeout;
    timeout.tv_sec= 5;
    timeout.tv_usec = 0;
    int fd_num;
    int fd_max = serv_sock;
    while(selectPoll(&toRead, timeout, &fd_max, buf, serv_sock)==0){}
    close(serv_sock);
}

int selectPoll(fd_set *toRead, struct timeval timeout, int *fd_max, char *buf, int serv_sock){
    int fd_num;
    fd_set temp = *toRead;
    if(((fd_num=select(*fd_max+1, &temp, 0, 0, &timeout))==-1)){
        printf("select error \n");
        return -1;
    }
    if(fd_num==0){
        printf("no fd ready to read \n");
        return 0;
    }
    for(int i=0;i<*fd_max+1;i++){
        if(FD_ISSET(i, &temp)){
            printf("ready read: %d \n", i);
            if(i == serv_sock){
                int clnt_sock = handleAccept(serv_sock, toRead);
                if(clnt_sock>*fd_max){
                    *fd_max = clnt_sock;
                }
                FD_SET(clnt_sock, toRead);
            }else{
                echoClient(i, toRead, buf);
            }
        }else{
            printf("fd not ready:%d \n", i);
        }        
    }
    return 0;
}

int prepareServSock(char *port){
    int serv_sock;
    struct sockaddr_in serv_addr;
    fd_set toRead, temp;
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock==-1){
        error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
        error_handling("bind() error");
    }
    if(listen(serv_sock, 5)==-1){
        error_handling("listen() error");
    }
    return serv_sock;
}

void echoClient(int clnt_sock, fd_set *readSet, char *buf){
    int str_len=read(clnt_sock, buf, BUF_SIZE);
    if(str_len==0){
        FD_CLR(clnt_sock, readSet);
        close(clnt_sock);
        printf("client disconnect: %d \n", clnt_sock);
    }else{
        write(clnt_sock, buf, str_len);
    }
}

int handleAccept(int serv_sock, fd_set *readSet){
    int clnt_sock;
    struct sockaddr_in clnt_addr;
    socklen_t addr_size;
    addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
    FD_SET(clnt_sock, readSet);
    printf("new client connected: %d \n", clnt_sock);
    return clnt_sock;
}
