/*
  A Spinlock based Event queue implementation
*/

#include <iostream>
#include <thread>
#include <algorithm>
#include <chrono>
#include "main.hpp"

using namespace E2;
using namespace std;

Loop::Loop(){
  this->initQueue();
}

Handle Loop::initQueue(){
  this->event_queue = new EventQueue();
  this->_isAlive = true;
  return nil;
}

void Loop::Join(){
  this->event_queue->join();
}

void Loop::Listen(std::string event_name, EventCallbackHandle listener){
  this->self.lock();
  auto hasEvent = this->event_map[event_name]; 
  if (hasEvent == nil){
    /* event doesn't exists */
    std::vector<EventCallbackHandle>
      *vec = new std::vector<EventCallbackHandle>(); 
    this->event_map[event_name] = vec;         
  }
  this->event_map[event_name]->push_back(listener);
  this->self.unlock();
}

void Loop::Listen(std::string event_name, E2::EventHandler *instance){
  this->self.lock();
  if (!this->_isAlive){
    this->self.unlock();
    return;
  }
  ListenCodeStub(instance, event_name, EventHandler)
  this->instance_map[event_name]->push_back(std::unique_ptr<E2::EventHandler>(instance));
  this->self.unlock();
}

int Loop::Trigger(std::string event_name, Handle *data){
  /* do not trigger if exited */
  this->self.lock();
  if (!this->_isAlive){
    this->self.unlock();
    return 0;
  }
  ReturnIfNoListeners(event, instance, event_name)
  
  int count = 0;
  /* trigger all the function handler */
  bool isInstance = false;
  std::shared_ptr<EventDataWrap> ptr = std::make_shared<EventDataWrap>();
  std::shared_ptr<std::mutex> shared_lock = std::make_shared<std::mutex>();
  TriggerListeners(event, event_name, callback, el, el)
  /* trigger all instance handler */
  isInstance = true;
  TriggerListeners(instance, event_name, instance, &el, (el.get()))
  ptr.reset();
  shared_lock.reset();
  this->self.unlock();
  return count;
}

void Loop::Unregister(std::string event_name){
  this->self.lock();
  ClearEvent(event, event_name)
  ClearEvent(instance, event_name)
  this->self.unlock();
}

/* Unregister from events_map of static functions */
void Loop::Unregister(std::string event_name, EventCallbackHandle handle){
  this->self.lock();
  if (this->event_map[event_name] != nil) {      
    auto _list = this->event_map[event_name];    
    _list->erase(std::remove(_list->begin(),      
       _list->end(), handle), _list->end());     
  } else {
    this->event_map.erase(event_name);
  }
  this->self.unlock();
}

/* Unregister from instance_map */
void Loop::Unregister(std::string event_name, EventHandler* handle){
  this->self.lock();
  ClearListener(instance, event_name, handle)
  this->self.unlock();
}

/* Unregister all the listeners */
void Loop::UnregisterAll(){
  this->self.lock();
  auto event_itr = this->event_map.begin();
  for(;event_itr != this->event_map.end(); ++event_itr){
    ClearEvent(event, event_itr->first)
  }
  auto instance_itr = this->instance_map.begin();
  for(;instance_itr != this->instance_map.end(); ++instance_itr){
    ClearEvent(instance, instance_itr->first)
  }
  this->self.unlock();
}

/* Blocks any new event trigger */
void Loop::Freeze(){
  this->self.lock();
  if (this->_isAlive == true){
    this->_isAlive = false;
  }
  this->self.unlock();
}

/* Unfreeze a frozen event queue */
bool Loop::Unfreeze(){
  bool done = false;
  this->self.lock();
  if (this->event_queue->isClosed()){
    this->_isAlive = true;
    done = true;
  }
  this->self.unlock();
  return done;
}

bool Loop::isAlive(){
  this->self.lock();
  bool _isAlive = this->_isAlive;
  this->self.unlock();
  return _isAlive;
}

void Loop::Stop(){
  this->self.lock();
  this->_isAlive = false;
  this->event_queue->Stop();
  this->self.unlock();
}

