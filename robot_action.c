
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

#include <getopt.h>
#include "uart_api.h"
#include "robot_protocol.h"
#include "robot_action.h"
#include <termios.h>


#include "robot_action.h"

//#include "Line_tracing_test.c"
static struct termios inittio, newtio;

#define AMAZON2_GRAPHIC_VERSION		"v0.5"

U8 check_over_idx = 0;
U8 Idx = 0;

int MaskN(U16 *imageIn, int N, U8 x, U8 y, int width, double Color) {
	int return_value = 0;
	U8 i, j;

	for (i = 0; i <= N / 2; i++) {
		for (j = 0; j <= N / 2; j++) {
			if (i == 0 && j == 0) {
				return_value += (GetPtr(imageIn, x, y, width) == Color ? 1 : 0);
			}
			else {
				return_value += (GetPtr(imageIn, y - i, x - j, width) == Color ? 1 : 0);
				return_value += (GetPtr(imageIn, y + i, x + j, width) == Color ? 1 : 0);
			}
		}
	}

	return return_value;
}

U8 yellow_gate_waiting = 0;
U8 Yellow_Gate() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 0, 0, 1, 0, 0);
	//Showframe(imageIn);
	U8 nMaskW = 5;
	U8 nMaskH = 5;
	U8 hW = nMaskW / 2;
	U8 hH = nMaskH / 2;

	U16 Yellow_count = 0;
	U8 up = 10, down = 60, left = 50, right = 130;
	U8 y, x;
	for (y = up - hH; y < down - hH; y++) {
		for (x = left - hW; x < right - hW; x++) {
			U8 c, r;
			U16 cnt = 0;
			for (r = 0; r < nMaskH; r++) {
				for (c = 0; c < nMaskW; c++) {
					if (GetPtr(bin_videodata, r - hH + y, c - hW + x, WIDTH) == RGB565YELLOW) {
						cnt++;
					}
				}
			}
			if ((float)cnt / ((float)nMaskH*(float)nMaskW) > 0.8) {
				Yellow_count++;
				//GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
			}
		}
	}

	//-----------------------------------------------------
	draw_ROI(bin_videodata, up, down, left, right);
	Showframe(bin_videodata);
	//-----------------------------------------------------

	float density = (float)Yellow_count / (float)((down - up) *(right - left));
	printf("density: %f \n", density);

	free(bin_videodata);
	free(imageIn);

	if (yellow_gate_waiting == 0) {
		if (density <= 0.2) {
			yellow_gate_waiting = 0;
			return 100;
		}
		else {
			yellow_gate_waiting = 1;
			return 100;
		}
	}
	else {
		if (density <= 0.2) {
			//RobotAction(M_walk_forward_10);
			//RobotAction(M_walk_forward_10);

			return 0; //빨간다리를 찾아서 이제 출발
		}
		else {
			return 100;
		}
	}
}

U8 numCount = 0;
U8 Red_Bridge() {

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 1, 0, 0, 0, 0, 0);

	U8 x, y;
	int redCount = 0;
	int redCountLeft = 0;
	int redCountRight = 0;

	for (x = 2; x <= 178; x++) {
		if (MaskN(bin_videodata, 5, x, 100, WIDTH, RGB565RED) > 13) {
			redCount++;
		}
	}
	for (x = 2; x <= 178; x++) {
		for (y = 60; y <= 80; y++) {
			if (x >= 2 && x <= 10) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565RED) > 13) {
					redCountLeft++;
				}
			}
			else if (x >= 170 && x <= 178) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565RED) > 13) {
					redCountRight++;
				}
			}
		}
	}

	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);


	printf("red::   %d\n", redCount);
	printf("redCountLeft : %d, redCountRight : %d\n", redCountLeft, redCountRight);
	if (redCount>40) { //빨간색을 일정이상 찾아왔다면,
		RobotAction(M_walk_forward_2);
		RobotAction(M_walk_forward_repeat_5);
		//RobotAction(M_walk_forward_repeat_5);
		return 1; //앞에 빨간색 다리와 가까이 있다고 판단.
	}
	else {

		if (redCountLeft > 80) {
			RobotAction(M_turn_left_30);
			return 0;
		}
		else if (redCountRight > 80) {
			RobotAction(M_turn_right_30);
			return 0;
		}
		RobotAction(M_walk_forward_4);
		numCount++;
		if (numCount > 6) {
			RobotAction(M_walk_forward_repeat_5);
			return 1;
		}
		return 0; //앞에 빨간색 다리가 가까이 있지 않다고 판단.
	}
}

U8 blue_huddle_flag = 0;
U8 Line_Tracing() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 0, 1, 0, 0, 1);

	U8 x, y;
	int whiteCountLeft = 0;
	int whiteCountRight = 0;
	int blueCount = 0;

	for (x = 2; x <= 178; x++) {
		for (y = 20; y <= 118; y++) {
			if (y >= 60 && y <= 118) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565BLUE) > 13) {
					blueCount++;
				}

			}
			if (x >= 2 && x <= 20 && y<=50) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565WHITE) > 13) {
					whiteCountLeft++;
				}
			}
			else if (x >= 160 && x <= 178 && y<=50) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565WHITE) > 13) {
					whiteCountRight++;
				}
			}

			
		}
	}
	draw_ROI(bin_videodata, 40, 70, 2, 178);
	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	printf("blue Count : %d\n", blueCount);
	//printf("leftwhite : %d, rightwhite : %d\n", whiteCountLeft, whiteCountRight);
	//if (whiteCountLeft > 20) {
	//	RobotAction(M_turn_left_30);
	//	if (blueCount >= 350) {
	//		RobotAction(M_walk_forward_8);
	//		return 20;
	//	}
	//	return 10;
	//}
	if (whiteCountRight > 20) {
		printf("rightWhite : %d", whiteCountRight);
		RobotAction(M_turn_right_30);
		if (blueCount >= 350) {
			RobotAction(M_walk_forward_8);
			sleep(1);
			//RobotAction(M_walk_forward_3);
			return 21;
		}
		return 10;
	}
	else {
		return 11;
	}

}

