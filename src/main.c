#include "../includes/ft_ping.h"
#include <signal.h>

static int g_stop = 0;

static void handle_sigint(int sig)
{
	(void)sig;
	g_stop = 1;
}

static void cleanup(t_ping *cmd)
{
	if (!cmd)
		return;
	if (cmd->sockfd >= 0)
		close(cmd->sockfd);
	if (cmd->target)
		free(cmd->target);
	if (cmd->send_buf)
		free(cmd->send_buf);
	if (cmd->recv_packet)
		free(cmd->recv_packet);
	free(cmd);
}

int main(int argc, char **argv)
{
	t_ping			*cmd;
	t_recv_packet	*recv_packet;
	struct timespec	start;
	struct timespec	end;
	struct sigaction sa;
	double			elapsed_ms;

	if (argc == 1)
	{
		printf("ft_ping: usage error: Destination address required\n");
		exit(2);
	}

	cmd = malloc(sizeof(t_ping));
	recv_packet = malloc(sizeof(t_recv_packet));

	if (!cmd || !recv_packet)
	{
		perror("malloc");
		exit(1);
	}

	cmd->recv_packet = recv_packet;
	init_args(cmd);
	intialize_recv_packet(recv_packet);
	parse_args(argc, argv, cmd);
	setup_send_buffer(cmd);
	resolve_host(cmd);
	connect_socket(cmd);

	clock_gettime(CLOCK_MONOTONIC, &cmd->start_time);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	while (!g_stop && !deadline_expired(cmd))
	{
		load_new_msg(cmd);
		clock_gettime(CLOCK_MONOTONIC, &start);
		send_ping(cmd);
		recv_msg(cmd);
		clock_gettime(CLOCK_MONOTONIC, &end);

		if (g_stop)
			break;

		elapsed_ms = (double)(end.tv_sec - start.tv_sec) * 1000.0
			+ (double)(end.tv_nsec - start.tv_nsec) / 1e6;

		display(cmd, elapsed_ms);
		usleep(1000000);
	}

	print_ping_statistics(cmd);
	cleanup(cmd);

	return (0);
}