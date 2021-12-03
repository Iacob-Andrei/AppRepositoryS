all:
	gcc server.c -o server
	gcc client.c -o client
cl:
	./client 10.0.2.15 2024
rm:
	rm -f server client
