#ifndef MAIN_SERV_H
#define MAIN_SERV_H

#include "def1.h"
#include "game.h"
#include "player.h"
#define WAIT_CLI_SIZE 500
#define GAMES_SIZE 10

class main_serv {
//protected:
public:
    game_list *game_queue;
    int ngames;
    event *ev_connclis;
    event *ev_assiclis;
    event_base *main_base;
    event_base *game_base;
    int listen_fd;
    int notify_tell;
    int notify_recv;
    pthread_t tid;
    /** cli queue waiting for game */
    player *clis[WAIT_CLI_SIZE];
    /** index of clis which is already in a game */
    map<uint32_t, player *> *player_map;
    int q_start, q_size;
    pthread_mutex_t cli_fds_lock;

//public:
    main_serv();
    ~main_serv() {
        event_base_free(main_base);
        event_free(ev_connclis);
    };
    void listen_clis();
    void main_loop();
    static void *main_listen(void *arg);
    static void connect_clis(int fd, short what, void *arg);
    static void assign_clis(int fd, short what, void *arg);
};

#endif
