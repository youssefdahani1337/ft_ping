#include "../includes/ft_ping.h"

void setup_send_buffer(t_ping *ping)
{
	size_t required_size;

	required_size = ICMP_HEADER_SIZE + ping->payload_size;
	ping->send_buf = malloc(required_size);
	if (!ping->send_buf)
	{
		perror("malloc");
		exit(1);
	}
	ping->send_buf_size = required_size;
}

void load_new_msg(t_ping *cmd)
{
	size_t			packet_len;
	struct icmp		*icmp;

	packet_len = ICMP_HEADER_SIZE + cmd->payload_size;
	if (cmd->send_buf_size != packet_len)
	{
		fprintf(stderr, "ft_ping: internal payload buffer mismatch\n");
		exit(1);
	}

	memset(cmd->send_buf, 0, cmd->send_buf_size);
	icmp = (struct icmp *)cmd->send_buf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = htons(cmd->pid);
	icmp->icmp_seq = htons(cmd->seq);

	memset(cmd->send_buf + ICMP_HEADER_SIZE, 'Y', cmd->payload_size);
	icmp->icmp_cksum = calculate_checksum(cmd->send_buf, packet_len);

	if (cmd->verbose)
		printf("debug: id=%u seq=%u cksum=%u\n", cmd->pid, cmd->seq,
			icmp->icmp_cksum);
}

void send_ping(t_ping *cmd)
{
	ssize_t bytes_sent;

	bytes_sent = (ssize_t)(ICMP_HEADER_SIZE + cmd->payload_size);
	if (sendto(cmd->sockfd, cmd->send_buf, bytes_sent, 0,
		(struct sockaddr *)cmd->target, sizeof(struct sockaddr_in)) != bytes_sent)
	{
		perror("sendto");
		exit(1);
	}

	cmd->packet_transmitted++;
	cmd->seq++;
}