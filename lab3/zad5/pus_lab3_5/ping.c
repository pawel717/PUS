/*
 * ping.c
 *
 *  Created on: 25 mar 2018
 *      Author: root
 */


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
#include <netinet/ip_icmp.h>
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
struct addrinfo *iterator;

void Close();
static void Error(char desc[]);

void sendICMP();
void receiveICMPReply();

int main(int argc, char *argv[ ])
{
    if (argc != 2)
    {
        fprintf(stderr, "Wywołanie: %s <adres IP lub nazwa domenowa>\n", argv[0]);
        system("pause");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, Close);

    srand(time(NULL));

    char * server_name = argv[1];

    struct addrinfo hints;

	struct addrinfo * result_addresses;

	int ttl = 128;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         =       AF_INET;
    hints.ai_socktype       =       SOCK_RAW;
    hints.ai_protocol       =       IPPROTO_ICMP;

    if(getaddrinfo(server_name, NULL, &hints, &result_addresses) != 0)
    	Error("Error getaddrinfo()");


    for(iterator=result_addresses; iterator!=NULL; iterator = iterator->ai_next)
    {
    	sock_fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);

    	if(sock_fd != -1)
    	   if(setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) != -1)
    		   break;

    	close(sock_fd);
    }

    if (iterator == NULL)
        Error("Nie moglem utworzyc gniazda.\n");
    else
    	printf("Gniazdo zostalo stworzone %d.\n",sock_fd);

	int ppid = fork();

    if(ppid == 0)
    	receiveICMPReply();

	else
		sendICMP();

	Close();

	exit(EXIT_SUCCESS);
}

void sendICMP()
{
	int i = 0;

    unsigned char datagram[sizeof(struct icmphdr) + 32];
    struct icmphdr *icmp_header = (struct icmphdr *)datagram;
    char data[32];
    char * chars = "abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ123456789";

    memset(icmp_header, 0, sizeof(struct icmphdr));

    icmp_header->type = ICMP_ECHO;
    icmp_header->un.echo.id = htons(getpid());

	for(i=0 ; i<4; i++)
	{
		int j = 0;
	    for(j=0; j<31; j++)
	    	data[j] = chars[rand()%strlen(chars)];

	    data[31] = '\0';

	    memcpy(datagram+sizeof(struct icmphdr), data, sizeof(data));

		icmp_header->un.echo.sequence = htons(i);
        icmp_header->checksum = 0;
        icmp_header->checksum = internet_checksum((unsigned short*)datagram, sizeof(datagram));

        printf("%d\n", sizeof(icmp_header));
        printf("Wysyłanie ICMP Echo Request\n");
		int retval = sendto(sock_fd, datagram, sizeof(datagram), 0,
				           iterator->ai_addr, iterator->ai_addrlen);
		if (retval == -1)
			Error("Błąd przy wysyłaniu");


		sleep(1);
	}

	exit(EXIT_SUCCESS);
}

void receiveICMPReply()
{
	unsigned char datagram[sizeof(struct icmphdr) + 32];

	struct ip *ipheader = (struct ip*) datagram;

	struct icmphdr *icmp_header = (struct icmphdr *) (datagram + sizeof(struct ip));

	struct sockaddr_in address;

	int i = 0;

	for(i=0 ;i<4; i++)
	{
		recvfrom(sock_fd, datagram, sizeof(datagram), 0, (struct sockaddr *)&address, sizeof(address));

        printf("Pakiet %d\n", i);
        printf("Adres źródłowy: %s\n", inet_ntoa(ipheader->ip_src));
        printf("TTL: %d\n", ipheader->ip_ttl);
        printf("Długość nagłówka: %d\n", ipheader->ip_hl);
        printf("Adres docelowy: %s\n", inet_ntoa(ipheader->ip_dst));
        printf("Type: %d\n", (int)icmp_header->type);
        printf("Code: %d\n", (int)icmp_header->code);
        printf("ID: %d\n", icmp_header->un.echo.id);
        printf("Sequence: %d\n\n", icmp_header->un.echo.sequence);
	}

	exit(EXIT_SUCCESS);
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


