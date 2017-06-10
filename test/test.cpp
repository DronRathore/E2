#include <iostream>
#include "../src/main.hpp"
using namespace std;
using namespace E2;
Handle listener(Handle, Handle);

int main(int argc, char* argv[]){
  E2::EventMan *man; /* you can share this instance with other threads */
  man = new E2::EventMan();
  man->Listen("push", &listener);
  
  int *data = new int(123);
  man->Trigger("push", (Handle *)data);
  man->Trigger("push", nil);
  man->Trigger("push", nil);
  /* If event is not present then its a noop */
  man->Trigger("lol", nil);
  // call all fs/net and other threads before this
  // this is the end marker which will block the code
  man->Exit();
  if (man->isAlive())
    man->Join();
  /* Exit method suspends the existing event queue thread
    flushes all the events and event datas that are queued

    man->Exit();
  */
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