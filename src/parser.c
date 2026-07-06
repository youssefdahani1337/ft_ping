#include "../includes/ft_ping.h"
#include <errno.h>

void intialize_recv_packet(t_recv_packet *recv_packet)
{
	memset(recv_packet, 0, sizeof(t_recv_packet));
	recv_packet->addr_len = sizeof(struct sockaddr_in);
}

void init_args(t_ping *ping)
{
	ping->verbose = 0;
	ping->help = 0;
	ping->numeric = 0;
	ping->timeout = 1;
	ping->deadline = 0;
	ping->ttl = -1;
	ping->host_str = NULL;
	ping->sockfd = -1;
	ping->target = NULL;
	ping->packet_transmitted = 0;
	ping->packet_received = 0;
	ping->icmp_hdr = NULL;
	ping->send_buf = NULL;
	ping->send_buf_size = 0;
	ping->payload_size = DEFAULT_PAYLOAD_SIZE;
	ping->target_name[0] = '\0';
	ping->target_ip[0] = '\0';
	ping->pid = (uint16_t)(getpid() & 0xFFFF);
	ping->init_seq = (uint16_t)(getpid() & 0xFFFF);
	ping->seq = ping->init_seq;
}

static void print_help_and_exit(void)
{
	print_usage();
	exit(EXIT_SUCCESS);
}

static void fail_invalid_option(char opt)
{
	fprintf(stderr, "ft_ping: invalid option -- '%c'\n", opt);
	print_usage();
	exit(EXIT_FAILURE);
}

static unsigned long parse_unsigned_long(const char *value,
	const char *label, unsigned long min, unsigned long max)
{
	char		*end;
	unsigned long	parsed;

	errno = 0;
	parsed = strtoul(value, &end, 10);
	if (errno != 0 || end == value || *end != '\0' ||
		parsed < min || parsed > max)
	{
		fprintf(stderr, "ft_ping: invalid %s: %s\n", label, value);
		exit(2);
	}
	return (parsed);
}

void print_usage(void)
{
	printf("Usage: ft_ping [OPTIONS] HOST\n");
	printf("  -v              verbose\n");
	printf("  -?              help\n");
	printf("  -n              numeric output only\n");
	printf("  -s SIZE         payload size\n");
	printf("  -W TIMEOUT      receive timeout\n");
	printf("  -w DEADLINE     stop after N seconds\n");
	printf("  --ttl VALUE     set ttl\n");
}

void parse_args(int argc, char **argv, t_ping *ping)
{
	int				opt;
	int				long_index;
	struct option	long_opts[] = {
		{"ttl", required_argument, 0, 1000},
		{0, 0, 0, 0}
	};

	opterr = 0;

	while ((opt = getopt_long(argc, argv, "vns:W:w:?", long_opts,
		&long_index)) != -1)
	{
		switch (opt)
		{
			case 'v':
				ping->verbose = 1;
				break;
			case 'n':
				ping->numeric = 1;
				break;
			case 's':
				ping->payload_size = parse_unsigned_long(optarg,
					"payload size", 0, 65507);
				break;
			case 'W':
				ping->timeout = (unsigned int)parse_unsigned_long(optarg,
					"timeout", 0, 3600);
				break;
			case 'w':
				ping->deadline = (unsigned int)parse_unsigned_long(optarg,
					"deadline", 1, 3600 * 24);
				break;
			case 1000:
				ping->ttl = (int)parse_unsigned_long(optarg,
					"ttl", 1, 255);
				break;
			case '?':
				if (strcmp(argv[optind - 1], "-?") == 0)
				{
					print_help_and_exit();
					exit(EXIT_SUCCESS);
				}

				fail_invalid_option((char)optopt);
				break;
							
			default:
				print_usage();
				exit(2);
		}
	}
	if (optind >= argc)
	{
		fprintf(stderr, "ft_ping: missing host\n");
		exit(EXIT_FAILURE);
	}
	if (optind < argc - 1)
	{
		print_usage();
		exit(EXIT_FAILURE);
	}
	ping->host_str = argv[optind];
}