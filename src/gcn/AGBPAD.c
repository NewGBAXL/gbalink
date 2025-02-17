/*
 * Copyright (C) 2025 NewGBAXL
 */

#include "AGBPAD.h"
#include <ogc/lwp_watchdog.h>

#define TICK_THRESHOLD 8100000

#ifdef __cplusplus
extern "C" {
#endif

static char* __gba_multiboot_image;
static u32 __gba_multiboot_size;
//static u8 __agbpad_initialized[4] = { 0, 0, 0, 0 };
static u8 __agbpad_hardware[4] = { 0, 0, 0, 0 };
static u8 __agbpad_rumble[4] = { 0, 0, 0, 0 };
static u8 __agbpad_currstate[4] = { 0, 0, 0, 0 };
static u16 __agbpad_prevstate[4] = { 0, 0, 0, 0 };
static u32 lastTicks = 0;

void AGBPAD_SetMultibootImage(char* image, u32 size) {
	__gba_multiboot_image = image;
	__gba_multiboot_size = size;
	return;
}

u32 AGBPAD_Init(int chan) {
	u8 hardware;
	return AGBPAD_Init_HW(chan, &hardware);
}

u32 AGBPAD_Init_HW(int chan, u8* hardware) {
	if (__gba_multiboot_image == NULL || __gba_multiboot_size == 0) {
		return -1;
	}

	//make this a thread!!
	u32 status = GBALINK_Init(chan, __gba_multiboot_image, __gba_multiboot_size);
	__gba_initialized[chan] = 1;
	return status;
}

/*
u32 AGBPAD_ScanPads(void) {
	return PAD_ScanPads() | AGBPAD_ScanGBA();
}
*/

u32 AGBPAD_Read(AGBPADStatus* status) {
	/*u32 currentTicks = gettick();
	if (currentTicks - lastTicks >= TICK_THRESHOLD) {
		lastTicks = currentTicks;
	}
	else {
		return;
	}*/
	
	for (int chan = 0; chan < 4; ++chan) {
		if (__gba_initialized[chan]) {
			status[chan].button = GBALINK_GetInput(chan);
			if (__agbpad_hardware[chan] & CARTTYPE_TILT) {
				u32 tilt = GBALINK_GetTiltInput(chan);
				status[chan].accel.x = tilt & 0xFFFF;
				status[chan].accel.y = tilt >> 16;
			}
			if (__agbpad_hardware[chan] & CARTTYPE_GYRO) {
				u64 gyro = GBALINK_GetGyroInput(chan);
				status[chan].gyro.x = gyro & 0xFFFF;
				status[chan].gyro.y = (gyro >> 16) & 0xFFFF;
				status[chan].gyro.z = gyro >> 32;
			}
			if (__agbpad_hardware[chan] & CARTTYPE_LIGHT) {
				status[chan].light = GBALINK_GetLightInput(chan);
			}
		}
    }
	return 1;
}

void AGBPAD_ScanGBA() {
	AGBPADStatus status[4];
	AGBPAD_Read(status);
	
	for (int chan = 0; chan < 4; ++chan) {
		if (SI_DecodeType(SI_GetType(chan)) != SI_GBA) {
			status[chan].err = AGBPAD_ERR_NO_CONTROLLER;
			__gba_initialized[chan] = 0;
			continue;
		}

		if (__gba_initialized[chan] != 1 && __gba_initialized[chan] != -5) {
			__gba_initialized[chan] = -5;
			status[chan].err = AGBPAD_ERR_TRANSFER_IN_PROGRESS;
			AGBPAD_Init(chan);
		}
		else {
			status[chan].err = AGBPAD_ERR_NONE;
			__agbpad_prevstate[chan] = __agbpad_currstate[chan];
			__agbpad_currstate[chan] = status[chan].button;
		}
	}
	return;
}

u16 AGBPAD_ButtonsUp(int chan) {
	return ~__agbpad_currstate[chan] & __agbpad_prevstate[chan];
}

u16 AGBPAD_ButtonsDown(int chan) {
	return __agbpad_currstate[chan] & ~__agbpad_prevstate[chan];
}

u16 AGBPAD_ButtonsHeld(int chan) {
	return __agbpad_currstate[chan] & __agbpad_prevstate[chan];
}

//rumble is killing the framerate!
//solution: log a rumble wave and send it at once
void AGBPAD_ControlMotor(int chan, u32 cmd) {
	if ((__agbpad_hardware[chan] & CARTTYPE_RUMBLE) && (cmd != __agbpad_rumble[chan])) {
		__agbpad_rumble[chan] = cmd;
		GBALINK_Rumble((u32)chan, cmd);
	}
}

u32 AGBPAD_DisplayFrame(int chan, u32* frame) {
	//todo
	return 1;
}

u32 AGBPAD_DisplayFramebuffer(int chan, void* framebuffer) {
	/*u8* indexedData = (u8*)malloc(240 * 160);
	u16 gbaPalette[256];
	__generate_palette(framebuffer, indexedData, gbaPalette);

	GBALINK_SetSplash(chan, 0, (char*)indexedData, sizeof(indexedData), (char*)gbaPalette, sizeof(gbaPalette));
	free(indexedData);*/
	return 1;
}

u32 AGBPAD_DisplayLookAtTv(int chan, u8 status) {
	return GBALINK_SetLookAtTv((u32)chan, status);
}

void AGBPAD_ControlSpeaker(int chan, s32 enable) {
	//todo
	return;
}

void AGBPAD_SetSpeakerVol(int chan, s8 vol) {
	//todo
	return;
}

#ifdef __cplusplus
}
#endif
