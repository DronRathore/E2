# E2
Create as many event loops you want, pass the loops to different network/fs/timer interrupt locked threads.
```c++
#include "src/main.hpp"
using namespace std;
using namespace E2;
Handle listener(Handle, Handle);
class FsWrap : public E2::EventHandler {
  public:
    void HandleEvent(Handle event, Handle data){
      /* Trigger the events as they appear */
      E2::EventData *instance = (E2::EventData *)event;
      /* switch(instance->name) {case "end": break;} */
      cout << "File Event: " << *(instance->name) << endl;
      clear_e2_event(instance)
    }

    ~FsWrap(){

    }
};
int main(int argc, char* argv[]){
  E2::Loop *man; // you can share this instance with other threads
  man = new E2::Loop();
  /* an object of derived class from EventHandler */
  auto fsObject = new FsWrap();

  man->Listen("push", &listener);
  man->Listen("push", fsObject); /* instance can listen too */

  man->Trigger("push", nil); // 2nd argument is for data
  
  Handle *data = (Handle *)new int(0x0fff);
  man->Freeze(); /* no trigger statement after this won't work */
  
  man->Trigger("push", data); /* won't work */
  
  man->Unfreeze(); /* triggers after this will work */

  man->Trigger("push", data); /* will work now */
  // call all fs/net and other threads before this
  // this is the end marker which will block the code
  // you can create more E2::Loop and then wait for all of them

  /* Exit method suspends the existing event queue thread
    flushes all the events and event datas that are queued
    event-man instance becomes useless after the exit call and
    will throw error

    man->Exit();
    delete man;
  */
  man->Join();
  return 0;
}

Handle listener(Handle event, Handle data){
  E2::EventData *e = (E2::EventData *)event;
  cout << "Event Triggered" << std::endl;
  cout << "Event name=> " << *(e->name) << std::endl;
  if (data != nil){
    cout << "The passed integer is: " << *((int*)data);
  }
  clear_e2_event(e); // a helper to clear event data
  return nil;
}
```
## API
```c++ 
bool Loop.isAlive()
```
Returns the current state of the event queue, if false than the queue will throw error for any operation

```c++ 
bool Loop.Freeze()
```
Freezes the event loop, all trigger calls will be ignored

```c++ 
bool Loop.Unfreeze()
```
Unfreeze the event loop, event trigger will resume from the point this function is called


```c++ 
void Loop.Push(string name, &handler)
```

Pushes a new event handler for an event, the handler parameter is a function pointer which can be of the below signature 
```c++
Handle func_name(Handle, Handle)
```
__Or an instance of any derived class from EventHandler__
```c++
class MyClass:public EventHandler{}
```

```c++
int Loop.Trigger(string name, Handle data)
```
Triggers all the event listeners of a specific event and passes the data to the handlers, if no such event exists than its a noop(). The function returns the number of listeners triggered


```c++ 
bool Loop.Unregister(string event_name)
```
Unregister all the event handlers for a given event_name


```c++ 
bool Loop.Unregister(string event_name, function listener)
```
Unregister a given listener for an event

```c++ 
bool Loop.Unregister(string event_name, EventHandler *instance)
```
Unregister a given EventHandler class instance for a given event


```c++ 
void Loop.Join(void)
```

Locks the Spinlock thread and waits for its completion.


```c++ 
void Loop.Exit(void)
```
Kills the Spinlock thread. Call ```delete Loop;``` manually after this to free its memory. You can use this to trigger the end of event loop manually.

### Listeners Helpers
```c++
  class EventHandler
```
You can extend the abstract EventHandler class to create your own event handling class that can be attached as an event listener.
The ```EventHandler``` class has following methods that needs to be overriden

```c++
  virtual void HandleEvent(Handle event, Handle data)
```
This function after overriden will be triggered for any event the instance is attached to, you can the event name from event structure.

```c++
  ~EventHandler()
```
You need to have a destructor of your class.


The library by default doesn't clear the parameters passed to the listeners, its upto the users to clear the memory.
```c++
clear_e2_event(event)
```

Use this to clear the complete event struct passed to your event listener.
```c++
Handle file_read_listener(Handle event, Handle data) {
  E2::EventData *eData = (E2::EventData*)event;
  /* do something with eData and data */
  clear_e2_event(e)
}
```

```c++ 
clear_e2_event_name(event)
```

Use this to clear the std::string event name if you are going to use the data pointer, this is not done by the library automatically
```c++
Handle file_read_listener(Handle event, Handle data) {
  E2::EventData *eData = (E2::EventData*)event;
  /* do something with eData and data */
  clear_e2_event_name(e)
}
```

__Note__: Call atleast one helper from the above to clear unused memory.

## ToDo
- Provide ```net/fs/time``` library helpers