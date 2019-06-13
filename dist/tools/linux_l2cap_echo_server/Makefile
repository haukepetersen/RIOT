LIBS += -lbluetooth -lcurses
CFLAGS += -Wextra -Wall -O3

all: linux_l2cap_echo_server scanner

linux_l2cap_echo_server: linux_l2cap_echo_server.c
	$(CC) $(CFLAGS) linux_l2cap_echo_server.c $(LIBS) -o linux_l2cap_echo_server

scanner: scanner.c
	$(CC) $(CFLAGS) scanner.c $(LIBS) -o scanner

clean:
	rm -f linux_l2cap_echo_server scanner
