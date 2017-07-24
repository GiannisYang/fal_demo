#ifndef GAME_CC
#define GAME_CC

#include "game.h"
#include "tools.h"

game::game(event_base *b):
    base(b),
    state(GAME_WAITING),
    pl_num(0),
    turn(0),
    ev_pl_timer(NULL),
    ev_keep_alive(NULL) {
    for(int i = 0; i < MAX_PSIZE; i++)
        pl[i] = NULL;
}

game::~game() {
    event_free(ev_pl_timer);
    event_free(ev_keep_alive);
    for(int i = 0; i < MAX_PSIZE; i++)
        if(pl[i]) delete pl[i];
};

//void game::wait_pl() {
//
//    sockaddr_in serv_addr;
//    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
//    memset(&serv_addr, 0, sizeof(serv_addr));
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_port = htons(SERVPORT);
//    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    bind(listen_fd, (sockaddr *) &serv_addr, sizeof(serv_addr));
//    listen(listen_fd, LISTENQ);
//    state = GAME_WAITING;
//
//    if (!ev_wait)
//        ev_wait = event_new(base, listen_fd,
//            EV_READ | EV_PERSIST, connect_pl, this);
////    else
////        event_assign(ev_wait, base, listen_fd,
////            EV_READ | EV_PERSIST, connect_pl, this);
//    event_add(ev_wait, NULL);
//}

void game::connect_pl(player *new_p) {
    char buf[BUFSIZE];
    pl[pl_num] = new_p;
    new_p->init(pl_num++, this);

    memset(buf, '\0', BUFSIZE);
    sprintf(buf, "Waiting for other %d player(s).",
        MAX_PSIZE - pl_num);
    add_cmd_head(buf, '$', 0);
    broadcast2cli(buf);

    /************************************************
      * if player num = 3, stop listen event and
      * begin this game
      ***********************************************/
    if(pl_num == MAX_PSIZE) {
        state = GAME_RUNNING;

        /** give prompts and determine the landlord */
        landlord = det_landlord();
        memset(buf, '\0', BUFSIZE);
        sprintf(buf, "Player %d is the landlord, beat him up.",
            landlord);
        add_cmd_head(buf, '$', 0);
        broadcast2cli(buf);

        /** init the keep alive packet event */
        if(!ev_keep_alive)
            ev_keep_alive = event_new(base, -1,
                EV_TIMEOUT, send_keep_alive, this);
        timeval tv = {ALIVE_INTERVAL, 0};
        event_add(ev_keep_alive, &tv);

        /** start this game */
        turn = (landlord - 1) % MAX_PSIZE;
        if(!ev_pl_timer)
            ev_pl_timer = event_new(base, -1,
                EV_TIMEOUT, schedule, this);
        schedule(-1, EV_READ, this);
    }
}

void game::broadcast2cli(char *buf) {
    for(int i = 0; i < pl_num; i++)
        if(pl[i])
            pl[i]->send2cli(buf);
}

void game::send_keep_alive(int fd, short what, void *arg) {
    if(!(what & EV_TIMEOUT)) return;

    game *obj = (game *)arg;
    player *p;
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    sprintf(buf, "ka");
    add_cmd_head(buf, '$', 0);
    for(int i = 0; i < MAX_PSIZE; i++) {
        p = obj->pl[i];
        if(p && p->is_alive()) {
            p->send2cli(buf);
            p->wait_res_ka++;
            if(!p->is_alive()) {
            /** call the offline function */
                cout << "player: "<<i<<" is offline.." << endl;
            }
        }
    }

    timeval tv = {ALIVE_INTERVAL, 0};
    event_add(obj->ev_keep_alive, &tv);
}

/** Schedule the actions of players */
void game::schedule(int fd, short what, void *arg) {
    game *obj = (game *)arg;

    char hint[BUFSIZE];
    /** Time out: in this turn, the player input nothing,
      * game::get_res did not tell any cli this result */
    if(what & EV_TIMEOUT) {
        memset(hint, '\0', BUFSIZE);
        sprintf(hint, "input nothing");
        add_cmd_head(hint, ':', obj->turn);
        obj->broadcast2cli(hint);
    }

    player *this_p = obj->pl[obj->turn];
    player *next_p;
    for(int i = 0; i < MAX_PSIZE; i++) {
        obj->turn = (obj->turn + 1) % MAX_PSIZE;
        next_p = obj->pl[obj->turn];
        if(next_p && next_p->is_alive())
            break;
    }

    /** all cli have quit the game */
    if(!next_p || !next_p->is_alive()) {
        cout << "all cli have quit the game" << endl;
        event_del(obj->ev_pl_timer);
        event_del(obj->ev_keep_alive);
        return;
    }

    /** print a hint to next player */
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
    schedule(-1, EV_READ, this);
};

void game::cli_exit(player *p) {
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    sprintf(buf, "Player %d has quit the game", p->id);
    add_cmd_head(buf, '$', 0);
    broadcast2cli(buf);
    delete p;
    p = NULL;
    pl_num--;
}

#endif
