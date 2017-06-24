/*
  console.cpp
  This is a utility for sync stdio ops, helps make your
  couts better.
*/
#include <iostream>
#include "main.hpp"
#include "console.hpp"

Handle E2::__console_printer(Handle event, Handle data){
  auto eInstance = (E2::EventData*) event;
  std::cout << *((std::string*)data) << std::endl;
  clear_e2_event(eInstance, std::string)
  return nil;
}

/* assign memory to static members */
E2::Loop console::loop;
std::mutex console::lock;
std::string console::buffer;
int console::num = 0;
