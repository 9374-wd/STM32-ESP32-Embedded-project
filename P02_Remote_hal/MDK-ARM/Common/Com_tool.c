#include "Com_tool.h"
/**
 * @brief 限制值在min和max之间
 * 
 * @param value 
 * @param min 
 * @param max 
 * @return int16_t 
 */
int16_t Com_limit(int16_t value, int16_t min, int16_t max)
{
    int16_t result=value;
    if(value<min)
    {
        result=min;
    }
    else if(value>max)
    {
        result=max;
    }
    return result;
}
