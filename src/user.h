/*
 * user.h
 *
 *  Created on: 2016年12月23日
 *      Author: wengle
 */

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include <cstl/cqueue.h>
#include <cstl/cset.h>
#include <cstl/cstring.h>

#include "dbg.h"

typedef struct _QuestVote {
	int questionId;
	int voted_count;
} QuestVote;

extern queue_t *unvisitedUser;
extern set_t *visitedUser;

extern set_t *visitedQuestions;

void initCustomType();
string_t* getUser();

#endif /* SRC_USER_H_ */
