#include "../includes/ft_ping.h"

void recv_msg(t_ping *cmd)
{
	t_recv_packet	*recv;

	recv = cmd->recv_packet;
	recv->addr_len = sizeof(recv->recv_addr);

	memset(recv->recv_buf, 0, sizeof(recv->recv_buf));

	recv->packet_size = recvfrom(cmd->sockfd, recv->recv_buf,
		sizeof(recv->recv_buf), 0, (struct sockaddr *)&recv->recv_addr,
		&recv->addr_len);

	if (recv->packet_size <= 0)
	{
		recv->packet_size = 0;
		if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		perror("recvfrom");
		return;
	}
}