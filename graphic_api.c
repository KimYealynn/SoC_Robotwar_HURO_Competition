#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>

#include "graphic_api.h"
#include "num.h"

#define PRINTLINE	printf("%s , %d \r\n",__FILE__,__LINE__)
#define PRINTVAR(A)	do{int ch;printf("%s=0x%x(%d)\r\n",#A,A,A);ch = getchar();} while (0);

#define AMAZON2_GRAPHIC_NAME  "/dev/amazon2_graphic"

#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		480

static int graphic_handle = -1;

static float __sintab[91] =
{
	0.0000000000000000f,	0.0174524064372835f,	0.0348994967025010f,	0.0523359562429438f,
	0.0697564737441253f,	0.0871557427476582f,	0.1045284632676535f,	0.1218693434051475f,
	0.1391731009600654f,	0.1564344650402309f,	0.1736481776669303f,	0.1908089953765448f,
	0.2079116908177593f,	0.2249510543438650f,	0.2419218955996677f,	0.2588190451025208f,
	0.2756373558169992f,	0.2923717047227367f,	0.3090169943749474f,	0.3255681544571567f,
	0.3420201433256687f,	0.3583679495453003f,	0.3746065934159120f,	0.3907311284892738f,
	0.4067366430758002f,	0.4226182617406994f,	0.4383711467890774f,	0.4539904997395468f,
	0.4694715627858908f,	0.4848096202463370f,	0.5000000000000000f,	0.5150380749100542f,
	0.5299192642332050f,	0.5446390350150271f,	0.5591929034707468f,	0.5735764363510461f,
	0.5877852522924731f,	0.6018150231520483f,					0.6156614753256583f,	0.6293203910498375f,
	0.6427876096865393f,	0.6560590289905073f,	0.6691306063588582f,	0.6819983600624985f,
	0.6946583704589973f,	0.7071067811865475f,	0.7193398003386511f,	0.7313537016191705f,
	0.7431448254773942f,	0.7547095802227720f,	0.7660444431189780f,	0.7771459614569709f,
	0.7880107536067220f,	0.7986355100472928f,	0.8090169943749474f,	0.8191520442889918f,
	0.8290375725550417f,	0.8386705679454240f,	0.8480480961564260f,	0.8571673007021123f,
	0.8660254037844386f,	0.8746197071393958f,	0.8829475928589269f,	0.8910065241883679f,
	0.8987940462991670f,	0.9063077870366500f,	0.9135454576426009f,	0.9205048534524403f,
	0.9271838545667874f,	0.9335804264972017f,	0.9396926207859084f,	0.9455185755993168f,
	0.9510565162951536f,	0.9563047559630355f,	0.9612616959383189f,	0.9659258262890683f,
	0.9702957262759965f,	0.9743700647852352f,	0.9781476007338056f,	0.9816271834476640f,
	0.9848077530122081f,	0.9876883405951377f,	0.9902680687415703f,	0.9925461516413220f,
	0.9945218953682733f,	0.9961946980917455f,	0.9975640502598242f,	0.9986295347545739f,
	0.9993908270190957f,	0.9998476951563912f,	1.0000000000000000f,
};

void clear_screen(void)
{
	ioctl(graphic_handle, AMAZON2_IOCTL_CLEAR_SCREEN, 0);
}

void flip(void)
{
	if (graphic_handle < 0)
		return;
	ioctl(graphic_handle, AMAZON2_IOCTL_FLIP, 0);
}
void flipwait(void)
{
	if (graphic_handle < 0)
		return;
	ioctl(graphic_handle, AMAZON2_IOCTL_FLIPWAIT, 0);
}

SURFACE* create_surface(int w, int h, int bpp)
{
	SURFACE* surf;
	CreateSurfaceArg arg;
	arg.w = w;
	arg.h = h;
	arg.bpp = bpp;
	surf = (SURFACE*)ioctl(graphic_handle, AMAZON2_IOCTL_CREATE_SURFACE, &arg);
	return surf;
}