U8 left_right_flag = 1;
U8 Black_Mine() {
	RobotAction(M_camera_down_golf);
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);
	Getframe(imageIn);//ø?ª? π?æ?ø¿±?(ø?ª? π?æ?ø¿?? ?øæ?∏∏ ?´∏?∂? ¿·±? ????∞° π?æ?ø? ??¿Ωø°?? ?´∏?∂? ≤®¡Æ¿÷¿Ω)

					  //void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
					  //	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 0, 0, 0, 1, 0);

	Showframe(bin_videodata);

	U8 x, y;
	U8 first_black_x = 999;
	U8 left_line_black = 0, right_line_black = 0;

	for (y = 20; y < 100; y++) {
		for (x = 10; x < 30; x++) {
			if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565GRAY) > 4) {
				left_line_black++;
			}
		}
		for (x = 150; x < 170; x++) {
			if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565GRAY) > 4) {
				right_line_black++;
			}
		}
	}
	printf("leftasdfasdfadsf : %d, rightadsfadsfdf : %d\n", left_line_black, right_line_black);
	for (y = 25; y < 85; y++) {
		for (x = 40; x < 140; x++) {
			if ((GetPtr(bin_videodata, y, x, WIDTH) == RGB565GRAY ? 1 : 0) + (GetPtr(bin_videodata, y, x + 1, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(bin_videodata, y, x + 2, WIDTH) == RGB565GRAY ? 1 : 0) + (GetPtr(bin_videodata, y, x + 3, WIDTH) == RGB565GRAY ? 1 : 0) > 2) {
				if (first_black_x > x)
					first_black_x = x;
			}
		}
	}
	if (left_line_black > 150) {
		RobotAction(M_turn_right_40);
		RobotAction(M_turn_right_40);
		RobotAction(M_walk_right_small);
		RobotAction(M_walk_right_small);
		RobotAction(M_walk_right_small);
		RobotAction(M_walk_right_small);
		//RobotAction(M_turn_right_40);
	}
	else if (right_line_black > 150) {
		RobotAction(M_turn_left_40); 
		RobotAction(M_turn_left_40);
		RobotAction(M_walk_left_small);
		RobotAction(M_walk_left_small);
		RobotAction(M_walk_left_small);
		RobotAction(M_walk_left_small);
		//RobotAction(M_turn_left_40);
	}
	draw_ROI(bin_videodata, 25, 85, 30, 150);
	Showframe(bin_videodata);

	if (first_black_x > 40 && first_black_x < 65) {
		RobotAction(M_walk_right_small);
		RobotAction(M_walk_right_small);
		RobotAction(M_turn_right_40);
		RobotAction(M_walk_forward_2);
		//RobotAction(M_turn_left_40);
	}
	else if (first_black_x > 65 && first_black_x < 115) {
		/*if (left_right_flag == 1) {
			left_right_flag = 0;
			RobotAction(M_walk_left_small);
			RobotAction(M_walk_left_small);
			RobotAction(M_walk_left_small);
			RobotAction(M_turn_left_40);
			RobotAction(M_walk_forward_3);
			//RobotAction(M_turn_right_40);
		}
		else if (left_right_flag == 0) {
			left_right_flag = 1;*/
			RobotAction(M_walk_right_small);
			RobotAction(M_walk_right_small);
			RobotAction(M_walk_right_small);
			RobotAction(M_walk_right_small);
			RobotAction(M_turn_right_40);
			RobotAction(M_walk_forward_3);
			//RobotAction(M_turn_left_40);
		}
	else if (first_black_x > 115 && first_black_x < 140) {
		RobotAction(M_walk_left_small);
		RobotAction(M_walk_left_small);
		RobotAction(M_turn_left_40);
		RobotAction(M_walk_forward_2);
		//RobotAction(M_turn_right_40);
	}
	else {
		RobotAction(M_walk_forward_2);
	}
	free(bin_videodata);
	free(imageIn);
	
	
	return 10;

	/*
	RobotAction(M_camera_up_golf);

	U16* bin_videodata1 = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata1, 0, 180 * 120 * sizeof(U16));

	U16* imageIn1 = (U16*)malloc(180 * 120 * 2);
	Getframe(imageIn1);//ø?ª? π?æ?ø¿±?(ø?ª? π?æ?ø¿?? ?øæ?∏∏ ?´∏?∂? ¿·±? ????∞° π?æ?ø? ??¿Ωø°?? ?´∏?∂? ≤®¡Æ¿÷¿Ω)

					   //void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
					   //	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn1, bin_videodata1, 0, 0, 120, 0, 180, 0, 0, 0, 0, 1, 0);

	draw_ROI(bin_videodata, 20, 40, 20, 160);
	Showframe(bin_videodata1);

	U8 hudle_black_count = 0;

	//for (y = 30; y < 60; y++) {
	//	for (x = 20; x < 160; x++) {
	//		if (MaskN(bin_videodata1, 3, x, y, WIDTH, RGB565GRAY) > 5) {
	//			hudle_black_count++;
	//		}
	//	}
	//}
	//printf("hudle_black : %d\n", hudle_black_count);


	free(bin_videodata1);
	free(imageIn1);
	//if (hudle_black_count > 300) {
	//	RobotAction(M_walk_forward_4);
	//	return 20;    //허들을 발견했다.
	//}
	//else {
		return 10; //반복한다.
	//}
	*/
}


