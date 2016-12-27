/*
 * config.h
 *
 *  Created on: 2016年12月23日
 *      Author: wengle
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#define SERVER "www.zhihu.com"
#define PORT 443

#define MAINPAGE "/people/"

#define AJAXPREFIX "/api/v4/members/"

#define ANSWERS "/answers?include=data%5B*%5D.is_normal,suggest_edit,comment_count,collapsed_counts,reviewing_comments_count,can_comment,content,voteup_count,reshipment_settings,comment_permission,mark_infos,created_time,updated_time,relationship.voting,is_author,is_thanked,is_nothelp,upvoted_followees;data%5B*%5D.author.badge%5B?(type=best_answerer)%5D.topics"

#define ASKS "/questions?include=data%5B*%5D.created%2Cfollower\
_count%2Canswer_count%2Crelationship.is_anonymous"

#define COOKIE "q_c1=1d4a78fd41604479b1dbbfd96432f82c|1482584367000|1482584367000; l_cap_id=\"ZjBhODAwNzEwNGE2NDg2MTllMzgzY2ViMjljNDAxNDE=|1482584367|396f88f4b0904a35b4bab605d7fa03006a94f144\"; cap_id=\"NDNmNWQ4ZDNjYWMxNDUwN2I3Y2MxOTk1MmQ5ZjhkNjc=|1482584367|ebd14d7653de8a4930a4a51cbde18aa04c947889\"; d_c0=\"AADCpTRODAuPTsMWtKn0hE46z4-Bk0guUvY=|1482584368\"; _zap=4d601d44-25c1-458c-9630-9ed994ae2b82; r_cap_id=\"ODdlMTVjOTA3NjBmNDBjYzk2NjFjZjQ0YTk1NmVhZTY=|1482584368|4d07d5561a03e261fcac0ee38330491c20c69085\"; __utma=51854390.1823998587.1482584372.1482584372.1482584372.1; __utmz=51854390.1482584372.1.1.utmcsr=zhihu.com|utmccn=(referral)|utmcmd=referral|utmcct=/people/raymond-wang/answers; __utmv=51854390.000--|3=entry_date=20161224=1; login=\"Yjg1NDM1Njk5OTMyNDE2ZWFjYWM0NTdjZjVkZjAwYmE=|1482584373|3e0d03857cfd9bd5fa0d0cacd3b988e90e8c0c99\"; z_c0=Mi4wQUFBQVd1OG1BQUFBQU1LbE5FNE1DeGNBQUFCaEFsVk5OZjZGV0FEQWs2T3N2Yk9yRVlucUtJTDhsbTViV05hSkRn|1482584373|a1343c217c56cbea5381c603bac6961f3aad3264"
/**
 * the following is userd for suffix of ajax's request
 */
#define OFFSET "&offset=0"
#define LIMIT "&limit=20"
#define SORT_BY "&sort_by=voteups"

#define START_FILE "./startfile/seeds.txt"

#endif /* SRC_CONFIG_H_ */
