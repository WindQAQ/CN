CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent receiver
HEADER=utility.h packet.h agent.h

INPUT=input.txt
OUTPUT=output.txt

DESTPORT=12346

all: $(TARGET)

sender: sender.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

receiver: receiver.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

runsender: sender
	./$< -srcPort 12345 -destPort $(DESTPORT) -destIP 127.0.0.1 -file $(INPUT)

runagent: agent
	./$<

runreceiver: receiver
	./$< -port $(DESTPORT) -file $(OUTPUT)

clean:
	rm -rf $(TARGET)
