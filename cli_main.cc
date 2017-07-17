#ifndef CLI_MAIN_CC
#define CLI_MAIN_CC

#include "def1.h"
#include "tools.h"

int main() {

//uint8_t ttt = 0;
//ttt--;
//cout << ttt<<endl;
//ttt = (ttt - 1) % 1;
//cout << ttt<<endl;
//return 0;

    sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVPORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr));

    char buf[BUFSIZE], *p;
    int res;

    while(1){
        if((res = tcp_recv(sockfd, buf)) != -1) {
            cout << "player " << res << ":";
            cout << buf << endl;
            continue;
        } else if (strcmp("yt", buf) == 0) {
            cout << buf << endl;
            memset(buf, '\0', BUFSIZE);
            read(STDIN_FILENO, buf, BUFSIZE);
            /** delete %\n */
            p = buf;
            while(*p != '\n') p++;
            *p = '\0';
            add_cmd_head(buf, '$', 0);
            send(sockfd, buf, strlen(buf), 0);
            continue;
        } else
            cout << buf << endl;
    }

    close(sockfd);
    return 0;
}

#endif
