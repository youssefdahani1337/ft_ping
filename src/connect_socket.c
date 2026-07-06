
#include "../includes/ft_ping.h"

void connect_socket(t_ping *ping)
{
	struct timeval	tv;
	int				ttl;

	ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ping->sockfd < 0)
	{
		perror("ft_ping: socket");
		fprintf(stderr, "ft_ping: cannot connect to %s\n", ping->host_str);
		exit(EXIT_CANNOT_CONNECT_SOCKET);
	}

	tv.tv_sec = ping->timeout;
	tv.tv_usec = 0;

	if (setsockopt(ping->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv,
		sizeof(tv)) < 0)
		perror("setsockopt SO_RCVTIMEO");

	if (ping->ttl > 0)
	{
		ttl = ping->ttl;
		if (setsockopt(ping->sockfd, IPPROTO_IP, IP_TTL, &ttl,
			sizeof(ttl)) < 0)
			perror("setsockopt IP_TTL");
	}
}