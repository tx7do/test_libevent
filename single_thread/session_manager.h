#pragma once

#include "object_pool.h"
#include "session.h"

#include <memory>
#include <mutex>
#include <unordered_map>

class SessionManager
{
	using mutex_t = std::mutex;

	using session_ptr = std::shared_ptr<Session>;
	using session_map = std::unordered_map<evutil_socket_t, session_ptr>;
	using session_pool = ObjectPool<Session, session_ptr>;

public:
	explicit SessionManager(std::size_t capacity = 256);
	~SessionManager();

public:
	Session* addSession(evutil_socket_t fd, const sockaddr_in& sin);

	void removeSession(evutil_socket_t fd);
	void removeSession(Session* session);

	void clear();

public:
	Session* getSession(evutil_socket_t fd) const;

	bool empty() const;

	bool full() const;

	bool contains(evutil_socket_t fd) const;

protected:
	mutable mutex_t _mutex;
	session_map _sessions;
	session_pool _pool;
};