U8 BLACK_line_check(int action, U8 order, U8 done, U8 isTurn) {
	RobotAction(M_camera_right);

	U16* imageIn = (U16*)malloc(180 * 120 * 2);
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(imageIn, 0, 180 * 120 * sizeof(U16));
	//memset(bin_videodata, 0, 180 * 120 * sizeof(U16));


	Getframe(bin_videodata);
	ColorLabeling(bin_videodata, imageIn, 0, 0, 100, 30, 150, 0, 0, 0, 0, 1, 0);
	draw_line_vertical(imageIn, 30);
	draw_line_vertical(imageIn, 150);
	//draw_line_horizon(imageIn, 100);
	Showframe(imageIn);

	int Y_B_cnt = 0;
	U8 Y_length = 2;
	U8 first, is_first = 1;
	U8 F = 0, L = 0;
	float bridge_X[100] = { 0, }, bridge_Y[100] = { 0, };
	//좌표 찾기
	U8 y, x;
	U8 black = 0;
	for (x = 30; x < WIDTH - 30; x += 2) {
		is_first = 1; black = 0;
		for (y = 0; y < HEIGHT - 20; y += 2)
		{
			//if (MaskN(imageIn, 5, x, y, WIDTH, RGB565GREEN) > 5) {

			if ((GetPtr(imageIn, y + 3, x - 1, WIDTH) == RGB565GRAY ? 1 : 0) + (GetPtr(imageIn, y + 3, x, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y + 3, x + 1, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y + 2, x - 1, WIDTH) == RGB565GRAY ? 1 : 0) + (GetPtr(imageIn, y + 2, x, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y + 2, x + 1, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y, x - 1, WIDTH) == RGB565GRAY ? 1 : 0) + (GetPtr(imageIn, y, x, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y, x + 1, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y + 1, x - 1, WIDTH) == RGB565GRAY ? 1 : 0) + (GetPtr(imageIn, y + 1, x, WIDTH) == RGB565GRAY ? 1 : 0)
				+ (GetPtr(imageIn, y + 1, x + 1, WIDTH) == RGB565GRAY ? 1 : 0) > 7) {
				black++;
				if (is_first) { is_first = 0; first = y; }
				GetPtr(imageIn, y, x, WIDTH) = RGB565RED;
			}
		}
		if (black > 1)
		{
			//if (x < 60 && first < 3) F++;
			//else if (x > 150 && first < 6) L++;
			bridge_X[Y_B_cnt] = x;
			bridge_Y[Y_B_cnt++] = first;
		}
	}

	float avg_Y = 0;

	//	free(imageIn);

	if ((Idx++) % 5 == 0 && isTurn == 3) {
		Idx = 0;
		return done;
	}


	if (Y_B_cnt > 4) {
		int slope = bridge_Y[Y_B_cnt - 1] - bridge_Y[0];
		for (x = Y_B_cnt / 2 - 2; x < Y_B_cnt / 2 + 2; x++) {
			avg_Y += bridge_Y[x];
		}
		avg_Y /= 3;


		if (isTurn == 1) {
			if (slope > 22) {
				RobotAction(M_CAMERA_right_turn_right40);
			}
			else if (slope < -12) {
				RobotAction(M_CAMERA_right_turn_left40);
			}
			else if (avg_Y < 40) {
				RobotAction(M_huro_CAM_Right_walk_right_small);
			}
			else if (avg_Y > 50) {
				RobotAction(M_huro_CAM_Right_walk_left_small);
			}
			else {
				RobotAction(action);
				return done;
			}
		}
		else if (isTurn == 0)
		{

			if (avg_Y < 40) {
				RobotAction(M_huro_CAM_Right_walk_right_small);
			}
			else if (avg_Y > 50) {
				RobotAction(M_huro_CAM_Right_walk_left_small);
			}
			else {
				RobotAction(action);
				return done;
			}
		}
		else if (isTurn == 2) {

			if (slope > 25) {
				RobotAction(M_CAMERA_right_turn_right40);
			}
			else if (slope < -15) {
				RobotAction(M_CAMERA_right_turn_left40);
			}
			else {
				RobotAction(action); //&&&&&
									 //RobotAction(58); //&&&&&
									 //clear_screen();

									 //free(bin_videodata);
									 //free(imageIn);
				return done;
			}
		}
		else if (isTurn == 3)
		{
			if (check_over_idx < 4)
			{

				if (slope > 25) {
					RobotAction(M_CAMERA_right_turn_right40);
				}
				else if (slope < -15) {
					RobotAction(M_CAMERA_right_turn_left40);
				}
				else if (avg_Y < 40) {
					RobotAction(M_huro_CAM_Right_walk_right_small);
				}
				else if (avg_Y > 50) {
					RobotAction(M_huro_CAM_Right_walk_left_small);
				}
				else {
					RobotAction(action);
					return done;
				}
				check_over_idx++;
			}
			else
			{
				check_over_idx = 0;
				RobotAction(action);

				return done;
			}
		}
	}
	else {
		/*(imageIn, 0, 180 * 120 * sizeof(U16));
		clear_screen();
		free(bin_videodata);
		free(imageIn);

		Getframe(bin_videodata);
		ColorLabeling(bin_videodata, imageIn, 0, 100, 120, 0, 130, 0, 0, 0, 0, 1, 0);
		Showframe(imageIn);
		int black_cnt = 0;
		for (x = 0; x < WIDTH - 50; x += 2) {
		for (y = 100; y < HEIGHT - 1; y += 2)
		{
		if (GetPtr(imageIn, y, x, WIDTH) == RGB565GRAY ) {
		black_cnt++;
		}
		}
		}
		if (black_cnt < 50) {
		RobotAction(M_huro_CAM_Right_walk_right_small);
		}
		else {*/
		RobotAction(M_huro_CAM_Right_walk_left_small);
		//}
	}
	clear_screen();
	free(bin_videodata);

	free(imageIn);

	return order;
}
U8 After_Blue_Huddle() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 10, 60, 100, 170, 0, 0, 0, 0, 0, 1);

	U8 x, y;
	int whiteCount = 0;
	int whiteCountRight = 0;

	for (x = 2; x <= 178; x++) {
		for (y = 20; y <= 50; y++) {
			if (x >= 110 && x <= 160) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565WHITE) > 13) {
					whiteCountRight++;
				}
			}
		}
	}
	draw_ROI(bin_videodata, 20, 50, 110, 160);
	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	printf("whiteCountRight : %d\n", whiteCountRight);
	if (whiteCountRight < 900) {//1250
		RobotAction(M_turn_left_30);
		return 22;	
	}
	else {
		return 23;
	}
	//usleep(1000000);
	//clear_screen();
}

