myDnsClient: functions.o timeout.o socket.o
	gcc -o myDnsClient functions.o timeout.o socket.o 
clean:
	rm -rf *.o myDnsClient
