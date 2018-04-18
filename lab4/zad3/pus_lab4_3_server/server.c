/*
 * server.c
 *
 *  Created on: 15 kwi 2018
 *      Author: root
 */

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <netinet/sctp.h>

void CloseServer();
static void Error(char desc[]);

int sock_fd;
int client_fd;

int main(int argc, char* argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Wywołanie: %s <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

	signal(SIGINT, CloseServer);

	struct sctp_initmsg initmsg;

	// MSG
	struct sctp_sndrcvinfo 			sndrcvinfo;
	struct sctp_event_subscribe 	events;
    struct sctp_status      		status;

    memset (&initmsg, 0, sizeof (initmsg));
    initmsg.sinit_num_ostreams = 2;
    initmsg.sinit_max_instreams = 2;

    struct sockaddr_in server_addr;

    int server_port = atoi(argv[1]);

    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    sock_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

    if (sock_fd == -1)
    	Error("[SERWER]: Nie moglem utworzyc gniazda.\n");
    else
    	printf("[SERWER]: Gniazdo zostalo stworzone %d.\n",sock_fd);

    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		Error("[SERWER]: Nie moge nadac nazwy gniazdu.\n");
    else
    	printf("[SERWER]: Gniazdo otrzymalo swoja nazwe.\n");

    if(setsockopt(sock_fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof (initmsg)) != 0)
		Error("[SERWER]: Błąd przy ustawianiu opcji SCTP_INITMSG");


    if (listen(sock_fd, SOMAXCONN) == -1)
		Error("[SERWER]: Nie moge utworzy kolejki.\n");
    else
    	printf("[SERWER]: Kolejka zostala utworzona.\n");

	printf("[SERWER]: Oczekuje na klientów...\n");

	memset (&events, 0, sizeof (events));
    events.sctp_data_io_event = 1;
    int retval = setsockopt(sock_fd, SOL_SCTP, SCTP_EVENTS,(const void *) &events, sizeof (events));

    int slen = sizeof(status);
    retval = getsockopt(sock_fd, SOL_SCTP, SCTP_STATUS,(void *) &status, (socklen_t *) & slen);
    

	while(1)
	{

		// char src[INET_ADDRSTRLEN];
		// struct sockaddr_in client_addr;
		// memset(&client_addr, 0, sizeof(client_addr));
	    // int client_length = sizeof(client_addr);

		// client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_length);

		/* if(client_fd == -1)
			Error("[SERWER]: Nie udało się zaakceptować połaczenia");
		else
			printf("[SERWER]: Nowe połaczenie od klienta (adres: %s:%d)\n",
					inet_ntop(client_addr.sin_family, &client_addr, src, sizeof(src)),
					client_addr.sin_port);
		*/
			
		// MSG
		char message[256];
        retval = sctp_recvmsg(sock_fd, (void *) message, sizeof(message),(struct sockaddr *) NULL, 0, &sndrcvinfo, NULL);
		
		printf("[SERWER]: MSG: %s \n", message);

		
		/* memset(message, '\0', sizeof(message));

		time_t t = time(NULL);
		struct tm current_time = *localtime(&t);

		sprintf(message, "Data: %d/%d/%d", current_time.tm_mday, current_time.tm_mon+1, current_time.tm_year+1900);

		if(sctp_sendmsg(client_fd, message, strlen(message), NULL, 0, 0, 0, 0, 0, 0) == -1)
			Error("[SERWER]: Błąd wysyłania");

		printf("[SERWER]: Wysłano date\n");

		memset(message, '\0', sizeof(message));
		sprintf(message, "Czas: %d:%d:%d", current_time.tm_hour, current_time.tm_min, current_time.tm_sec);

		if(sctp_sendmsg(client_fd, message, strlen(message), NULL, 0, 0, 0, 1, 0, 0) == -1)
			Error("[SERWER]: Błąd wysyłania");

		printf("[SERWER]: Wysłano czas\n");
		*/

	}

	CloseServer();

}

void CloseServer()
{
	printf("[SERWER] Zamykanie serwera...\n");
   	close(sock_fd);
   	close(client_fd);
  	exit(0);
}

static void Error(char desc[])
{
	printf("%s\n", desc);
	perror(NULL);
	exit(EXIT_FAILURE);
}
