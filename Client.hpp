#ifndef _CLIENT_H
#define _CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>

// macro definition
#define PORT 24816  // port
#define BUFFER_SIZE 102400  // buffer size

// utility
static void usage(void);  // indicate the usage

#endif