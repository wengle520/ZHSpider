/*
 * ssl.c
 *
 *  Created on: 2016年12月24日
 *      Author: wengle
 */

#include "sslayer.h"

// Establish a regular tcp connection
int tcpConnect() {
	int error, handle;
	struct hostent *host;
	struct sockaddr_in server;

	host = gethostbyname(SERVER);
	handle = socket(AF_INET, SOCK_STREAM, 0);
	if (handle == -1) {
		perror("Socket");
		handle = 0;
	} else {
		server.sin_family = AF_INET;
		server.sin_port = htons(PORT);
		server.sin_addr = *((struct in_addr *) host->h_addr);
		bzero(&(server.sin_zero), 8);

		error = connect(handle, (struct sockaddr *) &server,
				sizeof(struct sockaddr));
		if (error == -1) {
			perror("Connect");
			handle = 0;
		}
	}

	return handle;
}

// Establish a connection using an SSL layer
connection *sslConnect(void) {
	connection *c;

	c = malloc(sizeof(connection));
	if (c == NULL) {
		log_err("sslConnect->malloc");
		return c;
	}
	c->sslHandle = NULL;
	c->sslContext = NULL;

	c->socket = tcpConnect();
	if (c->socket) {
		// Register the error strings for libcrypto & libssl
		SSL_load_error_strings();
		// Register the available ciphers and digests
		SSL_library_init();

		// New context saying we are a client, and using SSL 2 or 3
		c->sslContext = SSL_CTX_new(SSLv23_client_method());
		if (c->sslContext == NULL)
			ERR_print_errors_fp(stderr);

		// Create an SSL struct for the connection
		c->sslHandle = SSL_new(c->sslContext);
		if (c->sslHandle == NULL)
			ERR_print_errors_fp(stderr);

		// Connect the SSL struct to our connection
		if (!SSL_set_fd(c->sslHandle, c->socket))
			ERR_print_errors_fp(stderr);

		// Initiate SSL handshake
		if (SSL_connect(c->sslHandle) != 1)
			ERR_print_errors_fp(stderr);
	} else {
		c = NULL;
	}
	return c;
}

// Disconnect & free connection struct
void sslDisconnect(connection *c) {
	if (c->socket)
		close(c->socket);
	if (c->sslHandle) {
		SSL_shutdown(c->sslHandle);
		SSL_free(c->sslHandle);
	}
	if (c->sslContext)
		SSL_CTX_free(c->sslContext);

	free(c);
}

// Read all available text from the connection
int sslRead(connection *c, string_t **res) {
	char buffer[HTMLSIZE] = { 0 };
	int curRec = 0;
	int sumRec = 0;
	string_t *response = create_string();
	if (response == NULL) {
		log_err("sslayer->create_string");
		return -1;
	}
	string_init(response);
	*res = response;

	while (1) {
		curRec = SSL_read(c->sslHandle, buffer+sumRec, HTMLSIZE-sumRec);
		if (curRec == 0) {
			close(c->socket);
			break;
		}
		if (curRec < 0) {
			if (errno != EAGAIN) {
				log_err("read err, and errno = %d", errno);
				goto err;
			}
			continue;
		}
		sumRec += curRec;
	}
	string_append_cstr(response, buffer);
	*res = response;
	return sumRec;

err:
	close(c->socket);
	return -1;
}

// Write text to the connection
int sslWrite(connection *c, char *text, size_t size) {
	if (c)
		return SSL_write(c->sslHandle, text, size);
	return -1;
}

// Very basic main: we send GET / and print the response.
//int main(int argc, char **argv) {
//	connection *c;
//	char *response;
//
//	c = sslConnect();
//
//	sslWrite(c,
//			"GET /api/v4/members/wengle/questions HTTP/1.1\r\nHost: "
//					"www.zhihu.com\r\nconnection: keep-alive\r\n"
//					"Cookie: _zap=81f012e3-f803-457a-8864-3c2bd467f270; q_c1=1d4a78fd41604479b1dbbfd96432f82c|1482584367000|1482584367000; _xsrf=62ab870ea70e49abfa9c3400e6fdc170; l_cap_id=\"ZjBhODAwNzEwNGE2NDg2MTllMzgzY2ViMjljNDAxNDE=|1482584367|396f88f4b0904a35b4bab605d7fa03006a94f144\"; cap_id=\"NDNmNWQ4ZDNjYWMxNDUwN2I3Y2MxOTk1MmQ5ZjhkNjc=|1482584367|ebd14d7653de8a4930a4a51cbde18aa04c947889\"; n_c=1; d_c0=\"AADCpTRODAuPTsMWtKn0hE46z4-Bk0guUvY=|1482584368\"; _zap=4d601d44-25c1-458c-9630-9ed994ae2b82; r_cap_id=\"ODdlMTVjOTA3NjBmNDBjYzk2NjFjZjQ0YTk1NmVhZTY=|1482584368|4d07d5561a03e261fcac0ee38330491c20c69085\"; __utma=51854390.1823998587.1482584372.1482584372.1482584372.1; __utmb=51854390.2.10.1482584372; __utmc=51854390; __utmz=51854390.1482584372.1.1.utmcsr=zhihu.com|utmccn=(referral)|utmcmd=referral|utmcct=/people/raymond-wang/answers; __utmv=51854390.000--|3=entry_date=20161224=1; __utmt=1; login=\"Yjg1NDM1Njk5OTMyNDE2ZWFjYWM0NTdjZjVkZjAwYmE=|1482584373|3e0d03857cfd9bd5fa0d0cacd3b988e90e8c0c99\"; z_c0=Mi4wQUFBQVd1OG1BQUFBQU1LbE5FNE1DeGNBQUFCaEFsVk5OZjZGV0FEQWs2T3N2Yk9yRVlucUtJTDhsbTViV05hSkRn|1482584373|a1343c217c56cbea5381c603bac6961f3aad3264\r\n"
//					"\r\n");
//	response = sslRead(c);
//
//	printf("%s\n", response);
//
//	sslDisconnect(c);
//	free(response);
//
//	return 0;
//}
