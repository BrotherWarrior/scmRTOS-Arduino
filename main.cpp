/*
 */

#include <avr/io.h>
#include <avr/builtins.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <util/delay.h>
#include "timer1.h"
#include "discr_io.h"
#include "uart0.h"
#include "command.h"

#include <scmRTOS.h>
#include "alarm.h"

typedef OS::process<OS::pr1, 500> TCommandTask;
TCommandTask CommandTask;

OS::channel<TAlarmMessage, ALARM_MSG_BOX_CAPACITY> AlarmMessageBox;

//��������� ������� ��������� �������
typedef OS::process<OS::pr0, 500> TAlarmTask;
TAlarmTask AlarmTask;


namespace OS
{

template<> OS_PROCESS void TAlarmTask::exec()
{

    for(;;)
    {
        TAlarmMessage msg;
        //��� �� ����� �� ��������� ���������� ���������
        AlarmMessageBox.pop(msg);

        //�������� ���������, ������ ���������� ���
        if (TAlarmMessage::DI_ALARAM == msg.src)
        {
            if (msg.state == 1)
                printf_P(PSTR("\r\nAlarm: raised\r\n"));
            else
                printf_P(PSTR("\r\nAlarm: failed\r\n"));
        }
    }
}// TAlarmTask::exec()

}// namespace OS

namespace OS
{
template<> OS_PROCESS void TCommandTask::exec()
{
    for(;;)
    {
        process_command();
    }
}//TCommandTask::exec()

}// namespace OS

void user_timer_ISR()
{
    discr_io_ISR();
}

void init_hw()
{
    //��������� ���������� ����������� ��� ��������� ��������� ���������
    __builtin_avr_cli();
    MCUCR |= ( 1 << IVCE );
    MCUCR &= ~(( 1 << IVSEL ) | (1 << IVCE)); // ... 2.Within four cycles, write the desired value to IVSEL while writing a zero to IVCE.

    TIMER0_CS_REG  = (1 << CS01) | (1 << CS00); // clk/64
    UNLOCK_SYSTEM_TIMER();

    //����������� ��������� ������� �������� ������������
    init_discr_io();

    //�������������� ������
    timer1_init();

    //��������� UART0
    init_uart0();

    //�������� ���������� �����������
    //__builtin_avr_sei();
}

OS_PROCESS int main(void)
{
    //����������� ������
    init_hw();

    printf_P(PSTR("\r\nStarted...\r\n"));

    OS::run();
}
