#ifndef GAME_H
#define GAME_H

#include "def1.h"
#include "player.h"

#define GAME_INIT 0x00
#define GAME_WAITING 0x01
#define GAME_RUNNING 0x10

class game {
protected:
    int pl_num, turn, landlord;
    player *pl[MAX_PSIZE];
    event *ev_pl_timer;
    event *ev_keep_alive;

public:
    uint8_t state;
    TAILQ_ENTRY(game) (game_next);
    event_base* const base;

    game(event_base *b);
    ~game();
    void connect_pl(player *new_p);
    void broadcast2cli(char *buf);
    void cli_exit(player *p);
    /** get commands from a player, this information
      * will be recorded by server and sent to players */
    void get_res(const char *str, int id);
    /** timeout func of sending keep alive packets */
    static void send_keep_alive(int fd, short what, void *arg);
    static void schedule(int fd, short what, void *arg);
    static uint8_t det_landlord() {
        srand(unsigned(time(0)));
        return (uint8_t)rand() % MAX_PSIZE;
    };
};

TAILQ_HEAD(game_list, game);

#endif
