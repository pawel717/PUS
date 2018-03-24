/*
 * tcp.c
 *
 *  Created on: 23 mar 2018
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
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include "checksum.h"

#ifndef __USE_BSD
#define __USE_BSD
#endif
#define __FAVOR_BSD
#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "192.168.1.10"

struct phdr {
    struct in_addr ip_src, ip_dst;
    unsigned char unused;
    unsigned char protocol;
    unsigned short length;

};

int sock_fd;
int clients_fd[SOMAXCONN];
fd_set read_set;

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

    unsigned short checksum;

    unsigned char segment[sizeof(struct ip) + sizeof(struct tcphdr) + sizeof(struct phdr)] = {0};

    struct ip *ip_header = (struct ip *)segment;

    struct tcphdr *tcp_header = (struct tcphdr *)(segment + sizeof(struct ip));

    struct phdr *pseudo_header = (struct phdr *)(segment + sizeof(struct ip) + sizeof(struct tcphdr));

    struct addrinfo hints;

	struct addrinfo * result_addresses;

	int socket_option = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         =       AF_INET;
    hints.ai_socktype       =       SOCK_RAW;
    hints.ai_protocol       =       IPPROTO_TCP;

    if(getaddrinfo(server_name, NULL, &hints, &result_addresses) != 0)
    	Error("Error getaddrinfo()");

    struct addrinfo *iterator;
    for(iterator=result_addresses; iterator!=NULL; iterator = iterator->ai_next)
    {
    	sock_fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

    	if(sock_fd != -1)
    	   if(setsockopt(sock_fd, IPPROTO_IP, IP_HDRINCL, &socket_option, sizeof(int)) != -1)
    		   break;

    	close(sock_fd);
    }

    if (iterator == NULL)
        Error("Nie moglem utworzyc gniazda.\n");
    else
    	printf("Gniazdo zostalo stworzone %d.\n",sock_fd);


    /********************************/
    /* Wypelnienie pol naglowka IP: */
    /********************************/
    ip_header->ip_hl = 5;
    ip_header->ip_v = 4;
    ip_header->ip_tos = 0;
    ip_header->ip_len = sizeof(struct ip) + sizeof(struct tcphdr);
    ip_header->ip_id = 0;
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 255;
    ip_header->ip_p = IPPROTO_TCP;
    ip_header->ip_src.s_addr = inet_addr(SOURCE_ADDRESS);
    ip_header->ip_dst.s_addr = ((struct sockaddr_in*)iterator->ai_addr)->sin_addr.s_addr;

    /*********************************/
    /* Wypelnienie pol naglowka UDP: */
    /*********************************/

    tcp_header->source = htons(SOURCE_PORT);
    tcp_header->dest = htons(dest_port);
    tcp_header->seq = 0;
    tcp_header->ack_seq = 0;
    tcp_header->window = htons(5840); //65635?
    tcp_header->check = 0;
    tcp_header->urg_ptr = 0;
    tcp_header->doff = 5;
    tcp_header->fin = 0;
    tcp_header->syn = 1;
    tcp_header->rst = 0;
    tcp_header->psh = 0;
    tcp_header->ack = 0;
    tcp_header->urg = 0;

    /************************************/
    /* Wypelnienie pol pseudo-naglowka: */
    /************************************/

    pseudo_header->ip_src.s_addr = ip_header->ip_src.s_addr;
    pseudo_header->ip_dst.s_addr = ip_header->ip_dst.s_addr;
    pseudo_header->unused = 0;
    pseudo_header->protocol = ip_header->ip_p;
    pseudo_header->length = htons(sizeof(struct tcphdr));

    tcp_header->check = 0;
    checksum = internet_checksum( (unsigned short *)tcp_header,
                                  sizeof(struct tcphdr) + sizeof(struct phdr));
    tcp_header->check = (checksum == 0) ? 0xffff : checksum;

	while (1)
	{
		int retval = sendto(sock_fd, segment, ip_header->ip_len,
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
	printf("%s\n", desc);
	exit(EXIT_FAILURE);
}


