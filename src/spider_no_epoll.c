/*
 * spider.c
 *
 *  Created on: 2016年12月24日
 *      Author: wengle
 */

#include "spider.h"

#define SEEDS_LEN 100

int init() {
	initCustomType();
	/**
	 * initial global data
	 */
	unvisitedUser = create_queue(string_t);
	visitedUser = create_set(string_t);
	visitedQuestions = create_set(long);

	check(unvisitedUser != NULL, "create_queue");
	check(visitedUser != NULL, "create_set");
	check(visitedQuestions != NULL, "create_set");

	string_t *line = create_string();
	check(line != NULL, "spider: create_string");

	queue_init(unvisitedUser);
	set_init(visitedUser);
	set_init(visitedQuestions);
	string_init(line);

	char buffer[SEEDS_LEN] = { 0 };
	FILE *pfile = fopen(START_FILE, "r");
	check(pfile != NULL, "fopen");

	int pos = 0;
	while (fgets(buffer, SEEDS_LEN, pfile) != NULL) {
		if (buffer[strlen(buffer) - 1] == '\n') {
			buffer[strlen(buffer) - 1] = '\0';
		}
		string_assign_cstr(line, buffer);
		if (!string_empty(line)) {
			pos = string_rfind_char(line, '/', string_length(line));
			line = string_substr(line, pos + 1, string_length(line) - pos);
			queue_push(unvisitedUser, line);
			set_insert(visitedUser, line);
		}
	}
	string_destroy(line);
	return 0;
}

int init_epoll() {
	int fd;
	fd = epoll_create1(0);
	check(fd > 0, "epoll_create1");
	events = (struct epoll_event *) malloc(sizeof(struct epoll_event) * MAXEVENTS);
	epollfd = fd;
	curConns = 0;
	return 0;
}

void getPage() {
	int reqfd;
	int retval = 0;
	char reqBuf[MAXREQSIZE] = {0};
	connection *con;
	string_t *reqUser;
	string_t *responsePage;
	list_t *resultAnswers; //for answers
	list_t *resultFollowers; //for followers
	int size = 0;

	while (1) {
		// start from seeds
		do{
			if(curConns < MAXCONNS){
				reqUser = getUser();
				if(reqUser == NULL && !queue_empty(unvisitedUser)){
					continue;
				}else if(reqUser == NULL){
					break;
				}
				con = makeConnection(&reqfd);
				if(con == NULL || reqfd == 0){
					continue;
				}
				size = MAXREQSIZE;
				retval = prepareGetTotal(reqBuf, &size, reqUser,  ANSWERS);
				if(retval < 0){
					string_destroy(reqUser);
					break;
				}
				retval = sslWrite(con, reqBuf, size);
				if(retval < 0){
					log_err("getPage->write");
					break;
				}
				StateMachine *pState = (StateMachine*)malloc(sizeof(StateMachine));
				pState->con = con;
				pState->iState = GETTOTALANSWERNUM;
				pState->iLast = 0;

				struct epoll_event event;
				event.data.ptr = (void*)pState;
				event.events = EPOLLIN | EPOLLET;
				retval = epoll_ctl(epollfd, EPOLL_CTL_ADD, reqfd, &event);
				check(retval == 0, "epoll_add");
				curConns++;
			}
		}while(0);


		reqUser = getUser();
		if(reqUser == NULL && !queue_empty(unvisitedUser)){
			continue;
		}else if(reqUser == NULL){
			break;
		}
		con = makeConnection(&reqfd);
		if(con == NULL || reqfd == 0){
			continue;
		}
		while(1){
			int total = 0;
			int curTimes = 0;
			int repeatTimes = 0;
			int file = 0;
			int preprocess = 1;
			{
				/* preprocess: get total answers */
				size = MAXREQSIZE;
				retval = prepareGetTotal(reqBuf, &size, reqUser,  ANSWERS);
				if(retval < 0){
					string_destroy(reqUser);
					break;
				}

				retval = sslWrite(con, reqBuf, size);

				if(retval < 0){
					log_err("getPage->write");
					break;
				}
				retval = sslRead(con, &responsePage);
				preprocess = 1;
				total = parseResponse(responsePage, &resultAnswers, parseAnswersJsonData, preprocess);
				if(total == 0){
					break;
				}
				if(preprocess != 1 && resultAnswers != NULL){
					list_destroy(resultAnswers);
				}
				string_destroy(responsePage);

				/* formal request for data */
				repeatTimes = ((total + REQUESTSIZE-1)/REQUESTSIZE);
				file = open(RAWDATA, O_WRONLY|O_APPEND);
				if(file == -1){
					log_err("open rawdata file failed.");
					break;
				}
				while(curTimes < repeatTimes){
					size = MAXREQSIZE;
					preprocess = 0;
					con = makeConnection(&reqfd);
					prepareAnswersRequest(reqBuf, &size, reqUser, curTimes);
					retval = sslWrite(con, reqBuf, size);
					retval = sslRead(con, &responsePage);
					parseResponse(responsePage, &resultAnswers, parseAnswersJsonData, preprocess);
					printf("times: %d\t%ld\n", curTimes+1, list_size(resultAnswers));
					retval = writeToFile(file, resultAnswers);
					if(retval != list_size(resultAnswers)){
						log_err("number is writed to file that is incorrect.");
						break;
					}
					curTimes++;
				}
				close(file);
			}

			/* get followers who answers great than 100 and followers more than 200 */
			{
				total = 0;
				curTimes = 0;
				size = MAXREQSIZE;
				retval = prepareGetTotal(reqBuf, &size, reqUser,  FOLLOWERS);
				if(retval < 0){
					string_destroy(reqUser);
					break;
				}
				con = makeConnection(&reqfd);
				retval = sslWrite(con, reqBuf, size);

				if(retval < 0){
					log_err("getPage->write");
					break;
				}
				retval = sslRead(con, &responsePage);
				preprocess = 1;
				total = parseResponse(responsePage, &resultFollowers, parseFollowersJsonData, preprocess);
				if(total == 0){
					break;
				}
				if(preprocess != 1 && resultFollowers != NULL){
					list_destroy(resultFollowers);
				}
				string_destroy(responsePage);
				/* formal request for followers data */
				repeatTimes = ((total + REQUESTSIZE-1)/REQUESTSIZE);
				while(curTimes < repeatTimes){
					size = MAXREQSIZE;
					preprocess = 0;
					con = makeConnection(&reqfd);
					prepareFollowersRequest(reqBuf, &size, reqUser, curTimes);
					retval = sslWrite(con, reqBuf, size);
					retval = sslRead(con, &responsePage);
					parseResponse(responsePage, &resultFollowers, parseFollowersJsonData, preprocess);
					debug("times: %d\t%ld\t%ld\n", curTimes+1, list_size(resultFollowers), list_size(resultFollowers));
					curTimes++;
				}
			}
			break;
		}
		string_destroy(responsePage);
		string_destroy(reqUser);
		list_destroy(resultAnswers);
		list_destroy(resultFollowers);
	}

}


