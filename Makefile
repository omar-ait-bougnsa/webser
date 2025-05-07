name = webserver 
CC=c++
flag= -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g

SRC= webserver.cpp pars_config.cpp print.cpp httpResponse.cpp
OBJ=$(SRC:.cpp=.o)

all:$(name)

$(name) : $(OBJ)
	$(CC) $(flag) $(OBJ) -o $(name)
%.o : %.cpp
	$(CC) $(flag) -c $< -o $@

clean:
	rm -f $(OBJ)
fclean : clean
	rm -f $(name)
re: fclean all

.PHONY : all fclean clean re