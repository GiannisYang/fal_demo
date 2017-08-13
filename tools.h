#ifndef TOOLS_H
#define TOOLS_H

#include "def1.h"

int tcp_recv(int sfd, char *buf);
void add_cmd_head(char *buf, char c, int id);
uint32_t ip2key(const string &str);

#endif
