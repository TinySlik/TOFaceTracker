//
//  EventManager.h
//  EventManager
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_EVENT_MANAGER_H
#define TO_EVENT_MANAGER_H

#include <vector>
#include <map>
#include <string>

using namespace std;
namespace TO {
    #define TEMPLATE template <typename Class> 
    
    // Abstract Class for EventHandler to notify of a change
    class EventHandlerBase {
    public:
        virtual void execute() = 0;
    };
    
    // Event Handler Class : Handles Callback
    template <typename Class>
    class EventHandler : public EventHandlerBase{
        // Defining type for function pointer
        typedef void (Class::*_fptr)(void);
        
        
        
    public:
        // Object of the Listener
        Class *object;
        // Function for callback
        _fptr function;
        
        EventHandler(Class *obj, _fptr func) {
            object = obj;
            function = func;
        }
        
        void execute() {
            (object->*function)();
        }
    };
    
    // Class to create a event
    class Event {
        // To store all listeners of the event
        typedef std::map<int, EventHandlerBase*> EventHandlerMap;
        EventHandlerMap handlers;
        int count;
    public:
        
        template <typename Class>
        void addListener(Class *obj, void (Class::*func)(void)) {
            handlers[count] = new EventHandler<Class>(obj, func);
            count++;
        }
        
        void execute() {
            for (EventHandlerMap::iterator it = handlers.begin(); it != handlers.end(); ++it) {
                it->second->execute();
            }
        }

    };
    
    class EventManager {
        struct EventType {
            Event *event;
            string name;
        };
        
        std::vector<EventType> _events;
        
        static EventManager *_Instance;
        
        EventManager(){};
    public:
        static void DeleteInstance(){
            if (_Instance) {
                delete  _Instance;
                _Instance = NULL;
            }
        }
        static EventManager* Instance() {
            if (!_Instance) {
                _Instance = new EventManager();
            }
            return _Instance;
        }
        
        void createEvent(string name) {
			for(vector<EventType>::iterator it = _events.begin(); it != _events.end(); ++it) {
				EventType e = *it;
				if(e.name.compare(name) == 0)
					return;
			}
            EventType e;
            e.event = new Event();
            e.name = name;
            _events.push_back(e);
        }
        
        template <typename Class>
        bool subscribe(string name, Class *obj, void (Class::*func)(void)) {
            for (vector<EventType>::iterator it = _events.begin(); it != _events.end(); ++it) {
                EventType e = *it;
                if (e.name.compare(name) == 0) {
                    e.event->addListener(obj, func);
                    return true;
                }
            }
            return false;
        }
        
        void execute(string name) {
            for (vector<EventType>::iterator it = _events.begin(); it != _events.end(); ++it) {
                EventType e = *it;
                if (e.name.compare(name) == 0) {
                    e.event->execute();
                }
            }
        }
    };

}
#endif
