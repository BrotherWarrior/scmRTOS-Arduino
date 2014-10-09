#ifndef DISCR_IO_H_INCLUDED
#define DISCR_IO_H_INCLUDED

//���������� ������ �������� <���>-<����>
#define BLINK_SIZE      25

/*!
 * @brief ������������� ����������� �����/������
 */
void init_discr_io(void);

/*!
 * @brief ��������� ��������� ���������� ��������
 */
void discr_io_ISR();

/*!
 * @brief ������������ ������� "������"
 */
typedef enum {
    wm_unknown,
    wm_normal,
    wm_alarm
}worker_mode_t;

/*!
 * @brief �������� ����� �������� ��� �������� �����������
 * @param mode ��������� �����, ������� ���� ����������
 */
void ch_blink_mode(worker_mode_t mode);


/*!
 * @brief �������� �������� �������� �� EEPROM
 */
void load_led_blink();

/*!
 * @brief ���������� �������� �������� � EEPROM
 * @param mode ��������� ��������� ������ ������ ��������
 * @param src �������� ������ ��� ������
 */
void save_led_blink(worker_mode_t mode, uint16_t src[BLINK_SIZE]);

/*!
 * @brief ���������� ������������� ������ ��������� ��������
 * @param mode ��������� ��������� ������ ������ �������
 * @param dst ����� ����������
 */
int get_led_blink(worker_mode_t mode, uint16_t dst[BLINK_SIZE]);


#endif // DISCR_IO_H_INCLUDED
