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

runsender: sender
	./$< -srcPort 12345 -destPort 12346 -destIP 127.0.0.1 -file input.txt

runreceiver: receiver
	./$< -port 12346 -file output.txt

clean:
	rm -rf $(TARGET)
