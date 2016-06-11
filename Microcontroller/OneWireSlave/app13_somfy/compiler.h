#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <pic.h>
#include "delay.h"

#define byte unsigned char
#define TRUE 1
#define FALSE 0
#define INPUT 1
#define OUTPUT 0
#define LOW 0
#define HIGH 1

#define __delay_us(x) DelayUs(x)
#define __delay_ms(x) isrDelayMs(x)
#define _ASSUME(cond)

#endif