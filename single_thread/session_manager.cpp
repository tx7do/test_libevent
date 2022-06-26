#include <netinet/in.h>
#include <event2/util.h>
#include "session_manager.h"

SessionManager::SessionManager(std::size_t capacity)
	: _pool(capacity)
{

}

SessionManager::~SessionManager() = default;

Session* SessionManager::addSession(evutil_socket_t fd, const sockaddr_in& sin)
{
	if (contains(fd))
	{
		removeSession(fd);
	}

	auto session = _pool.borrowObject();
	session->fd = fd;
	session->sin = sin;
	session->manager = this;

	std::lock_guard<std::mutex> lock(_mutex);
	_sessions[fd] = session;

	return session.get();
}

void SessionManager::removeSession(evutil_socket_t fd)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _sessions.find(fd);
	if (it != _sessions.end())
	{
		_pool.returnObject(it->second);
		_sessions.erase(it);
	}
}

void SessionManager::removeSession(Session* session)
{
	if (session != nullptr)
	{
		removeSession(session->fd);
	}
}

Session* SessionManager::getSession(evutil_socket_t fd) const
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _sessions.find(fd);
	if (it != _sessions.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void SessionManager::clear()
{
	std::lock_guard<std::mutex> lock(_mutex);
	for (auto& it : _sessions)
	{
		_pool.returnObject(it.second);
	}
	_sessions.clear();
}

bool SessionManager::empty() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _sessions.empty();
}

bool SessionManager::full() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _sessions.size() >= _pool.capacity();
}

bool SessionManager::contains(int fd) const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _sessions.find(fd) != _sessions.end();
}
