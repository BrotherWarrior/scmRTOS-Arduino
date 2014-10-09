#ifndef ALARM_H_INCLUDED
#define ALARM_H_INCLUDED

struct TAlarmMessage {
    enum TAlarmSrc
    {
        DI_ALARAM,
        AI_ALARM
    }
    src;
    uint8_t state;
} ;

#define ALARM_MSG_BOX_CAPACITY  5
extern OS::channel<TAlarmMessage, ALARM_MSG_BOX_CAPACITY> AlarmMessageBox;

#endif // ALARM_H_INCLUDED