U8 Go_To_GreenBridge() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//   BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 60, 120, 0, 180, 0, 1, 1, 0, 1, 0);

	U8 x, y;
	int up_greenCount = 0;
	int down_greenCount = 0;

	draw_ROI(bin_videodata, 90, 120, 20, 160);
	Showframe(bin_videodata);

	for (x = 20; x <= 160; x++) {
		for (y = 40; y <= 80; y++) {
			if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565GREEN) > 3) {
				up_greenCount++;
			}
		}
	}

	for (x = 40; x <= 140; x++) {
		for (y = 90; y <= 120; y++) {
			if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565GREEN) > 4) {
				down_greenCount++;
			}
		}
	}
	printf("2222up : %d\n", up_greenCount);
	printf("2222down : %d\n", down_greenCount);
	free(bin_videodata);
	free(imageIn);
	if (down_greenCount > 1000)
		return 30;
	else if (up_greenCount > 100) {
		RobotAction(M_walk_forward_10);
		return 30;
	}
	else {
		RobotAction(M_walk_forward_5);
		return 24;
	}
}

U8 CLOSE_TO_GREEN_Bridge() {
	//RobotAction(M_camera_up);
	RobotAction(M_camera_up_golf);
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//   BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 20, 80, 0, 180, 0, 1, 0, 0, 0, 0);

	U8 nMaskW = 10, nMaskH = 10, hW = nMaskW / 2, hH = nMaskH / 2;

	U8 close_to_bridge_Y[2] = { 20,80 };
	U8 green_bridge_X[40] = { 0, };
	U8 green_bridge_Y[40] = { 0, };
	U8 G_B_cnt = 0;

	U8 first;
	U8 last;
	U8 is_first = 1;

	U8 y, x;
	U16 green_cnt = 0;
	for (y = close_to_bridge_Y[0]; y < close_to_bridge_Y[1]; y += 2)
	{
		is_first = 1;
		green_cnt = 0;

		for (x = 1; x < WIDTH - 1; x += 2)
		{
			U8 c, r;
			U16 cnt = 0;
			for (r = 0; r < nMaskH; r++) // 마스크의 세로 방향
			{
				for (c = 0; c < nMaskW; c++) // 마스크의 가로 방향
				{
					if (GetPtr(bin_videodata, r - hH + y, c - hW + x, WIDTH) == RGB565GREEN)
					{
						cnt++;
					}
				}
			}

			if ((float)cnt / ((float)nMaskH*(float)nMaskW) > 0.7) {
				green_cnt++;

				if (is_first) {
					is_first = 0; first = x;
				}
				last = x;
				GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
			}
		}
		if (green_cnt > 6) {
			green_bridge_Y[G_B_cnt] = y; green_bridge_X[G_B_cnt++] = first;
		}
	}
	if (G_B_cnt > 3) {
		float avg_top_X = 0;
		for (y = 0; y < 3; y++) {
			avg_top_X += green_bridge_X[y];
		}
		avg_top_X /= 3.0;
		draw_line_vertical(bin_videodata, green_bridge_X[G_B_cnt - 1]);
		draw_line_horizon(bin_videodata, 57);
		draw_line_horizon(bin_videodata, 58);


		draw_ROI(bin_videodata, 0, green_bridge_Y[G_B_cnt - 1], 1, 180);
		if (green_bridge_Y[G_B_cnt - 1] < 30) {

			if (avg_top_X - 10 > green_bridge_X[G_B_cnt - 1]) {
				RobotAction(M_turn_left_30);
			}
			else if (avg_top_X + 10 < green_bridge_X[G_B_cnt - 1]) {
				RobotAction(M_turn_right_30);
			}
			else {
				RobotAction(M_walk_mine_20);
			}
		}
		else if (green_bridge_Y[1] < 57) {
			draw_line_horizon(bin_videodata, green_bridge_Y[1]);
			RobotAction(M_walk_forward_repeat_5);
		}
		else if (green_bridge_X[G_B_cnt - 1] < 60) {
			RobotAction(M_walk_left_small);
		}
		else if (green_bridge_X[G_B_cnt - 1] > 100) {
	 		RobotAction(M_walk_right_small);
		}
		else {
			RobotAction(M_walk_forward_repeat_5);
			//RobotAction(M_walk_forward_1);

			RobotAction(M_up_2cm);
			Showframe(bin_videodata);
			free(bin_videodata);
			free(imageIn);
			return 31;
		}
	}
	RobotAction(M_walk_forward_repeat_5);
	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	return 30;
}

U8 IN_GREEN_Bridge() {
	//RobotAction(M_camera_up);

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//   BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	//Getframe(imageIn);
	//Showframe(imageIn);
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 1, 0, 0, 0, 0);

	int G_B_cnt = 0;
	U8 Y_length = 2;
	U8 first, last, is_first = 1;
	U8 F = 0, L = 0;
	float green_bridge_X_first[100] = { 0, }, green_bridge_Y[100] = { 0, };
	//좌표 찾기
	U8 y, x;
	U8 green_cnt = 0;

	for (y = 1; y < HEIGHT - 1; y += Y_length) {
		is_first = 1; green_cnt = 0;
		for (x = 1; x < WIDTH - 1; x += 2)
		{
			if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565GREEN) > 10) {
				green_cnt++;
				if (is_first) { is_first = 0; first = x; }
				last = x;
				GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
			}
		}
		if (green_cnt>6)
		{
			green_bridge_Y[G_B_cnt] = y;
			if (first < 10) F++;
			if (last > 170) L++;
			green_bridge_X_first[G_B_cnt++] = first;
		}
	}
	draw_line_horizon(bin_videodata, green_bridge_Y[0]);

	if (green_bridge_Y[0] > 80) {
		/*RobotAction(M_walk_forward_2);//5번 걷기
		sleep(1);
		RobotAction(M_down_2cm);
		*/
		RobotAction(5);
		RobotAction(M_camera_down_golf);

		Showframe(bin_videodata);
		free(bin_videodata);
		free(imageIn);
		return 32;
	}
	else if (F>3) {
		RobotAction(M_turn_left_30);
	}
	else if (L>3) {
		RobotAction(M_turn_right_30);
	}
	else if (G_B_cnt>2 && green_bridge_X_first[G_B_cnt - 1] < 38) {//38
		RobotAction(M_walk_left_small);
	}
	else if (G_B_cnt>2 && green_bridge_X_first[G_B_cnt - 1] > 60) {//48
		RobotAction(M_walk_right_small);
	}
	else {
		RobotAction(5);
	}

	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	//sleep(1);
	return 31;
}