int writeToFile(int fd, list_t *result){
	if(result == NULL || list_size(result) == 0){
			return 0;
	}
	int num = 0;
	char lines[100] = {0};
	int size = 100;
	iterator_t it;
	for (it = list_begin(result); !iterator_equal(it, list_end(result));
			it = iterator_next(it), num++) {
		size = 100;
		QuestVote *tmp = (QuestVote*) iterator_get_pointer(it);
		size = snprintf(lines, size, "%d\t%ld\n", tmp->voted_count, tmp->questionId);
		write(fd, lines, size);
	}
	return num;
}

static int parseResponse(string_t *response, list_t **result, JSONFUNC func, int preprocess){
	int start = 0;
	int totals = 0;
	int contentLength = 0;
	string_t *responseJson;
	list_t *dealedJson;
	//first: get Content-Length:
	start = string_find_subcstr(response, "Content-Length: ", start, strlen("Content-Length: "));
	if(start == NPOS){
		log_err("can't find 'Content-Length: '");
		return -1;
	}
	contentLength = atoi(&(string_c_str(response)[start+strlen("Content-Length: ")]));

	//second: response body
	start = string_find_subcstr(response, "\r\n\r\n", start, strlen("\r\n\r\n"));
	start += strlen("\r\n\r\n");
	responseJson = string_substr(response, start, string_length(response)-start);
	if(responseJson == NULL){
		debug("contentLength: %d, responseJson length: %ld\n",
					contentLength, string_length(responseJson));
		return -1;
	}
	if(contentLength != string_length(responseJson)){
		return -ECOMPLETE;
	}

	//third: parse json data
	dealedJson = func(responseJson, &totals, preprocess);
	if(dealedJson == NULL && preprocess != 1){
		return -EINVAL;
	}
	*result = dealedJson;
	return totals;
}

