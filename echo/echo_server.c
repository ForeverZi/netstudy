#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[]){
    int sock, clnt_sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_sz;

    if(argc != 2){
        printf("Usage %s <port> \n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1){
        error_handling("socket() error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
        error_handling("bind() error");
    }

    if(listen(sock, 5)==-1){
        error_handling("listen() error");
    }
    while(1){
        clnt_addr_sz = sizeof(serv_addr);
        clnt_sock = accept(sock, (struct sockaddr*)&serv_addr, &clnt_addr_sz);
        if(clnt_sock==-1){
            error_handling("accept() error");
        }
        while((str_len=read(clnt_sock, message, BUF_SIZE))!=0){
            write(clnt_sock, message, str_len);
        }
        close(clnt_sock);
    }
    close(sock);
    exit(0);
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}