all: client server

client: Administrator.c
	gcc Administrator.c -o client

server: Record-Keeper.c
	gcc Record-Keeper.c -o server
