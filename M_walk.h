#pragma once

typedef enum {
	M_walk_forward_good_5 = 3,//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	M_walk_forward_repeat_5 = 4,

	M_walk_left_small = 6,
	M_walk_right_small = 7,
	M_walk_left_middle = 8,
	M_walk_right_middle = 9,
	M_walk_left_large = 10,
	M_walk_right_large = 11,

	M_walk_mine_20 = 23,// Áö·Ú20¹ú·Á°È±â
	M_walk_mine_30 = 25,// Áö·Ú30¹ú·Á°È±â

	M_walk_forward_1 = 61,
	M_walk_forward_2 = 62,
	M_walk_forward_3 = 63,
	M_walk_forward_4 = 64,
	M_walk_forward_5 = 65,//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	M_walk_forward_6 = 66,
	M_walk_forward_7 = 67,
	M_walk_forward_8 = 68,
	M_walk_forward_9 = 69,
	M_walk_forward_10 = 70,

	M_walk_small = 118,

	M_huro_camera_up_left_walk = 91,
	M_huro_camera_up_right_walk = 92,

	M_huro_camera_down_walk_golf = 94,
	M_huro_camera_down_left_walk_golf = 95,
	M_huro_camera_down_right_walk_golf = 96,

	M_huro_camera_right_left_walk_large = 98,
	M_huro_camera_right_right_walk_large = 99,

	M_huro_CAM_Right_walk_left_small = 100,
	M_huro_CAM_Right_walk_right_small = 101,



	M_head_down_walk = 123,
	M_head_down_turn_left = 124,
	M_head_down_turn_right = 125,

	M_walk_forward_good_3 = 134,
	M_walk_forward_repeat_1 = 118,

	//-------------------------------------------------------------------
	M_huro_CAM_Right_walk_left = 98,
	M_huro_CAM_Right_walk_right = 99,

	M_huro_ball_walk_forward_2 = 251,//252¹ø = ¼÷ÀÌ°í 2°ÉÀ½ ÀüÁø
	M_huro_ball_walk_forward = 252,//252¹ø = ¼÷ÀÌ°í ÀüÁø
	M_huro_ball_walk_left = 253,//253¹ø = ¼÷ÀÌ°í ÁÂ°È±â
	M_huro_ball_walk_right = 254,//254¹ø = ¼÷ÀÌ°í ¿ì°È±â

	M_huro_CAM_right_walk_left_large = 125,//¿À¸¥ÂÊº¸°í ÁÂ°È±â Å©°Ô
	M_huro_CAM_right_walk_right_large = 126,//¿À¸¥ÂÊº¸°í ¿ì°È±â Å©°Ô

	M_huro_walk_forward_in_competition = 212,
}Robot_Motion_walk;