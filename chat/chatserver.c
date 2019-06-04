#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "error_handling.h"

#define BUF_SIZE 128
#define MAX_CLNT 256

void *handle_clnt(void *args);
void send_msg(char *msg, int len);
int prepareServSock(char *port);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }
    pthread_mutex_init(&mutx, NULL);

    int serv_sock = prepareServSock(argv[1]);
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_sz;
    while(1){
        clnt_addr_sz = sizeof(clnt_addr);
        int clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_sz);
        if(clnt_sock==-1){
            error_handling("accept() error");
        }
        printf("hello client: %d \n", clnt_sock);
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);
        pthread_t handleThread;
        pthread_create(&handleThread, NULL, handle_clnt, &clnt_socks[clnt_cnt-1]);
        pthread_detach(handleThread);
        printf("Connected client IP: %s \n", inet_ntoa(clnt_addr.sin_addr));
    }
    close(serv_sock);
    return 0;
}

void *handle_clnt(void *args){
    int clnt_sock = *((int*)args);
    int str_len = 0;
    char msg[BUF_SIZE];
    while((str_len=read(clnt_sock, msg, BUF_SIZE))!=0){
        send_msg(msg, str_len);
    }
    pthread_mutex_lock(&mutx);
    for(int i=0; i < clnt_cnt; i++){
        if(clnt_socks[i]==clnt_sock){
            clnt_socks[i] = clnt_socks[clnt_cnt-1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    printf("client %d offline\n", clnt_sock);
    return NULL;
}

void send_msg(char *msg, int len){
    pthread_mutex_lock(&mutx);
    for(int i=0; i < clnt_cnt; i++){
        write(clnt_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx);
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