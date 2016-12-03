/******************** (C) COPYRIGHT 2014青风电子 ********************
 * 文件名  ：main
 * 描述    ：         
 * 实验平台：青云nRF5188开发板
 * 描述    ：串口输出
 * 作者    ：青风
 * 店铺    ：qfv5.taobao.com
**********************************************************************/
#include "nrf51.h"
#include  "led.h"
#include  "uart.h"
#include "nrf_delay.h"
void simple_uart_putstring(const uint8_t *str);
int main(void)
{
    // 
	 LED_Init();
	 USART_Configuration();
    while (1)
    {
        LED2_Toggle();
        simple_uart_putstring("testdddddd\r\n");
	   	printf("helloworld!\r\n");
		  nrf_delay_ms(500);
        
    } 
}

