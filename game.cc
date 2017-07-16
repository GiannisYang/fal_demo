#ifndef GAME_CC
#define GAME_CC

#include "game.h"

game::game(event_base *b): base(b) {
    id = 0;
    state = GAME_INIT;
    pl_num = 0;
    turn = 0;
    listen_fd = -1;
    ev_wait = NULL;
    memset(cmd, 0, CMDSIZE);
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
    string prompt = "Connection successful.";
    obj->send2cli(obj->pl_num - 1, prompt);
    prompt = "Waiting for other ";
    prompt += I2C(MAX_PSIZE - obj->pl_num);
    prompt += " player(s).";
    obj->send2cli(-1, prompt);

    /************************************************
      * if player num = 3, stop listen event and
      * begin this game
      ***********************************************/
    if(obj->pl_num == MAX_PSIZE) {
        /** give prompts and determine the landlord */
        prompt = "Beginning:";
        obj->send2cli(-1, prompt);
        obj->landlord = det_landlord();
        prompt = "You are the landlord.";
        obj->send2cli(obj->landlord, prompt);
        prompt = "Player ";
        prompt += I2C(obj->landlord);
        prompt += " is the landlord, beat him up";
        for(int i = 0; i < MAX_PSIZE; i++)
            if(i != obj->landlord)
                obj->send2cli(i, prompt);

        /** start this game */
        obj->turn = (obj->landlord - 1) % MAX_PSIZE;
        obj->ev_pl_timer = event_new(obj->base, -1, EV_TIMEOUT,
            schedule, obj);
        timeval tv = {0, 0};
        event_add(obj->ev_pl_timer, &tv);

        /** delete listen event */
        obj->state = GAME_RUNNING;
        event_del(obj->ev_wait);
        close(obj->listen_fd);
        obj->listen_fd = -1;
    }
}

int game::send2cli(int loc, const string &str) {
    /** 0 <= loc <= 2: send to a client
      * otherwise: send to all 3 clients
      */
    if(loc >= 0 && loc < MAX_PSIZE) {
        send(pl[loc]->conn_fd, str.c_str(),
            str.length(), MSG_DONTWAIT);
    } else {
        for(int i = 0; i < pl_num; i++)
            send(pl[i]->conn_fd, str.c_str(),
                str.length(), MSG_DONTWAIT);
    }
    return 0;
}

void game::schedule(int fd, short what, void *arg) {
    game *obj = (game *)arg;
    obj->turn = (obj->turn + 1) % MAX_PSIZE;
    player *next_p = obj->pl[obj->turn];
    if(PL_ONTERN & next_p->state
        || PL_HINTED & next_p->state
    ) cout <<"PL_ONTERN & next_p->state || PL_HINTED & next_p->state" <<endl;
//    return;

    next_p->state = PL_ONTERN;
    timeval tv = {TIME_INTERVAL, 0};
    event_add(obj->ev_pl_timer, &tv);
}

#endif
