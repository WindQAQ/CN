import socket
import random

from evalExp import evalExp

IRCSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
IRCServer, IRCPort = 'irc.freenode.net', 6667
NickName = 'WindRobot'

Config = {}
with open('config', 'r') as f:
	for line in f:
		tmp = line.split('=')
		Config[tmp[0]] = tmp[1].strip('\n\'')
Channel = Config['CHAN']
Key = Config['CHAN_KEY']

Bufsize = 4096
PRIVMSG = 'PRIVMSG ' + Channel + ' :'
HELP = ['@repeat <String>', '@cal <Expression>', '@play <Robot Name>', '@guess <Integer>']

def respformat(msg, user=None):
	if msg is None:
		return None
	suffix = '' if user == None else ' (' + user + ')'
	if isinstance(msg, list):
		return [(PRIVMSG + _ + suffix) for _ in msg]
	else:
		return PRIVMSG + msg + suffix

def IRCformat(msg):
	return (msg + '\r\n').encode()

def sendMsg(msg):
	if msg is None:
		return
	if isinstance(msg, list):
		for _ in msg:
			IRCSocket.send(IRCformat(_))
	else:
		IRCSocket.send(IRCformat(msg))

def IRCRobot():
	# connect to server
	IRCSocket.connect((IRCServer, IRCPort))

	# login
	sendMsg('USER {} {} {} {}'.format('robot', IRCServer, 'QAQ', 'WindRobot'))
	sendMsg('NICK {}'.format(NickName))

	# join
	sendMsg('JOIN {} {}'.format(Channel, Key))
	
	gamer, randnum, times = None, None, None

	while True:
		IRCMsg = IRCSocket.recv(Bufsize).decode()
		print(IRCMsg)
		if not IRCMsg:
			continue
		
		msg = IRCMsg.split()
		# PING-PONG response to server
		if msg[0] == 'PING':
			sendMsg('PONG ' + msg[1])
			continue

		username, IRCCommand = msg[0].split('!', 1)[0][1:], msg[1]
		print(username, IRCCommand)
		response = None

		# say hi while joining the channel
		if username == NickName and IRCCommand == 'JOIN':
			response = 'Hi, I am ' + NickName
			username = None
		elif IRCCommand == 'PRIVMSG':
			action = msg[3][1:] # remove ':'
			text = msg[4:]
			if action == '@repeat':
				response = IRCMsg[IRCMsg.find('@repeat')+8:-2]
			elif action == '@cal':
				try:
					response = str(evalExp(' '.join(text)))
				except Exception as err:
					response = str(err)
			elif action == '@play' and len(text) == 1 and text[0] == NickName and gamer == None:
				response = 'Start ! (0-100 with 5 times)'
				gamer, randnum, times = username, random.randint(0, 100), 5
			elif action == '@guess' and username == gamer:
				if len(text) != 1:
					response = 'ERROR - Please check your input'
					continue
				try:
					int(text[0])
					times -= 1
					if int(text[0]) == randnum:
						response = 'CORRECT' + ' (' + str(times) + ')'
						gamer, times = None, 5
					else:
						response = 'Higher' + ' (' + str(times) + ')' if int(text[0]) < randnum else 'Lower' + ' (' + str(times) + ')'
						if times == 0:
							response = 'You lose! The answer is ' + str(randnum) + ' (' + str(times) + ')'
							gamer, times = None, 5
				except ValueError:
					response = 'ERROR: ' + text[0] + ' is not a number'
			elif action == '@help':
				response = HELP
		print(respformat(response, username))
		sendMsg(respformat(response, username))

if __name__ == '__main__':
	IRCRobot()
