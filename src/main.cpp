/*
  A Spinlock based Event queue implementation
*/

#include <iostream>
#include <thread>
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
    std::vector<EventCallbackHandle> *vec = new std::vector<EventCallbackHandle>();
    this->event_map[event_name] = vec;
  }
  this->event_map[event_name]->push_back(listener);
  this->self.unlock();
}

void Loop::Listen(std::string event_name, E2::EventHandler *instance){
  this->self.lock();
  auto hasEvent = this->instance_map[event_name];
  if (hasEvent == nil){
    /* event doesn't exists */
    std::vector<E2::EventHandler*> *vec = new std::vector<E2::EventHandler*>();
    this->instance_map[event_name] = vec;
  }
  this->instance_map[event_name]->push_back(instance);
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
  TriggerListeners(event, event_name, callback)
  /* trigger all instance handler */
  TriggerListeners(instance, event_name, instance)
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
  ClearListener(event, event_name, handle)
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

void Loop::Exit(){
  this->self.lock();
  this->_isAlive = false;
  if (!this->event_queue->isClosed()){
    delete this->event_queue;
    this->event_queue = nil;
  }
  this->self.unlock();
}

Loop::~Loop(){
  if (this->event_queue != nil){
    delete this->event_queue;
  }
  if (this->event_map.empty() == false){
    this->event_map.clear();
  }
}

EventQueue::EventQueue(){
  this->events = *(new std::vector<EventData*>());
  this->thread = new std::thread(startThread, this, &this->closeFlag, &this->exitFlag);
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
  this->events.push_back(e);
  this->lock.unlock();
}

void EventQueue::join(){
  this->thread->join();
}
std::mutex* EventQueue::getLock(){
  return &this->lock;
}

EventQueue::~EventQueue(){
  if (this->isClosed()){
    return;
  }
  this->lock.lock();
  EventData *ptr;
  auto size = this->events.size();
  unsigned int index = 0;
  for(;index < size; index++){
    if (this->events[index]->data != nil) {
      delete this->events[index]->data;
    }
    delete this->events[index]->name;
    delete this->events[index];
  }
  /* prepare to send signal to the spinlock executor */
  this->closeFlag.lock();
  this->exitFlag = 1;
  this->closeFlag.unlock();
  /* wait for confirmation */
  this->closeFlag.lock();
  /* confirm exit, if not then let process handle it */
  if (this->exitFlag == THREAD_EXITED){
    delete this->thread;
  }
  this->closeFlag.unlock();
  this->events.clear();
  this->lock.unlock();
}
/* Spinlock strategy
*  On a modern multi-core processor CPU utilisation
*  will always be 0% if there isn't any element
*  in the queue i.e. thread will be in a pseudo
*  idle state. */

void E2::startThread(EventQueue *e, std::mutex *closeFlag, int *exitFlag){
  while(true){
    e->getLock()->lock();
    /* add a safe thread exit trigger */
    closeFlag->lock();
    if ((*exitFlag) == 1){
      *exitFlag = THREAD_EXITED;
      e->getLock()->unlock();
      closeFlag->unlock();
      break;
    }
    closeFlag->unlock();

    if (!e->events.empty()){
      /* we have a new entry */
      auto event = e->events[0];
      /* a function handler */
      if (event->instance == nil){
        event->callback(event, event->data);
      } else {
        /* a EventHandler class handler */
        event->instance->HandleEvent(event, event->data);
      }
      e->events.erase(e->events.begin());
    }
    e->getLock()->unlock();
    /* for a 2.1Ghz CPU, 2 cycles per nanoseconds */
    /* also thread sleep are timers and are not true */
    /* timers in nature */
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
  }
}