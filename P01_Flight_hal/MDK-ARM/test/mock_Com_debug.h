#ifndef _MOCK_COM_DEBUG_H_
#define _MOCK_COM_DEBUG_H_

#include <stdio.h>
#include <string.h>

#define DEBUG_LOG_ENABLE 1

#define __FILE_NAME__ (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : \
                        (strrchr(__FILE__,'/')  ? strrchr(__FILE__,'/')+1  : __FILE__))

#define debug_printf(format, ...) \
    printf("[%s:%d] " format, __FILE_NAME__, __LINE__, ##__VA_ARGS__)

#endif /* _MOCK_COM_DEBUG_H_ */
