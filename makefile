all:
	clear
	g++ server.cpp -o server -lsqlite3
	g++ client.cpp -o client -lsqlite3
cl:
	clear
	./client 10.0.2.15 2024
rm:
	clear
	rm -f server client
