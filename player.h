#ifndef PLAYER_H
#define PLAYER_H
#include "def1.h"
#include "game.h"

#define PL_INIT 0x00
#define PL_ONTERN 0x01
#define PL_OFFLINE 0x02
#define PL_EXIT 0x04

class player {
public:
    /** these info need to be renewed when reconnect a cli */
    int conn_fd;
    uint8_t state;
    int wait_res_ka;
    /*******************************************************/

    string ip_addr;
    string cards;
    int cards_num;
    int id;
    event *ev_res;
    uint32_t ip_key;
    game *gm;

    player(int cfd, sockaddr_in addr);
    void init(uint8_t new_id, game *g);
    void send2cli(char *buf) {
        if(state & (PL_OFFLINE | PL_EXIT))
            return;
        if(send(conn_fd, buf, strlen(buf), MSG_DONTWAIT) == -1
            && errno == SIGPIPE
        ) cout << "SIGPIPE is here"<<endl;
    };
    ~player() {
        if(ev_res) event_free(ev_res);
//        close(conn_fd);
    };
    bool is_alive() { return wait_res_ka < ALIVE_SIZE; };
    static void get_res(int fd, short what, void *arg);
};

#endif
