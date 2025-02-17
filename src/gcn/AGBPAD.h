/*
 * Copyright (C) 2025 NewGBAXL
 */

#ifndef _AGBPAD_H_
#define _AGBPAD_H_

#include "GBALink.h"

#define AGBPAD_CHAN0			0
#define AGBPAD_CHAN1			1
#define AGBPAD_CHAN2			2
#define AGBPAD_CHAN3			3
#define AGBPAD_CHANMAX			4

#define AGBPAD_BUTTON_A			0x0001
#define AGBPAD_BUTTON_B			0x0002
#define AGBPAD_BUTTON_SELECT	0x0004
#define AGBPAD_BUTTON_START		0x0008
#define AGBPAD_BUTTON_RIGHT		0x0010
#define AGBPAD_BUTTON_LEFT		0x0020
#define AGBPAD_BUTTON_UP		0x0040
#define AGBPAD_BUTTON_DOWN		0x0080
#define AGBPAD_BUTTON_R			0x0100
#define AGBPAD_BUTTON_L			0x0200

#define AGBPAD_MOTOR_STOP		0
#define AGBPAD_MOTOR_RUMBLE		1
#define AGBPAD_MOTOR_STOP_HARD	2

#define AGBPAD_ERR_NONE					0
#define AGBPAD_ERR_NO_CONTROLLER		-1
#define AGBPAD_ERR_NO_RESPONSE			-2
#define AGBPAD_ERR_TRANSFER_IN_PROGRESS	-3

#define AGBPAD_DATA_BUTTONS		0x01
#define AGBPAD_DATA_ACCEL		0x02
#define AGBPAD_DATA_GYRO		0x04
#define AGBPAD_DATA_LIGHT		0x08

#define PAD_CHAN0_BIT			0x80000000
#define PAD_CHAN1_BIT			0x40000000
#define PAD_CHAN2_BIT			0x20000000
#define PAD_CHAN3_BIT			0x10000000

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _agbtilt {
	s32 x;
	s32 y;
} agbtilt;

typedef struct _agbgyro {
	s16 x;
	s16 y;
	s16 z;
} agbgyro;

typedef struct _agbpadstatus {
	u16 button;
	agbtilt accel;
	agbgyro gyro;
	u8 light;
	u8 cart_hardware;
	//struct rtc_time_t rtc;
	s8 err;
} AGBPADStatus;

void AGBPAD_SetMultibootImage(char* image, u32 size);
u32 AGBPAD_Init(int chan);
u32 AGBPAD_Init_HW(int chan, u8* hardware);
//u32 AGBPAD_ScanPads();
void AGBPAD_ScanGBA();
u32 AGBPAD_Read(AGBPADStatus* status);
u16 AGBPAD_ButtonsUp(int chan);
u16 AGBPAD_ButtonsDown(int chan);
u16 AGBPAD_ButtonsHeld(int chan);
void AGBPAD_ControlMotor(int chan,u32 cmd);

//u32 AGBPAD_DisplayFrame(int chan, u32* frame);
//u32 AGBPAD_DisplayFramebuffer(int chan, u32* buffer);

//s32 AGBPAD_ControlSpeaker(u32 chan, s8 enable);
//s32 AGBPAD_SetSpeakerVol(u32 chan, s8 vol);

#ifdef __cplusplus
}
#endif

#endif