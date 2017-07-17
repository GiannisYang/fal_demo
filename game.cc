#ifndef GAME_CC
#define GAME_CC

#include "game.h"
#include "tools.h"

game::game(event_base *b): base(b) {
    id = 0;
    state = GAME_INIT;
    pl_num = 0;
    turn = 0;
    listen_fd = -1;
    ev_wait = NULL;
    for(int i = 0; i < MAX_PSIZE; i++)
        pl[i] = NULL;
}

void game::wait_pl() {
    sockaddr_in serv_addr;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVPORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listen_fd, (sockaddr *) &serv_addr, sizeof(serv_addr));
    listen(listen_fd, LISTENQ);
    state = GAME_WAITING;

    if (!ev_wait)
        ev_wait = event_new(base, listen_fd,
            EV_READ | EV_PERSIST, connect_pl, this);
    else
        event_assign(ev_wait, base, listen_fd,
            EV_READ | EV_PERSIST, connect_pl, this);
    event_add(ev_wait, NULL);
}

void game::connect_pl(int fd, short what, void *arg) {
    game *obj = (game *)arg;

    if(obj->pl_num >= MAX_PSIZE
        || !(what & EV_READ)
    ) {
        cout << "pl_num >= MAX_PSIZE || !(what & EV_READ)" << endl;
        return;
    }

    sockaddr_in cli_addr;
    socklen_t len;
    int connfd;
    if((connfd = accept(obj->listen_fd, (sockaddr *) &cli_addr, &len))
        == -1) {
        cout << "accept == -1" << endl;
        return;
    }

    obj->pl[obj->pl_num++] = new player(connfd,
        cli_addr, obj->pl_num, obj);
    /** now a connection is established */
    char buf[BUFSIZE];
    memset(buf, '\0', BUFSIZE);
    sprintf(buf, "Connection successful.");
    add_cmd_head(buf, '$', 0);
    obj->pl[obj->pl_num - 1]->send2cli(buf);

    memset(buf, '\0', BUFSIZE);
    sprintf(buf, "Waiting for other %d player(s).",
        MAX_PSIZE - obj->pl_num);
    add_cmd_head(buf, '$', 0);
    obj->broadcast2cli(buf);

    /************************************************
      * if player num = 3, stop listen event and
      * begin this game
      ***********************************************/
    if(obj->pl_num == MAX_PSIZE) {
        /** give prompts and determine the landlord */
        obj->landlord = det_landlord();
        memset(buf, '\0', BUFSIZE);
        sprintf(buf, "Player %d is the landlord, beat him up.",
            obj->landlord);
        add_cmd_head(buf, '$', 0);
        obj->broadcast2cli(buf);

        /** start this game */
        obj->turn = (obj->landlord - 1) % MAX_PSIZE;
        obj->ev_pl_timer = event_new(obj->base, -1, EV_TIMEOUT,
            schedule, obj);
//        timeval tv = {0, 0};
//        event_add(obj->ev_pl_timer, &tv);
        schedule(-1, EV_TIMEOUT, obj);

        /** delete listen event */
        obj->state = GAME_RUNNING;
        event_del(obj->ev_wait);
        close(obj->listen_fd);
        obj->listen_fd = -1;
    }
}

void game::broadcast2cli(char *buf) {
    for(int i = 0; i < pl_num; i++)
        pl[i]->send2cli(buf);
}

/** Schedule the actions of players */
void game::schedule(int fd, short what, void *arg) {
    game *obj = (game *)arg;
    obj->turn = (obj->turn + 1) % MAX_PSIZE;
    player *next_p = obj->pl[obj->turn];

    /** print a hint to next player */
    char hint[BUFSIZE];
    memset(hint, '\0', BUFSIZE);
    sprintf(hint, "yt");
    add_cmd_head(hint, '$', 0);
    next_p->send2cli(hint);
    /** the action of next player can be read */
    next_p->state |= PL_ONTERN;
    /** register a time limit event of this player */
    timeval tv = {TIME_INTERVAL, 0};
    event_add(obj->ev_pl_timer, &tv);
}

void game::get_res(const char *str, int id) {
    event_del(ev_pl_timer);
    char res[BUFSIZE];
    memset(res, 0, BUFSIZE);
    /** get an action of last player */
    sprintf(res, "%s", str);

    /** update game information to everybody
      * according to this action */
    add_cmd_head(res, ':', id);
    broadcast2cli(res);
    schedule(-1, EV_TIMEOUT, this);
};

#endif
