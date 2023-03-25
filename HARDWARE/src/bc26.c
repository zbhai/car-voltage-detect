#include "bc26.h"
#include "led.h"
#include "string.h"
#include "delay.h"
#include "usart.h"
#include "usart3.h"
#include "adc.h"
#include "math.h"

char Value[10];              // 存储电压值缓冲区
unsigned int Check_Mode = 0; // 检测模式

char *Receive_Data;            // 存储BC26信号强度缓冲区
BC26 BC26_Status;              // BC26信号状态
unsigned int Connect_flag = 0; // 连接标志位

/*
 * 校验返回内容
 * 返回值: 0:    没有找到对应的应答
 *         其他: 应答所在的位置
 */
uint8_t *BC26_check_cmd(uint8_t *str)
{
    char *strx = 0;
    if (USART_RX_STA & 0X8000) // 接收到一次数据了
    {
        USART_RX_BUF[USART_RX_STA & 0X7FFF] = 0; // 添加结束符
        strx = strstr((const char *)USART_RX_BUF, (const char *)str);
    }
    return (uint8_t *)strx;
}

/*
 * 向BC26发送指令
 * cmd:       AT指令
 * ack:       期待的回复
 * waittime:  响应查询周期, 单位ms
 * retry_num: 响应查询次数
 *************waittime * retry_num = 每次消息最长等待时间, 单位ms
 */
uint8_t BC26_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime, uint16_t retry_num)
{
    uint8_t res = 0;              // 返回值存储变量
    USART_RX_STA = 0;             // 清除串口1接收标志
    memset(USART_RX_BUF, 0, 200); // 清空接收空间

    if (retry_num)
    {
        while (--retry_num) // 等待倒计时
        {
            RS_INFO("%d ", retry_num);
            printf("%s\r\n", cmd);
            RS_INFO("CMD : %s\r\n", cmd);
            delay_ms(waittime);        // 等待延时,300ms
            if (USART_RX_STA & 0X8000) // 接收到期待的应答结果
            {
                if (BC26_check_cmd(ack))
                {
                    RS_INFO("received %s \r\n", USART_RX_BUF);
                    USART_RX_STA = 0;
                    break; // 得到有效数据
                }
                USART_RX_STA = 0; // 清除状态标志
            }
        }
        if (retry_num == 0)
        {
            res = 1;
        }
    }
    return res;
}

/*
 * BC26发送和服务器交互指令
 * cmd:       AT指令
 * ack:       期待的回复
 * waittime:  响应查询周期, 单位ms
 * retry_num: 响应查询次数
 *************waittime * retry_num = 每次消息最长等待时间, 单位ms
 */
uint8_t BC26_send_cmd2_sever(uint8_t *cmd, uint8_t *ack, uint16_t waittime, uint16_t retry_num)
{
    uint8_t res = 0;              // 返回值存储变量
    uint8_t num = 0;              // 最大重试次数
    USART_RX_STA = 0;             // 清除串口1接收标志
    memset(USART_RX_BUF, 0, 200); // 清空接收空间

    for (; num < 5; num++) // 最大重试次数
    {
        printf("%s\r\n", cmd);
        if (retry_num)
        {
            while (--retry_num) // 等待倒计时
            {
                delay_ms(waittime);        // 等待延时,300ms
                if (USART_RX_STA & 0X8000) // 接收到期待的应答结果
                {
                    if (BC26_check_cmd(ack))
                    {
                        RS_INFO("received %s \r\n", USART_RX_BUF);
                        USART_RX_STA = 0;
                        goto exit; // 得到有效数据
                    }
                    USART_RX_STA = 0; // 清除状态标志
                }
            }
        }
    }
exit:
    if ((retry_num == 0) && (num >= 5))
    {
        res = 1;
    } // 所有重试次数使用完,依旧没有连接上
    return res;
}