static float mysin(U32 angle)
{
	angle %= 360;

	if (angle <= 90)
		return __sintab[angle];
	else if (angle <= 180)
		return __sintab[180 - angle];
	else if (angle <= 270)
		return -__sintab[angle - 180];
	else
		return -__sintab[360 - angle];
}

static float mycos(U32 angle)
{
	return mysin(angle + 90);
}


void read_fpga_video_data(U16* buf)
{
	ioctl(graphic_handle, AMAZON2_IOCTL_READ_FPGA_VIDEO_DATA, buf);
}

void draw_fpga_video_data(U16* buf, int dx, int dy)
{
	DrawFPGADataArg ar;
	ar.buf = buf;
	ar.dx = dx;
	ar.dy = dy;
	ioctl(graphic_handle, AMAZON2_IOCTL_DRAW_FPGA_VIDEO_DATA, &ar);
}

void draw_fpga_video_data_full(U16* buf)
{
	ioctl(graphic_handle, AMAZON2_IOCTL_DRAW_FPGA_VIDEO_DATA_FULL, buf);
}

//---------------------------------------------------------------------
void direct_camera_display_on(void)
{
	ioctl(graphic_handle, AMAZON2_IOCTL_CAM_DISP_ON, 0);
}

void direct_camera_display_off(void)
{
	ioctl(graphic_handle, AMAZON2_IOCTL_CAM_DISP_OFF, 0);
	clear_screen();
}

void Getframe(U16* fpga_videodata)
{
	direct_camera_display_on();

	clear_screen();
	read_fpga_video_data(fpga_videodata);

	direct_camera_display_off();
}

void Showframe(U16* fpga_videodata)
{
	draw_fpga_video_data_full(fpga_videodata);
	flip();
}

void draw_ROI(U16* fpga_videodata, int UP_y, int DOWN_y, int LEFT_x, int RIGHT_x)
{
	int x, y;
	for (x = LEFT_x; x < RIGHT_x; x++)
	{
		GetPtr(fpga_videodata, UP_y, x, WIDTH) = RGB565RED;
		GetPtr(fpga_videodata, DOWN_y, x, WIDTH) = RGB565RED;
	}
	for (y = UP_y; y < DOWN_y; y++)
	{
		GetPtr(fpga_videodata, y, LEFT_x, WIDTH) = RGB565RED;
		GetPtr(fpga_videodata, y, RIGHT_x, WIDTH) = RGB565RED;
	}
}

void draw_line_vertical(U16* fpga_videodata, int x)
{
	int y;
	for (y = 0; y < HEIGHT; y++)
	{
		GetPtr(fpga_videodata, y, x, WIDTH) = RGB565BLUE;
	}
}

void draw_line_horizon(U16* fpga_videodata, int y)
{
	int x;
	for (x = 0; x < WIDTH; x++)
	{
		GetPtr(fpga_videodata, y, x, WIDTH) = RGB565BLUE;
	}
}
//---------------------------------------------------------------------

void RGB2HSV(const U16 pSrc, float* H, float* S, float* V)
{
	U8 vR, vG, vB;
	float vR_double, vG_double, vB_double;
	float vMin, vMax, delta, local_H;

	EXTRACT_RGB565(pSrc, vR, vG, vB);

	vB_double = vB / 31.0;
	vG_double = vG / 63.0;
	vR_double = vR / 31.0;

	vMax = MAX(MAX(vR_double, vG_double), vB_double);
	vMin = MIN(MIN(vR_double, vG_double), vB_double);
	delta = vMax - vMin;

	*V = vMax * 255;// V

	if (delta == 0)               // Gray
	{
		//GetPtr(image_H, y, x, WIDTH) = 0;
		*H = 0;
		//GetPtr(image_S, y, x, WIDTH) = 0;
		*S = 0;
	}

	else
	{
		*S = delta / vMax * 255;// S

		if (vR_double == vMax)
			local_H = (vG_double - vB_double) / delta;      // 노란색과 자홍색 사이
		else if (vG_double == vMax)
			local_H = 2 + (vB_double - vR_double) / delta;   // 하늘색과 노란색 사이
		else
			local_H = 4 + (vR_double - vG_double) / delta;   // 자홍색과 하늘색 사이

		local_H *= 60.0;
		if (local_H<0)
			local_H += 360.0;

		*H = local_H; // H
	}

}

