#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/builtins.h>
#include <scmRTOS.h>

#include "fifo.h"

// объявляем FIFO буферы для считывания и отправки данных в UART

#define TXFIFO_SIZE 128
static uint8_t tx_buf[TXFIFO_SIZE];
static OS::TChannel uart_tx_fifo = OS::TChannel(tx_buf, TXFIFO_SIZE);

#define RXFIFO_SIZE 128
static uint8_t rx_buf[RXFIFO_SIZE];
static OS::TChannel uart_rx_fifo = OS::TChannel(rx_buf, RXFIFO_SIZE);



/*!
 * @brief Вектор прерывания по приему байта
 */
ISR( USART0_RX_vect )
{
  OS::TISRW tisrw;
  unsigned char rxbyte = UDR0;
  // необходима проверка на заполненность входящего буфера, так как поведение самого класса в этой ситуации нас немного не устраивает
  if (uart_rx_fifo.get_count() < RXFIFO_SIZE){
    uart_rx_fifo.push(rxbyte);
  }
}

/*!
 * @brief Вектор прерывания по очистке регистра отправки байта (байт отправлен)
 */
ISR( USART0_UDRE_vect )
{
  OS::TISRW tisrw;
  if( uart_tx_fifo.get_count() == 0 ) {
    //если данных в fifo больше нет то запрещаем это прерывание
    UCSR0B &= ~( 1 << UDRIE0 );
  }
  else {
    //иначе передаем следующий байт
    char txbyte = uart_tx_fifo.pop( );
    UDR0 = txbyte;
  }
}

/*!
 * @brief обработчик отправки байта в порт для стандартного вывода
 */
int uart_putc(  char c, FILE *file )
{
  int ret;
  uart_tx_fifo.push(c);
  UCSR0B |= ( 1 << UDRIE0 );
  ret = 0;
  return ret;
}

/*!
 * @brief обработчик чтения данных из стандартного ввода
 * @warning функция будет ожидать получения символа из UART
 */
int uart_getc( FILE* file )
{
  int ret;
  ret = uart_rx_fifo.pop();
  return ret;
}

/*!
 * @brief инициализация аппаратной части UART
 */
void uart0_hw_init()
{
  #define UART_CALC_BR( br ) ( ( uint16_t )( ( F_CPU / ( 16UL * (br) ) ) - 1 ) )
  uint16_t br = UART_CALC_BR(9600);
  //настройка скорости обмена
  UBRR0H = br >> 8;
  UBRR0L = br & 0xFF;
  //8 бит данных, 1 стоп бит, без контроля четности
  UCSR0C = ( 1 << USBS0 ) | ( 1 << UCSZ01 ) | ( 1 << UCSZ00 );
  //разрешить прием и передачу данных
  UCSR0B = ( 1 << TXEN0 ) | ( 1 << RXEN0 ) | (1 << RXCIE0 );
}

//объявляем поток ввода/вывода, который будем использовать для перенаправления stdio
static FILE uart_stream;

void init_uart0()
{
    uart0_hw_init();
    fdev_setup_stream(&uart_stream, uart_putc, uart_getc, _FDEV_SETUP_RW);
    stdout = stdin = &uart_stream;
}
