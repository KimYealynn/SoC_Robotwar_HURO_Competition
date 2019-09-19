#pragma once

typedef enum {
	M_turn_left_30 = 13,
	M_turn_left_40 = 14,
	M_turn_right_30 = 15,
	M_turn_right_40 = 16,

	M_turn_left_in_place = 20,// 제자리작은좌회전
	M_turn_right_in_place = 21,// 제자리작은우회전

							   //new!!!!!!!!!!!!
							   M_CAMERA_right_turn_left40 = 115,// 고개오른쪽좌회전40
							   M_CAMERA_right_turn_right40 = 116,// 고개오른쪽우회전40

							   M_turn_left_real_small = 120,
							   M_turn_right_real_small = 121,

																 //----------------------------------------------------------

																 M_huro_turn_left_90 = 195,

																 M_huro_CAM_Right_turn_left = 222,
																 M_huro_CAM_Right_turn_right = 223,


}Robot_Motion_turn;