#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"

//定义printf的输出,SWO trace
int fputc(int c, FILE *f)
{
    ITM_SendChar(c);
    return(c);
}

