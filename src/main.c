#include "../includes/ft_ping.h"



int main(int argc, char **argv)
{
    t_ping *cmd;
    cmd = malloc(sizeof(t_ping));
    (void) argv;
    if (argc == 1)
        printf("ping: usage error: Destination address required");
    int i = 0;
    while (argv[i])
    {
        write (1, argv[i],strlen(argv[i]));
        write (1, "  ",2);
        i++;
    }
    parser(argv, cmd);
    free(cmd);
}