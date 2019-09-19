#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__
/*********************************************************************
Communication Protocol :

 Start Code(1byte) + Command(1byte) + Data(1byte) + CheckSum = total 4bytes
  0xFF    0x??   0x?? 0x??

*********************************************************************/
#define START_CODE    0xFF
#define START_CODE1   0x55
#define Hdata	      0x00
#define Hdata1        0xFF

void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata, unsigned char Ldata1);
void RobotAction(int RobotArea);
unsigned char Receive_ACK();


#endif // __ROBOT_PROTOCOL_H__


