CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -Iinclude
LIBS = -lssl -lcrypto

# Directories
SRCDIR = src
INCDIR = include
LIBDIR = lib
BUILDDIR = build

# Source files
SERVER_SRC = $(SRCDIR)/vpn_server.c $(SRCDIR)/crypto.c $(SRCDIR)/network.c $(SRCDIR)/tun.c
CLIENT_SRC = $(SRCDIR)/vpn_client.c $(SRCDIR)/crypto.c $(SRCDIR)/network.c $(SRCDIR)/tun.c

# Object files
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

# Targets
SERVER_TARGET = vpn-server
CLIENT_TARGET = vpn-client

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) -o $@ $(LIBS)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_TARGET) $(CLIENT_TARGET)

install: all
	cp $(SERVER_TARGET) /usr/local/bin/
	cp $(CLIENT_TARGET) /usr/local/bin/

.SUFFIXES: .c .o