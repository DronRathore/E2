# E2
Create as many event loops you want, pass the loops to different network/fs/timer interrupt locked threads.
```c++
#include "src/main.hpp"
using namespace std;
using namespace E2;
Handle listener(Handle, Handle);

int main(int argc, char* argv[]){
  E2::EventMan *man; // you can share this instance with other threads
  man = new E2::EventMan();
  man->Listen("push", &listener);
  man->Trigger("push", nil); // 2nd argument is for data
  Handle *data = (Handle *)new int(0x0fff);
  man->Trigger("push", data)
  // call all fs/net and other threads before this
  // this is the end marker which will block the code
  // you can create more EventMan and then wait for all of them

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
bool EventMan.isAlive()
```
Returns the current state of the event queue, if false than the queue will throw error for any operation


```c++ 
void EventMan.Push(string name, &handler)
```

Pushes a new event handler for an event, the handler parameter is a function pointer which should be of the below signature 
```c++
Handle func_name(Handle, Handle)
```


```c++
int EventMan.Trigger(string name, Handle data)
```
Triggers all the event listeners of a specific event and passes the data to the handlers, if no such event exists than its a noop(). The function returns the number of listeners triggered


```c++ 
void EventMan.Join(void)
```

Locks the Spinlock thread and waits for its completion.


```c++ 
void EventMan.Exit(void)
```
Kills the Spinlock thread. Call ```delete EventMan;``` manually after this to free its memory. You can use this to trigger the end of event loop manually.

### Listeners Helpers
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