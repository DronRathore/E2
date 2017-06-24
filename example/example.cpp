#include <iostream>
#include "../src/main.hpp"
#include "../src/console.hpp"
using namespace std;
using namespace E2;

Handle listener(Handle, Handle);

class FsWrap : public E2::EventHandler {
  public:
    void HandleEvent(Handle event, Handle data){
      /* Trigger the events as they appear */
      E2::EventData *instance = (E2::EventData *)event;
      /* if you are modifying the value of data then use the inbuilt lock */
      instance->lock.get()->lock();
      /* switch(instance->name) {case "end": break;} */
      if (data != nil) {
        E2::log("File Event:", instance->name, *(std::string*)data);
      }
      int input;
      E2::log("Input an integer:");
      console::read(input);
      E2::log("User input was", input);
      /* don't forget to release the lock */
      instance->lock.get()->unlock();
      /* second parameter is the pointer type of data */
      clear_e2_event(instance, std::string)
    }
    /*
      Event handlers to be overriden by extending class
    */
    virtual void onData(Handle data){}
    virtual void onEnd(){}
    virtual void onStart(){}
    virtual void onFlush(){}
};

int main(int argc, char* argv[]){
  E2::Loop *event_loop; /* you can share this instance with other threads */
  event_loop = new E2::Loop();
  event_loop->Listen("push", &listener);
  
  Handle* data = (Handle*)new int(123);
  event_loop->Trigger("push", data);
  event_loop->Trigger("push", nil);
  event_loop->Trigger("push", nil);
  /* If event is not present then its a noop */
  event_loop->Trigger("lol", nil);

  auto FsQueue = new E2::Loop();
  
  auto fsObject = new FsWrap();
  /* todo: Add thread pool */
  /* a generic to help extend the functionalities of an EventHandler*/

  /* you can directly push an EventHandler in queue through this */
  FsQueue->Trigger<FsWrap*>(fsObject, "onstart", nil);
  /* listening on the same event */
  FsQueue->Listen<FsWrap*>("onend", fsObject);
  /* you can have functions as well as EventHandler classes */
  FsQueue->Listen("onstart", &listener);
  FsQueue->Trigger("onstart", nil);
  
  int *sample = new int(0xfAAAAA);
  FsQueue->Trigger("onstart", (Handle*)sample);
  std::string *str = new string("Bow down puny species!");
  FsQueue->Trigger("onend", (Handle*)str);
  E2::log("Stopping the thread");
  FsQueue->StopSync();
  E2::log("Other way of killing the loop is to execute delete");
  /* clear you class instaces */
  delete event_loop;
  delete FsQueue;
  delete fsObject;
  return 0;
}

Handle listener(Handle event, Handle data){
  E2::EventData *e = (E2::EventData *)event;
  if (data != nil){
    E2::log("The data is an integer: ", *((int*)data));
  }
  /* 
    use the helper to clear event data if you want to clear
    the whole event object with data
    if not, then just call clear_e2_event_name(e) and clear_e2_event_obj(e)
    note: Don't call both together
  */
  // clear_e2_event_name(e) // delete event->name
  // clear_e2_event_obj(e) // delete event
  clear_e2_event(e, int) // this call deletes data, name and event itself
  return nil;
}