/*
 * ssl.h
 *
 *  Created on: 2016年12月25日
 *      Author: wengle
 */

#ifndef SRC_SSLAYER_H_
#define SRC_SSLAYER_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <cstl/cstring.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "config.h"
#include "dbg.h"
#include "global.h"


int tcpConnect();
connection *sslConnect(void);
void sslDisconnect(connection *c);
int sslRead(connection *c, string_t **res);
int sslWrite(connection *c, char *text, size_t size);

#endif /* SRC_SSLAYER_H_ */