void HSV2RGB(const U16 pSrc, float* R, float* G, float* B)
{
	float vS, vH, vV;
	float f, p, t, n;

	EXTRACT_RGB565(pSrc, vS, vH, vV);

	vS = vS / 32.0;
	vH = vH * 360 / 64.0;
	vV = vV / 32.0;


	if (vS == 0)
	{
		*R = vV * 255.0;
		*G = vV * 255.0;
		*B = vV * 255.0;
	}
	else
	{
		while (vH >= 360)
			vH -= 360.0;
		while (vH < 0)
			vH += 360.0;
		vH /= 60.0;
		int k = (int)vH;
		f = vH - k;
		t = vV * (1 - vS);
		n = vV * (1 - vS*f);
		p = vV * (1 - vS*(1 - f));

		switch (k) // 6개의 구간에 따라
		{
		case 1:
			*R = n * 255.0;
			*G = vV * 255.0;
			*B = t * 255;
			break;
		case 2:
			*R = t * 255.0;
			*G = vV * 255.0;
			*B = p * 255.0;
			break;
		case 3:
			*R = t * 255.0;
			*G = n * 255.0;
			*B = vV * 255.0;
			break;
		case 4:
			*R = p * 255.0;
			*G = t * 255.0;
			*B = vV * 255.0;
			break;
		case 5:
			*R = vV * 255.0;
			*G = t * 255.0;
			*B = n * 255.0;
			break;
		default: // case 0
			*R = vV * 255.0;
			*G = p * 255.0;
			*B = t * 255.0;
			break;
		}
	}

	*R = *R * 32.0 / 255.0;
	*G = *G * 32.0 / 255.0;
	*B = *B * 32.0 / 255.0;
}


