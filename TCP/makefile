make all:
	g++ -std=c++11 -o servermain servermain.cpp
	g++ -std=c++11 -o client client.cpp
# 	./servermain
# 	./client
# 	./client

servermain: servermain.o
	g++ -std=c++11 servermain.o -o servermain
# 	./servermain

client: client.o
	g++ -std=c++11 client.o -o client
# 	./client

servermain.o : servermain.cpp
	g++ -std=c++11 -c servermain.cpp		

client.o : client.cpp
	g++ -std=c++11 -c client.cpp



clean:
	rm -f *.o
	$(RM) servermain client