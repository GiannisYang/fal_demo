#ifndef CLI_MAIN_CC
#define CLI_MAIN_CC

#include "def1.h"
#include "tools.h"

#define OUTPUT_LINES() for(int i = 0; i < 20; i++) cout << "-"; \
        cout << endl;

string cards;

void print_cards(string cds) {
    if("input nothing" == cds) {
        cout << cds << endl;
        return;
    }
    for(int i = 0; i < cds.length(); i++) {
        if(cds[i] == ' ') continue;

        if(cds[i] == '0') cout << 10;
        else if(cds[i] == 'B') cout << "BLACK_JOKER";
        else if(cds[i] == 'R') cout << "RED_JOKER";
        else cout << cds[i];
        cout << " ";
    }
    cout << endl;
}

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
            if((res = tcp_recv(sockfd, buf)) == -2) {
                cout << "ERR: serv is down"<<endl;
                break;
            } else if (strcmp("ka", buf) == 0) {
                /** keep alive packet */
                memset(buf, '\0', BUFSIZE);
                strcpy(buf, "isal");
                add_cmd_head(buf, '$', 0);
                send(sockfd, buf, strlen(buf), 0);
            } else {
                cout << endl;
                OUTPUT_LINES();
                if (strcmp("yt", buf) == 0) {
                    cout << "Your cards: ";
                    print_cards(cards);
                    cout << "Your turn:" << endl;
                    input = true;
                } else {
                    /** game starts */
                    if (!memcmp("st", buf, 2)) {
                        cards = "";
                        for(int i = 3; buf[i] != '\0'; i++) {
                            cards += buf[i];
                        }
                        cout << "Game begins.." << endl
                        << "Your ID is: " << C2I(buf[2]) << endl
                        << "Your cards: ";
                        print_cards(cards);
                    } else if(res != -1) {
                    /** turn of someone */
                        cout << "player " << res << ": ";
                        print_cards((string)buf);
                    } else
                        cout << buf << endl;

                    OUTPUT_LINES();
                    cout << endl;
                }

            }
        }
        if(FD_ISSET(STDIN_FILENO, &rset)) {
            OUTPUT_LINES();
            cout << endl;
            char tmp[BUFSIZE];
            memset(tmp, 0, BUFSIZE);
            memset(buf, 0, BUFSIZE);
            read(STDIN_FILENO, tmp, BUFSIZE);
            p = tmp;
            for(int i = 0; *p != '\n'; p++) {
                if(*p == ' ') continue;
                else if(*p == '1') {
                    buf[i] = '0';
                    p++;
                } else buf[i] = *p;

            /** remove cards been sent in this turn */
                for(int j = 0; j < cards.length(); j++)
                    if(cards[j] == buf[i]) {
                        cards[j] = ' ';
                        break;
                    }

                i++;
            }
            add_cmd_head(buf, '$', 0);
            send(sockfd, buf, strlen(buf), 0);
            input = false;
        }
    }

    close(sockfd);
    return 0;
}

#endif
