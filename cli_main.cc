#ifndef CLI_MAIN_CC
#define CLI_MAIN_CC

#include "def1.h"
#include "tools.h"

int main() {

    sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVPORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr));

    char buf[BUFSIZE], *p;
    int res, maxfdp1;
    fd_set rset;
    FD_ZERO(&rset);
    bool input = false;

    while(1){
        if(input)
            FD_SET(STDIN_FILENO, &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(STDIN_FILENO, sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)) {
            if((res = tcp_recv(sockfd, buf)) != -1) {
//                if(res == -2) continue;

                cout << "player " << res << ":";
                cout << buf << endl;
            } else if (strcmp("yt", buf) == 0) {
                cout << "Your turn:" << endl;
                input = true;
            } else if (strcmp("ka", buf) == 0) {
                /** keep alive packet */
                memset(buf, '\0', BUFSIZE);
                strcpy(buf, "isal");
                add_cmd_head(buf, '$', 0);
                send(sockfd, buf, strlen(buf), 0);
            } else
                cout << buf << endl;
        }
        if(FD_ISSET(STDIN_FILENO, &rset)) {
            memset(buf, '\0', BUFSIZE);
            read(STDIN_FILENO, buf, BUFSIZE);
            /** delete %\n */
            p = buf;
            while(*p != '\n') p++;
            *p = '\0';
            add_cmd_head(buf, '$', 0);
            send(sockfd, buf, strlen(buf), 0);
            input = false;
        }
    }

    close(sockfd);
    return 0;
}

#endif
