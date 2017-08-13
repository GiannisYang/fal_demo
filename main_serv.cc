#ifndef MAIN_SERV_CC
#define MAIN_SERV_CC

#include "main_serv.h"
#include "tools.h"

/** pthread 1: listen to the port and connect clis */
void* main_serv::main_listen(void *arg) {
    main_serv *ms = (main_serv *)arg;
    ms->listen_clis();
    event_base_dispatch(ms->main_base);
    return NULL;
}

void main_serv::main_loop() {
    pthread_create(&tid, NULL, main_listen, this);

    signal(SIGPIPE, SIG_IGN);
    /** pthread 2: run some games in one event_base */
    if (!ev_assiclis)
        ev_assiclis = event_new(game_base, notify_recv,
            EV_READ | EV_PERSIST, assign_clis, this);
    event_add(ev_assiclis, NULL);
    event_base_dispatch(game_base);
}

main_serv::main_serv(): ev_connclis(NULL), ev_assiclis(NULL),
        q_start(-1), q_size(0), ngames(0), listen_fd(-1) {
    main_base = event_base_new();
    game_base = event_base_new();
    game_queue = new game_list;
    TAILQ_INIT(game_queue);
    for(int i = 0; i < WAIT_CLI_SIZE; i++)
        clis[i] = NULL;
    pthread_mutex_init(&cli_fds_lock, NULL);

    player_map = new map<uint32_t, player *>;

    int fds[2];
    if (pipe(fds)) {
        cout << "Can't create notify pipe" << endl;
        exit(1);
    }
    notify_tell = fds[1];
    notify_recv = fds[0];

    /** set pipe to non-block */
    fcntl(notify_tell, F_SETFL, O_NONBLOCK
        | fcntl(notify_tell, F_GETFL, 0));
    fcntl(notify_recv, F_SETFL, O_NONBLOCK
        | fcntl(notify_tell, F_GETFL, 0));
};

void main_serv::listen_clis() {
    sockaddr_in serv_addr;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVPORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listen_fd, (sockaddr *) &serv_addr, sizeof(serv_addr));
    listen(listen_fd, LISTENQ);

    if (!ev_connclis)
        ev_connclis = event_new(main_base, listen_fd,
            EV_READ | EV_PERSIST, connect_clis, this);
    event_add(ev_connclis, NULL);
}

void main_serv::connect_clis(int fd, short what, void *arg) {
    main_serv *ms = (main_serv *)arg;
    if(ms->ngames == GAMES_SIZE
        || ms->q_size == WAIT_CLI_SIZE
    ) return;

    sockaddr_in cli_addr;
    socklen_t len;
    int connfd;
    if((connfd = accept(ms->listen_fd, (sockaddr *) &cli_addr, &len))
        == -1) {
        cout << "accept == -1" << endl;
        return;
    }

    /** create player obj and insert into waiting queue */
    ++ms->q_size;
    int new_loc = (ms->q_start + ms->q_size) % WAIT_CLI_SIZE;
    pthread_mutex_lock(&ms->cli_fds_lock);

    ms->clis[new_loc] = new player(connfd, cli_addr);
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    sprintf(buf, "Connection successful.");
    add_cmd_head(buf, '$', 0);
    ms->clis[new_loc]->send2cli(buf);

    pthread_mutex_unlock(&ms->cli_fds_lock);
    /** tell the game_base to stop epoll_wait */
    /** have to use "write". why ??? */
    write(ms->notify_tell, "a", 1);
}

void main_serv::assign_clis(int fd, short what, void *arg) {
    main_serv *ms = (main_serv *)arg;
    pthread_mutex_lock(&ms->cli_fds_lock);

    player *new_pl = ms->clis[(ms->q_start + 1) % WAIT_CLI_SIZE];
    char tmp;
    ms->notify_recv;
    read(ms->notify_recv, &tmp, 1);

    game *g = NULL;
    pair<map<uint32_t, player *>::iterator, bool> res
        = ms->player_map->insert(
        pair<uint32_t, player *>(new_pl->ip_key, new_pl));
    /** insert this cli into the map
      * if a cli exit before join a game, there is
      * no need to insert it to this map */
    if(res.second
            /** just for debug .. */
        || new_pl->ip_addr == "127.0.0.1"
    ) {
        /** fifo find a game waiting for player(s) */
        TAILQ_FOREACH(g, ms->game_queue, game_next) {
            if(g->state & GAME_WAITING)
                break;
        }
    } else {
        g = (res.first)->second->gm;
        g->re_connect_pl(new_pl);
        delete new_pl;
        pthread_mutex_unlock(&ms->cli_fds_lock);
        return;
    }

    /** if there is no game waiting, create one;
      * if there are too many active games, cli fd will
      * be waiting in the cli_fds queue */
    if(!g) {
        if(ms->ngames == GAMES_SIZE) {
            pthread_mutex_unlock(&ms->cli_fds_lock);
            return;
        }
        g = new game(ms->game_base);
        TAILQ_INSERT_TAIL(ms->game_queue, g, game_next);
        ++ms->ngames;
    }
    ms->q_start = (ms->q_start + 1) % WAIT_CLI_SIZE;
    --ms->q_size;
    g->connect_pl(ms->clis[ms->q_start]);
    ms->clis[ms->q_start] = NULL;
    pthread_mutex_unlock(&ms->cli_fds_lock);
}

#endif