// 初始化BC26
uint8_t BC26_Init(void)
{
    RS_INFO("开始进行配置\r\n");

    if (BC26_send_cmd((uint8_t *)"AT+QRST=1", (uint8_t *)"BROM", 300, 60))
    {
        return 1;
    }
    RS_INFO("重启成功\r\n"); // 检测是否可连接
    if (BC26_send_cmd((uint8_t *)"ATE0", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("关闭回显成功\r\n"); // 检测是否可连接
    if (BC26_send_cmd((uint8_t *)"AT", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("连接成功\r\n"); // 检测是否可连接
    if (BC26_send_cmd((uint8_t *)"AT+CFUN=1", (uint8_t *)"OK", 300, 60))
    {
        return 2;
    }
    RS_INFO("MT设置成功\r\n"); // 设置MT完整功能
    if (BC26_send_cmd((uint8_t *)"AT+CIMI", (uint8_t *)"460", 300, 60))
    {
        return 1;
    }
    RS_INFO("获取卡号成功\r\n"); // 获取卡号
    if (BC26_send_cmd((uint8_t *)"AT+CGATT=1", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("激活网络成功\r\n"); // 激活网络, PDP
    if (BC26_send_cmd((uint8_t *)"AT+CGATT?", (uint8_t *)"+CGATT: 1", 300, 60))
    {
        return 1;
    }
    RS_INFO("网络激活查询成功\r\n"); // 激活网络
    if (BC26_send_cmd((uint8_t *)"AT+CESQ", (uint8_t *)"OK", 300, 60))
    {
        return 1;
    }
    RS_INFO("查看成功\r\n"); // 查看ESQ

    RS_INFO("BC26初始化成功\r\n");
    return 0;
}

/*MQTT初始化*/
uint8_t MQTT_Init(void)
{
    uint8_t buff[200] = {0};

    // 发送阿里云三元组
    RS_INFO("准备发送阿里云三元组\r\n");
    sprintf((char *)buff, "AT+QMTCFG=\"ALIAUTH\",0,\"%s\",\"%s\",\"%s\"", ProductKey, DeviceName, DeviceSecret);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"OK", 100, 100))
    {
        RS_INFO("发送阿里云三元组失败，准备重启\r\n");
        return 1;
    }
    RS_INFO("发送阿里云三元组成功\r\n");

    // 连接阿里云服务器
    RS_INFO("准备连接阿里云服务器\r\n");
    memset(buff, 0, 200); // 清空临时存储空间
    sprintf((char *)buff, "AT+QMTOPEN=0,\"%s\",%s", Domain, Port);
    RS_INFO("服务器命令: %s\r\n", buff);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTOPEN: 0,0", 1000, 200))
    {
        RS_INFO("连接阿里云服务器失败，准备重启\r\n");
        return 1;
    }
    RS_INFO("连接阿里云服务器成功\r\n");

    // 连接设备
    RS_INFO("准备连接设备\r\n");
    memset(buff, 0, 200); // 清空临时存储空间
    sprintf((char *)buff, "AT+QMTCONN=0,\"%s\"", ConnectID);
    RS_INFO("设备命令: %s\r\n", buff);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTCONN: 0,0,0", 100, 100))
    {
        RS_INFO("连接设备失败，准备重启\r\n");
        return 1;
    }
    RS_INFO("连接设备成功\r\n");

    // 订阅主题
    RS_INFO("准备订阅主题\r\n");
    memset(buff, 0, 200); // 清空临时存储空间
    sprintf((char *)buff, "AT+QMTSUB=0,1,\"%s\",0", SubTopic);
    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTSUB: 0,1,0,1", 100, 100))
    {
        RS_INFO("订阅失败，准备重启\r\n");
        return 1;
    }
    RS_INFO("订阅成功\r\n");

    return 0;
}

// 连接服务器
uint8_t WiFi_Connect_IoTServer(void)
{
    LED0 = 0; // 指示灯亮，表示与服务器断开

    TIM_Cmd(TIM2, DISABLE); // 关闭TIM2

    RS_INFO("准备初始化BC26\r\n");

    if (BC26_Init())
    {
        RS_INFO("BC26初始化失败，准备重启\r\n");
        return 1;
    }
    RS_INFO("BC26初始化成功\r\n");

    RS_INFO("准备连接服务器\r\n");
    if (MQTT_Init())
    {
        RS_INFO("连接服务器失败，准备重启\r\n");
        return 1;
    }
    RS_INFO("连接服务器成功\r\n");

    Connect_flag = 1; // 连接成功

    Post_Send();         // 发送电压值到服务器
    Asso_QMTPUB_event(); // 发送检测模式到服务器

    TIM_Cmd(TIM2, ENABLE); // 打开TIM2
    LED0 = 1;              // 指示灯灭，表示与服务器连接
    return 0;
}

// 发布属性主题, 向服务器汇报电压
void Asso_QMTPUB(void)
{
    uint8_t buff[200] = {0};

    sprintf((char *)buff, "AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%s}}\"\r\n", PubTopic, Param_Post, Value);

    //    RS_INFO( "发布属性主题：AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%s}}\"\r\n", PubTopic, Param_Post, Value );

    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTPUB: 0,1,0", 100, 300))
    {
        Connect_flag = 0;
    } // 连接断了
    else
        Connect_flag = 1;
}

// 发布事件主题, 向服务器上报检测模式,同时担当心跳包
// 发布事件主题
void Asso_QMTPUB_event(void)
{
    uint8_t buff[200] = {0};

    sprintf((char *)buff, "AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%d}}\"\r\n", PubTopic, Param_Set, Check_Mode);
    RS_INFO("发布属性主题：AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"%s\":%d}}\"\r\n", PubTopic, Param_Set, Check_Mode);

    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTPUB: 0,1,0", 100, 300))
    {
        Connect_flag = 0;
    } // 连接断了
    else
        Connect_flag = 1;
}

// 发送低电压警号
void Asso_QMTPUB_remind(void)
{
    uint8_t buff[200] = {0};

    sprintf((char *)buff, "AT+QMTPUB=0,1,1,0,\"%s\",\"{\"params\":{\"error_info\":1}}\"\r\n", Remind_Topic);
    //    RS_INFO( "发布属性主题：AT+QMTPUB=0,1,1,0,\"%s\",\"{params:{Error:1}}\"\r\n",Remind_Topic );

    if (BC26_send_cmd2_sever(buff, (uint8_t *)"+QMTPUB: 0,1,0", 100, 300))
    {
        Connect_flag = 0;
    } // 连接断了
    else
        Connect_flag = 1;
}

// 发送数据给服务器
void Post_Send(void)
{
    u16 adcx;
    float temp;
    static float buff = 0; // 记录上一次数据

    adcx = Get_Adc_Average(ADC_Channel_1, 10); // 采集电压值
    temp = (float)(adcx * (3.3 / 4096)) * 10;
    sprintf(Value, "%.2f", temp);
    Asso_QMTPUB(); // 发送电压值到服务器

    if (fabs(temp - buff) >= 0.05)
    {
        buff = temp;
        RS_INFO("上报警告\r\n");
        if (temp < 11.5)
            Asso_QMTPUB_remind(); // 电压过低,发送低压警告
    }
}
