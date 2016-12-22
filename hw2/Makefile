CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent receiver
HEADER=utility.h packet.h agent.h

INPUT=input/input.txt
OUTPUT=output/output.txt

DESTPORT=12346
SRCPORT=12345
AGENTPORT=12040

DESTIP=127.0.0.1
AGENTIP=127.0.0.1

THRES=16
BUFFER_SIZE=32

LOSS_PROB=0.1

all: $(TARGET)

sender: sender.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

receiver: receiver.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

runsender: sender
	./$< -srcPort $(SRCPORT) -agentPort $(AGENTPORT) -agentIP $(AGENTIP) -destPort $(DESTPORT) -destIP $(DESTIP) -file $(INPUT) -thres $(THRES)

runagent: agent
	./$< -ip $(AGENTIP) -port $(AGENTPORT) -loss_prob $(LOSS_PROB)

runreceiver: receiver
	./$< -port $(DESTPORT) -file $(OUTPUT) -buffer $(BUFFER_SIZE)

clean:
	rm -rf $(TARGET)
