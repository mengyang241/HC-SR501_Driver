/**
  ******************************************************************************
  * @file    hcsr501.c
  * @brief   HC-SR501 PIR motion sensor driver implementation
  * @author  MengYang
  *
  * Wiring, module adjustments and usage: see hcsr501.h file header.
  *
  * ==================== Software flow ====================
  *   1. Call HCSR501_Init() once at start-up to configure the GPIO
  *      (pull-down input on the OUT pin)
  *   2. Call HCSR501_proc() repeatedly in the main loop (10 ms per call)
  *   3. Every 10 calls (= 100 ms) the pin level is read into hcsr.value:
  *        hcsr.value == 1  -> person detected (OUT high)
  *        hcsr.value == 0  -> nobody (OUT low)
  *   4. Read the global hcsr.value whenever you need the latest result
  *
  *   Why no ADC? The HC-SR501 output is a digital high/low level, so only
  *   GPIO input is needed.
  ******************************************************************************
  */

#include "hcsr501.h"

/* 全局手柄实例（只能在一个 .c 文件里定义一次）：
   头文件里只有 extern 声明，这里才是真正的定义。
   主程序通过 extern 直接使用这个 hcsr 变量。
   = {0} 清零初始化，避免上电读到垃圾值 */
HCSR501_HandleTypeDef hcsr = {0};

/**
 * @brief  初始化 HC-SR501 的 GPIO（开机调用一次）
 * @param  hcsr  手柄指针，使用前先填好端口和引脚，例如：
 *         hcsr.port = GPIOB;
 *         hcsr.pin  = GPIO_PIN_12;
 *         再调用 HCSR501_Init(&hcsr);
 * @note   先给对应的 GPIO 端口打开时钟（不打开时钟寄存器写不进去），
 *         再把引脚配置成"下拉输入"：没人时被下拉读到 0，有人时为高电平读到 1
 */
void HCSR501_Init(HCSR501_HandleTypeDef* hcsr){
	/* 根据传入的端口打开对应时钟，支持 GPIOA/GPIOB/GPIOC */
	switch ((uint32_t)hcsr->port) {
		case (uint32_t)GPIOA: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
		case (uint32_t)GPIOB: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
		case (uint32_t)GPIOC: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
		default: break;   // 其他端口（GPIO D/E...）一般不常用，需要可自己加
	}

	GPIO_InitTypeDef GPIO_InitStruct ={0};

	GPIO_InitStruct.Pin  = hcsr->pin;        // OUT 接的那根引脚
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // 输入模式：读取传感器输出的电平
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;    // 下拉模式：没人时读到 0，避免悬空乱跳

	HAL_GPIO_Init(hcsr->port, &GPIO_InitStruct); // 把上面的配置真正写入寄存器
}

/**
 * @brief  立即读一次引脚电平
 * @param  hcsr  手柄指针
 * @retval 1（GPIO_PIN_SET，高电平）= 检测到人体运动
 *         0（GPIO_PIN_RESET，低电平）= 没检测到人
 */
uint8_t Read_HCSR501(HCSR501_HandleTypeDef* hcsr){
	return HAL_GPIO_ReadPin(hcsr->port, hcsr->pin);
}

/**
 * @brief  周期调度函数：主循环反复调用，把它当"10ms 心跳"
 * @note   每次调用内部延时 10ms；每满 10 次调用（100ms）才刷新一次 hcsr->value。
 *         主程序用法：
 *           while (1) {
 *               HCSR501_proc(&hcsr);          // 内部自己睡 10ms，每 100ms 刷一次 value
 *               if (hcsr.value == 1) {        // 有人！
 *                   // ...
 *               }
 *           }
 */
void HCSR501_proc(HCSR501_HandleTypeDef* hcsr){
	if(hcsr->HUMAN_t % 10 == 0){           // 数到 10 的整数倍才采样一次 --> 正好 100ms
		hcsr->value = Read_HCSR501(hcsr);  // 把最新电平存进 value，主程序直接读
	}
	HAL_Delay(10);                         // 延时 10ms，让上面的 "%10" 真的代表 100ms
	hcsr->HUMAN_t++;                       // 计数器 +1（8 位溢出自动回 0，取模结果不变）
}
