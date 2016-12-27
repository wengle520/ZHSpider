/*
 * spider.h
 *
 *  Created on: 2016年12月24日
 *      Author: wengle
 */

#ifndef SRC_SPIDER_H_
#define SRC_SPIDER_H_

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <cstl/cstring.h>
#include <cstl/cqueue.h>
#include <cstl/cset.h>

#include "sslayer.h"
#include "config.h"
#include "dbg.h"
#include "user.h"
#include "global.h"
#include "cJSON.h"

queue_t *unvisitedUser;
set_t *visitedUser;

/* Buffer where events are returned */
int epollfd;
struct epoll_event *events;


int init();
int init_epoll();
void getPage();
int prepareAnswersRequest(char *reqBuf, int *size, string_t *reqUser);
connection *makeConnection(int *reqfd);

static int make_socket_non_blocking(int sfd);
static int parseResponse(string_t *response, char *keywords);
static cJSON* parseJsonData(string_t *responseJson);

#endif /* SRC_SPIDER_H_ */