U8 FIN_GREEN_Bridge() {
	//RobotAction(M_camera_up);

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);


	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//   BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	//Getframe(imageIn);
	//Showframe(imageIn);
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 1, 0, 0, 0, 0);

	int G_B_cnt = 0;
	U8 Y_length = 2;
	U8 first, last, is_first = 1;
	U8 F = 0, L = 0;
	float green_bridge_X_first[100] = { 0, }, green_bridge_Y[100] = { 0, };
	//좌표 찾기
	U8 y, x;
	U8 green_cnt = 0;

	for (y = 1; y < HEIGHT - 1; y += Y_length) {
		is_first = 1; green_cnt = 0;
		for (x = 1; x < WIDTH - 1; x += 2)
		{
			if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565GREEN) > 10) {
				green_cnt++;
				if (is_first) { is_first = 0; first = x; }
				last = x;
				GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
			}
		}
		if (green_cnt >6)
		{
			green_bridge_Y[G_B_cnt] = y;
			if (first < 20) F++;
			if (last > 160) L++;
			green_bridge_X_first[G_B_cnt++] = first;
		}
	}
	draw_line_horizon(bin_videodata, green_bridge_Y[0]);

	if (green_bridge_Y[0] > 80) {
		/*RobotAction(M_walk_forward_2);//5번 걷기
		sleep(1);
		RobotAction(M_down_2cm);
		*/
		RobotAction(118);
		RobotAction(M_camera_down_golf);
		draw_line_horizon(bin_videodata, 100);
		draw_line_horizon(bin_videodata, 102);
		draw_line_horizon(bin_videodata, 104);

		Showframe(bin_videodata);
		free(bin_videodata);
		free(imageIn);
		return 33;
	}
	else if (F>3) {
		RobotAction(M_head_down_turn_left);
	}
	else if (L>3) {
		RobotAction(M_head_down_turn_right);
	}
	else if (G_B_cnt>2 && green_bridge_X_first[G_B_cnt - 1] < 30) {//38
		RobotAction(M_walk_left_small);
		RobotAction(M_camera_down_golf);
	}
	else if (G_B_cnt>2 && green_bridge_X_first[G_B_cnt - 1] > 55) {//48
		RobotAction(M_walk_right_small);
		RobotAction(M_camera_down_golf);
	}
	else {
		RobotAction(118);
		RobotAction(M_camera_down_golf);
	}

	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	//sleep(1);
	return 32;
}

U8 DOWN_GREEN_Bridge() {
	//RobotAction(M_camera_up);
	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	Getframe(imageIn);
	//Showframe(imageIn);
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 0, 0, 0, 1, 0);


	U8 Y_range[2] = { 1,119 };
	U8 LEFT_Y = 0, RIGHT_Y = 0;
	U16 left_cnt = 0, right_cnt = 0;

	U8 y, x;
	U16 pixcel_cnt = 0;
	for (y = Y_range[0]; y < Y_range[1]; y++) {
		for (x = 1; x < WIDTH - 1; x++)
		{
			if (x == 30) x = 150;

			if (GetPtr(bin_videodata, y, x, WIDTH) == RGB565GRAY)
			{
				if (x < 30) left_cnt++;
				else right_cnt++;
				pixcel_cnt++;
				GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
			}
		}
	}

	printf("------%d//%d \n", left_cnt, right_cnt);

	if (pixcel_cnt > 1800) {//많은 경우,
		draw_line_horizon(bin_videodata, 100);

		if (left_cnt > right_cnt + 200) {
			RobotAction(M_head_down_turn_right);
		}
		else if (right_cnt > left_cnt + 200) {
			RobotAction(M_head_down_turn_left);
		}
		else {
			RobotAction(118);
			RobotAction(M_camera_down_golf);
		}
	}
	else if (pixcel_cnt>1000) { // 적은 경우
		draw_line_horizon(bin_videodata, 60);

		if (left_cnt > right_cnt + 120) {
			RobotAction(M_head_down_turn_right);
		}
		else if (right_cnt > left_cnt + 120) {
			RobotAction(M_head_down_turn_left);
		}
		else {
			RobotAction(118);
			RobotAction(M_camera_down_golf);
		}
	}

	else {
		draw_line_horizon(bin_videodata, 10);

		if (left_cnt > right_cnt + 75) {//80
			RobotAction(M_head_down_turn_right);
		}
		else if (right_cnt > left_cnt + 75) {
			RobotAction(M_head_down_turn_left);
		}
		else {
			RobotAction(M_down_2cm);
			//   Showframe(bin_videodata);
			//free(bin_videodata);
			//free(imageIn);
			return 40;
		}

	}

	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	return 33;
}

///////////////////////여기서부턴 공보기///////////////////////

U8 BALL_x, BALL_y, GOAL_x, GOAL_y;
BOOL isFInd_yellow_ball = FALSE;
BOOL isFInd_blue_hole = FALSE;

//공 위치만 확인하는 함수
U8 watch_ORANGE_ball()
{
	RobotAction(M_camera_down_golf);

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	ColorLabeling(imageIn, bin_videodata, 1, 0, 120, 0, 180, 1, 0, 0, 0, 0, 0);

	U8 BALL_up, BALL_down, BALL_left, BALL_right, BALL_size;
	isFInd_yellow_ball = FALSE;


	U8 y, x;
	//공찾기


	for (y = 0; y < HEIGHT - 1; y++) // 입력 영상의 세로 방향
	{
		for (x = 1; x < WIDTH - 1; x++) // 입력 영상의 가로 방향
		{
			if (GetPtr(bin_videodata, y, x, WIDTH) == RGB565RED)
			{
				if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565RED) > 5)//마스크 크기 지금도 괜찮긴 한데 좀더 나은 거 있나 찾아보기@@@@@@@@@@@@@@@@@@@@@
				{
					if (!isFInd_yellow_ball) {
						BALL_up = y;
						BALL_down = y;
						BALL_left = x;
						BALL_right = x;

						isFInd_yellow_ball = TRUE;
					}
					if (x > BALL_right) BALL_right = x;
					else if (x < BALL_left) BALL_left = x;

					if (y > BALL_down) BALL_down = y;
					else if (y < BALL_up) BALL_up = y;
				}
			}
		}
	}
	BALL_size = BALL_right - BALL_left;
	BALL_x = BALL_left + BALL_size / 2;
	BALL_y = BALL_up + (BALL_down - BALL_up) / 2;

	//----------------------------------------------------
	draw_line_vertical(bin_videodata, BALL_x);
	draw_line_horizon(bin_videodata, BALL_y);

	Showframe(bin_videodata);

	clear_screen();
	free(bin_videodata);
	free(imageIn);
	//----------------------------------------------------
}

