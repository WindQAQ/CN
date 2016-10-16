import urllib.request, urllib.parse
from bs4 import BeautifulSoup

YOUTUBE = 'https://www.youtube.com'

SearchPrefix = {
	'youtube': 'https://www.youtube.com/results?search_query=',
}

SearchClass = {
	'youtube': 'yt-uix-tile-link',
}

def formatUrl(action, query):
	query = urllib.parse.quote(query)
	prefix = SearchPrefix[action]
	return '{}{}'.format(prefix, query)

def getHTML(url):
	with urllib.request.urlopen(url) as res:
		html = res.read()
		soup = BeautifulSoup(html, 'lxml')
		return soup

def findItem(action, soup):	
	for match in soup.find_all('a', class_=SearchClass[action], limit=2):
		if action == 'youtube':
			if 'https://googleads' in  match['href']:
				continue
			title, link = match['title'], '{}{}'.format(YOUTUBE, match['href'])
		return '{} {}'.format(title, link)
	return None

def Search(action, query):
	try:
		url = formatUrl(action, query)
		soup = getHTML(url)
		item = findItem(action, soup)
		return item
	except:
		return 'No corresponding item'
