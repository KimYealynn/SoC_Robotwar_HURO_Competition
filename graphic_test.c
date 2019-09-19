//영상처리
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
#include <getopt.h>
#include "uart_api.h"
#include "robot_protocol.h"
#include "robot_action.h"
#include <termios.h>
static struct termios inittio, newtio;


#define AMAZON2_GRAPHIC_VERSION		"v0.5"

void init_console(void)
{
	tcgetattr(0, &inittio);
	newtio = inittio;
	newtio.c_lflag &= ~ICANON;
	newtio.c_lflag &= ~ECHO;
	newtio.c_lflag &= ~ISIG;
	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;

	cfsetispeed(&newtio, B115200);

	tcsetattr(0, TCSANOW, &newtio);
}

int main(int argc, char **argv)
{
	int ret;
	if (open_graphic() < 0) {
		return -1;
	}

	BOOL loop = TRUE;

	while (loop)
	{
		printf("Enter command:\n");

		int ch = getchar();

		switch (ch)
		{
		case 'r':
			init_console();

			ret = uart_open();
			if (ret < 0) return EXIT_FAILURE;

			uart_config(UART1, 57600, 8, UART_PARNONE, 1);
			HURO();

			uart_close();
			break;
		case 't':
			init_console();

			ret = uart_open();
			if (ret < 0) return EXIT_FAILURE;

			uart_config(UART1, 57600, 8, UART_PARNONE, 1);

			//RobotAction(M_camera_down_golf);

			U16* fpga_videodata = (U16*)malloc(180 * 120 * 2);
			
			RobotAction(M_base);
			while (1)
			{
				watch_BLUE_hole();
			}
			
			/*/
			RobotAction(M_turn_left_30);
			RobotAction(M_turn_right_30);
			RobotAction(M_walk_forward_repeat_5);
			RobotAction(M_turn_left_30);
			RobotAction(M_turn_right_30);
			*/

			uart_close();
			break;
		case 'q':
			loop = FALSE;
			break;
		}
	}

	close_graphic();
	return 0;
}

