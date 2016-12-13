CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent agent-drop receiver
HEADER=utility.h packet.h agent.h

INPUT=input.txt
OUTPUT=output.txt

DESTPORT=12346
SRCPORT=12345
LOCAL_HOST=127.0.0.1

THRES=16
BUFFER_SIZE=32

DROP_FLAG=DROP
LOSS_PROB=0.5

all: $(TARGET)

sender: sender.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent-drop: agent.c $(HEADER)
	$(CC) $(CFLAGS) -D $(DROP_FLAG) $< -o $@

receiver: receiver.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

runsender: sender
	./$< -srcPort $(SRCPORT) -destPort $(DESTPORT) -destIP $(LOACL_HOST) -file $(INPUT) -thres $(THRES)

runagent: agent
	./$<

runagent-drop: agent-drop
	./$< -loss_prob $(LOSS_PROB)

runreceiver: receiver
	./$< -port $(DESTPORT) -file $(OUTPUT) -buffer $(BUFFER_SIZE)

clean:
	rm -rf $(TARGET)
