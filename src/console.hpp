/*
  Console Lib v0.1
  Console utility for systematic stdio operations
  throughout the use of E2 library
*/
#include <iostream>
#include <string>
#include <sstream>

#ifndef E2_CONSOLE_H_
#define E2_CONSOLE_H_
#include "main.hpp"
using namespace E2;

namespace E2{
  Handle __console_printer(Handle event, Handle data);
  class console {
    private:
      static std::mutex lock;
      static std::string buffer;
      static int num;
      static E2::Loop loop;  // feed me!
    public:
      static std::mutex* getLock(){
        return &lock;
      }
      template <typename... Args>
        static void log(Args&&... args){
          console::prepare(std::forward<Args>(args)...);
      }
      /* this function will work in sync to avoid race conditions */
      template <typename... Args>
        static void read(Args&&... args){
          lock.lock();
          console::prepareForRead(std::forward<Args>(args)...);
          lock.unlock();
      }
      template <typename Arg>
        static void prepare(Arg&& args){
          std::stringstream stream;
          stream << std::forward<Arg>(args);
          buffer += (num != 0 ? " ": "") + stream.str();
          /* donot kill the original string data */
          std::string *buffer_ptr = new std::string(buffer);
          loop.Trigger(&__console_printer, "stdout", (Handle*)buffer_ptr);
          buffer = "";
          num = 0;
      }
      template <typename T, typename... Args>
        static void prepare(T&& val, Args&&... args){
          std::stringstream stream;
          num = 1;
          stream << std::forward<T>(val);
          buffer += stream.str();
          console::prepare(std::forward<Args>(args)...);
      }
      
      template <typename Arg>
        static void prepareForRead(Arg&& args){
          std::cin >> std::forward<Arg>(args);
      }
      
      template <typename T, typename... Args> 
        static void prepareForRead(T&& val, Args&&... args){
          std::cin >> std::forward<T>(val);
          console::prepareForRead(std::forward<Args>(args)...);
    }
  };

  template <typename... Args> static void log(Args&&... args);
  template <typename... Args> static void log(Args&&... args){
    console::getLock()->lock();
    console::log(std::forward<Args>(args)...);
    console::getLock()->unlock();
  }
}
#endif //E2_CONSOLE_H_