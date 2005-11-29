#include "connection.h"

Connection::Connection(buffer_t * host, unsigned short port) : tcp_port(port) {
	buffer_init(&hostname);
	buffer_add_buffer(&hostname,host);
}

Connection::~Connection() {
	buffer_free(&hostname);
}

bool Connection::connect() {
	return false;
}

bool Connection::disconnect() {
	return false;
}

int Connection::doRead(buffer_t * buf, unsigned int len) {
	(void)buf;
	(void)len;
	return -1;
}

int Connection::readUntilSeparator(buffer_t * buf, char sep) {
	(void)buf;
	(void)sep;
	return -1;
}

int Connection::readLine(buffer_t * buf) {
	(void)buf;
	return -1;
}

unsigned short Connection::port() {
	return tcp_port;
}

bool Connection::canRead() {
	return false;
}