void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
{
	float vR, vG, vB;

	//범위 H는 ~64, S는 ~32, V는 ~32
	U16 Red_Start_H[10] = { 0,0 };//첫번쨰는 빨간 다리, 두번쨰는 공찰 때 오렌지 공,
	U16 Red_End_H[10] = { 5,5 };
	U16 Red_Start_H_2[10] = { 58,58 };
	U16 Red_End_H_2[10] = { 64,64 };
	U16 Red_Start_S[10] = { 0,10 };
	U16 Red_End_S[10] = { 32,32 };
	U16 Red_Start_V[10] = { 0,10 };
	U16 Red_End_V[10] = { 32, 32};


	U16 Green_Start_H[10] = { 13, };//13
	U16 Green_End_H[10] = { 29, };//29
	U16 Green_Start_S[10] = { 5, };//5
	U16 Green_End_S[10] = { 32, };
	U16 Green_Start_V[10] = { 7, };
	U16 Green_End_V[10] = { 32, };


	U16 Blue_Start_H[10] = { 37,37 };//첫번째는 허들, 두번째는 공 골대
	U16 Blue_End_H[10] = { 48,41 };
	U16 Blue_Start_S[10] = { 5,5 };
	U16 Blue_End_S[10] = { 32,32 };
	U16 Blue_Start_V[10] = { 6,6 };
	U16 Blue_End_V[10] = { 32,32 };


	U16 Yellow_Start_H[10] = { 3, };
	U16 Yellow_End_H[10] = { 11, };
	U16 Yellow_Start_S[10] = { 0, };
	U16 Yellow_End_S[10] = { 32, };
	U16 Yellow_Start_V[10] = { 0, };
	U16 Yellow_End_V[10] = { 32, };


	U16 Black_Start_H[10] = { 0, };
	U16 Black_End_H[10] = { 64, };
	U16 Black_Start_S[10] = { 0, };
	U16 Black_End_S[10] = { 32, };
	U16 Black_Start_V[10] = { 0, };
	U16 Black_End_V[10] = { 5, };


	U16 White_Start_H[10] = { 16, 0}; //첫번째는 테스트 두번째는 축구공차는 골대에서 볼대쓰는 흰색
	U16 White_End_H[10] = {32, 64};
	U16 White_Start_S[10] = { 16, 0};
	U16 White_End_S[10] = { 32, 34};
	U16 White_Start_V[10] = {16, 12};
	U16 White_End_V[10] = { 32, 32};

	float H, S, V;

	//draw_ROI(imageIn, up, down, left, right);
	U8 y, x;

	//U8 cnt = 0;
	for (y = up; y < down; y += 2)
	{
		for (x = left; x < right; x += 2)
		{
			U16 pixel = GetPtr(imageIn, y, x, WIDTH);

			//RGB2HSV(pixel, y, x, &H, &S, &V);
			EXTRACT_RGB565(pixel, S, H, V);

			if (Find_Black)
			{
				//HSV2RGB(pixel, &vR, &vG, &vB);

				//printf("R %f", vR); printf("  G %f", vG); printf("  B %f \n", vB);

				if (H > Black_Start_H[obstacle_num] && H < Black_End_H[obstacle_num] &&
					S > Black_Start_S[obstacle_num] && S < Black_End_S[obstacle_num] &&
					V > Black_Start_V[obstacle_num] && V < Black_End_V[obstacle_num])
				{
					GetPtr(bin_videodata, y, x, WIDTH) = RGB565GRAY;
					GetPtr(bin_videodata, y, x + 1, WIDTH) = RGB565GRAY;
					GetPtr(bin_videodata, y + 1, x, WIDTH) = RGB565GRAY;
					GetPtr(bin_videodata, y + 1, x + 1, WIDTH) = RGB565GRAY;
					//	cnt++;
				}
			}

			if (Find_White)
			{
				float H1, S1, V1;
				if (obstacle_num == 0) {
					HSV2RGB(pixel, &H1, &S1, &V1);
				}
				else
				{
					EXTRACT_RGB565(pixel, S1, H1, V1);
				}

				//printf("R %f", vR); printf("  G %f", vG); printf("  B %f \n", vB);

				if (H1 > White_Start_H[obstacle_num] && H1 < White_End_H[obstacle_num] &&
					S1 > White_Start_S[obstacle_num] && S1 < White_End_S[obstacle_num] &&
					V1 > White_Start_V[obstacle_num] && V1 < White_End_V[obstacle_num])
				{
					GetPtr(bin_videodata, y, x, WIDTH) = RGB565WHITE;
					GetPtr(bin_videodata, y, x + 1, WIDTH) = RGB565WHITE;
					GetPtr(bin_videodata, y + 1, x, WIDTH) = RGB565WHITE;
					GetPtr(bin_videodata, y + 1, x + 1, WIDTH) = RGB565WHITE;
					//	cnt++;
				}
			}




			if (Find_Red)
			{
				if (((H > Red_Start_H[obstacle_num] && H < Red_End_H[obstacle_num]) || (H > Red_Start_H_2[obstacle_num] && H < Red_End_H_2[obstacle_num])) &&
					S > Red_Start_S[obstacle_num] && S < Red_End_S[obstacle_num] &&
					V > Red_Start_V[obstacle_num] && V < Red_End_V[obstacle_num])
				{
					GetPtr(bin_videodata, y, x, WIDTH) = RGB565RED;
					GetPtr(bin_videodata, y, x + 1, WIDTH) = RGB565RED;
					GetPtr(bin_videodata, y + 1, x, WIDTH) = RGB565RED;
					GetPtr(bin_videodata, y + 1, x + 1, WIDTH) = RGB565RED;
				}
			}

			if (Find_Green)
			{
				if (H > Green_Start_H[obstacle_num] && H < Green_End_H[obstacle_num] &&
					S > Green_Start_S[obstacle_num] && S < Green_End_S[obstacle_num] &&
					V > Green_Start_V[obstacle_num] && V < Green_End_V[obstacle_num])
				{
					GetPtr(bin_videodata, y, x, WIDTH) = RGB565GREEN;
					GetPtr(bin_videodata, y, x + 1, WIDTH) = RGB565GREEN;
					GetPtr(bin_videodata, y + 1, x, WIDTH) = RGB565GREEN;
					GetPtr(bin_videodata, y + 1, x + 1, WIDTH) = RGB565GREEN;
				}
			}
			if (Find_Blue)
			{
				if (H > Blue_Start_H[obstacle_num] && H < Blue_End_H[obstacle_num] &&
					S > Blue_Start_S[obstacle_num] && S < Blue_End_S[obstacle_num] &&
					V > Blue_Start_V[obstacle_num] && V < Blue_End_V[obstacle_num])
				{
					GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
					GetPtr(bin_videodata, y, x + 1, WIDTH) = RGB565BLUE;
					GetPtr(bin_videodata, y + 1, x, WIDTH) = RGB565BLUE;
					GetPtr(bin_videodata, y + 1, x + 1, WIDTH) = RGB565BLUE;
				}
			}
			if (Find_Yellow)
			{
				if (H > Yellow_Start_H[obstacle_num] && H < Yellow_End_H[obstacle_num] &&
					S > Yellow_Start_S[obstacle_num] && S < Yellow_End_S[obstacle_num] &&
					V > Yellow_Start_V[obstacle_num] && V < Yellow_End_V[obstacle_num])
				{
					GetPtr(bin_videodata, y, x, WIDTH) = RGB565YELLOW;
					GetPtr(bin_videodata, y, x + 1, WIDTH) = RGB565YELLOW;
					GetPtr(bin_videodata, y + 1, x, WIDTH) = RGB565YELLOW;
					GetPtr(bin_videodata, y + 1, x + 1, WIDTH) = RGB565YELLOW;
				}
			}

		}
	}
	//	printf("\n%d\n", cnt);
}


