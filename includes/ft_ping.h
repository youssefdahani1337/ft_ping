
#ifndef FT_PING_H
#define FT_PING_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FLAGS ""

typedef struct s_ping {
    char *target;
    int verbose;

} t_ping;

void parser(char **av, t_ping *cmd);
void parse_flags(char *flag, t_ping *cmd);
void handle_addr(char *flag, t_ping *cmd);

#endif