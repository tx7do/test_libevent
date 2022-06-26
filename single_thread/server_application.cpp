#include "server_application.h"

#include <cassert>
#include <cstring>
#include <csignal>
#include <netinet/tcp.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/bufferevent_struct.h>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>

ServerApplication::ServerApplication(uint32_t port)
	: _port(port)
{
}

ServerApplication::~ServerApplication()
{
	stopEvent();
	stopThread();
}

int ServerApplication::run()
{
	start();
	return 0;
}

void ServerApplication::start()
{
	startThread();
}

void ServerApplication::stop()
{
	stopEvent();
	stopThread();
}

void ServerApplication::startEvent()
{
	if (_base != nullptr)
	{
		stopEvent();
	}

	::evthread_use_pthreads();

	auto cfg = ::event_config_new();
	assert(cfg);
	if (::event_config_set_flag(cfg, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST) < 0)
	{
		return;
	}

	_base = ::event_base_new_with_config(cfg);
	assert(_base);
	::event_config_free(cfg);

	startAcceptor(_port);

	addInterruptSignalEvent();
}

void ServerApplication::stopEvent()
{
	if (_interrupt_signal_event != nullptr)
	{
		::event_free(_interrupt_signal_event);
		_interrupt_signal_event = nullptr;
	}
	if (_listener != nullptr)
	{
		::evconnlistener_free(_listener);
		_listener = nullptr;
	}
	if (_base != nullptr)
	{
		::event_base_loopbreak(_base);
		::event_base_free(_base);
		_base = nullptr;
	}
}

void ServerApplication::startThread()
{
	if (_thread == nullptr)
	{
		_thread = std::make_shared<thread_t>([this]()
		{
			//fmt::print("Work Thread ID: {}\n", std::this_thread::get_id());
			startEvent();
			if (_base != nullptr)
			{
				::event_base_loop(_base, EVLOOP_NO_EXIT_ON_EMPTY);
			}
			stopEvent();
		});
		assert(_thread);
	}
}

void ServerApplication::stopThread()
{
	if (_thread != nullptr)
	{
		_thread->join();
		_thread.reset();
	}
}

void ServerApplication::startAcceptor(uint32_t port)
{
	struct sockaddr_in sin{};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	_listener = ::evconnlistener_new_bind(_base,
		listener_cb,
		(void*)this,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		-1,
		(struct sockaddr*)&sin,
		sizeof(sin));
}

void ServerApplication::addInterruptSignalEvent()
{
	_interrupt_signal_event = ::evsignal_new(_base, SIGINT, signal_cb, (void*)this);
	::event_add(_interrupt_signal_event, nullptr);
}

#define BUFFER_MAX_READ (1024 * 1024)

void ServerApplication::listener_cb(struct evconnlistener* listener,
	evutil_socket_t fd,
	struct sockaddr* sa,
	int socklen,
	void* user_data)
{
	auto self = (ServerApplication*)user_data;
	auto* pSin = (sockaddr_in*)sa;

	int optval = 1;
	::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

	int nRecvBufLen = BUFFER_MAX_READ;
	::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBufLen, sizeof(int));

	auto bev = ::bufferevent_socket_new(self->_base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev)
	{
		fprintf(stderr, "Error constructing bufferevent!");
		::evutil_closesocket(fd);
		::event_base_loopbreak(self->_base);
		return;
	}

	auto session = self->_session_manager.addSession(fd, *pSin);

	::bufferevent_setcb(bev, conn_read_cb, conn_write_cb, conn_event_cb, user_data);

	::bufferevent_enable(bev, EV_READ | EV_WRITE | EV_CLOSED | EV_TIMEOUT | EV_PERSIST);

	::event_set_fatal_callback(event_fatal_cb);

	conn_event_cb(bev, BEV_EVENT_CONNECTED, (void*)session);

	::bufferevent_set_max_single_read(bev, BUFFER_MAX_READ);
	::bufferevent_set_max_single_write(bev, BUFFER_MAX_READ);
}

void ServerApplication::conn_read_cb(struct bufferevent* bev, void* user_data)
{
	auto session = (Session*)user_data;

	auto input = ::bufferevent_get_input(bev);
	if (!input)
	{
		return;
	}

	size_t len = ::evbuffer_get_length(input);
	unsigned char* pData = ::evbuffer_pullup(input, len);
//	session->add_buff((const char *)pData, len);
	::evbuffer_drain(input, len);
}

void ServerApplication::conn_write_cb(struct bufferevent* bev, void* user_data)
{
	auto session = (Session*)user_data;
}

void ServerApplication::conn_event_cb(struct bufferevent* bev, short events, void* user_data)
{
	auto session = (Session*)user_data;

	if (events & BEV_EVENT_CONNECTED)
	{

	}
	else
	{
		session->manager->removeSession(session);
	}
}

void ServerApplication::log_cb(int severity, const char* msg)
{

}

void ServerApplication::event_fatal_cb(int err)
{

}

void ServerApplication::signal_cb(int sig, short events, void* user_data)
{
	auto self = (ServerApplication*)user_data;

	struct timeval delay = { 2, 0 };
	::event_base_loopexit(self->_base, &delay);
}
