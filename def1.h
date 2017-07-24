#ifndef DEF1_H
#define DEF1_H

#include "def0.h"

#define MAX_PSIZE 1
#define LISTENQ 20
#define SERVPORT 6666
#define TIME_INTERVAL 30
#define I2C(x) (char)((x) + 48)
#define BUFSIZE 500
#define ALIVE_SIZE 3
#define ALIVE_INTERVAL 10

class game;
class player;
class main_serv;

#endif
