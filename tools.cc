#ifndef TOOLS_CC
#define TOOLS_CC

#include "tools.h"

/** update information: <strlen>#<cmd>:<player_id>
  * prompt command: <strlen>#<cmd>$
  *
  * tcp_recv return -1: it's a command
  */

int tcp_recv(int sfd, char *buf) {
    int res = 0;
    char c;
    memset(buf, '\0', BUFSIZE);
    while(1) {
        recv(sfd, &c, 1, 0);
        if(c == '#')
            break;
        res = res * 10 + (int)c - 48;
    }
    recv(sfd, buf, res, 0);
    recv(sfd, &c, 1, 0);
    if(c == ':') {
        recv(sfd, &c, 1, 0);
        return (int)c - 48;
    }
    return -1;
}

void add_cmd_head(char *buf, char c, int id) {
    char tmp[BUFSIZE];
    memset(tmp, 0, BUFSIZE);
    sprintf(tmp, "%d#%s%c", strlen(buf), buf, c);
    if(c == ':')
        sprintf(buf, "%s%c", tmp, I2C(id));
    else
        sprintf(buf, "%s", tmp);
}

//int read_int(const char *p, const char e) {
//    if(!p)
//        return -1;
//    int res = 0;
//    while(*p != '\0' && *p != e)
//        res = res * 10 + (int)(*p) - 48;
//    return res;
//}

//string get_loc_ip() {
//    ifaddrs * ifAddrStruct = NULL;
//    void * tmpAddrPtr = NULL;
//
//    getifaddrs(&ifAddrStruct);
//
//    while (ifAddrStruct != NULL) {
//        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
//            tmpAddrPtr = &((sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
//            char addressBuffer[INET_ADDRSTRLEN];
//            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
//            if(ifAddrStruct->ifa_name[0] == 'e'
//                && !has_str((const char *)"127.0.0.1", addressBuffer)) {
//                    string res(addressBuffer);
//                    return res;
//                }
//        }
//        ifAddrStruct = ifAddrStruct->ifa_next;
//    }
//    return "";
//}

#endif
