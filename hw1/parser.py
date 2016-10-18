import urllib.request, urllib.parse
from bs4 import BeautifulSoup

YOUTUBE = '{}: https://www.youtube.com{}'
BOOKS = '{}: {}'

SearchPrefix = {
	'youtube': 'https://www.youtube.com/results?search_query={}',
	'books': 'http://search.books.com.tw/search/query/key/{}/cat/all'
}

SearchClass = {
	'youtube': 'yt-uix-sessionlink yt-uix-tile-link yt-ui-ellipsis yt-ui-ellipsis-2 spf-link ',
	'books': 'searchbook'
}

def formatUrl(action, query):
	query = urllib.parse.quote(query)
	prefix = SearchPrefix[action]
	return prefix.format(query)

def findItem(action, soup):
	if action == 'youtube':
		match = soup.find('a', class_=SearchClass[action])
		# if not match['href'].startswith('/watch?v='):
		title, link = match['title'], match['href']
		return YOUTUBE.format(title, link)
	if action == 'books':
		match = soup.find('ul', class_=SearchClass[action]).find('li', class_='item').a
		title, link = match['title'], urllib.request.urlopen(match['href']).geturl()
		return BOOKS.format(title, link)
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
