#ifndef PLAYER_H
#define PLAYER_H
#include "def1.h"
#include "game.h"

#define PL_INIT 0x00
#define PL_ONTERN 0x01

class player {
protected:
    string ip_addr;
    game *gm;
    event *ev_res;
    const int conn_fd;

public:
    uint8_t id;
    uint8_t state;
    int wait_res_ka;

    player(int cfd, sockaddr_in addr);
    void init(uint8_t new_id, game *g);
    void send2cli(char *buf) {
        send(conn_fd, buf, strlen(buf), MSG_DONTWAIT);
    };
    ~player() {
        event_del(ev_res);
        event_free(ev_res);
        close(conn_fd);
    };
    bool is_alive() { return wait_res_ka < ALIVE_SIZE; };
    static void get_res(int fd, short what, void *arg);
};

#endif
