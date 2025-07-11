CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g3

NAME     = webserv
SRCS     = ./src/main.cpp \
			./src/WebServer.cpp \
			./src/EpollManager.cpp\
			./src/ClientConnection.cpp\
			./src/HttpRequest.cpp\
			./src/Validator.cpp\
			./src/HttpError.cpp\
			./src/Tools.cpp\
			./src/Route.cpp\
			./src/ConfigParser.cpp\
			./src/RequestProcessor.cpp\
			./src/VirtualHost.cpp\
			./src/HttpResponse.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

# For the 42 norm: phony targets
.PHONY: all clean fclean re