static list_t* parseFollowersJsonData(string_t *responseJson, int *total, int preprocess){
	list_t *res = NULL;
	cJSON *item, *array, *answer, *follower, *url_token;
	string_t *userUrl;
	int totals = 0;
	int followerCount = 0;
	int answerCount = 0;
	int i = 0, count = 0;
	res = create_list(string_t);
	userUrl = create_string();
	if(res == NULL){
		log_err("create follower list failed.");
		return res;
	}
	list_init(res);
	string_init(userUrl);
	cJSON *object = cJSON_Parse(string_c_str(responseJson));
	if(object == NULL){
		debug("parse json failed.");
	}
	item = cJSON_GetObjectItem(object, "paging");
	item = cJSON_GetObjectItem(item, "totals");
	totals = atoi(cJSON_Print(item));
	if(totals == 0){
		debug("json data is empty. need not parse.");
		return NULL;
	}
	*total = totals;
	if(preprocess == 1){
		list_destroy(res);
		return NULL;
	}
	array = cJSON_GetObjectItem(object, "data");
	count = cJSON_GetArraySize(array);
	for(i = 0; i < count; i++){
		item = cJSON_GetArrayItem(array, i);
		//get answerCount followerCount
		answer = cJSON_GetObjectItem(item, "answer_count");
		answerCount = atoi(cJSON_Print(answer));
		if(answerCount < FOLLOWERANSWER){
			continue;
		}
		follower = cJSON_GetObjectItem(item, "follower_count");
		followerCount = atoi(cJSON_Print(follower));
		if(followerCount < FOLLOWERNUM){
			continue;
		}
		url_token = cJSON_GetObjectItem(item, "url_token");
		string_assign_cstr(userUrl, cJSON_Print(url_token));
		debug("totals: %d, answerCount: %d, followerCount: %d, userUrl: %s\n",
				totals, answerCount, followerCount, string_c_str(userUrl));
		if(!iterator_equal(set_find(visitedUser, userUrl), set_end(visitedUser))){
			continue;
		}else{
			set_insert(visitedUser, userUrl);
		}
		list_push_back(res, userUrl);
	}
	cJSON_Delete(object);
	string_destroy(userUrl);
	return res;
}

static list_t* parseAnswersJsonData(string_t *responseJson, int *total, int preprocess){
	list_t *res = NULL;
	QuestVote qv;
	cJSON *item, *array, *question, *voted;
	int totals = 0;
	long questionId = 0;
	int votedCount = 0;
	int i = 0, count = 0;
	res = create_list(QuestVote);
	if(res == NULL){
		log_err("create Questvote list failed.");
		return res;
	}
	list_init(res);
	cJSON *object = cJSON_Parse(string_c_str(responseJson));
	if(object == NULL){
		debug("parse json failed.");
	}
	item = cJSON_GetObjectItem(object, "paging");
	item = cJSON_GetObjectItem(item, "totals");
	totals = atoi(cJSON_Print(item));
	if(totals == 0){
		debug("json data is empty. need not parse.");
		return NULL;
	}
	*total = totals;
	if(preprocess == 1){
		list_destroy(res);
		return NULL;
	}
	array = cJSON_GetObjectItem(object, "data");
	count = cJSON_GetArraySize(array);
	for(i = 0; i < count; i++){
		item = cJSON_GetArrayItem(array, i);
		//get question
		question = cJSON_GetObjectItem(item, "question");
		question = cJSON_GetObjectItem(question, "id");
		questionId = (long)atoi(cJSON_Print(question));
		if(!iterator_equal(set_find(visitedQuestions, questionId), set_end(visitedQuestions))){
			continue;
		}else{
			set_insert(visitedQuestions, questionId);
		}
		//get vote
		voted = cJSON_GetObjectItem(item, "voteup_count");
		votedCount = atoi(cJSON_Print(voted));
		debug("totals: %d, questionId: %ld, votedCount: %d\n", totals, questionId, votedCount);
		qv.questionId = questionId;
		qv.voted_count = votedCount;
		list_push_back(res, &qv);
	}
	cJSON_Delete(object);
	return res;
}

int prepareGetTotal(char *reqBuf, int *size, string_t *reqUser, char *type){
	int retval = 0;
	retval = snprintf(reqBuf, *size,
					"GET %s%s%s%s%s HTTP/1.1\r\n"
					"Host: www.zhihu.com\r\n"
					"Connection: keep-alive\r\n"
					"Cache-Control: max-age=0\r\n"
					"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
					"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36\r\n"
					"Accept-Language: zh-CN,zh;q=0.8\r\n"
					"Cookie: %s\r\n"
					"\r\n", AJAXPREFIX, string_c_str(reqUser), type, TOTALOFFSET, TOTALLIMIT, COOKIE);
	if(retval < 0){
		log_err("prepareAnswersRequest->snprintf");
		return retval;
	}
	*size = retval;
	return retval;
}

