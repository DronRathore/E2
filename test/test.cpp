#include <iostream>
#include "src/main.hpp"
using namespace std;
using namespace E2;
Handle listener(Handle, Handle);

int main(int argc, char* argv[]){
	E2::EventMan *man; // you can share this instance with other threads
	man = new E2::EventMan();
	std::string eName("push");
	cout << "Pushing Event"<<std::endl;
	man->Listen(&eName, &listener);
	man->Trigger(&eName, nil);
	// call all fs/net and other threads before this
	// this is the end marker which will block the code
	man->Join();
	return 0;
}

Handle listener(Handle event, Handle data){
	E2::EventData *e = (E2::EventData *)event;
	cout << "Event Triggered" << std::endl;
	cout << "Event name=> " << *(e->name) << std::endl;
	return nil;
}