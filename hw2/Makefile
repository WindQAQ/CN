CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent receiver
HEADER=utility.h packet.h agent.h

all: $(TARGET)

sender: sender.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

receiver: receiver.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)
