#include "../includes/ft_ping.h"

/*
** ICMP message type handlers
*/

const char *get_unreachable_message(int code)
{
	if (code == ICMP_UNREACH_NET)
		return ("Network Unreachable");
	if (code == ICMP_UNREACH_HOST)
		return ("Host Unreachable");
	if (code == ICMP_UNREACH_PROTOCOL)
		return ("Protocol Unreachable");
	if (code == ICMP_UNREACH_PORT)
		return ("Port Unreachable");
	return (NULL);
}

const char *get_time_exceeded_message(int code)
{
	if (code == ICMP_TIMXCEED_INTRANS)
		return ("TTL exceeded in transit");
	if (code == ICMP_TIMXCEED_REASS)
		return ("Fragment reassembly time exceeded");
	return (NULL);
}

void handle_destination_unreachable(t_ping *cmd, struct icmp *icmp)
{
	const char	*source_ip;
	const char	*message;

	source_ip = inet_ntoa(cmd->recv_packet->recv_addr.sin_addr);
	message = get_unreachable_message(icmp->icmp_code);
	
	if (cmd->verbose)
	{
		if (message != NULL)
			printf("From %s: %s (code=%d)\n", source_ip, message, icmp->icmp_code);
		else
			printf("From %s: Destination unreachable (code=%d)\n",
				source_ip, icmp->icmp_code);
	}
	else
	{
		if (message != NULL)
			printf("From %s: %s\n", source_ip, message);
		else
			printf("From %s: Destination unreachable\n", source_ip);
	}
}

void handle_time_exceeded(t_ping *cmd, struct icmp *icmp)
{
	const char	*source_ip;
	const char	*message;

	source_ip = inet_ntoa(cmd->recv_packet->recv_addr.sin_addr);
	message = get_time_exceeded_message(icmp->icmp_code);
	
	if (cmd->verbose)
	{
		if (message != NULL)
			printf("From %s: %s (code=%d)\n", source_ip, message, icmp->icmp_code);
		else
			printf("From %s: Time exceeded (code=%d)\n",
				source_ip, icmp->icmp_code);
	}
	else
	{
		if (message != NULL)
			printf("From %s: %s\n", source_ip, message);
		else
			printf("From %s: Time exceeded\n", source_ip);
	}
}

void handle_unknown_icmp(t_ping *cmd, struct icmp *icmp)
{
	const char *source_ip;

	source_ip = inet_ntoa(cmd->recv_packet->recv_addr.sin_addr);
	if (cmd->verbose)
		printf("From %s: ICMP type=%u code=%u\n", source_ip,
			icmp->icmp_type, icmp->icmp_code);
	else
		printf("From %s: ICMP type=%u\n", source_ip, icmp->icmp_type);
}

void handle_echo_reply(t_ping *cmd, struct ip *ip_hdr, struct icmp *icmp, int icmp_len, double elapsed_ms)
{
	const char *source_ip;
	int result;

	source_ip = inet_ntoa(cmd->recv_packet->recv_addr.sin_addr);
	cmd->packet_received++;

	if (!cmd->numeric)
	{
		result = getnameinfo((struct sockaddr *)&cmd->recv_packet->recv_addr,
			cmd->recv_packet->addr_len, cmd->target_name,
			sizeof(cmd->target_name), NULL, 0, NI_NAMEREQD);
		if (result == 0)
		{
			printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.2f ms\n",
				icmp_len, cmd->target_name,
				ntohs(icmp->icmp_seq) - cmd->init_seq,
				ip_hdr->ip_ttl, elapsed_ms);
			return;
		}
	}

	printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.2f ms\n",
		icmp_len,
		cmd->target_name[0] ? cmd->target_name : source_ip,
		ntohs(icmp->icmp_seq) - cmd->init_seq,
		ip_hdr->ip_ttl,
		elapsed_ms);
    if (cmd->verbose)
        printf("debug: id=%u seq=%u cksum=%u\n", ntohs(icmp->icmp_id),
            ntohs(icmp->icmp_seq), icmp->icmp_cksum);
    }


