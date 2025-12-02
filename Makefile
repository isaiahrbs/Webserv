# Nom du programme à créer
NAME = webserv

# Compilateur à utiliser
CXX = c++

# Flags de compilation
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Liste des fichiers source
SRCS = webserv.cpp \
	src/server.cpp \
	src/socket.cpp

# Fichiers objets
OBJS = $(SRCS:.cpp=.o)

# Fichiers d'en-tête
HEADERS = inc/server.hpp \
		  inc/socket.hpp

# Règle par défaut
all: $(NAME)

# Règle pour compiler le programme
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $(OBJS) -o $(NAME)

# Règle pour compiler les fichiers objets
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Règle pour nettoyer les fichiers objets
clean:
	rm -f $(OBJS)

# Règle pour nettoyer tout
fclean: clean
	rm -f $(NAME)

# Règle pour recompiler
re: fclean all

.PHONY: all clean fclean re% 