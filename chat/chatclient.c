#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "error_handling.h"

#define BUF_SIZE 128
#define NAME_SIZE 20

void *recv_msg(void *arg);
void *send_msg(void *arg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Usage : %s <ip> <port> <name> \n", argv[0]);
        exit(1);
    }
    sprintf(name, "[%s]", argv[3]);
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    socklen_t addr_len = sizeof(serv_addr);
    if(connect(sock, (struct sockaddr*)&serv_addr, addr_len)==-1){
        error_handling("connect() error");
    }
    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_msg, &sock);
    pthread_create(&recv_thread, NULL, recv_msg, &sock);
    void *code;
    pthread_join(send_thread, &code);
    pthread_join(recv_thread, &code);
    close(sock);
    return 0;
}

void *recv_msg(void *arg){
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1){
        int str_len = read(sock, name_msg, sizeof(name_msg));
        if(str_len==-1){
            return (void*)-1;
        }
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void *send_msg(void *arg){
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1){
        fgets(msg, BUF_SIZE, stdin);
        if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")){
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}