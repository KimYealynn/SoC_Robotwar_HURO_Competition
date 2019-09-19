#pragma once

typedef enum {
	M_base = 1,

	M_BLUE_hurdle_overcome = 28,

	M_up_2cm = 33,
	M_down_2cm = 37,

	M_shoot = 41,

	M_YELLOW_bridge = 44, //(다리벌려서기어가서일어나기)
	M_RED_bridge = 51,

	M_camera_down_golf = 58,// 골프공앞 카메라 아래로
	M_camera_up_golf = 59,//골프공앞 카메라 위로	

	M_RED_bridge_set = 72, //그냥도 있고 세트도 있고
	M_BLUE_hurdle_overcome_set = 81,//그냥도 있고 세트도 있고

	M_camera_up_RED_bridge = 89,
	M_camera_right = 90,

	//new!!!!!!!!!!!!
	BALL_turn_left15 = 112,
	BALL_turn_right15 = 113,

	M_CAMERA_UP550 = 103,// 머리상하550
	M_CAMERA_UP500 = 104,// 머리상하500
	M_CAMERA_UP450 = 105,// 머리상하450
	M_CAMERA_BASE = 106,// 머리상하400
	M_CAMERA_DOWN350 = 107,// 머리상하350
	M_CAMERA_DOWN300 = 108,// 머리상하300
	M_CAMERA_DOWN250 = 109,// 머리상하250
	M_CAMERA_DOWN200 = 110,// 머리상하200

	M_CAMERA_LEFT100 = 131,
	M_CAMERA_RIGHT100 = 132,

	 //------------------------------------------------------------------------------------
						   //------------------------------------------------------------------------------------

						   M_huro_first_yellow_set = 227,


						   M_huro_hudle_set1 = 241,

						   M_huro_hudle_set2 = 244,



						   M_huro_ball_base = 201,//201번 = 고개 숙이기


												  //-----------------------
												  Y_walk_forward = 215,
												  Y_turn_left = 216,
												  Y_turn_right = 217,
												  Y_walk_left = 218,
												  Y_walk_right = 219,

												  B_walk_forward = 112,
												  B_left = 113,
												  B_right = 114
}Robot_Motion_else;