#pragma once

#include <event2/util.h>

class SessionManager;

class Session
{
public:
	Session();
	~Session();

	evutil_socket_t fd = -1;
	sockaddr_in sin = {};

	SessionManager* manager = nullptr;
};
