#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>

#include "amazon2_sdk.h"
#include "graphic_api.h"
#include "num.h"

//로봇 제어
//#include <getopt.h>
#include "uart_api.h"
#include "robot_protocol.h"

void dilate(U16* imageIn)
{
	U8 nMaskW = 5;
	U8 nMaskH = 5;
	U8 hW = nMaskW / 2;
	U8 hH = nMaskH / 2;

	int r, c;
	for (r = hH; r < HEIGHT - hH; r++) // 입력 영상의 세로 방향
	{
		for (c = hW; c < WIDTH - hW; c++) // 입력 영상의 가로 방향
		{
			U8 x, y;

			for (y = 0; y < nMaskH; y++) // 마스크의 세로 방향
			{
				for (x = 0; x < nMaskW; x++) // 마스크의 가로 방향
				{
					if (GetPtr(imageIn, r, c, WIDTH))
					{
						if (GetPtr(imageIn, r - hH + y, c - hW + x, WIDTH) == RGB565BLACK)
						{

						}
					}
				}
			}
		}
	}
}

//void Line_tracing_forward(U16* imageIn)//카메라 정면 / 상단만 볼 것임
//{
//	U8 nMaskW = 5;
//	U8 nMaskH = 5;
//	U8 hW = nMaskW / 2;
//	U8 hH = nMaskH / 2;
//
//	U8 first_x[5] = { 0, };
//	U8 first_y[5] = { 0, };
//	U8 idx = 0;
//
//	BOOL isFind_black_line = FALSE;
//
//	int r, c;
//	for (r = hH; r < 30 && !isFind_black_line; r++) // 입력 영상의 세로 방향
//	{
//		for (c = hW; c < WIDTH - hW && !isFind_black_line; c++) // 입력 영상의 가로 방향
//		{
//			U8 x, y;
//
//			U16 black_count = 0;
//
//			for (y = 0; y < nMaskH; y++) // 마스크의 세로 방향
//			{
//				BOOL FIRST = TRUE;
//				for (x = 0; x < nMaskW; x++) // 마스크의 가로 방향
//				{
//					if (GetPtr(imageIn, r - hH + y, c - hW + x, WIDTH) == RGB565BLACK)
//					{
//						black_count++;
//
//						if (FIRST)
//						{
//							first_x[idx] = c - hW + x;
//							first_y[idx] = r - hH + y;
//							idx++;
//							FIRST = FALSE;
//						}
//					}
//				}
//			}
//
//
//			if ((float)black_count / (float)nMaskH*(float)nMaskW >= 0.8)
//			{
//				isFind_black_line = TRUE;
//				/*
//				for (y = 0; y < nMaskH; y++) // 마스크의 세로 방향
//				{
//				for (x = 0; x < nMaskW; x++) // 마스크의 가로 방향
//				{
//				if (GetPtr(imageIn, r - hH + y, c - hW + x, WIDTH) == RGB565BLACK)
//				{
//				GetPtr(imageIn, r - hH + y, c - hW + x, WIDTH) = RGB565RED;
//				}
//				}
//				}
//				*/
//			}
//
//
//			for (r = 0; r < idx; r++)//확인 위해서 그림 그릴거임
//			{
//				GetPtr(imageIn, first_y[idx], first_x[idx], WIDTH) = RGB565BLUE;
//				GetPtr(imageIn, first_y[idx], first_x[idx] + 1, WIDTH) = RGB565BLUE;
//				GetPtr(imageIn, first_y[idx], first_x[idx] + 2, WIDTH) = RGB565BLUE;
//			}
//
//			if (first_x[idx - 1] - first_x[0] > 5)
//			{
//				printf("left\n");
//				//RobotAction(M_huro_turn_left_30);
//			}
//			else if (first_x[idx - 1] - first_x[0] < -5)
//			{
//				printf("right\n");
//				//RobotAction(M_huro_turn_right_30);
//			}
//
//			draw_fpga_video_data_full(imageIn);
//			flip();
//
//		}
//	}
//}


void Line_tracing_forward()
{
	U16 imageIn[180 * 120] = { 0, };
	read_fpga_video_data(imageIn);

	U8 nMaskW = 3;
	U8 nMaskH = 3;
	U8 hW = nMaskW / 2;
	U8 hH = nMaskH / 2;

	U8 left_x[50] = { 0, };
	U8 left_y[50] = { 0, };

	U8 right_x[50] = { 0, };
	U8 right_y[50] = { 0, };

	U8 left_idx = 0;
	U8 right_idx = 0;

	//왼쪽에서부터 찾음
	int r, c;
	for (r = hH; r < 50; r++) // 입력 영상의 세로 방향
	{
		for (c = hW; c < WIDTH / 2; c++) // 입력 영상의 가로 방향
		{
			U16 black_count = 0;

			U8 x, y;
			for (y = 0; y < nMaskH; y++) // 마스크의 세로 방향
				for (x = 0; x < nMaskW; x++) // 마스크의 가로 방향
					if (GetPtr(imageIn, r - hH + y, c - hW + x, WIDTH) == RGB565BLACK)
						black_count++;


			if (black_count > 5)
			{
				left_x[left_idx] = c;
				left_y[left_idx] = r;
				left_idx++;
				break;
			}

		}
	}


	//오른쪽에서부터 찾음
	for (r = hH; r < 50; r++) // 입력 영상의 세로 방향
	{
		for (c = WIDTH - hW; c >= WIDTH / 2; c--) // 입력 영상의 가로 방향
		{
			U16 black_count = 0;

			U8 x, y;
			for (y = 0; y < nMaskH; y++) // 마스크의 세로 방향
				for (x = 0; x < nMaskW; x++) // 마스크의 가로 방향
					if (GetPtr(imageIn, r - hH + y, c - hW + x, WIDTH) == RGB565BLACK)
						black_count++;


			if (black_count > 5)
			{
				right_x[right_idx] = c;
				right_y[right_idx] = r;
				right_idx++;
				break;
			}
		}
	}




	for (r = 0; r < left_idx; r++)//확인 위해서 그림 그릴거임
	{
		GetPtr(imageIn, left_y[r], left_x[r], WIDTH) = RGB565BLUE;
		GetPtr(imageIn, left_y[r], left_x[r] + 1, WIDTH) = RGB565BLUE;
		GetPtr(imageIn, left_y[r], left_x[r] - 1, WIDTH) = RGB565BLUE;
	}

	for (r = 0; r < right_idx; r++)//확인 위해서 그림 그릴거임
	{
		GetPtr(imageIn, right_y[r], right_x[r], WIDTH) = RGB565BLUE;
		GetPtr(imageIn, right_y[r], right_x[r] + 1, WIDTH) = RGB565BLUE;
		GetPtr(imageIn, right_y[r], right_x[r] - 1, WIDTH) = RGB565BLUE;
	}

	if (left_y[left_idx - 1] - right_y[right_idx - 1] > 5)//y 좌표 마지막끼리 비교
	{
		printf("right\n");
		//RobotAction(M_turn_right_30);
	}
	else if (left_y[left_idx - 1] - right_y[right_idx - 1] < -5)
	{
		printf("left\n");
		//RobotAction(M_turn_left_30);
	}
	else
	{
		printf("forward\n");
		RobotAction(M_huro_walk_forward_in_competition);
	}

	draw_fpga_video_data_full(imageIn);
	flip();


}