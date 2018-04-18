/*
 * client.c
 *
 *  Created on: 15 kwi 2018
 *      Author: root
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h> 
#include <errno.h>
#include <netdb.h>
#include <netinet/sctp.h>

static void Error(char desc[]);
void CloseClient();

int sock_fd;

int main(int argc, char* argv[])
{

	if(argc != 3)
	{
		 fprintf(stderr, "Wywołanie: %s <IPv6 ADDRESS> <PORT>\n", argv[0]);
		 system("pause");
		 exit(EXIT_FAILURE);
	}

	signal(SIGINT, &CloseClient);

	int server_port = atoi(argv[2]);
	char* server_address = argv[1];

	struct addrinfo address_info;
    struct addrinfo *result_addresses;

    struct sctp_sndrcvinfo sndrcvinfo;

    struct sctp_status status;
    int status_len = sizeof(status);

    struct sctp_initmsg initmsg;
    initmsg.sinit_max_attempts = 5;
    initmsg.sinit_max_instreams = 1;
    initmsg.sinit_num_ostreams = 4;

    memset(&address_info, 0, sizeof(struct addrinfo));
    address_info.ai_family = AF_UNSPEC;
    address_info.ai_socktype = SOCK_STREAM;
    address_info.ai_flags = 0;
    address_info.ai_protocol = IPPROTO_SCTP;

    if(getaddrinfo(argv[1], argv[2],  &address_info, &result_addresses) == -1)
    {
		fprintf(stderr, "getaddrinfo() failed\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo *iterator;
    for(iterator = result_addresses; iterator!=NULL; iterator = iterator->ai_next)
    {
    	sock_fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

    	if(sock_fd != -1)
    	{
    	    if(setsockopt(sock_fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof (initmsg)) != 0)
    	    	Error("[KLIENT]: Błąd przy ustawianiu opcji SCTP_INITMSG");

    		// if(connect(sock_fd, iterator->ai_addr, iterator->ai_addrlen) != -1)
    		break;
    	}

    	close(sock_fd);
    }

	if (iterator == NULL)
		Error("[KLIENT]: Nie moglem utworzyc gniazda.\n");

	freeaddrinfo(result_addresses);

	struct sctp_event_subscribe s_events;

    memset (&s_events, 0, sizeof (s_events));
    s_events.sctp_data_io_event = 1;
    setsockopt (sock_fd, SOL_SCTP, SCTP_EVENTS,(const void *) &s_events, sizeof (s_events));

	// print association id, association state and number of in and out streams getsockopt(SCTP_STATUS)
    getsockopt(sock_fd, SOL_SCTP, SCTP_STATUS,(void *) &status, (socklen_t *) &status_len);

	printf ("[KLIENT]: Id asocjacji %d\n", status.sstat_assoc_id);
	printf ("[KLIENT]: Stan asocjacji: %d\n", status.sstat_state);
	printf ("[KLIENT]: Ilość strumieni wychodzacych: %d\n", status.sstat_outstrms);
	printf ("[KLIENT]: Ilość strumieni przychodzacych: %d\n", status.sstat_instrms);

	// MSG
	char message[256];
    memset(message, '\0', sizeof(message) );
	
	sprintf(message, "Test");
	
	if(sctp_sendmsg(sock_fd, message, strlen(message), NULL, 0, 0, 0, 0, 0, 0) == -1)
		Error("[SERWER]: Błąd wysyłania");

	// receive date and time from different streams
	/* int i;
    for (i = 0; i <2; i++)
    {
    	char message[256];
    	memset(message, '\0', sizeof(message) );

        int retval = sctp_recvmsg(sock_fd, (void *) message, sizeof(message),(struct sockaddr *) NULL, 0, &sndrcvinfo, NULL);

        if (retval > 0)
            printf ("[KLIENT]: Strumień %d odebrano: %s\n", sndrcvinfo.sinfo_stream, message);

    }
	*/
	// then terminate

	CloseClient();
	return 0;
}

static void Error(char desc[])
{
	printf("%s", desc);
	printf("%d", errno);
	exit(EXIT_FAILURE);
}

void CloseClient()
{
	printf("[KLIENT] Zamykanie klienta...\n");
   	close(sock_fd);
  	exit(0);
}
