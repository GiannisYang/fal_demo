#ifndef PLAYER_H
#define PLAYER_H
#include "def1.h"
#include "game.h"

#define PL_INIT 0x00
#define PL_ONTERN 0x01
#define PL_HINTED 0x02

class player {
protected:
    string ip_addr;
    uint8_t id;
    game* const gm;
    event *ev_hint;
    event *ev_res;

public:
    uint8_t state;
    const int conn_fd;
    player(int cfd, sockaddr_in addr, uint8_t id, game *g);
    ~player() {
        event_del(ev_hint);
        event_del(ev_res);
        event_free(ev_hint);
        event_free(ev_res);
    };
//    int send2cli(const char *str);
    static void hint(int fd, short what, void *arg);
    static void get_res(int fd, short what, void *arg);
};

#endif
