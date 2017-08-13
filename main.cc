#ifndef MAIN_CC
#define MAIN_CC

#include "def1.h"
#include "game.h"
#include "main_serv.h"

using namespace std;

int main() {

//    map<int, char> *mm;
//    mm = new map<int, char>;
//    pair<map<int, char>::iterator, bool> res;
//    cout << mm->insert(pair<int, char>(5, 'a')).second << endl;
//    cout << mm->insert(pair<int, char>(6, 'a')).second << endl;
//    res = mm->insert(pair<int, char>(5, 'c'));
//    cout << res.second<<", "<<(res.first)->second<<endl;
//    return 0;

    main_serv *ms = new main_serv;
    ms->main_loop();

    return 0;
}

#endif
