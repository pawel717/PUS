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
int checkNewConnections();

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

	CloseServer();
}

int checkNewConnections()
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
	char method[256], url[256], protocol[256];

	int received_bytes;

	received_bytes = recv(context->client, &request, sizeof(request), 0);
	if(received_bytes == -1)
		Error("[SERVER]: Błąd! Nie można odebrać wiadomosci");
	else if(received_bytes == 0)
		Error("[SERVER]: Błąd! Nie otrzymano żadnych danych");

	//parse request
	request[received_bytes] = '\0';
	sscanf(request, "%s %s %s", method, url, protocol);
	method[strlen(method)] = '\0';
	url[strlen(url)] = '\0';
	protocol[strlen(protocol)] = '\0';
	//check if it is proper http request
	if(strcmp(protocol, "HTTP/1.1") == 0)
	{
		char *p=(char*)malloc(strlen(url) - 1);
		memset(p, 0, strlen(p));

		int i;
		for(i = 1; i < strlen(url); i++)
			*(p+i-1) = url[i];

		*(p+strlen(url) - 1) = '\0';

		if(strchr(p,'/') == NULL) // request demands image
		{
			DIR * directory;
			char message[512];
			struct dirent * image;

			strcpy(message, "HTTP/1.0 200 OK\n"
			                       "Content-Type: image\n"
			                       "Content-Length:512\n"
								   "\n" );//strcpy?
			strcat(message, "<html><body><center>");

			if((directory = opendir("img/")) != NULL)
			{
				while((image = readdir(directory)) != NULL)
				{
					if(strstr(image->d_name, p) != NULL)
					{
						strcat(message, "<img src='");
						strcat(message, image->d_name);
						strcat(message, "'/></img><br/>");
					}
				}


			}

			strcat(message, "</center></body></html>");
			message[strlen(message)] = '\0';
			closedir(directory);

			if (write(context->client, page_HTML, strlen(page_HTML)) < 0)
						    Error("[SERVER]: Nie można wysłać wiadomości");

		}
		else // request needs html_page
		{
			if (write(context->client, page_HTML, strlen(page_HTML)) < 0)
			    Error("[SERVER]: Nie można wysłać wiadomości");
		}


	}
	//if GET demands image -> url starts with /img/ and ends with .jpeg, .jpg, .png, .gif?

	//send image as respone


	//else send header+page_HTML

	//add header to

	//send(context->client, page_HTML, sizeof(page_HTML));

	return 0;
}

char * createHTMLpage()
{
	char page_HTML[512];
	DIR * directory;
	struct dirent * image;

	//add header
	strcat(page_HTML, "HTTP/1.0 200 OK\n"
                       "Content-Type: image\n"
                       "Content-Length:512\n"
					   "\n" );//strcpy?
	strcat(page_HTML, "<html><body><center>\n");

	if((directory = opendir("img/")) != NULL)
	{

		while((image = readdir(directory)) != NULL)
		{
			 if (strstr(image->d_name, ".png") != NULL
			     || strstr(image->d_name, ".jpg") != NULL
				 || strstr(image->d_name, ".jpeg") != NULL
				 || strstr(image->d_name, ".gif") != NULL)
			{
				strcat(page_HTML ,"<img src=”");
				strcat(page_HTML, image->d_name);
				strcat(page_HTML, "”></img><br/>\n");
			}
		}
	}

	strcat(page_HTML, "</center></body></html> ");
	page_HTML[strlen(page_HTML)] = '\0';

	close(directory);

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
