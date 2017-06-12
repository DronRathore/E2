/*
  main.hpp
*/
#include <map>
#include <vector>
#include <list>
#include <thread>

#ifndef E2_LOOP_H_
#define E2_LOOP_H_

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
            if (event->name != nil)  \
              delete event->name;
#endif
/* delete struct event */
#ifndef clear_e2_event_obj
  #define clear_e2_event_obj(event) \
            if (event != nil)       \
              delete event;
#endif

/* completley delete the event object */
#ifndef clear_e2_event
  #define clear_e2_event(event)      \
          if (event != nil){         \
            if (event->name != NULL) \
              delete event->name;    \
            if (event->data != nil)  \
              delete event->data;    \
            delete event;            \
          }
#endif

#ifndef ReturnIfNoListeners
  #define ReturnIfNoListeners(type1, type2, name)                           \
    if (this->type1##_map[name] == nil && this->type2##_map[name] == nil) { \
      this->type1##_map.erase(name);                                        \
      this->type2##_map.erase(name);                                        \
      this->self.unlock();                                                  \
      return 0;                                                             \
    }
#endif

#ifndef TriggerListeners
  #define TriggerListeners(type, event_name, key)       \
    if (this->type##_map[event_name] != nil) {          \
      auto listeners = *(this->type##_map[event_name]); \
      for(auto el : listeners){                         \
        auto e = new EventData();                       \
        e->name = new std::string(event_name);          \
        e->key = el;                                    \
        e->data = data;                                 \
        this->event_queue->push(e);                     \
        ++count;                                        \
      }                                                 \
    } else {                                            \
      this->type##_map.erase(event_name);               \
    }
#endif

#ifndef ClearEvent
  #define ClearEvent(type, event_name)           \
    if (this->type##_map[event_name] != nil) {   \
      auto _list = this->type##_map[event_name]; \
      _list->clear();                            \
    }                                            \
    this->type##_map.erase(event_name);
#endif

#ifndef ClearListener
  #define ClearListener(type, event_name, handler)  \
    if (this->type##_map[event_name] != nil) {      \
      auto _list = this->type##_map[event_name];    \
      _list->erase(std::remove(_list->begin(),      \
         _list->end(), handler), _list->end());     \
    } else {                                        \
      this->type##_map.erase(event_name);           \
    }
#endif

namespace E2{
  /* a generic void pointer, typecasting is user-domain task */
  typedef void* Handle;
  /* signature of our thread function */
  typedef Handle (*EventCallbackHandle)(Handle, Handle);
  
  class EventHandler{
    public:
      virtual void HandleEvent(E2::Handle, E2::Handle){}
      virtual ~EventHandler(){}
  };

  typedef std::map<std::string, std::vector<EventCallbackHandle>*> EventsMap;
  typedef std::map<std::string, std::vector<EventHandler*>*> HandlerInstance;
  
  typedef struct edata{
    std::string *name;
    EventCallbackHandle callback;
    EventHandler* instance;
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
  class Loop{
    private:
      EventsMap event_map;
      HandlerInstance instance_map;
      Handle initQueue();
      EventQueue *event_queue;
      bool _isAlive;
      std::mutex self;
    public:
      Loop();
      void Listen(std::string event_name, EventCallbackHandle listener);
      void Listen(std::string event_name, EventHandler *instance);
      int Trigger(std::string event_name, Handle *data);
      void Unregister(std::string event_name);
      void Unregister(std::string event_name, EventCallbackHandle listener);
      void Unregister(std::string event_name, EventHandler *instance);
      void UnregisterAll();
      void Exit();
      void Join();
      void Freeze();
      bool Unfreeze();
      bool isAlive();
      ~Loop();
  };
  // Callback Return type
  typedef struct cr{
    int type;
    Handle data;
  }CallbackReturn;
  void startThread(EventQueue*, std::mutex*, int*);
}
#endif /* E2_LOOP_H_ */