#include "timer.h"
#include "key.h"
#include "bc26.h"
#include "usart3.h"
#include "adc.h"
#include "stm32f10x.h"
#include "usart.h"
#include "led.h"

uint8_t post_flag = 0;	  // 向服务器发送数据标志位
uint8_t ping_flag = 0;	  // 心跳包标志位
uint8_t ten_min_flag = 0; // 十分钟掉线

// 通用定时器3中断初始化
// 这里时钟选择为APB1的2倍，而APB1为36M
// arr：自动重装值。
// psc：时钟预分频数
void TIM3_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 时钟使能

	// 定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr;						// 设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					// 设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);				// 根据指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); // 使能指定的TIM3中断,允许更新中断

	// 中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			  // TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; // 先占优先级4级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);							  // 初始化NVIC寄存器
}

// 通用定时器中断初始化
// 这里时钟选择为APB1的2倍，而APB1为36M
// arr：自动重装值。
// psc：时钟预分频数
void TIM2_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr - 1;					// 设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc - 1;				// 设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// 设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				// 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // TIM2

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;			  // TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM2, ENABLE); // 使能TIMx外设
}

void TIM2_IRQHandler(void) // TIM2中断
{
	static uint32_t heartbeat_count = 0, post_sec_count = 0, ten_minCount = 0;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) // 检查指定的TIM中断发生与否:TIM 中断源
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除TIMx的中断待处理位:TIM 中断源
		heartbeat_count++;							// 开始计时
		post_sec_count++;							// 发送电量计时
		if (!Connect_flag)
			ten_minCount++; // 断连计时
		else
			ten_minCount = 0;

		//        switch(Check_Mode)
		//        {
		//            case 0:if(post_sec_count >= 600)       {post_flag = 1; post_sec_count = 0;} break;   // 1分钟
		//            case 1:if(post_sec_count >= 36000)  {post_flag = 1; post_sec_count = 0;} break;   // 1小时
		//            case 2:if(post_sec_count >= 72000)  {post_flag = 1; post_sec_count = 0;} break;   // 2小时
		//            case 3:if(post_sec_count >= 108000) {post_flag = 1; post_sec_count = 0;} break;   // 3小时
		//            case 4:if(post_sec_count >= 216000) {post_flag = 1; post_sec_count = 0;} break;   // 6小时
		//            case 5:if(post_sec_count >= 432000) {post_flag = 1; post_sec_count = 0;} break;   // 12小时
		//            default: if(post_sec_count >= 600)       {post_flag = 1; post_sec_count = 0;} break;   // 5S
		//        }
		// if(post_sec_count >= 36000)     // 1小时
		// {
		//     post_flag = 1; post_sec_count = 0;
		// }
		if (post_sec_count >= 1800) // 1小时
		{
			post_flag = 1;
			post_sec_count = 0;
		}
		if (heartbeat_count >= 9000) // 九百秒
		{
			ping_flag = 1;
			heartbeat_count = 0;
		}
		if (ten_minCount >= 6000) // 10分钟
		{
			ten_min_flag = 1;
			ten_minCount = 0;
		}
	}
}

// 定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	uint8_t *p = 0;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // 更新中断
	{
		USART_RX_STA |= 1 << 15;					// 标记接收完成
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除TIM3更新中断标志
		p = (uint8_t *)strstr((char *)USART_RX_BUF, "Check_Mode");
		if (p)
		{
			//            USART_RX_STA = 0;
			Check_Mode = !Check_Mode; // *(p+12) - '0'
			memset(USART_RX_BUF, 0, 200);
		}
		TIM_Cmd(TIM3, DISABLE); // 关闭TIM3
	}
}
