#ifndef __MANEJA_ERROR_H__
#define __MANEJA_ERROR_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>

int maneja_error(ssize_t valor);


#endif /* __MANEJA_ERROR_H__ */