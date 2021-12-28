#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799

int main(void) {
    int c_sock;
    struct sockaddr_in server_addr, clinet_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};
    char hello[] = "Hello~I am Client!\n";

    c_sock = socket(AF_INET, SOCK_STREAM, 0); // 소켓을 열어줍니다.

    bzero(&server_addr, sizeof(server_addr)); //server_addr 변수를 비워줍니다.

    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVERPORT); // server의 port지정
    server_addr.sin_addr.s_addr = inet_addr("10.0.0.73"); //server의 ip지정

    printf("[C] Connecting...\n");

    if(connect(c_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){ // server에 conncet를 시도합니다.
        perror("[C] Can't connect a socket");
        exit(1);
    }

    printf("[C] Connected!\n");

    if(recv(c_sock, buf, BUFFSIZE, 0) == -1) { // server에서 온 message를 받습니다.
        perror("[C] Can't receive message");
        exit(1);
    }

    printf("[C] Server says: %s\n", buf);

    if(send(c_sock, hello, sizeof(hello)+1, 0) == -1) { //server에게 message 를 보냅니다.
        perror("[C] Can't send message");
        exit(1);
    }

    printf("[C] I said hi to server!!\n");

    /* // p-client_sleep 에 사용하는 코드
    printf("I am going to sleep\n"); 
    sleep(10);
    */



    close(c_sock); // 소켓을 닫아줍니다.

    return 0;
}