void Loop::StopSync(){
  this->self.lock();
  this->_isAlive = false;
  this->event_queue->Stop();
  this->event_queue->join();
  this->self.unlock();
}

Loop::~Loop(){
  if (this->event_queue != nil){
    this->event_queue->Stop();
    this->event_queue->join();
  }
  if (this->event_map.empty() == false){
    auto iterator = this->event_map.begin();
    do {
      iterator->second->clear();
      delete iterator->second;
      ++iterator;
    } while(iterator != this->event_map.end());
    this->event_map.clear();
  }
  if (this->instance_map.empty() == false){
    auto iterator = this->instance_map.begin();
    do {
      auto vector_iterator = iterator->second->begin();
      
      do {
        vector_iterator->release();
        ++vector_iterator;
      } while(vector_iterator != iterator->second->end());

      iterator->second->clear();
      delete iterator->second;
      ++iterator;
    } while(iterator != this->instance_map.end());

    this->instance_map.clear();
  }
  delete this->event_queue;
}

EventQueue::EventQueue(){
  this->exitFlag = 0;
  this->events = new std::vector<EventData*>();
  this->thread = new std::thread(startThread, this, &this->closeFlag);
}

void EventQueue::Stop(){
  this->setExitFlag(THREAD_EXITED);
}

int EventQueue::getExitFlag(){
  return this->exitFlag;
}
void EventQueue::setExitFlag(int flag){
  this->exitFlag = flag;
}
bool EventQueue::isClosed (){
  if (this->thread == nil)
    return true;
  this->closeFlag.lock();
  bool hasClosed = this->exitFlag == THREAD_EXITED;
  this->closeFlag.unlock();
  return hasClosed;
}

void EventQueue::push(EventData *e){
  this->lock.lock();
  this->events->push_back(e);
  this->lock.unlock();
}

void EventQueue::join(){
  if (this->thread->joinable())
    this->thread->join();
}
std::mutex* EventQueue::getLock(){
  return &this->lock;
}

EventQueue::~EventQueue(){
  if (this->getExitFlag() != THREAD_EXITED){
    this->closeFlag.lock();
    this->setExitFlag(THREAD_EXITED);
    this->closeFlag.unlock();
  }
  if (this->thread->joinable())
    this->thread->join();
  this->lock.lock();
  this->closeFlag.lock();
  auto size = this->events->size();
  auto index = this->events->begin();
  if (size > 0){
    do {
      if ( (*index)->data != nullptr)
        delete (*index)->data;

      if (*index != nil) {
        delete *index;
      }
      ++index;
    } while(index != this->events->end());
  }
  this->closeFlag.unlock();
  this->events->clear();
  delete this->events;
  delete this->thread;
  this->lock.unlock();
}
/* Spinlock strategy
*  On a modern multi-core processor CPU utilisation
*  will always be 0% if there isn't any element
*  in the queue i.e. thread will be in a pseudo
*  idle state. */

void E2::startThread(EventQueue *e, std::mutex *closeFlag){
  int eFlag = 0;
  while(true){
    /* add a safe thread exit trigger */
    if(closeFlag->try_lock()){
      if (e->getExitFlag() == THREAD_EXITED){
        if (e->events->empty()){
          eFlag = THREAD_EXITED;
          break;
        }
      }
    }
    if(e->getLock()->try_lock()){
      if (!e->events->empty()){
        /* we have a new entry */
        auto event = (*(e->events))[0];
        /* a function handler */
        if (event->instance == nil){
          event->callback(event, event->data);
        } else {
          /* a EventHandler class handler */
          event->instance->HandleEvent(event, event->data);
        }
        e->events->erase(e->events->begin());
      }
    }
    closeFlag->unlock();
    if (eFlag != THREAD_EXITED)
      e->getLock()->unlock();
    /* for a 2.1Ghz CPU, 2 cycles per nanoseconds */
    /* also thread sleep are timers and are not true */
    /* timers in nature */
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
  }
  if (eFlag == THREAD_EXITED){
    closeFlag->unlock();
    e->getLock()->unlock();
  }
  return;
}