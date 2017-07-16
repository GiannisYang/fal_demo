#ifndef GAME_H
#define GAME_H

#include "def1.h"
#include "player.h"

#define GAME_INIT 0x00
#define GAME_WAITING 0x01
#define GAME_RUNNING 0x10

class game {
protected:
    uint32_t id;
    uint8_t state, pl_num, turn, landlord;
    player *pl[MAX_PSIZE];
    uint32_t listen_fd;
    event *ev_wait;
    event *ev_pl_timer;

public:
    char cmd[CMDSIZE];
    event_base* const base;

    game(event_base *b);
    void wait_pl();
    int send2cli(int loc, const string &str);
    /** get commands from a player
      * this info will be sent to next player */
    void get_res(const char *str, int id) {
        memset(cmd, 0, CMDSIZE);
        sprintf(cmd, "%d#%s", id, str);
        event_del(ev_pl_timer);
        schedule(-1, EV_TIMEOUT, this);
    };
    static void connect_pl(int fd, short what, void *arg);
    static void schedule(int fd, short what, void *arg);
    static uint8_t det_landlord() {
        srand(unsigned(time(0)));
        return (uint8_t)rand() % MAX_PSIZE;
    };
};

#endif
