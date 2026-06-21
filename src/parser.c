#include "../includes/ft_ping.h"


void parser(char **av, t_ping *cmd)
{
    int i =1;

    while (av[i])
    {
        if (av[i][0] == '-')
            parse_flags(av[i], cmd);
        else
            handle_addr(av[i], cmd);
    }
}

void parse_flags(char *flag, t_ping *cmd)
{
    if (strcmp("-v", flag) == 0)
        cmd->verbose = 1;
    if 
}

void handle_addr(char *flag, t_ping *cmd)
{
    
}