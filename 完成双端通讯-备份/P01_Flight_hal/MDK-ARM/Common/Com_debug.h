#ifndef _COM_DEBUG_H_
#define _COM_DEBUG_H_

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include <string.h> 
// 日志开关
#define DEBUG_LOG_ENABLE 1

#ifdef DEBUG_LOG_ENABLE

// Windows 用 \，Linux/Keil 用 /，统一取纯文件名
#define __FILE_NAME__ (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : \
                        (strrchr(__FILE__,'/')  ? strrchr(__FILE__,'/')+1  : __FILE__))

#define debug_printf(format,...) \
    printf("[%s:%d] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#else
#define debug_printf(format,...)
#endif

#endif
