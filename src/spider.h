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
set_t *visitedQuestions;

/* Buffer where events are returned */
int epollfd;
struct epoll_event *events;

typedef list_t* (*JSONFUNC)(string_t *responseJson, int *total, int preprocess);

int init();
int init_epoll();
connection *makeConnection(int *reqfd);
static int make_socket_non_blocking(int sfd);

void getPage();

int prepareGetTotal(char *reqBuf, int *size, string_t *reqUser, char *type);
int prepareAnswersRequest(char *reqBuf, int *size, string_t *reqUser, int times);
int prepareFollowersRequest(char *reqBuf, int *size, string_t *reqUser, int times);

static int parseResponse(string_t *response, list_t **result, JSONFUNC func, int preprocess);
static list_t* parseAnswersJsonData(string_t *responseJson, int *total, int preprocess);
static list_t* parseFollowersJsonData(string_t *responseJson, int *total, int preprocess);

int writeToFile(int fd, list_t *result);

#endif /* SRC_SPIDER_H_ */
