#ifndef DISCR_IO_H_INCLUDED
#define DISCR_IO_H_INCLUDED

//количество блоков моргания <вкл>-<выкл>
#define BLINK_SIZE      25

/*!
 * @brief инициализация дискретного ввода/вывода
 */
void init_discr_io(void);

/*!
 * @brief обработка состояния дискретных датчиков
 */
void discr_io_ISR();

/*!
 * @brief перечисление режимов "работы"
 */
typedef enum {
    wm_unknown,
    wm_normal,
    wm_alarm
}worker_mode_t;

/*!
 * @brief изменяет набор сигналов для моргания светодиодом
 * @param mode указывает режим, который надо установить
 */
void ch_blink_mode(worker_mode_t mode);


/*!
 * @brief загрузка настроек моргания из EEPROM
 */
void load_led_blink();

/*!
 * @brief сохранение настроек моргания в EEPROM
 * @param mode указывает настройки какого режима записать
 * @param src источник данных для записи
 */
void save_led_blink(worker_mode_t mode, uint16_t src[BLINK_SIZE]);

/*!
 * @brief возвращает установленные сейчас настройки моргания
 * @param mode указывает настройки какого режима вернуть
 * @param dst адрес назначения
 */
int get_led_blink(worker_mode_t mode, uint16_t dst[BLINK_SIZE]);


#endif // DISCR_IO_H_INCLUDED
