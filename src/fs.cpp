
class FsWrap : public E2::EventHandler {
  private:
    int identifier; /* inode, fd */
    EventMan* queue;
  public:
    FsWrap(){
      this->identifier = -1; /* init state */
    }
    void setQueue(EventMan *queue){
      this->queue = queue;
    }
    bool Open(std::string file_name){
      if (this->queue == nil){
        throw std::invalid_argument("cannot open file without a event queue");
      }
      /* open file, push listeners in event loop */
      return false;
    }
    void HandleEvent(Handle event_instance, Handle data){
      /* Trigger the events as they appear */
      E2::EventData *event = (E2::EventData *)event;
      /* switch(event->name) {case "end": break;} */
      cout << "File Event: " << *(event->name) << endl;
      clear_e2_event(event)
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
