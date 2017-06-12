/*
  E2 lib's code stubs to avoid duplicating code
*/

#ifndef E2_STUB_H_
#define E2_STUB_H_

/* delete std::string instance of event name */
#define clear_e2_event_name(event) \
  if (event->name != nil)          \
    delete event->name;

/* delete struct event */
#define clear_e2_event_obj(event) \
  if (event != nil)               \
    delete event;

/* completley delete the event object */
#define clear_e2_event(event)  \
  if (event != nil){           \
    if (event->name != NULL)   \
      delete event->name;      \
    if (event->data != nil)    \
      delete event->data;      \
    delete event;              \
  }


#define ReturnIfNoListeners(type1, type2, name)                           \
  if (this->type1##_map[name] == nil && this->type2##_map[name] == nil) { \
    this->type1##_map.erase(name);                                        \
    this->type2##_map.erase(name);                                        \
    this->self.unlock();                                                  \
    return 0;                                                             \
  }

#define TriggerListeners(type, event_name, key)       \
  if (this->type##_map[event_name] != nil) {          \
    auto listeners = *(this->type##_map[event_name]); \
    for(auto el : listeners){                         \
      auto e = new EventData();                       \
      e->name = new std::string(event_name);          \
      e->key = el;                                    \
      e->data = data;                                 \
      this->event_queue->push(e);                     \
      ++count;                                        \
    }                                                 \
  } else {                                            \
    this->type##_map.erase(event_name);               \
  }

#define ClearEvent(type, event_name)           \
  if (this->type##_map[event_name] != nil) {   \
    auto _list = this->type##_map[event_name]; \
    _list->clear();                            \
  }                                            \
  this->type##_map.erase(event_name);

#define ClearListener(type, event_name, handler)  \
  if (this->type##_map[event_name] != nil) {      \
    auto _list = this->type##_map[event_name];    \
    _list->erase(std::remove(_list->begin(),      \
       _list->end(), handler), _list->end());     \
  } else {                                        \
    this->type##_map.erase(event_name);           \
  }

#define ListenCodeStub(map_name, event_name, cast)  \
  auto hasEvent = this->map_name##_map[event_name]; \
  if (hasEvent == nil){                             \
    /* event doesn't exists */                      \
    std::vector<E2:: cast>                          \
      *vec = new std::vector<E2:: cast>();          \
    this->map_name##_map[event_name] = vec;         \
  }

#endif // end E2_STUB_H_