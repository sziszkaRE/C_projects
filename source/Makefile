all: cuncurrentWebCrawler.exe

install_curl:
	../

cuncurrentWebCrawler.exe: cuncurrentWebCrawler.o
	gcc cuncurrentWebCrawler.o /usr/local/lib/libcurl.a -o cuncurrentWebCrawler.exe
	echo All done.

cuncurrentWebCrawler.o:
	gcc -v -c -g3 cuncurrentWebCrawler.c

clean:
	rm cuncurrentWebCrawler.o cuncurrentWebCrawler.exe
	echo Clean done.
	
test:
	./cuncurrentWebCrawler.exe http://hu.wikipedia.org/wiki/Blog 2
	

