/*
  main.hpp
*/
#include <map>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include "stub.hpp"

#ifndef E2_LOOP_H_
#define E2_LOOP_H_
#define THREAD_EXITED 0x2
#define BLOCK_AND_EXIT 0x03

#ifndef MAX_THREADS
  #define MAX_THREADS 5
#endif

#ifndef nil
  #define nil NULL
#endif

namespace E2 {
  /* a generic void pointer, typecasting is user-domain task */
  typedef void* Handle;
  /* signature of our thread function */
  typedef Handle (*EventCallbackHandle)(Handle, Handle);
  
  class EventHandler{
    public:
      virtual void HandleEvent(E2::Handle, E2::Handle){}
      virtual ~EventHandler(){}
  };
  /* a wrapper to store data */
  class EventDataWrap{};

  typedef std::map<std::string, std::vector<EventCallbackHandle>*> EventsMap;
  typedef std::map<std::string, std::vector<std::unique_ptr<EventHandler>>*> HandlerInstance;
  
  struct EventData{
    std::string name;
    EventCallbackHandle callback;
    EventHandler* instance;
    Handle *data;
    std::shared_ptr<std::mutex> lock;
    std::shared_ptr<EventDataWrap> ptr;
  };

  class EventQueue{
    private:
      std::thread *thread;
      std::mutex closeFlag;
      int exitFlag;
    public:
      std::vector<EventData*> *events;
      std::mutex lock;
      EventQueue();
      void push(EventData* e);
      std::mutex* getLock();
      void trigger();
      void join();
      bool isClosed();
      int getExitFlag();
      void setExitFlag(int);
      void Stop();
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
      /* for more extensible listener types */
      template <typename Instance> 
        void Listen(
          std::string event_name,
          Instance instance
      ){
        this->self.lock();
        ListenCodeStub(instance, event_name, EventHandler)
        auto cast = reinterpret_cast<EventHandler*>(instance);
        this->instance_map[event_name]->push_back(
            std::unique_ptr<EventHandler>(cast));
        this->self.unlock();
      }

      int Trigger(std::string event_name, Handle *data);
      /* a template based event trigger for generic purpose */
      template <typename Instance> 
          bool Trigger (
            Instance instance,
            std::string event_name,
            Handle *data
      ) {
        EventHandler* handler = reinterpret_cast<EventHandler*>(instance);
        std::shared_ptr<EventDataWrap> ptr = std::make_shared<EventDataWrap>();
        std::shared_ptr<std::mutex> shared_lock = std::make_shared<std::mutex>();
        auto e = new EventData();
        e->name = event_name;
        e->instance = handler;
        e->data = data;
        e->ptr = ptr;
        e->lock = shared_lock;
        this->event_queue->push(e);
        ptr.reset();
        shared_lock.reset();
        return true;
      }
      bool Trigger (
        EventCallbackHandle callback,
        std::string event_name,
        Handle *data
      ) {
        std::shared_ptr<EventDataWrap> ptr = std::make_shared<EventDataWrap>();
        std::shared_ptr<std::mutex> shared_lock = std::make_shared<std::mutex>();
        auto e = new EventData();
        e->name = event_name;
        e->callback = callback;
        e->data = data;
        e->ptr = ptr;
        e->lock = shared_lock;
        this->event_queue->push(e);
        ptr.reset();
        shared_lock.reset();
        return true;
      }
      void Unregister(std::string event_name);
      void Unregister(std::string event_name, EventCallbackHandle listener);
      void Unregister(std::string event_name, EventHandler *instance);
      void UnregisterAll();
      void Join();
      void Freeze();
      bool Unfreeze();
      bool isAlive();
      void Stop();
      void StopSync();
      ~Loop();
  };
  void startThread(EventQueue*, std::mutex*);
}
#endif /* E2_LOOP_H_ */