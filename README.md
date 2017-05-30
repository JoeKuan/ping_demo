A simple ping request/reply demo using SOCK_DGRAM showing kernel modified the identifier before it sent.

For compiling on OSX, gcc -Wall -D_OSX_ ping.c -o pingdemo

For compiling on Linux, gcc -Wall ping.c -o pingdemo
