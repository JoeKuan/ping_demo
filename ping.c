#include <stdio.h>
#include <stdlib.h>

#ifdef _OSX_
#include <malloc/malloc.h>
#include <netinet/ip.h>
#else
#include <malloc.h>
#endif 

#include <string.h>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#define ipv4_hl(a) ((*a & 0x0F) << 2)
#define _icmp_type(a) (*a)
#define _icmp_id(a) *((unsigned short *)(a + 4))
#define _icmp_cksum(a) *((unsigned short *)(a + 2))
/**
 * Code from https://www.cs.utah.edu/~swalton/listings/sockets/programs/part4/chap18/myping.c
 */
unsigned short checksum(void *b, int len)
{	unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;
    
    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char** argv)
{
    struct sockaddr_in s_in, r_in;
    
#ifdef _OSX_
    struct icmp icmp_hdr;
#else
    struct icmphdr icmp_hdr;
#endif
    char packet[sizeof(icmp_hdr) + 5];
    struct hostent *he;
    unsigned char reply[1024];
    unsigned char *icmp_buf;
    int in_size, bytes, sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    int ident = 6789;

    if(sock < 0) {
        perror("Socket error");
	exit(1);
    }

    he = gethostbyname("www.google.com");
    if (!he->h_length) {
        herror("gethostbyname - www.googe.com"); 
        exit(1);
    }

    memset(&s_in, 0, sizeof(s_in));
    s_in.sin_family = AF_INET;
    s_in.sin_addr.s_addr = *((int *)he->h_addr_list[0]);

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    
#ifdef _OSX_
    icmp_hdr.icmp_type = ICMP_ECHO;
    icmp_hdr.icmp_id = htons(ident);
    icmp_hdr.icmp_seq = htons(1);
#else
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = ntohs(ident);
    icmp_hdr.un.echo.sequence = htons(1);
#endif

    memcpy(packet, &icmp_hdr, sizeof(icmp_hdr)+5);
    memcpy(packet + sizeof(icmp_hdr), "hello", 5);
    _icmp_cksum(packet) = checksum(packet, sizeof(packet));
    
    // Send the packet
    if(sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*) &s_in, sizeof(s_in)) < 0) {
        perror("Socket sendto");
	exit(1);
    }
    
    printf("Sent ICMP ECHO (ident : %d) to %s\n", ident, inet_ntoa(s_in.sin_addr));

    in_size = sizeof(r_in);
    bytes = recvfrom(sock, reply, sizeof(reply), 0, (struct sockaddr*) &r_in, (unsigned int *)&in_size);
    if (!bytes) {
        perror("Socket recvfrom");
	exit(1);
    }

    // Check reply success - display the id
    icmp_buf = reply + ipv4_hl(reply);
    
    if (_icmp_type(icmp_buf) != 0) {
        fprintf(stderr, "Not an ICMP error reply\n");
        exit(1);
    } 

    // Receive ICMP identifier
    printf("Recieved ICMP REPLY - id: %hu\n", ntohs(_icmp_id(icmp_buf)));
    exit(0);
}
