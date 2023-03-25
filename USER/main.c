#include "led.h"
#include "key.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart3.h"
#include "adc.h"
#include "bc26.h"
#include "timer.h"
#include "string.h"

uint8_t BACK_UP_CHECKk_MODE = 0; // 记录未更改的Mode

// 接收服务器命令更改数据发送模式
void mode_change_by_sever(void)
{
    if (BACK_UP_CHECKk_MODE != Check_Mode)
    {
        USART_RX_STA = 0;
        BACK_UP_CHECKk_MODE = Check_Mode;
        Post_Send(); // Asso_QMTPUB_event();
        RS_INFO("接收到状态转换\r\n");
    }
    //    if(strstr((char *)USART_RX_BUF,"+QMTSTAT")) Connect_flag = 0;       //接收到服务器返回异常断开信息，Connect_flag置0，重新连接服务器
    //    else Connect_flag = 1;
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    delay_init();                  // 延时初始化
    uart_init(115200);             // 串口1初始化为115200
    Usart3_Init(115200);           // 串口3初始化为115200
    LED_Init();                    // 初始化与LED连接的硬件接口
                                   //    KEY_Init();          	            // 初始化与按键连接的硬件接口, 按键未使用
    Adc_Init();                    // ADC初始化
    TIM2_Init(1000 - 1, 7200 - 1); // 10KHz的计数频率，计数到1000为100ms
    while (WiFi_Connect_IoTServer())
        ; // 开始联网

    while (1)
    {
        if (Connect_flag || !ten_min_flag)
        {
            mode_change_by_sever();
            if (post_flag) // 每隔指定时间向服务器发送一次电压采集值
            {
                Post_Send();
                post_flag = 0;
            }
            if (ping_flag) // 发送心跳包
            {
                Asso_QMTPUB_event();
                ping_flag = 0;
            }
        }
        else
        {
            ten_min_flag = 0;
            while (WiFi_Connect_IoTServer())
                ;
        }
    }
}
