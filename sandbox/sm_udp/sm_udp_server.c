/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Simple UDP RA
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int p = atoi(argv[1]);

    // int socket(int __domain, int __type, int __protocol)
    int s = socket(AF_INET, SOCK_STREAM, 0);
    // int socket
    
    if (s == -1)
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stderr, "Socket created!\n");
    }
    struct sockaddr_in addr;
    memset(&addr, '\0', sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(p);

    // int bind(int __fd, const struct sockaddr *__addr, socklen_t __len)
    int status = bind(s, (struct sockaddr*)&addr, sizeof(addr));
    // status

    if (status == 0)
    {
        fprintf(stderr, "Bind completed!\n");
    }
    else
    {
        fprintf(stderr, "Could not bind to address!\n");
        close(s);
        exit(EXIT_FAILURE);
    }
    status = listen(s, 5);
    if (status == -1)
    {
        fprintf(stderr, "Cannot listen on socket!\n");
        close(addr);
        exit(EXIT_FAILURE);
    }
    while(true)
    {

    }
    exit(EXIT_SUCCESS);
}
