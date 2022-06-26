#pragma once

#include <mutex>
#include <thread>
#include <memory>

#include <event2/event.h>
#include <event2/event_struct.h>

#include "session_manager.h"

class ServerApplication
{
	using thread_t = std::thread;
	using thread_ptr = std::shared_ptr<thread_t>;

	using mutex_t = std::mutex;

	typedef struct event_base context_t;
	using context_ptr = context_t*;

	typedef struct event event_t;
	using event_ptr = event_t*;

	typedef struct evconnlistener listener_t;
	using listener_ptr = listener_t*;

public:
	explicit ServerApplication(uint32_t port);
	~ServerApplication();

public:
	int run();

protected:
	void start();
	void stop();

protected:
	/// 启动事件队列
	void startEvent();
	/// 停止事件队列
	void stopEvent();

	/// 启动工作线程
	void startThread();
	/// 停止工作线程
	void stopThread();

protected:
	/// 启动监听器
	void startAcceptor(uint32_t port);

	/// 添加程序中断信号事件
	void addInterruptSignalEvent();

protected:
	static void
	listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* user_data);
	static void conn_read_cb(struct bufferevent* bev, void* user_data);
	static void conn_write_cb(struct bufferevent* bev, void* user_data);
	static void conn_event_cb(struct bufferevent* bev, short events, void* user_data);
	static void log_cb(int severity, const char* msg);
	static void event_fatal_cb(int err);
	static void signal_cb(evutil_socket_t sig, short events, void* user_data);

private:
	context_ptr _base = nullptr;
	event_ptr _interrupt_signal_event = nullptr;
	listener_ptr _listener = nullptr;

	thread_ptr _thread = nullptr;

	uint32_t _port = 0;

	SessionManager _session_manager;
};
