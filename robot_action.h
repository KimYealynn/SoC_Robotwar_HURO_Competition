#ifndef __AMAZON2_GRAPHIC_API_H__
#define __AMAZON2_GRAPHIC_API_H__


#include "amazon2_sdk.h"


int MaskN(U16 *imageIn, int N, U8 x, U8 y, int width, double Color);

U8 Yellow_Gate();
U8 Red_Bridge();
U8 Line_Tracing();
U8 Black_Mine();
U8 After_Blue_Huddle();
//U8 IN_GREEN_Bridge(U16* imageIn, U8 order);
U8 BLACK_line_check(int action, U8 order, U8 done, U8 isTurn);

U8 watch_ORANGE_ball();
U8 watch_BLUE_hole();
U8 close_ORANGE_Ball();
U8 goto_ORANGE_ball();
U8 After_Shoot_Ball();

void HURO();

#endif //__AMAZON2_GRAPHIC_API_H__