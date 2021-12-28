#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define BUFFSIZE 4096

int main(int argc, char** argv) {
    int c_sock, len;
    struct sockaddr_in server_addr;
    char buf[BUFFSIZE];
    FILE *fp;
    bool is_file_exist;

    if(argc != 4) { // 명령행 인자를 적절히 넣었는지 확인합니다.
        printf("<Usage : ./client IP_ADDRESS PORT FILENAME>\n");
        exit(1);
    }

    memset(buf, 0, BUFFSIZE); //buffer를 초기화해줍니다.

    c_sock = socket(AF_INET, SOCK_STREAM, 0); // 소켓을 생성해줍니다.
    memset(&server_addr, 0, sizeof(server_addr)); //server_addr을 비워줍니다.

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); // 명령행 인자로 받은 포트번호를 넣어줍니다.
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // 명령행 인자로 받은 ip를 넣어줍니다.


    if(connect(c_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) { // server에게 connect를 합니다.
        perror("Can't connect a socket");
        exit(1);
    }

    if(send(c_sock, argv[3], strlen(argv[3]), 0) == -1) { // 파일의 이름을 서버에게 보내줍니다.
        perror("Can't send a filename");
        exit(1);
    }

    if(recv(c_sock, &is_file_exist, sizeof(bool), 0) == -1) { //서버에서의 파일 존재 여부를 받습니다.
        perror("Can't receive file existence");
        exit(1);
    }

    strcpy(buf, "downloaded_");
    strcat(buf, argv[3]);
    
    if(is_file_exist) { // 파일이 존재 할 경우
        if((fp = fopen(buf, "w")) == NULL) { // 복사를 해줄 파일을 열어줍니다.
            perror("Can't open file");
            exit(1);
        }

        printf("receiving request file[%s] from server\n", argv[3]); 
        do { // 열어준 파일 안에 받아온 것들을 적어줍니다.
            memset(buf, 0, BUFFSIZE);
            if((len=recv(c_sock, buf, BUFFSIZE, 0)) == -1) {  // buf에 받아온 것들을 넣어주고
                perror("Can't receive a file");
                exit(1);
            }
            fwrite(buf, 1, len, fp); // buf에 저장한 것들을 파일에 적습니다
        } while(len); // 끝이 났을 경우에는 len이 0이기 때문에 더이상 반복하지 않습니다.
        printf("end of receive(filename : downloaded_%s)\n", argv[3]);

        fclose(fp); // 파일을 닫아줍니다.

    }
    else printf("request file[%s] does not exist on the server\n", argv[3]); // 파일이 존재하지 않을 경우
    close(c_sock); // 소켓을 정리해줍니다.

    return 0;
}