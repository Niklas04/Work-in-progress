#include <SDL2/SDL.h>
#include "../../../../os/host/source/bios/Bios.h"
#include "../../../../os/host/source/framework/Wnd.h"
#define sfp_print vsprintf

void Set_Posi(uint16_t x, uint16_t y);
void Set_Pixel(uint16_t Color);
void Set_Block(int x1, int y1, int x2, int y2);
