#ifndef PLAYER_CC
#define PLAYER_CC

#include "player.h"
#include "tools.h"

player::player(int cfd, sockaddr_in addr):
    ip_addr(inet_ntoa(addr.sin_addr)), state(PL_INIT),
    conn_fd(cfd), wait_res_ka(0) {

    cout << "Player " << ip_addr
        << " has joined this game." << endl;
}

void player::init(uint8_t new_id, game *g) {
    id = new_id;
    gm = g;
    ev_res = event_new(gm->base, conn_fd,
        EV_READ | EV_PERSIST, get_res, this);
    event_add(ev_res, NULL);
};

/** arg is (*this) of a player object */
void player::get_res(int fd, short what, void *arg) {
    player *pl = (player *)arg;
    if(!(PL_ONTERN & pl->state)
        && pl->wait_res_ka <= 0
    ) return;

    char buf[BUFSIZE];
    /** 1. cannot read anything, this cli could be down */
    if(-2 == (tcp_recv(pl->conn_fd, buf))) {
    cout << "tcp_recv return -2"<<endl;
        pl->gm->cli_exit(pl);
        return;
    }
    cout << "player::get_res: "<<buf<<endl;

    /** 2. response of keep alive packet */
    if (strcmp("isal", buf) == 0) {
        pl->wait_res_ka = 0;
        return;
    }
    /** 3. one turn of this player is over */
    pl->state &= (~PL_ONTERN);
    pl->gm->get_res(buf, pl->id);
}


#endif
