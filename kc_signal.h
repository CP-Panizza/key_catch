//
// Created by cmj on 2020/11/4.
//

#ifndef EVENT_SERVER_KC_SIGNAL_H
#define EVENT_SERVER_KC_SIGNAL_H
#include <windows.h>
#include <string>

LPCWSTR stringToLPCWSTR(std::string orig);


class kc_signal{
public:
    enum Type{
        SENDER, //signal sender
        RECEIVER //signal recevier
    };

    kc_signal(std::string _uid, Type type):m_uid(_uid){
        if(type == SENDER){
#if defined(_MSC_VER)
            h_event = CreateEvent(NULL, FALSE, FALSE, stringToLPCWSTR(m_uid));
#else
            h_event = CreateEventA(NULL, FALSE, FALSE, m_uid.c_str());
#endif
            if(h_event == NULL){
                throw std::string("CreateEvent err");
            }
        } else if(type == RECEIVER) {
#if defined(_MSC_VER)
            h_event = OpenEvent(EVENT_ALL_ACCESS, NULL, stringToLPCWSTR(m_uid));
#else
            h_event = CreateEventA(NULL, FALSE, FALSE, m_uid.c_str());
#endif
            if(h_event == NULL){
                throw std::string("OpenEvent err");
            }
        }
    }

    ~kc_signal(){
        CloseHandle(h_event);
    }

    //block wait for signal
    void wait(){
        WaitForSingleObject(h_event,  INFINITE);
    }

    //active signal
    void active(){
        SetEvent(h_event);
    }
    //unactive signal
    void un_active(){
        ResetEvent(h_event);
    }

    std::string m_uid;
    HANDLE h_event = NULL;
};


#endif //EVENT_SERVER_KC_SIGNAL_H
