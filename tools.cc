#ifndef TOOLS_CC
#define TOOLS_CC

#include "tools.h"

//static bool has_str(const u_char *p, const char *str) {
//    if(!p || !str)
//        return false;
//
//    for(int i = 0; i < strlen(str); i++)
//        if(*(p + i) != *(str + i))
//            return false;
//    return true;
//}
//
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
