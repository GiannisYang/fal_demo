#ifndef CLI_MAIN_CC
#define CLI_MAIN_CC

#include "def1.h"

int main() {

    sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVPORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr));

    char buf[500];
    while(1){
        recv(sockfd, buf, 500, 0);
        cout << buf<<endl;
        memset(buf, '\0', 500);
        read(STDIN_FILENO, buf, 500);
        send(sockfd, buf, 500, 0);
        memset(buf, '\0', 500);
    }

    close(sockfd);
    return 0;
}

#endif