//골 위치만 보는 함수
U8 watch_BLUE_hole()
{
	RobotAction(M_CAMERA_BASE);
	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	ColorLabeling(imageIn, bin_videodata, 1, 0, 120, 0, 180, 0, 0, 1, 0, 0, 1);

	U8 GOAL_up, GOAL_down, GOAL_left, GOAL_right, GOAL_size;
	isFInd_blue_hole = FALSE;


	U8 y, x;
	//공찾기


	for (y = 0; y < HEIGHT - 1 && !isFInd_blue_hole; y++) // 입력 영상의 세로 방향
	{
		for (x = 1; x < WIDTH - 1 && !isFInd_blue_hole; x++) // 입력 영상의 가로 방향
		{
			if (GetPtr(bin_videodata, y, x, WIDTH) == RGB565BLACK)
			{
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565BLUE) > 6 && MaskN(bin_videodata, 5, x, y, WIDTH, RGB565WHITE) < 13)//@@@@@@@@@
				{
					if (!isFInd_blue_hole) {
						GOAL_up = y;
						GOAL_down = y;
						GOAL_left = x;
						GOAL_right = x;

						GOAL_x = x;
						GOAL_y = y;

						isFInd_blue_hole = TRUE;
					}
					if (x > GOAL_right) GOAL_right = x;
					else if (x < GOAL_left) GOAL_left = x;

					if (y > GOAL_down) GOAL_down = y;
					else if (y < GOAL_up) GOAL_up = y;
				}
			}
		}
	}
	GOAL_size = GOAL_right - GOAL_left;
	//GOAL_x = GOAL_left + GOAL_size / 2;
	//GOAL_y = GOAL_up + (GOAL_down - GOAL_up) / 2;

	//----------------------------------------------------
	draw_line_vertical(bin_videodata, GOAL_x);
	draw_line_horizon(bin_videodata, GOAL_y);

	Showframe(bin_videodata);

	clear_screen();
	free(bin_videodata);
	free(imageIn);
	//----------------------------------------------------
}

//공에 어느정도 가까이 붙어서 쓰는 함수
U8 close_ORANGE_Ball()
{
	watch_ORANGE_ball();

	if (isFInd_yellow_ball && BALL_y < 40) { //공이 매우 멀리 있다 - > 앞으로
		printf("walk_forward");
		RobotAction(M_walk_forward_good_3);//일반 전진
	}
	else if (isFInd_yellow_ball && BALL_y < 65) { //공이 약간 멀리 있다 - > 앞으로
		printf("walk_forward");
		RobotAction(M_walk_forward_repeat_5);//작게 걷기 여러번 걷는 거
	}
	else if (isFInd_yellow_ball && BALL_y < 80) { //공이 거의 가까이 있다 - > 앞으로@@@@@@@@@@@@@@@
		printf("walk_forward");
		RobotAction(M_walk_forward_repeat_1);//작게 걷기 한 번 걷는 거
	}
	else if (isFInd_yellow_ball) {//공이 매우 가까이 있다 => 중심 맞추기
		if (BALL_x < 120) {//115
			printf("walk_left");
			RobotAction(M_walk_left_small);//왼쪽 걷기
		}
		else if (BALL_x > 135) {//140
			printf("walk_right");
			RobotAction(M_walk_right_small);//오른쪽 걷기
		}
		else {//중심 맞췄으면 공이랑 골이랑 비교
			while (1)
			{
				watch_ORANGE_ball();

				//블루홀 볼 때 왼쪽 오른쪽 보는 거@@@@@@@@@@@@@@@

				while (!isFInd_blue_hole)
					watch_BLUE_hole();
				isFInd_blue_hole = FALSE;

				//블루홀 볼 때 왼쪽 오른쪽 보는 거@@@@@@@@@@@@@@@

				if (GOAL_x < BALL_x + 5)//구멍이 공보다 왼쪽에 있다@@@@@@@@@@@@@@@@@@
				{
					RobotAction(M_walk_right_small);//오른쪽 걷기
					RobotAction(M_turn_left_real_small);//좌회전 작게
				}
				else if (GOAL_x > BALL_x + 17)//구멍이 공보다 오른쪽에 있다@@@@@@@@@@@@@@@@@@
				{
					RobotAction(M_walk_left_small);//왼쪽 걷기
					RobotAction(M_turn_right_real_small);// 우 회전 작게
				}
				else
					break;
			}
			while (1)//공차려고 오른발 앞에 공이 오도록 게걸음 치고 가까이 붙기
			{
				watch_ORANGE_ball();
				if (BALL_x < 125) {//120
					printf("walk_left");
					RobotAction(M_walk_left_small);//왼쪽 걷기
				}
				else if (BALL_x > 135) {//145
					printf("walk_right");
					RobotAction(M_walk_right_small);//오른쪽 걷기
				}
				else if (BALL_y < 87) { //공이 멀리 있다 - > 앞으로
					printf("walk_forward");
					RobotAction(M_walk_forward_repeat_1);//작게 걷기 한 번 걷는 거
				}
				else
					break;
			}
			RobotAction(M_shoot);
			//RobotAction(M_huro_turn_left_90);
			printf("shoot\n");
			return 42;
		}
	}
	else
	{
		printf("can't find ball");
		RobotAction(M_walk_forward_good_3);
	}

	printf("%d %d\n", BALL_x, BALL_y);

	return 41;
}

