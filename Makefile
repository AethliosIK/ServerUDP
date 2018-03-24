CC = gcc
CFLAGS = -Wall -Wconversion -Werror -Wextra -std=gnu11 -pedantic -pthread
OBJECTS_SERVER = serverUDP.o user_set.o user.o history.o utils.o
OBJECTS_CLIENT = clientUDP.o utils.o
EXEC_SERVER = serverUDP
EXEC_CLIENT = clientUDP

all: $(EXEC_CLIENT) $(EXEC_SERVER)

$(EXEC_SERVER): $(OBJECTS_SERVER)
	$(CC) $(CFLAGS) $(OBJECTS_SERVER) $(LDFLAGS) -o $(EXEC_SERVER)

$(EXEC_CLIENT): $(OBJECTS_CLIENT)
	$(CC) $(CFLAGS) $(OBJECTS_CLIENT) $(LDFLAGS) -o $(EXEC_CLIENT)

clean:
	$(RM) $(OBJECTS_SERVER) $(OBJECTS_CLIENT)

fclean: clean
	$(RM) $(EXEC_SERVER) $(EXEC_CLIENT)
