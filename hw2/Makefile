CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent receiver

all: $(TARGET)

sender: sender.c
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c
	$(CC) $(CFLAGS) $< -o $@

receiver: receiver.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)
