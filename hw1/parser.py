import urllib.request, urllib.parse
from bs4 import BeautifulSoup

YOUTUBE = 'https://www.youtube.com'

SearchPrefix = {
	'youtube': 'https://www.youtube.com/results?search_query=',
}

SearchClass = {
	'youtube': 'yt-uix-sessionlink yt-uix-tile-link yt-ui-ellipsis yt-ui-ellipsis-2 spf-link ',
}

def formatUrl(action, query):
	query = urllib.parse.quote(query)
	prefix = SearchPrefix[action]
	return '{}{}'.format(prefix, query)

def findItem(action, soup):
	match = soup.find('a', class_=SearchClass[action])
	if action == 'youtube':
		# if not match['href'].startswith('/watch?v='):
		title, link = match['title'], '{}{}'.format(YOUTUBE, match['href'])
		return '{} {}'.format(title, link)
	return None

def Search(action, query):
	try:
		url = formatUrl(action, query)
		with urllib.request.urlopen(url) as res:
			html = res.read()
			soup = BeautifulSoup(html, 'lxml')
			item = findItem(action, soup)
			return item
	except:
		return 'No corresponding item.'