/******************************************************************
BMP load
******************************************************************/

#define BI_RGB        0L
typedef struct {
	U32   bfSize;
	U32    bfReserved;
	U32   bfOffBits;
	U32  biSize;
	S32   biWidth;
	S32   biHeight;
	U16   biPlanes;
	U16   biBitCount;
	U32  biCompression;
	U32  biSizeImage;
	S32   biXPelsPerMeter;
	S32   biYPelsPerMeter;
	U32  biClrUsed;
	U32  biClrImportant;
} BITMAPFILEHEADER;

typedef struct {
	U8    rgbBlue;
	U8    rgbGreen;
	U8    rgbRed;
	U8    rgbReserved;
} RGBQUAD;

#define EXTRACT_READ32(startaddr,offset) (U32)(startaddr[offset] + (U32)(startaddr[offset+1]<<8) + (U32)(startaddr[offset+2]<<16) + (U32)(startaddr[offset+3]<<24))

static BITMAPFILEHEADER bmpfh;

SURFACE* LoadSurfaceInfoFromRGB(U8* bmpdata, U8 bpp, U32 w, U32 h, U32 bmpdatasize, U8* pal)
{
	SURFACE *surface = NULL;
	long i;
	U32  j;
	if (!((bpp == 24) || (bpp == 8) || (bpp == 4))) {
		return 0;
	}
	if ((bpp == 8) || (bpp == 4))
	{
		if (!(surface = create_surface(w, h, bpp)))
		{
			return 0;
		}
		U32 ibpl = (bmpfh.bfSize - bmpfh.bfOffBits) / h;
		surface->pitch = ibpl;
		if (bpp == 8)
		{
			for (i = 0; i < h; i++)
			{
				memcpy(surface->pixels + i*ibpl, bmpdata + (h - 1 - i)*ibpl, surface->w);
			}
			surface->pal->nColors = 256;
			memcpy(surface->pal->colors, pal, 256 * 4);
		}
		else
		{
			for (i = 0; i < h; i++)
			{
				memcpy(surface->pixels + i*ibpl, bmpdata + (h - 1 - i)*ibpl, surface->w / 2);
			}
			surface->pal->nColors = 16;
			memcpy(surface->pal->colors, pal, 16 * 4);
		}

		return surface;
	}
	else if (bpp == 24)
	{
		U32 screenbpp = 16;
		if (!(surface = create_surface(w, h, screenbpp)))
		{
			return 0;
		}

		if (screenbpp == 32)//screen: 4byte(1byte reserved) per pixel
		{
			U32 ibpl;
			U32* rgb32;
			ibpl = ((w * 3) + 3) & 0xfffffffc;
			rgb32 = (U32*)surface->pixels;
			bmpdata += ((h - 1)*ibpl);
			for (i = 0; i < h; i++)
			{
				for (j = 0; j < w; j++)
				{
					rgb32[j] = MAKE_RGB888(bmpdata[j * 3 + 2], bmpdata[j * 3 + 1], bmpdata[j * 3 + 0]);
				}
				bmpdata -= ibpl; //4byte align
				rgb32 += (surface->pitch / 4);
			}
			surface->pixtype = PIX_FMT_RGB888;
		}
		else // convert to rgb888 to rgb565
		{
			U32 ibpl;
			U16* rgb565data;
			ibpl = ((w * 3) + 3) & 0xfffffffc;
			rgb565data = (U16*)surface->pixels;

			bmpdata += ((h - 1)*ibpl);
			for (i = 0; i < h; i++)
			{
				for (j = 0; j < w; j++)
				{
					rgb565data[j] = MAKE_RGB565(bmpdata[j * 3 + 2], bmpdata[j * 3 + 1], bmpdata[j * 3 + 0]);
				}
				bmpdata -= ibpl; //4byte align
				rgb565data += (surface->pitch / 2);
			}
			surface->pixtype = PIX_FMT_RGB565;
		}

	}
	return surface;
}