int prepareAnswersRequest(char *reqBuf, int *size, string_t *reqUser, int times){
	int retval = 0;
	int offset = 0;
	offset = times*REQUESTSIZE;
	char offsetNum[20] = {0};
	sprintf(offsetNum, "%d", offset);
	retval = snprintf(reqBuf, *size,
					"GET %s%s%s%s%s%s%s HTTP/1.1\r\n"
					"Host: www.zhihu.com\r\n"
					"Connection: keep-alive\r\n"
					"Cache-Control: max-age=0\r\n"
					"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
					"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36\r\n"
					"Accept-Language: zh-CN,zh;q=0.8\r\n"
					"Cookie: %s\r\n"
					"\r\n", AJAXPREFIX, string_c_str(reqUser), ANSWERS, OFFSET,
					offsetNum, LIMIT, SORT_BY, COOKIE);
	if(retval < 0){
		log_err("prepareAnswersRequest->snprintf");
		return retval;
	}
	*size = retval;
	return retval;
}

int prepareFollowersRequest(char *reqBuf, int *size, string_t *reqUser, int times){
	int retval = 0;
	int offset = 0;
	offset = times*REQUESTSIZE;
	char offsetNum[20] = {0};
	sprintf(offsetNum, "%d", offset);
	retval = snprintf(reqBuf, *size,
					"GET %s%s%s%s%s%s HTTP/1.1\r\n"
					"Host: www.zhihu.com\r\n"
					"Connection: keep-alive\r\n"
					"Cache-Control: max-age=0\r\n"
					"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
					"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36\r\n"
					"Accept-Language: zh-CN,zh;q=0.8\r\n"
					"Cookie: %s\r\n"
					"\r\n", AJAXPREFIX, string_c_str(reqUser), FOLLOWERS, OFFSET,
					offsetNum, LIMIT, COOKIE);
	if(retval < 0){
		log_err("prepareFollowersRequest->snprintf");
		return retval;
	}
	*size = retval;
	return retval;
}

connection *makeConnection(int *reqfd) {
	int retval = 0;

	connection *c;
	c = sslConnect();
	if(c == NULL){
		log_err("makeConnection->sslConnect");
		return c;
	}

	retval = make_socket_non_blocking(c->socket);
	if(retval == -1){
		log_err("makeConnection->make_socket_non_blocking");
		*reqfd = 0;
		return c;
	}
	*reqfd = c->socket;
	return c;
}

static int make_socket_non_blocking(int sfd) {
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) {
		perror("fcntl");
		return -1;
	}

	return 0;
}

//int main(int argc, char **argv) {
//	init();
//	getPage();
//	return 0;
//}

void print() {

//	printf("--------------\n");
//	while (!queue_empty(unvisitedUser)) {
//		string_t *line = (string_t*) queue_front(unvisitedUser);
//		printf("%s\n", string_c_str(line));
//		queue_pop(unvisitedUser);
//	}
//
//	iterator_t it;
//	for (it = set_begin(visitedUser); !iterator_equal(it, set_end(visitedUser));
//			it = iterator_next(it)) {
//		printf("%s\n", string_c_str((string_t*) iterator_get_pointer(it)));
//	}
//
//	iterator_t it;
//			for (it = list_begin(result); !iterator_equal(it, list_end(result));
//					it = iterator_next(it)) {
//				QuestVote *tmp = (QuestVote*) iterator_get_pointer(it);
//				debug("questionId: %ld, voted count: %d", tmp->questionId, tmp->voted_count);
//			}
	/*
	//static void equal_id(const void* cpv_input, const void* cpv_second, void* pv_output);

	static void removeDuplicateAnswers(list_t *result){
		if(result == NULL || list_size(result) == 0){
			return;
		}
		vector_t *needRemove = create_vector(QuestVote);
		vector_init(needRemove);
		iterator_t it;
		for (it = list_begin(result); !iterator_equal(it, list_end(result));
				it = iterator_next(it)) {
			QuestVote *tmp = (QuestVote*) iterator_get_pointer(it);
			if(!iterator_equal(set_find(visitedQuestions, tmp->questionId), set_end(visitedQuestions))){
				vector_push_back(needRemove, tmp);
			}else{
				set_insert(visitedQuestions, tmp->questionId);
			}
		}

		for (it = vector_begin(needRemove);
			 !iterator_equal(it, vector_end(needRemove));
			 it = iterator_next(it)) {
			QuestVote *tmp = (QuestVote*) iterator_get_pointer(it);
			list_remove(result, tmp);
		}
		vector_destroy(needRemove);
	}


	static void equal_id(const void* cpv_input, const void* cpv_second, void* pv_output)
	{
		assert(cpv_input != NULL && (cpv_second != NULL) && pv_output != NULL);
		if((*(QuestVote*)cpv_input).questionId  == *(long*)cpv_second){
			*(bool_t*)pv_output = true;
		} else {
			*(bool_t*)pv_output = false;
		}
	}
	*/
}