//공이 아주 멀리 있을 때 쓰는 함수
U8 goto_ORANGE_ball()
{
	RobotAction(M_CAMERA_BASE);


	U16* imageIn = (U16*)malloc(180 * 120 * 2);
	Getframe(imageIn);

	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	ColorLabeling(imageIn, bin_videodata, 1, 0, 120, 0, 180, 1, 0, 0, 0, 0, 0);

	U8 BALL_up, BALL_down, BALL_left, BALL_right, BALL_size;
	isFInd_yellow_ball = FALSE;


	U8 y, x;
	//공찾기


	for (y = 0; y < HEIGHT - 1; y++) // 입력 영상의 세로 방향
	{
		for (x = 1; x < WIDTH - 1; x++) // 입력 영상의 가로 방향
		{
			if (GetPtr(bin_videodata, y, x, WIDTH) == RGB565RED)
			{
				if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565RED) > 4)//마스크 크기 확인@@@@@@@@@@@@@@@@@@@@@@
				{
					if (!isFInd_yellow_ball) {
						BALL_up = y;
						BALL_down = y;
						BALL_left = x;
						BALL_right = x;

						isFInd_yellow_ball = TRUE;
					}
					if (x > BALL_right) BALL_right = x;
					else if (x < BALL_left) BALL_left = x;

					if (y > BALL_down) BALL_down = y;
					else if (y < BALL_up) BALL_up = y;
				}
			}
		}
	}
	BALL_size = BALL_right - BALL_left;
	BALL_x = BALL_left + BALL_size / 2;
	BALL_y = BALL_up + (BALL_down - BALL_up) / 2;

	//----------------------------------------------------
	draw_line_vertical(bin_videodata, BALL_x);
	draw_line_horizon(bin_videodata, BALL_y);
	draw_line_horizon(bin_videodata, 70);

	Showframe(bin_videodata);

	clear_screen();
	free(bin_videodata);
	free(imageIn);
	//----------------------------------------------------

	BOOL is_ball_forward = TRUE, is_ball_side = FALSE;

	if (isFInd_yellow_ball)
	{
		if (BALL_x < 60) { //틀어졌다
			printf("walk_forward");
			RobotAction(M_turn_left_30);
			is_ball_side = TRUE;
		}
		if (BALL_x > 120) { //틀어졌다
			printf("walk_forward");
			RobotAction(M_turn_right_30);
			is_ball_side = TRUE;
		}
		if (BALL_y < 70) { //공이 멀리 있다 - > 앞으로
			printf("walk_forward");
			RobotAction(M_walk_forward_good_3);//일반 전진
			is_ball_forward = FALSE;
		}
	}
	else//공이 아예 안 보이면 숙이기
	{
		return 41;//이거 리턴되면 close_ORANGE_ball로 가도록
				   /*
				   RobotAction(M_walk_forward_good_5);
				   is_ball_side = TRUE;
				   is_ball_forward = FALSE;
				   */
	}

	if (is_ball_forward && !is_ball_side)
		return 41;//이거 리턴되면 close_ORANGE_ball로 가도록

	return 40;
}

U8 After_Shoot_Ball() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 10, 60, 100, 170, 0, 0, 0, 0, 0, 1);

	U8 x, y;
	int whiteCount = 0;
	int whiteCountRight = 0;

	for (x = 2; x <= 178; x++) {
		for (y =20; y <= 50; y++) {
			if (x >= 110 && x <= 160) {
				if (MaskN(bin_videodata, 5, x, y, WIDTH, RGB565WHITE) > 13) {
					whiteCountRight++;
				}
			}
		}
	}
	draw_ROI(bin_videodata, 20, 50, 110, 160);
	Showframe(bin_videodata);
	free(bin_videodata);
	free(imageIn);
	printf("whiteCountRight : %d\n", whiteCountRight);
	if (whiteCountRight < 900) {
		RobotAction(M_turn_left_30);
		return 42;
	}
	else {
		RobotAction(M_walk_right_middle);
		RobotAction(M_walk_right_middle);
		RobotAction(M_walk_right_middle);
		return 50;
	}
	//usleep(1000000);
	//clear_screen();
}

U8 Go_To_YellowTrap() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 60, 120, 0, 180, 0, 0, 0, 1, 0, 0);

	U8 x, y;
	int yellow_count = 0;

	draw_ROI(bin_videodata, 90, 120, 20, 160);
	Showframe(bin_videodata);

	for (x = 20; x <= 160; x++) {
		for (y = 90; y <= 120; y++) {
			if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565YELLOW) > 3) {
				yellow_count++;
			}
		}
	}
	//printf("Yellow trap count : %d\n", up_greenCount);
	free(bin_videodata);
	free(imageIn);
	if (yellow_count > 2000)
		return 51;
	else {
		RobotAction(M_walk_forward_7);
		return 50;
	}
}

U8 Go_To_YellowGate() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 60, 120, 0, 180, 0, 0, 0, 1, 0, 0);

	U8 x, y;
	int yellow_count = 0;

	draw_ROI(bin_videodata, 90, 120, 20, 160);  // 이 부분 수치 파악
	Showframe(bin_videodata);

	for (x = 20; x <= 160; x++) {
		for (y = 90; y <= 120; y++) {
			if (MaskN(bin_videodata, 3, x, y, WIDTH, RGB565YELLOW) > 3) {
				yellow_count++;
			}
		}
	}
	//printf("Yellow Gate count : %d\n", up_greenCount);
	free(bin_videodata);
	free(imageIn);
	if (yellow_count > 2000)
		return 62;
	else {
		RobotAction(M_walk_forward_7);
		return 61;
	}
}

