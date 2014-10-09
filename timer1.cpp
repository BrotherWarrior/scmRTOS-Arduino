#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer1.h"
#include <scmRTOS.h>

//объ€вл€ем прототип функции пользовательского обработчика прерывани€ по таймеру
extern void user_timer_ISR();

/*!
 * @brief ќбработчик прерывани€ таймера часов (каждую миллисекунду)
 */
ISR(TIMER1_COMPA_vect)
{
    OS::TISRW tisrw;
	//вызываем пользовательский обработчик прерывани€ по таймеру
	user_timer_ISR();
}

/*!
 * @brief »нициализаци€ таймера каждую 1 мс
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

