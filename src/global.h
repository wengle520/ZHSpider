/*
 * global.h
 *
 *  Created on: 2016年12月23日
 *      Author: wengle
 */

#ifndef SRC_GLOBAL_H_
#define SRC_GLOBAL_H_

#include <cstl/cqueue.h>
#include <cstl/cset.h>
#include <cstl/cstring.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define ECOMPLETE 10 /* receive data incomplete*/

#define MAXEVENTS 1024
#define MAXCONNS 500
#define MAXREQSIZE 4096
#define MAXRESPSIZE 4096
#define HTMLSIZE 524288

#define EPOLLTIMEOUT 1000

// Simple structure to keep track of the handle, and
// of what needs to be freed later.
struct _connection{
    int socket;
    SSL *sslHandle;
    SSL_CTX *sslContext;
};

typedef struct _connection connection;

typedef struct _StateMachine {
	int iState;
	connection *con;
	int iLen; //total length of response's page
	int iLast; //last pos of read operation
	char responseHtml[HTMLSIZE];

	int totalAnswers;
	int curAnswers;

	int totalFollowers;
	int curFollowers;

	int times; //times that request has processed
} StateMachine;

enum REQUESTSTATE { GETTOTALANSWERNUM = 1, GETANSWERS, GETTOTALFOLLOWERNUM, GETFOLLOWERS};

#define FINDVOTECOUNT "voteup_count"

/**
 * unvisitedUser's size equal that following plus followers
 */
extern queue_t *unvisitedUser;
extern set_t *visitedUser;

extern set_t *visitedQuestions;

#endif /* SRC_GLOBAL_H_ */
