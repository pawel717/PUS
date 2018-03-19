/*
 * server.c
 *
 *  Created on: 19 mar 2018
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
#include <dirent.h>

#define REENTRANT
#define MAX_THREADS 4

int server_fd;
char * page_HTML;

void CloseServer();
static void Error(char desc[]);
char * createHTMLpage();
void* handleHTTPRequest(void * context);
int checkNewConnection();

struct context
{
	//data needed to process http request
	int client;
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

	page_HTML = createHTMLpage();

	struct sockaddr_in server_addr;

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

		struct context context;
		//add necesarry data to struct context
		context.client = checkNewConnections();

		if (pthread_create(&thread_id, NULL, &handleHTTPRequest, (void*)&context) == -1)
			Error("[SERWER]: Błąd! Nie utowrzono wątku...\n");
	}
}

int checkNewConnection()
{
	char src[INET_ADDRSTRLEN];
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	int client_length = sizeof(client_addr);

	int client = accept(server_fd, (struct sockaddr *)&client_addr, &client_length);

	if(client == -1)
		Error("[SERWER]: Nie udało się zaakceptować połaczenia");

	return client;
}

void* handleHTTPRequest(void * arg)
{
	struct context * context = (struct context *) arg;

	char request[2000];

	int received_bytes;

	received_bytes = recv(server_fd, &request, sizeof(request), 0);
	//check if it is proper http request

	//if GET demands image -> url starts with /img/ and ends with .jpeg, .jpg, .png, .gif?

	//send image as respone

	//else send header+page_HTML

	send(context->client, page_HTML, sizeof(page_HTML));

	return 0;
}

char * createHTMLpage()
{
	char * page_HTML;
	DIR * directory;
	struct dirent * image;

	strcat(page_HTML, "<html><body><center>\n");

	if((directory = opendir("img/")) != NULL)
	{
		while((image = readdir(directory)) != NULL)
		{
			strcat(page_HTML ,"<img src=”");
			strcat(page_HTML, image->d_name);
			strcat(page_HTML, "”></img><br/>\n");
		}
	}

	strcat(page_HTML, "</center></body></html> ");

	return page_HTML;
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