U8 Last_Yellow_Gate() {
	U16* bin_videodata = (U16*)malloc(180 * 120 * 2);
	memset(bin_videodata, 0, 180 * 120 * sizeof(U16));

	U16* imageIn = (U16*)malloc(180 * 120 * 2);

	Getframe(imageIn);

	//void ColorLabeling(U16* imageIn, U16* bin_videodata, U8 obstacle_num, U8 up, U8 down, U8 left, U8 right,
	//	BOOL Find_Red, BOOL Find_Green, BOOL Find_Blue, BOOL Find_Yellow, BOOL Find_Black, BOOL Find_White)
	ColorLabeling(imageIn, bin_videodata, 0, 0, 120, 0, 180, 0, 0, 0, 1, 0, 0);
	//Showframe(imageIn);
	U8 nMaskW = 5;
	U8 nMaskH = 5;
	U8 hW = nMaskW / 2;
	U8 hH = nMaskH / 2;

	U16 Yellow_count = 0;
	U8 up = 10, down = 60, left = 50, right = 130;  // 이 부분 확인 후 수정
	U8 y, x;
	for (y = up - hH; y < down - hH; y++) {
		for (x = left - hW; x < right - hW; x++) {
			U8 c, r;
			U16 cnt = 0;
			for (r = 0; r < nMaskH; r++) {
				for (c = 0; c < nMaskW; c++) {
					if (GetPtr(bin_videodata, r - hH + y, c - hW + x, WIDTH) == RGB565YELLOW) {
						cnt++;
					}
				}
			}
			if ((float)cnt / ((float)nMaskH*(float)nMaskW) > 0.8) {
				Yellow_count++;
				//GetPtr(bin_videodata, y, x, WIDTH) = RGB565BLUE;
			}
		}
	}

	//??????????????????????????
	draw_ROI(bin_videodata, up, down, left, right);
	Showframe(bin_videodata);
	//??????????????????????????

	float density = (float)Yellow_count / (float)((down - up) *(right - left));
	printf("density: %f \n", density);

	free(bin_videodata);
	free(imageIn);

	if (yellow_gate_waiting == 0) {
		if (density <= 0.3) {
			yellow_gate_waiting = 0;
			return 62;
		}
		else {
			yellow_gate_waiting = 1;
			return 62;
		}
	}
	else {
		if (density <= 0.3) {
			//RobotAction(M_walk_forward_10);
			//RobotAction(M_walk_forward_10);

			return 77; //ª°∞???∏Æ∏¶ √?æ?º≠ ¿?¡¶ √?π?
		}
		else {
			return 62;
		}
	}
}


void HURO() {
	U8 order = 30;
	U8 Obstacle_idx = 1;

	printf("pp\n");
	RobotAction(M_base);
	//RobotAction(M_camera_down_golf);

	//printf("order : %d", order);
	while (1) {
		//U16* fpga_videodata = (U16*)malloc(180 * 120 * 2);
		//printf("order : %d", order);
		//---------------------------------------------------
		//Getframe(fpga_videodata);
		//---------------------------------------------------
		if (order == 100) { //노란 게이트내에서 작동하는 함수입니다.
			order = Yellow_Gate();
		}
		else if (order == 0) { //빨간다리를 찾을때까지 달려갑니다.
			order = Red_Bridge();
		}
		else if (order == 1) { //빨간것을 찾았다면 빨간세트를 실행합니다.
			RobotAction(M_RED_bridge_set);
			//RobotAction(4);
			order = 10;
		}
		else if (order == 10) { //빨간것을 내려오고 난 다음 라인을 맞춰줄겁니다.
			order = Line_Tracing();
		}
		else if (order == 11) { //검은 지뢰구간에서 작동하는 함수입니다.
			order = Black_Mine(); 
		}
		else if (order == 20) { //허들찾아서 그쪽으로 간 다음에, 고개를 오른쪽으로 돌려서 블랙라인을 체크합니다.
			//RobotAction(M_walk_forward_repeat_5);
			//order = BLACK_line_check(1, 20, 21, 0);
			//RobotAction(M_walk_forward_3);
			order = 21;
		}
		else if (order == 21) { //라인까지 맞추었다면 허들세트를 실행시킵니다.
			//RobotAction(M_walk_forward_3);
			RobotAction(M_BLUE_hurdle_overcome_set);
			sleep(1);
			order = 22;
		}
		else if (order == 22) { //블루허들 넘어간 다음 라인트레이싱을 한번 실행시킵니다.
			order = After_Blue_Huddle();
		}
		else if (order == 23) { //초록다리를 찾으러 가는 곳입니다.
			order = Go_To_GreenBridge();
		}
		else if (order == 24) {	//초록 다리 발견을 못했을시 라인트레이싱을 합니다.
			order = BLACK_line_check(M_base, 24, 23, 1);
			//order = black_line_forward();
		}
		else if (order == 30) {	//초록 다리에 가까워져 초록 다리 중심을 맞춰 올라갑니다.
			order = CLOSE_TO_GREEN_Bridge();
		}
		else if (order == 31) {	//초록 다리를 건넙니다.
			order = IN_GREEN_Bridge();
		}
		else if (order == 32) {	//초록 다리를 건넙니다.
			order = FIN_GREEN_Bridge();
		}
		else if (order == 33) {	//초록 다리를 내려갑니다.
			order = DOWN_GREEN_Bridge();
		}
		else if (order == 40) { //공을 멀리서 찾습니다.
			order = goto_ORANGE_ball();
		}
		else if (order == 41) { //공을 가까이서 찾습니다. 그리고 공을 차면 42번을 호출해서 나갑니다.
			order = close_ORANGE_Ball();
		}
		else if (order == 42) {

			order = After_Shoot_Ball(); //공을 차고 난 다음 라인을 한번 맞춰줍니다.
		}
		//////////////////////////////////////////////////////////////////////////////////////////
		else if (order == 50) { // 노란 함정을 찾아서 전진하고 찾으면 라인 트레이싱을 합니다.
			order = Go_To_YellowTrap();
		}
		else if (order == 51) {   // 노란 함정 앞에서 라인 트레이싱을 합니다.
			order = BLACK_line_check(M_base, 51, 52, 1);
		}
		else if (order == 52) { // 노란 함정을 극복합니다.(빨간 계단 처럼 내려오면 그냥 세트로 묶으면 되는데 아니라면 내려오는 동작 섬세하게 맞춰야 함)
			//종종걸음하기
			//계단 올라가기
			//노란 장애물 극복하기
			order = 60;
		}
		else if (order == 60) { // 노란 바리게이트가 관심 영역 내에 일정 수 이상 들어올때까지 전진, 일정 수 이상 들어오지 않으면 라인 트레이싱
			order = Go_To_YellowGate();
		}
		else if (order == 61) {	// 노란 픽셀이 일정 수 이상 들어오지 않아 라인 트레이싱 후 다시 Go_To_YellowGate
			order = BLACK_line_check(M_base, 61, 60, 1);
		}
		else if (order == 62) {   // 노란 바리 게이트 처음에는 가만히 있다가 빠지면 전진
			order = Last_Yellow_Gate();
		}
		else if (order == 77) {
			RobotAction(1); //끝
		}

		//free(fpga_videodata);

	}
out:
	//free(fpga_videodata);
	printf("huro end\n");
}
