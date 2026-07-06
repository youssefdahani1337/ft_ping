#ifndef FT_PING_H
#define FT_PING_H

/*
** Includes
*/
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

/*
** Constants
*/
#define ICMP_HEADER_SIZE 8
#define DEFAULT_PAYLOAD_SIZE 56
#define MAX_RECV_BUFFER 1024

/*
** Error codes
*/
#define EXIT_CANNOT_RESOLVE_HOST 68
#define EXIT_CANNOT_CONNECT_SOCKET 69

/*
** Type definitions
*/
typedef struct s_recv_packet
{
	int					packet_size;
	char				recv_buf[MAX_RECV_BUFFER];
	time_t				recv_time;
	socklen_t			addr_len;
	struct sockaddr_in	recv_addr;
	struct icmp			*icmp_hdr;
}	t_recv_packet;

typedef struct s_ping
{
	char				*host_str;
	struct sockaddr_in	*target;
	size_t				packet_transmitted;
	size_t				packet_received;
	struct icmp			*icmp_hdr;
	int					verbose;
	int					help;
	int					numeric;
	unsigned int		timeout;
	unsigned int		deadline;
	int					ttl;
	int					sockfd;
	t_recv_packet		*recv_packet;
	char				*send_buf;
	size_t				send_buf_size;
	size_t				payload_size;
	char				target_name[NI_MAXHOST];
	char				target_ip[INET_ADDRSTRLEN];
	struct timespec		start_time;
	uint16_t			pid;
	uint16_t			init_seq;
	uint16_t			seq;
}	t_ping;

/*
** Function declarations - initialization
*/
void				init_args(t_ping *ping);
void				intialize_recv_packet(t_recv_packet *recv_packet);

/*
** Function declarations - argument parsing
*/
void				parse_args(int argc, char **argv, t_ping *cmd);
void				print_usage(void);

/*
** Function declarations - socket operations
*/
void				resolve_host(t_ping *ping);
void				connect_socket(t_ping *ping);

/*
** Function declarations - packet handling
*/
void				setup_send_buffer(t_ping *ping);
void				load_new_msg(t_ping *cmd);
void				send_ping(t_ping *cmd);
void				recv_msg(t_ping *cmd);

/*
** Function declarations - ICMP utilities
*/
unsigned short		calculate_checksum(void *b, int len);

/*
** Function declarations - display/output
*/
void				display(t_ping *cmd, double elapsed_ms);
void				display_ping_result(t_ping *cmd, double elapsed_ms);
void				print_ping_statistics(t_ping *cmd);

/*
** Function declarations - ICMP handlers
*/
void				handle_echo_reply(t_ping *cmd, struct ip *ip_hdr,
					struct icmp *icmp, int icmp_len, double elapsed_ms);
void				handle_destination_unreachable(t_ping *cmd,
					struct icmp *icmp);
void				handle_time_exceeded(t_ping *cmd, struct icmp *icmp);
void				handle_unknown_icmp(t_ping *cmd, struct icmp *icmp);
void				handle_icmp_packet(t_ping *cmd, struct ip *ip_hdr,
					struct icmp *icmp, int icmp_len, double elapsed_ms);

/*
** Function declarations - utilities
*/
int					deadline_expired(t_ping *cmd);
const char			*get_unreachable_message(int code);
const char			*get_time_exceeded_message(int code);

#endif