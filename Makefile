CC=cc
CFLAGS= -Wall -Wextra -Werror
NAME=ft_ping
DIR_UTILS=./src/
UTILSFILES=  main.c parser.c

SRCS= $(UTILSFILES:%.c=$(DIR_UTILS)%.c)

OBJS=$(SRCS:.c=.o)

all:  $(NAME)
	@echo "\033[032m ready \033[0m"

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "\033[032m compiled succefuly !!\033[0m"


clean:
	@rm -f $(OBJS)

	@echo "\033[034m files are cleaned !!\033[0m"

fclean:clean
	@rm -f $(NAME)
	@echo "\033[034m cleaned !!\033[0m"

re:fclean all

.PHONY:all ,clean ,fclean , re
.SILENT:$(OBJS)