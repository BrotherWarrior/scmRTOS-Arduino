#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer1.h"
#include <scmRTOS.h>

//��������� �������� ������� ����������������� ����������� ���������� �� �������
extern void user_timer_ISR();

/*!
 * @brief ���������� ���������� ������� ����� (������ ������������)
 */
ISR(TIMER1_COMPA_vect)
{
    OS::TISRW tisrw;
	//�������� ���������������� ���������� ���������� �� �������
	user_timer_ISR();
}

/*!
 * @brief ������������� ������� ������ 1 ��
 */
void timer1_init(void)
{
    TCCR1A = 0x00;
    TCCR1B = ( 1 << WGM12 ) | ( 1 << CS11 ) | (1 << CS10);
    TCCR1C = 0x00;
    TCNT1 = 0x0000;
    OCR1A = 2500;
    OCR1B = 0x0000;
    OCR1C = 0x0000;
    TIMSK1 |= (1 << OCIE1A);
}

