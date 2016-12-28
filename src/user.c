/*
 * user.c
 *
 *  Created on: 2016年12月24日
 *      Author: wengle
 */

#include "user.h"
#include "global.h"
string_t* getUser() {
	string_t *ret = create_string();
	if (ret == NULL) {
		log_err("user->create_string");
		return ret;
	}
	string_init(ret);
	if (!queue_empty(unvisitedUser)) {
		string_assign(ret, (string_t*) queue_front(unvisitedUser));
		queue_pop(unvisitedUser);
	} else {
		string_destroy(ret);
		log_warn("user queue is empty. Process will exit. Finished!!!");
		return NULL;
	}
	return ret;
}

static void _QuestVote_init(const void* cpv_input, void* pv_output) {
	((QuestVote*) cpv_input)->questionId = 0L;
	((QuestVote*) cpv_input)->voted_count = 0;
	*(bool_t*) pv_output = true;
}

static void _QuestVote_destroy(const void* cpv_input, void* pv_output) {
	((QuestVote*) cpv_input)->questionId = 0L;
	((QuestVote*) cpv_input)->voted_count = 0;
	*(bool_t*) pv_output = true;
}

static void _QuestVote_copy(const void* cpv_first, const void* cpv_second,
		void* pv_output) {
	((QuestVote*) cpv_first)->questionId = ((QuestVote*) cpv_second)->questionId;
	((QuestVote*) cpv_first)->voted_count = ((QuestVote*) cpv_second)->voted_count;
	*(bool_t*) pv_output = true;
}

static void _QuestVote_less(const void* cpv_first, const void* cpv_second,
		void* pv_output) {
	*(bool_t*) pv_output =
			((QuestVote*) cpv_first)->voted_count < ((QuestVote*) cpv_second)->voted_count ?
					true : false;
}

void initCustomType(){
	type_register(QuestVote, _QuestVote_init, _QuestVote_copy, _QuestVote_less, _QuestVote_destroy);
}

