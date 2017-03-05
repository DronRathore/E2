__This is a fun project not for prod usage, many things need to be fixed in it.__
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
	std::string eName("push");
	man->Listen(&eName, &listener);
	man->Trigger(&eName, nil); // 2nd argument is for data
	// call all fs/net and other threads before this
	// this is the end marker which will block the code
  // you can create more EventMan and then wait for all of them
	man->Join();
	return 0;
}

Handle listener(Handle event, Handle data){
	E2::EventData *e = (E2::EventData *)event;
	cout << "Event Triggered" << std::endl;
	cout << "Event name=> " << *(e->name) << std::endl;
	return nil;
}
```
##ToDo
- Share a common semaphore to wait for all the eventMan to end and terminate all of them.
- Provide ```net/fs/time``` library helpers