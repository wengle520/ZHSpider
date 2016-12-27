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

#define ECOMPLETE 10 /* receive data incomplete*/

#define MAXEVENTS 1024
#define MAXREQSIZE 4096
#define MAXRESPSIZE 4096
#define HTMLSIZE 524288

#define FINDVOTECOUNT "voteup_count"

/**
 * unvisitedUser's size equal that following plus followers
 */
extern queue_t *unvisitedUser;
extern set_t *visitedUser;

extern set_t *visitedQuestions;

#endif /* SRC_GLOBAL_H_ */
