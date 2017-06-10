/*
  A Spinlock based Event queue implementation
*/

#include <iostream>
#include <thread>
#include <chrono>
#include "main.hpp"
using namespace E2;
using namespace std;

EventMan::EventMan(){
  this->initQueue();
}

Handle EventMan::initQueue(){
  this->event_queue = new EventQueue();
  this->_isAlive = true;
  return nil;
}

void EventMan::Join(){
  this->event_queue->join();
}

void EventMan::Listen(std::string event_name, EventCallbackHandle listener){
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

int EventMan::Trigger(std::string event_name, Handle *data){
  /* do not trigger if exited */
  this->self.lock();
  if (!this->_isAlive){
    this->self.unlock();
    return 0;
  }
  if (this->event_map[event_name] == nil){
    this->event_map.erase(event_name);
    this->self.unlock();
    return 0;
  }
  auto listeners = *(this->event_map[event_name]);
  for(auto el : listeners){
    auto e = new EventData();
    e->name = new std::string(event_name);
    e->callback = el;
    e->data = data;
    this->event_queue->push(e);
  }
  this->self.unlock();
  return this->event_map[event_name]->size();
}

bool EventMan::isAlive(){
  this->self.lock();
  bool _isAlive = this->_isAlive;
  this->self.unlock();
  return _isAlive;
}

void EventMan::Exit(){
  this->self.lock();
  this->_isAlive = false;
  if (!this->event_queue->isClosed()){
    delete this->event_queue;
    this->event_queue = nil;
  }
  this->self.unlock();
}
EventMan::~EventMan(){
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
      break;
    }
    closeFlag->unlock();

    if (!e->events.empty()){
      /* we have a new entry */
      auto event = e->events[0];
      event->callback(event, event->data);
      e->events.erase(e->events.begin());
    }
    e->getLock()->unlock();
    /* for a 2.1Ghz CPU, 2 cycles per nanoseconds */
    /* also thread sleep are timers and are not true */
    /* timers in nature */
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
  }
}