#ifndef __LOG_H__
#define __LOG_H__

#include "usart3.h"

/* 调试LOG信息打印 */
#define RS_INFO(fmt, arg...) \
    do{ \
        u3_printf(fmt, ##arg); \
    }while(0)








#endif  /* __LOG_H__ */
