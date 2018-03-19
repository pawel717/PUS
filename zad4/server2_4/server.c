/*
 * server.c
 *
 *  Created on: 18 mar 2018
 *      Author: root
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define REENTRANT
#define MAX_THREADS 4

int server_fd;
int clients_fd[SOMAXCONN];
fd_set read_set;

void CloseServer();
static void Error(char desc[]);
void checkNewConnections();
void processMessages();

struct clients
{
	int descriptors[SOMAXCONN];
	int max_descriptor;
	int size;

}clients;

void clients_remove(int index, struct clients *clients)
{

	if(clients->max_descriptor == clients->descriptors[index])
	{
		clients->max_descriptor = server_fd;
		int i;
		for(i=0; i<clients->size; i++)
			if(clients->descriptors[i] > clients->max_descriptor)
				clients->max_descriptor = clients->descriptors[i];
	}

	memmove(&clients->descriptors[index], &clients->descriptors[index+1], sizeof(int)*(clients->size-index));

	clients->size -= 1;

}

int clients_add(int descriptor, struct clients *clients)
{

	if(clients->size+1 < SOMAXCONN)
	{
		if(descriptor > clients->max_descriptor)
			clients->max_descriptor = descriptor;

		clients->descriptors[clients->size] = descriptor;
		clients->size += 1;

		return 1;
	}

	return 0;

}

int main(int argc, char *argv[ ])
{
    if (argc != 2)
    {
        fprintf(stderr, "Wywołanie: %s <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

	signal(SIGINT, CloseServer);

    struct sockaddr_in server_addr;
    clients.size = 0;

    int server_port = atoi(argv[1]);

    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    	Error("[SERWER]: Nie moglem utworzyc gniazda.\n");
    else
    	printf("[SERWER]: Gniazdo zostalo stworzone %d.\n",server_fd);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		Error("[SERWER]: Nie moge nadac nazwy gniazdu.\n");
    else
    	printf("[SERWER]: Gniazdo otrzymalo swoja nazwe.\n");

    if (listen(server_fd, SOMAXCONN) == -1)
		Error("[SERWER]: Nie moge utworzy kolejki.\n");
    else
    	printf("[SERWER]: Kolejka zostala utworzona.\n");

	printf("[SERWER]: Oczekuje na klientów...\n");

	clients.max_descriptor = server_fd;

	while (1)
	{

		FD_SET(server_fd, &read_set);

		int i;
		for(i=0; i<clients.size; i++)
			FD_SET(clients.descriptors[i], &read_set);

		int ready_descriptors = select(clients.max_descriptor+1, &read_set, NULL, NULL, NULL);
		if(ready_descriptors < 0)
			Error("Bład w funkcji select");

		checkNewConnections();
		processMessages();

	}
}

void checkNewConnections()
{
	if (FD_ISSET(server_fd, &read_set))
	{
		char src[INET_ADDRSTRLEN];
		struct sockaddr_in client_addr;
		memset(&client_addr, 0, sizeof(client_addr));
	    int client_length = sizeof(client_addr);

		int client = accept(server_fd, (struct sockaddr *)&client_addr, &client_length);

		if(client == -1)
			Error("[SERWER]: Nie udało się zaakceptować połaczenia");
		else
			printf("[SERWER]: Nowe połaczenie od klienta (adres: %s:%d)\n",
					inet_ntop(client_addr.sin_family, &client_addr, src, sizeof(src)),
					client_addr.sin_port);

		clients_add(client, &clients);
	}
}

void processMessages()
{
	int i;
	for(i=0; i<clients.size; i++)
	{
		if(FD_ISSET(clients.descriptors[i], &read_set))
		{
			char message[256] = "";
			if(recv(clients.descriptors[i], message, sizeof(message), 0) > 0)
			{
				int j;
				for(j=0; j<clients.size; j++)
					if(j != i)
						send(clients.descriptors[j], message, sizeof(message), 0);
			}
			else
			{
				close(clients.descriptors[i]);
				FD_CLR(clients.descriptors[i], &read_set);
				clients_remove(i, &clients);
			}
		}
	}
}

void CloseServer()
{
	printf("[SERWER] Zamykanie serwera...\n");
   	close(server_fd);
  	exit(0);
}

static void Error(char desc[])
{
	printf("%s\n", desc);
	exit(EXIT_FAILURE);
}