void handle_icmp_packet(t_ping *cmd, struct ip *ip_hdr,
	struct icmp *icmp, int icmp_len, double elapsed_ms)
{
	switch (icmp->icmp_type)
	{
		case ICMP_ECHOREPLY:
			handle_echo_reply(cmd, ip_hdr, icmp, icmp_len, elapsed_ms);
			break;
		case ICMP_UNREACH:
			handle_destination_unreachable(cmd, icmp);
			break;
		case ICMP_TIMXCEED:
			handle_time_exceeded(cmd, icmp);
			break;
		default:
			handle_unknown_icmp(cmd, icmp);
			break;
	}
}

/*
** Display functions
*/

void display(t_ping *cmd, double elapsed_ms)
{
	if (cmd->recv_packet->packet_size > 0)
	{
		display_ping_result(cmd, elapsed_ms);
		return;
	}
	printf("Request timeout for icmp_seq %u\n",
		(unsigned int)(cmd->seq - cmd->init_seq));
}

void display_ping_result(t_ping *cmd, double elapsed_ms)
{
	struct ip	*ip_hdr;
	struct icmp	*icmp;
	int			ip_hdr_len;
	int			icmp_len;

	if (cmd->recv_packet->packet_size < (int)sizeof(struct ip) + ICMP_HEADER_SIZE)
	{
		printf("ft_ping: received packet too small ");
		if (cmd->verbose)
            printf("(size: %d bytes, need at least %zu bytes)",
                cmd->recv_packet->packet_size, sizeof(struct ip) + ICMP_HEADER_SIZE);
        printf("\n");
        return;
	}

	ip_hdr = (struct ip *)cmd->recv_packet->recv_buf;
	ip_hdr_len = ip_hdr->ip_hl * 4;

	if (ip_hdr_len < (int)sizeof(struct ip) || ip_hdr_len > cmd->recv_packet->packet_size)
	{
		printf("ft_ping: invalid IP header");
        if (cmd->verbose)
            printf(" (header length: %d bytes, packet size: %d bytes)",
                ip_hdr_len, cmd->recv_packet->packet_size);
        printf("\n");
		return;
	}

	icmp_len = cmd->recv_packet->packet_size - ip_hdr_len;

	if (icmp_len < ICMP_HEADER_SIZE)
	{
		printf("ft_ping: ICMP packet incomplete ");
		if (cmd->verbose)
			printf("(size: %d bytes, need at least %d bytes)",
				icmp_len, ICMP_HEADER_SIZE);
		printf("\n");
		return;
	}

	icmp = (struct icmp *)(cmd->recv_packet->recv_buf + ip_hdr_len);
	handle_icmp_packet(cmd, ip_hdr, icmp, icmp_len, elapsed_ms);
}

void print_ping_statistics(t_ping *cmd)
{
	double packet_loss;
	const char *host_label;

	host_label = cmd->target_name[0] ? cmd->target_name :
		(cmd->host_str ? cmd->host_str : "unknown");
	printf("\n--- %s ping statistics ---\n", host_label);
	printf("%zu packets transmitted, %zu packets received",
		cmd->packet_transmitted, cmd->packet_received);

	if (cmd->packet_transmitted == 0)
	{
		printf(", 0%% packet loss\n");
		return;
	}

	packet_loss = ((double)(cmd->packet_transmitted - cmd->packet_received) /
		(double)cmd->packet_transmitted) * 100.0;
	printf(", %.0f%% packet loss\n", packet_loss);
}

/*
** Utility functions
*/

int deadline_expired(t_ping *cmd)
{
	struct timespec	now;
	double			elapsed_seconds;

	if (cmd->deadline == 0)
		return (0);

	clock_gettime(CLOCK_MONOTONIC, &now);
	elapsed_seconds = (double)(now.tv_sec - cmd->start_time.tv_sec)
		+ (double)(now.tv_nsec - cmd->start_time.tv_nsec) / 1e9;

	return (elapsed_seconds >= (double)cmd->deadline);
}

unsigned short calculate_checksum(void *b, int len)
{
	unsigned short	*buf;
	unsigned int	sum;
	unsigned short	result;

	buf = (unsigned short *)b;
	sum = 0;

	while (len > 1)
	{
		sum += *buf++;
		len -= 2;
	}

	if (len == 1)
		sum += *(unsigned char *)buf;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;

	return (result);
}

