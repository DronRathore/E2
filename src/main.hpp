/*
  main.hpp
*/
#include <map>
#include <vector>
#include <list>
#include <thread>

#ifndef E2_MAIN_H_
#define E2_MAIN_H_

#ifndef nil
  #define nil NULL
#endif

#ifndef MAX_THREADS
  #define MAX_THREADS 5
#endif

#ifndef THREAD_EXITED
  #define THREAD_EXITED 0x2
#endif

/* delete std::string instance of event name */
#ifndef clear_e2_event_name
  #define clear_e2_event_name(event) \
            if (event->name != nil)\
              delete event->name;
#endif
/* delete struct event */
#ifndef clear_e2_event_obj
  #define clear_e2_event_obj(event) \
            if (event != nil)\
              delete event;
#endif

/* completley delete the event object */
#ifndef clear_e2_event
  #define clear_e2_event(event) \
          if (event != nil){ \
            if (event->name != NULL) \
              delete event->name; \
            if (event->data != nil) \
              delete event->data; \
            delete event; \
          }
#endif

namespace E2{
  /* a generic void pointer, typecasting is user-domain task */
  typedef void* Handle;
  /* signature of our thread function */
  typedef Handle (*EventCallbackHandle)(Handle, Handle);
  typedef std::map<std::string, std::vector<EventCallbackHandle>*> EventsMap;
  typedef struct edata{
    std::string *name;
    EventCallbackHandle callback;
    Handle *data;
  }EventData;
  class EventQueue{
    private:
      std::thread *thread;
      std::mutex closeFlag;
      int exitFlag = 0;
    public:
      std::vector<EventData*> events;
      std::mutex lock;
      EventQueue();
      void push(EventData* e);
      std::mutex* getLock();
      void trigger();
      void join();
      bool isClosed();
      ~EventQueue();
  };
  class EventMan{
    private:
      EventsMap event_map;
      Handle initQueue();
      EventQueue *event_queue;
    public:
      EventMan();
      void Listen(std::string event_name, EventCallbackHandle listener);
      int Trigger(std::string event_name, Handle *data);
      void Exit();
      void Join();
      ~EventMan();
  };
  // Callback Return type
  typedef struct cr{
    int type;
    Handle data;
  }CallbackReturn;
  void startThread(EventQueue*, std::mutex*, int*);
}
#endif /* E2_MAIN_H_ */