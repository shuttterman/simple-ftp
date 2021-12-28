#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799

int main(void) {
    int s_sock, c_sock, status;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    char buf[BUFFSIZE] = {0};
    char hello[] = "Hello~I am Server!\n";
    pid_t pid;

    s_sock = socket(AF_INET, SOCK_STREAM, 0); // socket을 열어줍니다.

    int option = 1; // already in use 를 위한 옵션
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // 소켓에 그 옵션을 적용하였습니다

    bzero(&server_addr, sizeof(server_addr)); // server_addr 을 비워줍니다.

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT); // server의 포트를 지정을 해주고
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); //server의 ip를 지정을 해줍니다.

    if(bind(s_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) { // 지정한것들을 server socket에 bind 해줍니다. 
        perror("Can't bind a socket");
        exit(1);
    }

    listen(s_sock, 1); // socket이 client를 기다리고 있습니다.
    c_addr_size = sizeof(struct sockaddr); // accept를 하기 위해 sockaddr의 크기를 c_addr_size에 넣어줍니다.

    for(int i=0 ; i<3 ; i++) {
        printf("[S] Waiting for a client...#%02d\n", i);
        
        if((c_sock = accept(s_sock, (struct sockaddr *) &client_addr, &c_addr_size)) == -1) { //accpet가 에러가 났을 경우 예외 처리
            perror("[S] Can't accept socket");
            exit(1);
        }
        else { // socket이 accept 될때
            printf("[S] Connected : Client IP addr=%s port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            pid = fork(); // fork를 해줍니다.
        }

        if(pid == 0) { // 자식 process

            printf("[SS] Service: I am your child! pid=%d\n", getpid());

            if(send(c_sock, hello, sizeof(hello)+1, 0) == -1) { // client에게 메시지를 보냅니다.
                perror("[SS] Can't send message");
                exit(1);
            }

            printf("[SS] I said hello to client\n");

            if(recv(c_sock, buf, BUFFSIZE, 0) == -1) { // client에게 온 메시지를 receive 합니다.
                perror("[SS] Can't receive message");
                exit(1);
            }
            printf("[SS] Client says : %s\n", buf);
	close(c_sock); // 소켓을 닫아줍니다.
            break;
        }
    }
    
    close(s_sock); // 소켓을 닫아줍니다.
    if(pid > 0) { // 부모 process 
        for(int i=0 ; i<3 ; i++) { // 자식들에게 오는 종료상태값을 wait해서 받고 출력합니다.
            wait(&status); // 종료상태값을 stauts에 저장합니다.
            printf("[S] Child #%02d is finished with %d\n", i, status);
        }
    }
    return 0;
}