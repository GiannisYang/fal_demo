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

void game::re_connect_pl(player *new_p) {
    int i;
    char buf[BUFSIZE];
    for(i = 0; i < MAX_PSIZE; i++)
        if(pl[i]->ip_key == new_p->ip_key)
            break;
    pl[i]->state = PL_INIT;
    close(pl[i]->conn_fd);
    pl[i]->conn_fd = new_p->conn_fd;
    pl[i]->wait_res_ka = 0;
    pl[i]->init(i, this);
    /** for the reconnecting cli */
    memset(buf, '\0', BUFSIZE);
    sprintf(buf, "st%d%s", i, pl[i]->cards.c_str());
    add_cmd_head(buf, '$', 0);
    pl[i]->send2cli(buf);
    /** for other clis */
    memset(buf, '\0', BUFSIZE);
    sprintf(buf, "Player %d has reconnected", i);
    add_cmd_head(buf, '$', 0);
    for(int j = 0; j < MAX_PSIZE; j++)
        if(j != i)
            pl[j]->send2cli(buf);
}

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

        /** init the keep alive packet event */
        if(!ev_keep_alive)
            ev_keep_alive = event_new(base, -1,
                EV_TIMEOUT, send_keep_alive, this);
        timeval tv = {ALIVE_INTERVAL, 0};
        event_add(ev_keep_alive, &tv);

        /** determine the landlord */
        srand(unsigned(time(0)));
        landlord = (uint8_t)rand() % MAX_PSIZE;
        for(int i = 0; i < MAX_PSIZE; i++)
            if(i == landlord) pl[i]->cards_num = 20;
            else pl[i]->cards_num = 17;

        /** send cards */
        string cards[MAX_PSIZE];
        int tmp_p;
        for(int i = 0; i < 54; i++) {
            tmp_p = rand() % MAX_PSIZE;
            if(cards[tmp_p].length() == pl[tmp_p]->cards_num) {
                i--;
                continue;
            }
            cards[tmp_p] += all_cards[i];
        }

        /** give prompts */
        for(int i = 0; i < MAX_PSIZE; i++) {
//            cards[i] = "6666666666"; debug
            pl[i]->cards = cards[i];
            memset(buf, '\0', BUFSIZE);
            /** the id of player is i, cards are cards[i] */
            sprintf(buf, "st%d%s", i, cards[i].c_str());
            add_cmd_head(buf, '$', 0);
            pl[i]->send2cli(buf);
        }

        memset(buf, '\0', BUFSIZE);
        sprintf(buf, "Player %d is the landlord, beat him up.",
            landlord);
        add_cmd_head(buf, '$', 0);
        broadcast2cli(buf);

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
        if(p
            && ~p->state & PL_EXIT
        ) {
            p->send2cli(buf);
            if(!p->is_alive()) {
            /** call the offline function */
                cout << "player: "<<i<<" is offline.." << endl;
                p->state |= PL_OFFLINE;
                continue;
            }
            p->wait_res_ka++;
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
    if(this_p->cards_num <= 0) {
        memset(hint, '\0', BUFSIZE);
        if(obj->turn == obj->landlord)
            sprintf(hint, "Landlord is winner !");
        else
            sprintf(hint, "Folks are winners !");
        add_cmd_head(hint, '$', 0);
        obj->broadcast2cli(hint);
        /** one game is over */
    }

    player *next_p;
    for(int i = 0; i < MAX_PSIZE; i++) {
        obj->turn = (obj->turn + 1) % MAX_PSIZE;
        next_p = obj->pl[obj->turn];
        if(!(next_p->state & PL_EXIT)
            && next_p->is_alive()
        ) break;
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
    int len = strlen(str);
    memset(res, 0, BUFSIZE);
    /** get an action of last player */
    sprintf(res, "%s", str);
    for(int i = 0; i < len; i++) {
        if(str[i] == ' ') continue;

        for(int j = 0; j < pl[id]->cards.length(); j++)
            if(pl[id]->cards[j] == str[i]) {
                pl[id]->cards_num--;
                pl[id]->cards[j] = ' ';
                break;
            }
    }

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
    event_del(p->ev_res);
    p->state = PL_EXIT;
}

#endif
