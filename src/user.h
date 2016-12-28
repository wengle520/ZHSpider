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
	long questionId;
	int voted_count;
} QuestVote;

void initCustomType();
string_t* getUser();

#endif /* SRC_USER_H_ */
