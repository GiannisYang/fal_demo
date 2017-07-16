#ifndef PLAYER_CC
#define PLAYER_CC

#include "player.h"

#define SENT2CLI(f, x) send((f), (x), strlen(x), MSG_DONTWAIT)
#define RECV_CLI(f, x) recv((f), (x), CMDSIZE, 0)
//int player::send2cli(const char *str) {
//    send(conn_fd, str, strlen(str), MSG_DONTWAIT);
//    return 0;
//}

player::player(int cfd, sockaddr_in addr, uint8_t id, game *g):
    ip_addr(inet_ntoa(addr.sin_addr)), state(PL_INIT),
    id(id), gm(g), conn_fd(cfd) {

    ev_hint = event_new(gm->base, conn_fd,
        EV_WRITE | EV_PERSIST, hint, this);
    ev_res = event_new(gm->base, conn_fd,
        EV_READ | EV_PERSIST, get_res, this);

    event_add(ev_hint, NULL);
    event_add(ev_res, NULL);

    cout << "Player " << ip_addr
        << " has joined this game." << endl;
}

/** arg is (*this) of a player object */
void player::hint(int fd, short what, void *arg) {
    player *pl = (player *)arg;
    if(!(PL_ONTERN & pl->state)
        || !(what & EV_WRITE)
    ) return;

    char buf[CMDSIZE];
    memset(buf, 0, CMDSIZE);
    /** last cmd from the privous player */
    sprintf(buf, "%s#", pl->gm->cmd);
    SENT2CLI(pl->conn_fd, buf);
    pl->state = PL_HINTED;
}

void player::get_res(int fd, short what, void *arg) {
    player *pl = (player *)arg;
    if(!(PL_HINTED & pl->state)
//        || !(PL_ONTERN & pl->state)
        || !(what & EV_READ)
    ) return;

    char buf[CMDSIZE];
    memset(buf, 0, CMDSIZE);
    RECV_CLI(pl->conn_fd, buf);
    /** one turn of this player is over */
    pl->state = PL_INIT;
    pl->gm->get_res(buf, pl->id);
}


#endif
