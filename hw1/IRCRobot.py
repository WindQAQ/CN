import socket
import random

from evalExp import evalExp
from parser import Search

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
HELP = ['@repeat <String>', '@cal <Expression>', '@play <Robot Name>', '@guess <Integer>', '@youtube <String>']

GAME_START = 'Start! (0-100 with 8 times)'
GAME_OVER = 'GAME OVER! The answer is'
HL = {
	True: 'Higher!',
	False: 'Lower!'
}

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

def play(gamer, user):
	if gamer is not None:
		raise Exception

	res = GAME_START
	return res, user, random.randint(0, 100), 8

def guess(ans, plain, gamer, times):
	if len(plain) != 1:
		raise Exception('Error: The input should contain only a number.')
	try:
		num = int(plain[0])
		times -= 1
		res = ''
		if ans == num:
			res = 'CORRECT!'
			gamer, times = None, 8
		else:
			if times == 0:
				res = '{} {}.'.format(GAME_OVER, ans)
				gamer, times = None, 8
			else:
				res = '{} ({})'.format(HL[ans > num], times)
		return res, gamer, times
	except ValueError:
		raise Exception('Error: {} is not an integer.'.format(plain[0]))

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
		IRCMsg = IRCSocket.recv(Bufsize).decode().strip('\r\n')
		if not IRCMsg:
			continue
		print(IRCMsg)	
		msg = IRCMsg.split()
		# PING-PONG response to server
		if msg[0] == 'PING':
			sendMsg('PONG ' + msg[1])
			print('receive PING')
			continue

		username, IRCCommand = msg[0].split('!', 1)[0][1:], msg[1]
		response = None

		# say hi while joining the channel
		if username == NickName and IRCCommand == 'JOIN':
			response = 'Hi, I am ' + NickName
			username = None
		elif IRCCommand == 'PRIVMSG':
			action = msg[3][1:] # remove ':'
			text = msg[4:]
			if action == '@repeat':
				start = IRCMsg.find('@repeat ')
				response = IRCMsg[start+8:] if start != -1 else None
			elif action == '@cal':
				try:
					response = str(evalExp(' '.join(text)))
				except Exception as err:
					response = str(err)
			elif action == '@play' and len(text) == 1 and text[0] == NickName and gamer is None:
				try:
					response, gamer, randnum, times = play(gamer, username)
				except:
					response = None
			elif action == '@guess' and username == gamer:
				try:
					response, gamer, times = guess(randnum, text, gamer, times)
				except Exception as err:
					response = str(err)
			elif action == '@youtube':
				if not text:
					response = 'ERROR: Please check your Iiput.'
				else:
					response = Search('youtube', '+'.join(text))
			elif action == '@help':
				response = HELP
			
			response = respformat(response, username)
			sendMsg(response)
			print('\n>-------------------------<\n')
			print(IRCMsg)
			print(username, IRCCommand, action)
			print(response)
			print('\n>-------------------------<\n')

if __name__ == '__main__':
	IRCRobot()
