#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFSIZE 4096
#define SERVERPORT 7799
#define MAX_THREADS (10)

typedef struct _arg { // 쓰레드에 넘겨줄 argument 구조체
    int c_sock; // fd 저장
    int my_number; // 쓰레드 번호 저장
} arg_t;

pthread_t tid[MAX_THREADS]; 
int s_sock; // 쓰레드들에서도 사용할 것이기 때문에 전역변수로 했습니다.

void* filetp(void* arg) {
    char buf[BUFFSIZE];
    FILE *fp;
    bool is_file_exist = true;
    int c_sock = ((arg_t*)arg)->c_sock, my_num = ((arg_t*)arg)->my_number, len, *ret;

    memset(buf, 0, BUFFSIZE); // buffer를 비워주고

    if(recv(c_sock, buf, BUFFSIZE, 0) == -1) { // client에게 파일 이름을 받아옵니다.
        perror("[T] Can't receive a filename");
        exit(1);
    }

    if(access(buf, F_OK) == -1) is_file_exist = false; // 파일 존재 여부를 판단합니다.

    if(send(c_sock, &is_file_exist, sizeof(bool), 0) == -1) { // 파일 존재 여부를 client에게 알려줍니다.
        perror("[T] Can't send file existence");
        exit(1);
    }

    if(is_file_exist) { // 파일이 있을 경우
        if((fp=fopen(buf, "r")) == NULL) { // 파일을 엽니다.
            perror("[T] Can't open file");
            exit(1);
        }

        for(int i=0 ; i<=my_num ; i++)printf("\t"); // 동시에 처리됨 확인을 편하게 보기 위한 들여쓰기 입니다.
        printf("[%s]\n", buf);
        memset(buf, 0, BUFFSIZE); // buffer를 비워줍니다.
        while((len = fread(buf, 1, BUFFSIZE, fp)) > 0) { // 파일을 client에게 보냅니다.
            if(send(c_sock, buf, len, 0) == -1) { // 읽은 만큼 보내는 것을 반복합니다.
                perror("[T] Can't send a file");
                exit(1);
            }
        }
        for(int i=0 ; i<=my_num ; i++)printf("\t"); // 동시에 처리됨 확인을 편하게 보기 위한 들여쓰기 입니다.
        printf("End of sending\n");

        fclose(fp); // 파일을 닫습니다.
    }
    else printf("[T] Requested file[%s] is not exist in current directory\n", buf); // 파일이 존재하지 않을 경우

    close(c_sock); //바인딩한 소켓을 닫아줍니다.

    ret = (int *)malloc(sizeof(int)); // return value를 담기위해 동적할당을 했습니다.
    *ret = my_num; // 값 대입
    pthread_exit(ret); // return value입니다.
}

int main(void) {
    int i, *status;
    struct sockaddr_in server_addr, client_addr;
    socklen_t c_addr_size;
    arg_t args[MAX_THREADS] = {0}; 

    s_sock = socket(AF_INET, SOCK_STREAM, 0); // 소켓을 생성해줍니다.

    int option = 1;
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // TIME-WAIT 상태에 있는 소켓에 ip와 port를 새로 시작하는 소켓에 할당하는 옵션입니다.

    memset(&server_addr, 0, sizeof(server_addr)); //server_addr를 비워줍니다.
    
    server_addr.sin_family = AF_INET; // server_addr 구조체에 적절한 값들을 넣어줍니다.
    server_addr.sin_port = htons(7799); //server port 지정
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); //server IP 지정

    if(bind(s_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) { // 소켓을 bind 해줍니다.
        perror("[S] Can't bind a socket");
        exit(1);
    }

    listen(s_sock, 1); // s_sock이 client를 기다립니다.

    c_addr_size = sizeof(struct sockaddr); 

    for(i=0 ; i<MAX_THREADS ; i++) { // 멀티쓰레드를 위한 반복문
        args[i].my_number = i; // 구조체에 쓰레드번호를 저장해줍니다.
        if((args[i].c_sock = accept(s_sock, (struct sockaddr *) &client_addr, &c_addr_size)) == -1) { // accept 함수를 실행하고 예외처리를 해줍니다.
            perror("[S] Can't accept");
            exit(1);
        }
        else { 
            if(pthread_create(&tid[i], NULL, filetp, &args[i]) != 0) { //쓰레드를 만듭니다.
                perror("[S] Fail to create thread");
                goto exit;
            }
        }
    }

exit:
    for(i=0 ; i<MAX_THREADS ; i++){
        pthread_join(tid[i], (void **) &status); // 쓰레드들이 종료될때까지 기다립니다.
        printf("[Thread #%d] i'm finish\n", *status); // 몇번 쓰레드가 종료 되었는지 출력합니다.
    }

    close(s_sock); // 소켓을 닫습니다.
    return 0;
}
