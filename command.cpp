#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>
//#include <util/delay.h>
#include <ctype.h>
#include "discr_io.h"


//объявляем указатель на функцию-обработчик команды
typedef void (*cmd_handler_t)(char *);

//объявляем тип данных для связки команда-обработчик
typedef struct
{
    const char * PROGMEM cmd;
    cmd_handler_t handler;
} cmd_list_t;

//объявляем список команд
static const char str_set[] PROGMEM     = "set";
static const char str_get[] PROGMEM     = "get";
static const char str_reload[] PROGMEM  = "reload";
static const char str_eeprom[] PROGMEM  = "eeprom";

//список режимов работы для настройки
static const char str_mode_alarm[] PROGMEM  = "alarm";
static const char str_mode_norm[] PROGMEM   = "norm";


/*!
 * @brief Парсер режима
 */
worker_mode_t parse_mode(char *buf)
{
    worker_mode_t mode;
    if (strncmp_P(buf, PSTR("alarm"), 5) == 0)
    {
        mode = wm_alarm;
    }
    else if(strncmp_P(buf, PSTR("norm"), 4) == 0)
    {
        mode = wm_normal;
    }
    else
        mode = wm_unknown;
    return mode;
}

/*!
 * @brief Обработчик команды set
 */
void cmd_set(char *buf)
{
    worker_mode_t mode;
    char *p = buf;

    //пропускаем тело команды
    while(!isspace(*p)) ++p;

    //пропускаем разделитель аргумента
    while(isspace(*p)) ++p;

    mode = parse_mode(p);
    if (mode != wm_unknown)
    {
        //пропускаем аргумент названия режима
        int8_t param_idx = 0;
        uint16_t blink_seq[BLINK_SIZE];
        char *param = strtok_P(p, PSTR(" \t"));
        while (((param = strtok_P(NULL, PSTR(" \t"))) != NULL)&&( param_idx < BLINK_SIZE - 1))
        {
            if (sscanf_P(param, PSTR("%ud"), &blink_seq[param_idx]) != 1)
            {
                param_idx = -1;
                break;
            }else

            if (blink_seq[param_idx] == 0)
                break;
            ++param_idx;
        }
        if (param_idx == -1)
            printf_P(PSTR("\r\nInvalid decimal at %d \"%s\"\r\n"), param_idx, param);
        else if (param_idx >= BLINK_SIZE - 1)
            printf_P(PSTR("\r\nToo many arguments\r\n"));
        else
        {
            if (param == NULL)
                blink_seq[param_idx] = 0;
            printf_P(PSTR("\r\nWriting new parameters\r\n"));
            save_led_blink(mode, blink_seq);
            printf_P(PSTR("OK\r\n"));
        }

    }
    else
        printf_P(PSTR("\r\nError: Unknown mode\r\n"));
}


/*!
 * @brief Обработчик команды get
 */
void cmd_get(char *buf)
{
    worker_mode_t mode;
    char *p = buf;
    //пропускаем тело команды
    while(!isspace(*p)) ++p;
    //пропускаем разделитель аргумента
    while(isspace(*p)) ++p;

    mode = parse_mode(p);
    if (mode != wm_unknown)
    {
        uint16_t blink_seq[BLINK_SIZE];
        get_led_blink(mode, blink_seq);
        uint8_t _idx = 0;
        printf_P(PSTR("\r\n"));
        while(blink_seq[_idx] != 0)
        {
            if (_idx != 0)
                printf_P(PSTR(", "));
            printf_P(PSTR("%0d"), blink_seq[_idx]);
            ++_idx;
        }
        printf_P(PSTR("\r\n"));
    }
    else
        printf_P(PSTR("\r\nError: Unknown mode\r\n"));
}

/*!
 * @brief Обработчик команды reload
 */
void cmd_reload(char *buf)
{
    printf_P(PSTR("\r\nReloading settings\r\n"));
    load_led_blink();
    printf_P(PSTR("OK\r\n"));
}

/*!
 * @brief Обработчик команды eeprom
 */
void cmd_eeprom(char *buf)
{
    uint8_t ee[32];
    uint16_t addr = 0;
    printf_P(PSTR("\r\nReading EEPROM:\r\n"));
    while (addr < 4096)
    {
        eeprom_read_block(ee, (void*)addr, 32);
        uint8_t i;
        printf_P(PSTR("%04X: "), addr);
        for (i =0; i < 32; ++i)
        {
            printf_P(PSTR("%02X "), ee[i]);
//            _delay_ms(10);
        }
        printf_P(PSTR("\r\n"));
        addr += 32;
    }
}

static const cmd_list_t PROGMEM cmd_list[]=
{
    {str_set, cmd_set},
    {str_get, cmd_get},
    {str_reload, cmd_reload},
    {str_eeprom, cmd_eeprom},
    {NULL, NULL}
};

void process_command()
{
    char cmd[100];
    printf_P(PSTR("\r\nArduino> "));
    fflush(stdout);
    if (fgets(cmd, 100, stdin) == NULL)
        return;

    cmd_handler_t cmd_handler = NULL;
    cmd_list_t cmd_list_rec;
    uint8_t idx = 0;
    //ищем обрабочтик для полученной команды
    do
    {
        memcpy_P(&cmd_list_rec, &cmd_list[idx++], sizeof(cmd_list_t));
        if ((cmd_list_rec.cmd != NULL) && (strncmp_P(cmd, cmd_list_rec.cmd, strlen_P(cmd_list_rec.cmd)) == 0))
        {
            cmd_handler = cmd_list_rec.handler;
        }
    }
    while((cmd_list_rec.cmd != NULL) && (cmd_handler == NULL));

    //если обработчик найден, вызовем его
    if (cmd_handler != NULL)
        cmd_handler(cmd);
    else
        //обработчика нет, значит команда неизвестна
        printf_P(PSTR("\r\nUnknown command\r\n"));
}
