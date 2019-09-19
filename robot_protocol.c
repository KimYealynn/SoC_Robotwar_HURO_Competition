#include <stdio.h>
#include <string.h>
#include "robot_protocol.h"
#include "uart_api.h"
//////////////////////////////////////////////////// Protocol Test

void DelayLoop(int delay_time)
{
	while(delay_time)
		delay_time--;
}

void Send_Command(unsigned char Ldata, unsigned char Ldata1)
{
	unsigned char Command_Buffer[6] = {0,};

	Command_Buffer[0] = START_CODE;	// Start Byte -> 0xff
	Command_Buffer[1] = START_CODE1; // Start Byte1 -> 0x55
    Command_Buffer[2] = Ldata;
	Command_Buffer[3] = Ldata1;
	Command_Buffer[4] = Hdata;  // 0x00
	Command_Buffer[5] = Hdata1; // 0xff

	uart1_buffer_write(Command_Buffer, 6);
}

unsigned char Receive_ACK()
{
	unsigned char Command[6] = { 0, };
	//-----------------------------------------------------
	uart1_buffer_read(Command, 6);

	int i = 0;
	for (i = 0; i < 6; i++)
	{
		printf("buf_uart1[%d]:%d\n", i, Command[i]);
	}

	return Command[2];
}

void RobotAction(int RobotArea)
{
	Send_Command(RobotArea, 255 - RobotArea);
	while(Receive_ACK() != 255)
	{ }
	usleep(700000);
	printf("out\n");
}

