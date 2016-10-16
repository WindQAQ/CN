import urllib.request
from bs4 import BeautifulSoup

SearchPrefix = {
	'youtube': 'https://www.youtube.com/results?search_query=',
	'ruten': 'http://search.ruten.com.tw/search/s000.php?enc=u&searchfrom=searchf&t=0&k='
}

SearchClass = {
	'youtube': 'yt-uix-tile-link',
	'ruten': 'item-name-anchor'
}

def formatUrl(action, query):
	prefix = SearchPrefix[action]
	return '{}{}'.format(prefix, query)

def getHTML(url):
	with urllib.request.urlopen(url) as res:
		html = res.read()
		soup = BeautifulSoup(html, 'lxml')
		return soup

def findItem(action, soup):	
	for match in soup.find_all('a', class_=SearchClass[action]):
		if 'https://googleads' in  match['href']:
			continue	
		link = '{}{}'.format(SearchPrefix[action], match['href']) if action == 'youtube' else match['href']
		return '{} {}'.format(match['title'], link)
	return None

def Search(action, query):
	url = formatUrl(action, query)
	soup = getHTML(url)
	item = findItem(action, soup)
	print(item)
	return item

# print(Search('youtube', 'PPAP'))
