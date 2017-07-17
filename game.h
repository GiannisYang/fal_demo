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
    uint8_t state;
    int pl_num, turn, landlord;
    player *pl[MAX_PSIZE];
    uint32_t listen_fd;
    event *ev_wait;
    event *ev_pl_timer;

public:
    event_base* const base;

    game(event_base *b);
    void wait_pl();
    void broadcast2cli(char *buf);
    /** get commands from a player, this information
      * will be recorded by server and sent to players */
    void get_res(const char *str, int id);
    static void connect_pl(int fd, short what, void *arg);
    static void schedule(int fd, short what, void *arg);
    static uint8_t det_landlord() {
        srand(unsigned(time(0)));
        return (uint8_t)rand() % MAX_PSIZE;
    };
};

#endif
