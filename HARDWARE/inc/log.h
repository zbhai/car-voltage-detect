#ifndef __LOG_H__
#define __LOG_H__

#include "usart3.h"

/* ����LOG��Ϣ��ӡ */
#define RS_INFO(fmt, arg...) \
    do{ \
        u3_printf(fmt, ##arg); \
    }while(0)








#endif  /* __LOG_H__ */
