#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "error_handling.h"

#define BUF_SIZE 40
#define EPOLL_SIZE 50

int prepareServSock(char *port);

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage %s <port> \n", argv[0]);
        exit(1);
    }
    int serv_sock = prepareServSock(argv[1]);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    struct epoll_event *events = (struct epoll_event *)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    int epollFd = epoll_create(EPOLL_SIZE);
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, serv_sock, &event)==-1){
        error_handling("epoll_ctl error");
    }
    char buf[BUF_SIZE];
    while(1){
        int eventNum = epoll_wait(epollFd, events, EPOLL_SIZE, -1);
        if(eventNum==-1){
            error_handling("epoll_wait() error");
            break;
        }
        printf("event num: %d \n", eventNum);
        for(int i=0; i < eventNum;i++){
            if(events[i].data.fd==serv_sock){
                struct sockaddr_in clnt_addr;
                socklen_t addrLen = sizeof(clnt_addr);
                int clnt_sock = accept(events[i].data.fd, (struct sockaddr *)&clnt_addr, &addrLen);
                if(clnt_sock==-1){
                    printf("accept() error");
                    continue;
                }
                printf("new client: %d \n", clnt_sock);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clnt_sock, &event);
            }else{
                int msgLen = 0;
                if((msgLen=read(events[i].data.fd, buf, BUF_SIZE))>0){
                    write(events[i].data.fd, buf, msgLen);
                }else{
                    epoll_ctl(epollFd, EPOLL_CTL_ADD, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                    printf("closed client: %d \n", events[i].data.fd);
                }
            }
        }
    }
    close(serv_sock);
    close(epollFd);
    return 0;
}

int prepareServSock(char *port){
    int serv_sock;
    struct sockaddr_in serv_addr;
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