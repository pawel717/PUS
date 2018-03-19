/*
 * server.c
 *
 *  Created on: 15 mar 2018
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
#include "libpalindrome.h"


#define REENTRANT
#define MAX_THREADS 4

int server_fd;

void CloseServer();
void *ProcessClient(void *arg);
static void Error(char desc[]);

struct context
{
	struct sockaddr_in client_addr;
	int client_length;
	char message[256];
};

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

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1)
    	Error("[SERWER]: Nie moglem utworzyc gniazda.\n");
    else
    	printf("[SERWER]: Gniazdo zostalo stworzone %d.\n",server_fd);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		Error("[SERWER]: Nie moge nadac nazwy gniazdu.\n");
    else
    	printf("[SERWER]: Gniazdo otrzymalo swoja nazwe.\n");

	printf("[SERWER]: Oczekuje na klientów...\n");

	while (1)
	{
		pthread_t thread_id;
		char src[INET_ADDRSTRLEN];
		char message[256] = "";
		int received_bytes;

		received_bytes = recvfrom(server_fd, message, sizeof(message), 0, (struct sockaddr *)&client_addr, &client_length);

		if(received_bytes == -1)
			Error("Nie udało się odebrać danych");
		else
			printf("[SERWER]: Nowe połaczenie od klienta (adres: %s:%d)\n",
							inet_ntop(client_addr.sin_family, &client_addr, src, sizeof(src)),
							client_addr.sin_port);

		printf("[SERWER]: Odebrano: %s\n", message);

		if(strlen(message) == 0)
			CloseServer();

		struct context context;
		context.client_addr = client_addr;
		context.client_length = client_length;
		strncpy(context.message, message, strlen(message));

		if (pthread_create(&thread_id, NULL, &ProcessClient, (void*)&context) == -1)
			Error("[SERWER]: Błąd! Nie utowrzono wątku...\n");

	}
}

void *ProcessClient(void *arg)
{
	struct context *context = (struct context*) arg;
	char* response;

	if(is_palindrome(context->message, strlen(context->message)) == 1)
		response = "Dane sa palindromem";
	else
		response = "Dane nie sa palindromem";

	sendto(server_fd, response, strlen(response), 0, (struct sockaddr *)&context->client_addr, context->client_length);

	return NULL;

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


