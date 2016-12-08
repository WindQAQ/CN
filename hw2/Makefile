CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent receiver
PACKET=packet.h

all: $(TARGET)

sender: sender.c $(PACKET)
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c $(PACKET)
	$(CC) $(CFLAGS) $< -o $@

receiver: receiver.c $(PACKET)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)
