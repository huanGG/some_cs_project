##
## EPITECH PROJECT, 2017
## Makefile
## File description:
## Makefile
##

#CC	=	gcc -Wall -Wextra -O3 -g
CC	=	gcc -Wall -Wextra -g


RM	=	rm -f

NAME	=	detect

SRC	=	detect.c	\
		hashtable.c	


all:	$(NAME) 

$(NAME):
	$(CC) -o $(NAME) $(SRC) 

clean:
	$(RM) $(NAME)

.PHONY: all clean 
