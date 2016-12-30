CC=gcc
CFLAGS=-O2 -std=c99

TARGET=sender agent receiver sender-MPTCP agent-MPTCP
HEADER=utility.h packet.h

INPUT=input/input.txt
OUTPUT=output/output.txt

DESTPORT=12346
SRCPORT=12345
AGENTPORT=12040

DESTIP=127.0.0.1
SRCIP=127.0.0.1
AGENTIP=127.0.0.1

THRES=16
BUFFER_SIZE=32

LOSS_PROB=0.1

SUBFLOW=2
AGENTPORT_MPTCP=12040 12041

all: $(TARGET)

sender: sender.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent: agent.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

receiver: receiver.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

sender-MPTCP: sender-MPTCP.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

agent-MPTCP: agent-MPTCP.c $(HEADER)
	$(CC) $(CFLAGS) $< -o $@ -lpthread

runsender: sender
	./$< -srcPort $(SRCPORT) -srcIP $(SRCIP) -agentPort $(AGENTPORT) -agentIP $(AGENTIP) -destPort $(DESTPORT) -destIP $(DESTIP) -file $(INPUT) -thres $(THRES)

runagent: agent
	./$< -ip $(AGENTIP) -port $(AGENTPORT) -loss_prob $(LOSS_PROB)

runreceiver: receiver
	./$< -ip $(DESTIP) -port $(DESTPORT) -file $(OUTPUT) -buffer $(BUFFER_SIZE)

runsender-MPTCP: sender-MPTCP
	./$< -srcPort $(SRCPORT) -subflow_num $(SUBFLOW) -agentPort $(AGENTPORT_MPTCP) -agentIP $(AGENTIP) -destPort $(DESTPORT) -destIP $(DESTIP) -file $(INPUT) -thres $(THRES)

runagent-MPTCP: agent-MPTCP
	./$< -subflow_num $(SUBFLOW) -ip $(AGENTIP) -port $(AGENTPORT_MPTCP) -loss_prob $(LOSS_PROB)
	
clean:
	rm -rf $(TARGET)
