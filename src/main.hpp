/*
	main.hpp
*/
#include <map>
#include <vector>
#include <list>
#include <thread>
#ifndef nil
	#define nil NULL
#endif

#ifndef MAX_THREADS
	#define MAX_THREADS 5
#endif

namespace E2{
	typedef void* Handle;
	typedef Handle (*EventCallbackHandle)(Handle, Handle);
	typedef std::map<std::string, std::vector<EventCallbackHandle>*> EventsMap;
	typedef struct edata{
		std::string *name;
		EventCallbackHandle callback;
		std::map<std::string, Handle> *data;
	}EventData;
	typedef struct event{
		std::string *name;
		EventCallbackHandle callback;
	} Event;
	class EventQueue{
		private:
			// std::thread current_proc;
			std::thread *thread;
		public:
			std::vector<EventData*> events;
			std::mutex lock;
			EventQueue();
			void push(EventData* e);
			std::mutex* getLock();
			void trigger();
			void join();
	};
	class EventMan{
		private:
			EventsMap event_map;
			Handle initQueue();
			EventQueue *event_queue;
		public:
			EventMan();
			Handle Listen(Handle event_name, EventCallbackHandle listener);
			Handle Trigger(Handle event_name, EventData *data);
			Handle Join();
	};
	// Callback Return type
	typedef struct cr{
		int type;
		Handle data;
	}CallbackReturn;
	void startThread(EventQueue*);
}
