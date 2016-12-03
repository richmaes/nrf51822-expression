#ifndef __LED_H
#define	__LED_H

#include "nrf51.h"

#define LED_START      18
#define LED_0          18
#define LED_1          19
#define LED_STOP       19

void LED_Init(void);
void LED1_Open(void);
void LED1_Close(void);
void LED1_Toggle(void);
void LED2_Open(void);
void LED2_Close(void);
void LED2_Toggle(void);



#endif /* __LED_H */

