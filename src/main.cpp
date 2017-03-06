/*

	Pseudo Events lib C++
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
	return nil;
}
Handle EventMan::Join(){
	this->event_queue->join();
	return nil;
}
Handle EventMan::Listen(Handle event_name, EventCallbackHandle listener){
	std::string *name  = (std::string *) event_name;
	auto hasEvent = this->event_map[*name];
	if (hasEvent == nil){
		// event doesn't exists
		std::vector<EventCallbackHandle> *vec = new std::vector<EventCallbackHandle>();
		this->event_map[*name] = vec;
	}
	this->event_map[*name]->push_back(listener);
	return nil;
}

Handle EventMan::Trigger(Handle event_name, EventData *data){
	auto name = (std::string *) event_name;
	auto listeners = *(this->event_map[*name]);
	for(auto el : listeners){
		auto e = new EventData();
		e->name = name;
		e->callback = el;
		e->data = nil;
		this->event_queue->push(e);
	}
	return this->event_map[*name];
}

EventQueue::EventQueue(){
	this->events = *(new std::vector<EventData*>());
	this->thread = new std::thread(startThread, this);
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

void E2::startThread(EventQueue *e){
	while(true){
		e->getLock()->lock();
		if (!e->events.empty()){
			// we have a new entry
			auto event = e->events[0];
			event->callback(event, event->data);
			e->events.erase(e->events.begin());
		}
		e->getLock()->unlock();
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}
}