int open_graphic(void)
{
	if ((graphic_handle = open(AMAZON2_GRAPHIC_NAME, O_RDWR)) < 0) {
		printf("Open Error %s\n", AMAZON2_GRAPHIC_NAME);
		return -1;
	}
	return 0;
}

void close_graphic(void)
{
	if (graphic_handle != -1)
		close(graphic_handle);
}



int black_line_forward()
{
	//	RobotAction(M_camera_up_golf);

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);//영상 받아오기(영상 받아오는 동안만 카메라 잠깐 켰다가 받아온 다음에는 카메라 꺼져있음)
	//Showframe(imageIn);

	U16* bin_videodata= (U16*)malloc(180 * 120 * 2);

	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 0, 0, 0, 1, 0);
	
	U8 x, y;
	U8 left_cnt = 0, right_cnt = 0;


	for (y = 100; y < 120; y += 2)
	{
		for (x = 10; x < 80; x += 2)
		{
			//if (MaskN(imageIn, 3, x, y, WIDTH, RGB565GRAY) > 1) {//화면상에서 검정색을 회색으로 표현
			if (GetPtr(imageIn, y, x, WIDTH) == RGB565GRAY) {
				left_cnt++;
			}
		}
	}
	for (y = 100; y < 120; y += 2)
	{
		for (x = 100; x < 170; x += 2)
		{
			//if (MaskN(imageIn, 3, x, y, WIDTH, RGB565GRAY) > 1) {
			if (GetPtr(imageIn, y, x, WIDTH) == RGB565GRAY) {
				right_cnt++;
			}
		}
	}

	printf("right %d ", right_cnt);

	printf("left %d\n", left_cnt);
	printf("==================================");
	if (right_cnt > 70) { RobotAction(M_turn_left_30); }
	if (left_cnt > 70) { RobotAction(M_turn_right_30); }

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	return 23;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
}
