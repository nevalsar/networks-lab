run: master
	./master 0

master:	master.cpp Node.o Socket.o
	g++ -std=c++11 -o master master.cpp

Node.o:	Node.cpp
	g++ -std=c++11 -c Node.cpp

Socket.o:	Socket.cpp
	g++ -c Socket.cpp

clean:
	rm -f master Node.o Socket.o
