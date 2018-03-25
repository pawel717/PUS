/*
 * udp.c
 *
 *  Created on: 24 mar 2018
 *      Author: root
 */

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <errno.h>

#ifndef __USE_BSD
#define __USE_BSD
#endif
#define __FAVOR_BSD
#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "fe80::6dd1:f14c:52e1:2dfa%11"

struct phdr {
    struct in6_addr ip_src, ip_dst;
    unsigned int length;
    unsigned char unused[3];
    unsigned char next;

};

int sock_fd;

void Close();
static void Error(char desc[]);

int main(int argc, char *argv[ ])
{
    if (argc != 3)
    {
        fprintf(stderr, "Wywołanie: %s <adres IP lub nazwa domenowa> <PORT>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, Close);

    char * server_name = argv[1];

    int dest_port = atoi(argv[2]);

    unsigned char datagram[sizeof(struct udphdr) + sizeof(struct phdr)] = {0};

    struct udphdr *udp_header = (struct udphdr *)(datagram);

    struct phdr *pseudo_header = (struct phdr *)(datagram + sizeof(struct udphdr));

    struct addrinfo hints;

	struct addrinfo * result_addresses;

	int offset = 6;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         =       AF_INET6;
    hints.ai_socktype       =       SOCK_RAW;
    hints.ai_protocol       =       IPPROTO_UDP;

    int retval=0;
    if((retval = getaddrinfo(server_name, NULL, &hints, &result_addresses)) != 0)
    	{printf("%d %s\n",retval, strerror(errno));}

    struct addrinfo *iterator;
    for(iterator=result_addresses; iterator!=NULL; iterator = iterator->ai_next)
    {
    	sock_fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

    	if(sock_fd != -1)
    	   if(setsockopt(sock_fd, IPPROTO_IPV6, IPV6_CHECKSUM, &offset, sizeof(int)) != -1)
    		   break;

    	close(sock_fd);
    }

    if (iterator == NULL)
        Error("Nie moglem utworzyc gniazda.\n");
    else
    	printf("Gniazdo zostalo stworzone %d.\n",sock_fd);

    /*********************************/
    /* Wypelnienie pol naglowka UDP: */
    /*********************************/

    udp_header->source = htons(SOURCE_PORT);
    udp_header->dest = htons(dest_port);
    udp_header->len = htons(sizeof(struct udphdr));

    /************************************/
    /* Wypelnienie pol pseudo-naglowka: */
    /************************************/

    inet_pton(AF_INET6, SOURCE_ADDRESS, &(pseudo_header->ip_src));
	inet_pton(AF_INET6, argv[1], &(pseudo_header->ip_dst));
    pseudo_header->unused[0] = 0;
    pseudo_header->unused[1] = 0;
    pseudo_header->unused[2] = 0;
    pseudo_header->next = (unsigned char)IPPROTO_UDP;
    pseudo_header->length = udp_header->len;

	while (1)
	{
		int retval = sendto(sock_fd, datagram, udp_header->len,
			                     0, iterator->ai_addr, iterator->ai_addrlen);

		if (retval == -1)
			Error("Błąd przy wysyłaniu");
		else
			printf("Pakkiet wysłany\n");

		sleep(1);
	}

	Close();
}

void Close()
{
	printf("Zamykanie ...\n");
   	close(sock_fd);
  	exit(0);
}

static void Error(char desc[])
{
	printf("%s %d %s\n", desc, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

