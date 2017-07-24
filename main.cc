#ifndef MAIN_CC
#define MAIN_CC

#include "def1.h"
#include "game.h"
#include "main_serv.h"

using namespace std;

int main() {

    main_serv *ms = new main_serv;
    ms->main_loop();

//    g->wait_pl();
//
//    cout << "Hello world!" << endl;
//    if(event_base_dispatch(game_base) != 0)
//        cout << "event_base_dispatch err" << endl;
//    event_base_free(game_base);

    return 0;
}

#endif
