/*
 * server.c
 *
 *  Created on: 11 mar 2018
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

void CloseServer();
void *ProcessClient(void *arg);
static void Error(char desc[]);

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
    struct sockaddr_in client_addr;

    int client_length = sizeof(client_addr);
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

	while (1)
	{
		pthread_t thread_id;
		long int client = accept(server_fd, (struct sockaddr *)&client_addr, &client_length);

		if(client == -1)
			Error("[SERWER]: Błąd! Nie udało się zaakceptować połączenia od klienta");

		char src[INET_ADDRSTRLEN];
		printf("[SERWER]: Nowe połaczenie od klienta (adres: %s:%d)\n",
				inet_ntop(client_addr.sin_family, &client_addr, src, sizeof(src)),
				client_addr.sin_port);

		if (pthread_create(&thread_id, NULL, &ProcessClient, (void*)(client)) == -1)
			Error("[SERWER]: Błąd! Nie utowrzono wątku...\n");

		//pthread_join(tempID[i], NULL);
	}
}

void *ProcessClient(void *arg)
{
	long int client = (long int) arg;

	char * message = "Laboratorium PUS";
	send(client, message, strlen(message), 0);

	close(client);
	return NULL;
	//pthread_exit(NULL);
}

void CloseServer()
{
	printf("[SERWER] Zamykanie serwera...\n");
   	close(server_fd);
  	exit(0);
}

static void Error(char desc[])
{
	printf("%s/n", desc);
	exit(EXIT_FAILURE);
}


