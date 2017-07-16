#ifndef MAIN_CC
#define MAIN_CC

#include "def1.h"
#include "game.h"

using namespace std;

int main() {

    event_base* game_base = event_base_new();
    game *g = new game(game_base);
    g->wait_pl();

    cout << "Hello world!" << endl;
    if(event_base_dispatch(game_base) != 0)
        cout << "event_base_dispatch err" << endl;
    event_base_free(game_base);

    return 0;
}

#endif
