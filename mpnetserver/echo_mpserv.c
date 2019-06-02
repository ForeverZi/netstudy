#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "error_handling.h"

#define BUF_SIZE 30

void read_childproc(int sig);

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    
    pid_t pid;
    socklen_t addr_size;
    int str_len, state;
    char buf[BUF_SIZE];
    if(argc != 2){
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock==-1){
        error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
        error_handling("bind() error");
    }
    if(listen(serv_sock, 5)==-1){
        error_handling("listen() error");
    }

    while(1){
        addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_size);
        if(clnt_sock == -1){
            continue;
        }else{
            puts("new client connected...");
        }
        pid=fork();
        if(pid==-1){
            close(clnt_sock);
            continue;
        }
        if(pid==0){
            close(serv_sock);
            while((str_len=read(clnt_sock, buf, BUF_SIZE))!=0){
                write(clnt_sock, buf, str_len);
            }
            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }else{
            close(clnt_sock);
        }
    }
}

void read_childproc(int sig){
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d \n", pid);
}
