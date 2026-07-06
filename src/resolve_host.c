#include "../includes/ft_ping.h"

void resolve_host(t_ping *ping)
{
	struct addrinfo	hints;
	struct addrinfo	*result;
	char			ip_str[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	if (getaddrinfo(ping->host_str, NULL, &hints, &result) != 0)
	{
		fprintf(stderr, "ft_ping: cannot resolve %s: unknown host\n",
			ping->host_str);
		exit(EXIT_CANNOT_RESOLVE_HOST);
	}

	ping->target = malloc(sizeof(struct sockaddr_in));
	if (!ping->target)
	{
		perror("malloc");
		exit(1);
	}

	memset(ping->target, 0, sizeof(*ping->target));
	ping->target->sin_family = AF_INET;
	ping->target->sin_port = 0;
	ping->target->sin_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;

	inet_ntop(AF_INET, &ping->target->sin_addr, ip_str, INET_ADDRSTRLEN);
	strncpy(ping->target_ip, ip_str, INET_ADDRSTRLEN - 1);
	ping->target_ip[INET_ADDRSTRLEN - 1] = '\0';

	freeaddrinfo(result);
}