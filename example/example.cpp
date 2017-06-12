#include <iostream>
#include "../src/main.hpp"
using namespace std;
using namespace E2;
Handle listener(Handle, Handle);

class FsWrap : public E2::EventHandler {
  private:
    int identifier; /* inode, fd */
  public:
    FsWrap(){
      this->identifier = -1; /* init state */
    }
    void HandleEvent(Handle event, Handle data){
      /* Trigger the events as they appear */
      E2::EventData *instance = (E2::EventData *)event;
      /* switch(instance->name) {case "end": break;} */
      cout << "File Event: " << *(instance->name) << endl;
      clear_e2_event(instance)
    }
    /*
      Event handlers to be overriden by extending class
    */
    virtual void onData(Handle data){}
    virtual void onEnd(){}
    virtual void onStart(){}
    virtual void onFlush(){}
    ~FsWrap(){

    }
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
  // call all fs/net and other threads before this
  // this is the end marker which will block the code
  auto FsQueue = new E2::Loop();
  auto fsObject = new FsWrap();
  /* todo: Add thread pool */
  /* Till then you can namespace events with inode values */
  /* or fd in case of net connection */
  FsQueue->Listen("onstart", fsObject);
  /* you can have functions as well as EventHandler classes */
  /* listening on the same event */
  FsQueue->Listen("onstart", &listener);
  FsQueue->Trigger("onstart", nil);
  /* Exit method suspends the existing event queue thread
    flushes all the events and event datas that are queued

    event_loop->Exit();
  */
  FsQueue->Unregister("onstart");
  int *sample = new int(0xfAAAAA);
  /* None will be triggered */
  FsQueue->Trigger("onstart", (Handle*)sample);
  FsQueue->Join();
  event_loop->Exit();
  if (event_loop->isAlive())
    event_loop->Join();
  return 0;
}

Handle listener(Handle event, Handle data){
  E2::EventData *e = (E2::EventData *)event;
  cout << "Event Triggered" << std::endl;
  cout << "Event name=> " << *(e->name) << std::endl;
  if (data != nil){
    cout << "The data is an integer: " << *((int*)data) <<endl;
  }
  /* 
    use the helper to clear event data if you want to clear
    the whole event object with data
    if not, then just call clear_e2_event_name(e) and clear_e2_event_obj(e)
    note: Don't call both together
  */
  // clear_e2_event_name(e) // delete event->name
  // clear_e2_event_obj(e) // delete event
  clear_e2_event(e) // this call deletes data, name and event itself
  return nil;
}