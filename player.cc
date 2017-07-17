#ifndef PLAYER_CC
#define PLAYER_CC

#include "player.h"
#include "tools.h"

player::player(int cfd, sockaddr_in addr, uint8_t id, game *g):
    ip_addr(inet_ntoa(addr.sin_addr)), state(PL_INIT),
    id(id), gm(g), conn_fd(cfd) {

    ev_res = event_new(gm->base, conn_fd,
        EV_READ | EV_PERSIST, get_res, this);
    event_add(ev_res, NULL);

    cout << "Player " << ip_addr
        << " has joined this game." << endl;
}

/** arg is (*this) of a player object */
void player::get_res(int fd, short what, void *arg) {
    player *pl = (player *)arg;
    if(!(PL_ONTERN & pl->state)
        || !(what & EV_READ)
    ) return;

    char buf[BUFSIZE];
    tcp_recv(pl->conn_fd, buf);
    /** one turn of this player is over */
    cout << "player::get_res: "<<buf<<endl;

    pl->state &= (~PL_ONTERN);
    pl->gm->get_res(buf, pl->id);
}


#endif
