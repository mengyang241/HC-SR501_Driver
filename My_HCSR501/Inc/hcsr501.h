/**
  ******************************************************************************
  * @file    hcsr501.h
  * @brief   HC-SR501 PIR motion sensor driver
  * @author  MengYang
  *
  * ============================ Hardware connection ============================
  *   HC-SR501 is a passive infrared (PIR) human motion sensor module. The white
  *   hemispherical "Fresnel lens" on the front detects moving heat sources and
  *   outputs a DIGITAL level — no ADC needed.
  *
  *   | Module pin | Connect to STM32                                  |
  *   |------------|---------------------------------------------------|
  *   | VCC        | 3.3V or 5V (module rated 4.5~20V, 5V is typical)  |
  *   | GND        | GND (must share ground with the MCU)              |
  *   | OUT        | any GPIO, e.g. PB0 (set by hcsr.port / hcsr.pin)  |
  *
  *   OUT level meaning:
  *     HIGH (read 1) = human movement detected
  *     LOW  (read 0) = nobody
  *
  * ==================== Module adjustments (on the back) ====================
  *   Sx (sensitivity) : detection distance, ~3~7 m
  *   Tx (delay)       : how long OUT stays HIGH after being triggered
  *   Jumper H / L     : H = retriggerable, L = non-retriggerable
  *
  * ==================== Usage warnings ====================
  *   - Wait ~30~60 s after power-up for the sensor to warm up (its output
  *     is unstable during this period — this is normal)
  *   - Heat sources / direct sunlight / wind can cause false triggers
  *   - The sensor detects MOVING people; standing still may not trigger it
  *
  * ==================== Usage ====================
  *   hcsr.port = GPIOB;                 // tell the driver where OUT is wired
  *   hcsr.pin  = GPIO_PIN_0;
  *   HCSR501_Init(&hcsr);               // configures the pin (GPIOA/B/C clock
  *                                      //   enabled automatically)
  *   while (1) {
  *       HCSR501_proc(&hcsr);           // 10 ms heartbeat, refreshes
  *                                      //   hcsr.value every 100 ms
  *       if (hcsr.value == 1) {         // someone detected!
  *           // do something
  *       }
  *   }
  ******************************************************************************
  */

#ifndef __HCSR501_H
#define __HCSR501_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stm32f1xx_hal.h"
	
/* HC-SR501 驱动用到的东西都装在这个"手柄"结构体里 */
typedef struct {
	uint8_t value;          // 最新检测结果：1=有人，0=没人（由 HCSR501_proc 自动刷新）
	GPIO_TypeDef* port;     // OUT 引脚所在的端口，如 GPIOB
	uint16_t pin;           // OUT 引脚编号，如 GPIO_PIN_12
	uint8_t HUMAN_t;        // 内部计数器：凑 100ms 节拍用，主程序不用管它

}HCSR501_HandleTypeDef;	

void HCSR501_Init(HCSR501_HandleTypeDef* hcsr);     // 初始化 GPIO（开机调用一次）
uint8_t Read_HCSR501(HCSR501_HandleTypeDef* hcsr); // 只读一次，并返回1/0
void HCSR501_proc(HCSR501_HandleTypeDef* hcsr);    // 周期调度：每 10ms 调一次，每 100ms 刷新 value

/* 全局实例：头文件里只用 extern 声明（声明不占内存），
   真正的定义放在 hcsr501.c 里。
   如果整个定义写进头文件，被多个 .c 包含后会各自生成一份，
   链接时报 "multiply defined symbol 'hcsr'" 错误 */
extern HCSR501_HandleTypeDef hcsr;

#ifdef __cplusplus
}
#endif

#endif /* __HCSR501_H */
