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

	check(unvisitedUser != NULL, "create_queue");
	check(visitedUser != NULL, "create_set");

	string_t *line = create_string();
	check(line != NULL, "spider: create_string");

	queue_init(unvisitedUser);
	set_init(visitedUser);
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
	return 0;
}

void getPage() {
	int reqfd;
	int retval = 0;
	char reqBuf[MAXREQSIZE] = {0};
	connection *con;
	string_t *responsePage;
	int size = 0;
	while (1) {
		string_t *reqUser = getUser();
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
		retval = prepareAnswersRequest(reqBuf, &size, reqUser);
		if(retval < 0){
			string_destroy(reqUser);
			continue;
		}
		retval = sslWrite(con, reqBuf, size);
		printf("send to server: %d bytes\n", retval);
		printf("%s\n", reqBuf);
		if(retval < 0){
			log_err("getPage->write");
			continue;
		}
		retval = sslRead(con, &responsePage);
		int totals = parseResponse(responsePage, "totals");
		printf("totals: %d\n", totals);
		printf("receive data: %d  jsonbody: %d\n", retval, string_length(responsePage));
		printf("\n%s\n", string_c_str(responsePage));
	}

}

static int parseResponse(string_t *response, char *keywords){
	int start = 0;
	int end = 0;
	int num = 0;
	int contentLength = 0;
	string_t *responseJson;
	char tmp[20] = {0};
	//first: get Content-Length:
	start = string_find_subcstr(response, "Content-Length: ", start, strlen("Content-Length: "));
	if(start == NPOS){
		log_err("can't find 'Content-Length: '");
		return NPOS;
	}
	contentLength = atoi(&(string_c_str(response)[start+strlen("Content-Length: ")]));

	//response body
	start = string_find_subcstr(response, "\r\n\r\n", start, strlen("\r\n\r\n"));
	start += strlen("\r\n\r\n");
	responseJson = string_substr(response, start, string_length(response)-start);
	if(responseJson == NULL){
		debug("contentLength: %d, responseJson length: %d\n",
					contentLength, string_length(responseJson));
		return -1;
	}
	if(contentLength != string_length(responseJson)){
		return -ECOMPLETE;
	}
	parseJsonData(responseJson);

	start = string_rfind_subcstr(response, keywords,
			string_length(response), strlen(keywords));
	if(start == NPOS){
		log_err("can't find %s.", keywords);
		return NPOS;
	}
	start = string_find_subcstr(response, ":", start, strlen(":"));
	if(start == NPOS){
		log_err("can't find %s.", keywords);
		return NPOS;
	}
	start++;
	end = string_find_first_of_subcstr(response, "},", start, strlen(","));
	if(start == NPOS){
		log_err("can't find %s.", keywords);
		return NPOS;
	}
	num = string_copy(response, tmp, end-start, start);
	if(num != (end-start)){
		return -EINVAL;
	}
	num = atoi(tmp);
	if(num < 0){
		return -EINVAL;
	}
	return num;
}

static cJSON* parseJsonData(string_t *responseJson){
	list_t *res = NULL;
	QuestVote qv;
	cJSON *item, *array, *question, *voted;
	int totals = 0;
	int questionId = 0;
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
	array = cJSON_GetObjectItem(object, "data");
	count = cJSON_GetArraySize(array);
	for(i = 0; i < count; i++){
		item = cJSON_GetArrayItem(array, i);
		//get question
		question = cJSON_GetObjectItem(item, "question");
		question = cJSON_GetObjectItem(question, "id");
		questionId = atoi(cJSON_Print(question));
		//get vote
		voted = cJSON_GetObjectItem(item, "voteup_count");
		votedCount = atoi(cJSON_Print(voted));
		//debug("totals: %d, questionId: %d, votedCount: %d\n", totals, questionId, votedCount);
		qv.questionId = questionId;
		qv.voted_count = votedCount;
		list_push_back(res, &qv);
	}
	return NULL;
}

int prepareAnswersRequest(char *reqBuf, int *size, string_t *reqUser){
	int retval = 0;
	retval = snprintf(reqBuf, *size,
					"GET %s%s%s%s%s%s HTTP/1.1\r\n"
					"Host: www.zhihu.com\r\n"
					"Connection: keep-alive\r\n"
					"Cache-Control: max-age=0\r\n"
					"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
					"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36\r\n"
					"Accept-Language: zh-CN,zh;q=0.8\r\n"
					"Cookie: %s\r\n"
					"\r\n", AJAXPREFIX, string_c_str(reqUser), ANSWERS, OFFSET, LIMIT, SORT_BY, COOKIE);
	if(retval < 0){
		log_err("prepareAnswersRequest->snprintf");
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

int main(int argc, char **argv) {
	init();
	getPage();
	return 0;
}

void print() {

	printf("--------------\n");
	while (!queue_empty(unvisitedUser)) {
		string_t *line = (string_t*) queue_front(unvisitedUser);
		printf("%s\n", string_c_str(line));
		queue_pop(unvisitedUser);
	}

	iterator_t it;
	for (it = set_begin(visitedUser); !iterator_equal(it, set_end(visitedUser));
			it = iterator_next(it)) {
		printf("%s\n", string_c_str((string_t*) iterator_get_pointer(it)));
	}
}
