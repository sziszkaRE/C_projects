This is a basic implementation of a concurrent web crawler written in C. 
Code use curl. More info about curl: http://curl.haxx.se/

To be able to compile, libcurl must be placed /usr/local/lib/libcurl.a.
Header files used by libcurl.a must be placed /usr/local/include/curl. 

This version of the program is built under windows using cygwin. But it can be easily built under linux. In order to do this libcurl must be recompiled.
Read how_to_install_libcurl.txt. (under curl_source folder)

To compile the program use "make all" command.

Example usage of the program:

./cuncurrentWebCrawler.exe www.example.com 2

or use

make test

