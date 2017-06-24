/*
  E2 lib's code stubs to avoid duplicating code
*/

#ifndef E2_STUB_H_
#define E2_STUB_H_

#define efree(obj)    \
  if (obj != nullptr) \
    delete obj;
/* delete struct event and data too if we are the last */
/* to hold the shared_ptr */
#define clear_e2_event_obj(event) \
  if (event != nil)               \
    delete event;

/* completley delete the event object */
#define clear_e2_event(event, data_type)  \
  if (event != nil){           \
    if (event->ptr.use_count() == 1 && event->data != nil)  \
      delete (data_type *)event->data;  \
    delete event;              \
  }


#define ReturnIfNoListeners(type1, type2, name)                           \
  auto type1##iterator = this->type1##_map.find(name);                    \
  auto type2##iterator = this->type2##_map.find(name);                    \
  if ( type1##iterator != this->type1##_map.end() &&                      \
   type2##iterator != this->type2##_map.end()) {                          \
    this->self.unlock();                                                  \
    delete data;                                                          \
    return 0;                                                             \
  }

#define TriggerListeners(type, event_name, key, itr_type, ret_type)  \
    if (type##iterator != this->type##_map.end()) {                  \
      for(auto itr_type : *(type##iterator->second)){                \
        auto e = new EventData();                                    \
        e->name = event_name;                                        \
        e->key = ret_type;                                           \
        e->ptr = ptr;                                                \
        e->lock = shared_lock;                                       \
        e->data = data;                                              \
        this->event_queue->push(e);                                  \
        ++count;                                                     \
      }                                                              \
  }

#define ClearEvent(type, event_name)           \
  if (this->type##_map[event_name] != nil) {   \
    auto _list = this->type##_map[event_name]; \
    _list->clear();                            \
  }                                            \
  this->type##_map.erase(event_name);

#define ClearListener(type, event_name, handler)          \
  if (this->type##_map[event_name] != nil) {              \
    auto _list = this->type##_map[event_name];            \
    auto itr = std::find_if(_list->begin(), _list->end(), \
     [&](std::unique_ptr<EventHandler> &ptr){             \
      return ptr.get() == handler;                        \
    });                                                   \
    while (itr != _list->end()){                          \
      itr->release();                                     \
      _list->erase(itr);                                  \
      ++itr;                                              \
    }                                                     \
  } else {                                                \
    this->type##_map.erase(event_name);                   \
  }

#define ListenCodeStub(map_name, event_name, cast)  \
  auto hasEvent = this->map_name##_map[event_name]; \
  if (hasEvent == nil){                             \
    /* event doesn't exists */                      \
    std::vector<std::unique_ptr<E2:: cast>>                 \
      *vec = new std::vector<std::unique_ptr<E2:: cast>>(); \
    this->map_name##_map[event_name] = vec;         \
  }

#endif // end E2_STUB_H_