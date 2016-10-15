import sys, os

LeftParenthesis, RightParenthesis = '(', ')'

OPERATOR = {
	'(': True,
	')': True,
	'+': True,
	'-': True,
	'*': True,
	'/': True,
	'^': True
}

PRIORITY = {
	'(': 0,
	'+': 1,
	'-': 1,
	'*': 2,
	'/': 2,
	'P': 3,
	'N': 3,
	'^': 4
}

UNARY = {
	'+': 'P',
	'-': 'N'
}

def Str2Number(str):
	try:
		return int(str)
	except ValueError:
		try:
			return float(str)
		except ValueError:
			raise Exception('SYNTAX ERROR')

def doOp(op, operand):
	if op == 'P' or op == 'N':
		if not operand:
			raise Exception('SYNTAX ERROR')
		else:
			operand[-1] = operand[-1] if op == 'P' else -operand[-1]
	else:
		if len(operand) < 2:
			raise Exception('SYNTAX ERROR')
		else:
			top = operand.pop()
			if op == '+':
				operand[-1] += top
			elif op == '-':
				operand[-1] -= top
			elif op == '*':
				operand[-1] *= top
			elif op == '/':
				try:
					operand[-1] /= top
				except:
					raise Exception('DIVISION BY ZERO')
			elif op == '^':
				operand[-1] **= top
			else:
				raise Exception('SYNTAX ERROR')

def processOp(op, operator, operand):
	while operator and PRIORITY[op] <= PRIORITY[operator[-1]]:
		if PRIORITY[op] == 3 and PRIORITY[operator[-1]]:
			break
		if PRIORITY[op] == 4 and PRIORITY[operator[-1]] == 4:
			break
		top = operator.pop()
		doOp(top, operand)
	operator.append(op)

def processPa(operator, operand):
	flag = False
	while operator:
		top = operator.pop()
		if top == LeftParenthesis:
			flag = True
			break
		else:
			doOp(top, operand)
	if not flag:
		raise Exception('UNBALANCED PARENTHESES')

def parseExp(exp):
	# exp = exp.replace(' ', '')
	length = len(exp)
	expParse = []
	i = 0
	flag = False
	while i < length:
		if exp[i] == ' ':
			i += 1
			continue
		elif exp[i].isdigit() or exp[i] == '.':
			number = ''
			while (i < length) and (exp[i].isdigit() or exp[i] == '.'):
				number += exp[i]
				i += 1
			i -= 1
			expParse.append(Str2Number(number))
		elif exp[i] in OPERATOR:
			if exp[i] == '+' or exp[i] == '-':
				expParse.append(UNARY[exp[i]] if not flag or expParse[-1] in PRIORITY else exp[i])
			else:
				expParse.append(exp[i])
		else:
			raise Exception('SYNTAX ERROR')

		i += 1
		flag = True

	return expParse

def evalExp(exp):
	expParse = parseExp(exp)
	operator, operand = [], []
	for s in expParse:
		if s == LeftParenthesis:
			operator.append(s)
		elif s == RightParenthesis:
			processPa(operator, operand)
		elif s in PRIORITY:
			processOp(s, operator, operand)
		else:
			operand.append(s)
	
	while operator:
		doOp(operator.pop(), operand)

	if len(operand) != 1:
		raise Exception('SYNTAX ERROR')

	return operand[-1]

# exp = '((2^-2^-2^2^(+-+-2*42/4567^2)-791314/798746)+134)'
# print(parseExp(exp))
# print("my: ")
# evalExp(exp)
# print("correct: ")
# check = exp.replace('^', '**')
# command = 'python3 -qc \"print(' + check + ')\"'
# os.system(command)