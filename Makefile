CC=gcc

FTPClient: FTPClient.c
	$(CC) -g -Wall FTPClient.c -o FTPClient -lnsl -lm
