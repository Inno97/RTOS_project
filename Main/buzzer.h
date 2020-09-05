#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#ifndef BUZZER_H_   /* Include guard */
#define BUZZER_H_

extern osMutexId_t buzzer_mutex;

void play_main_music(uint8_t[], uint8_t[]);
void initBuzzer(void);
void beep(uint8_t[], uint8_t[], uint8_t);


#endif
