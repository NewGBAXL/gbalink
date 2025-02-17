/*
 * Copyright (C) 2025 NewGBAXL
 */

#include "GBALink.h"

#ifdef __cplusplus
extern "C" {
#endif

//Sets up SI for GBA communication after GBA is detected,
//and pushes the multiboot image to the GBA.
u8 GBALINK_Init(int chan, char* image, u32 size) {
	if (__gba_initialized[chan]) return 1;

	init_transfer_rom(chan, image, size);
	SI_SetCommand(chan, 0x00); // Simple status command
	SI_EnablePolling(chan);

	__gba_initialized[chan] = 1;
	return 1;
}

//We're going to assume if we are sending
//multiple packets to the GBA, we are loading
//images, sound, etc and it would still be nice to
//have controller input during that time.

bool isSendingFile = false;
u32 gba_keys = 0;

//Sends a packet to the GBA and returns a response.
u32 GBALINK_SendPacket(int chan, u32 packet) {
	u4 success = send(chan, packet);
	if (!success) {
		__gba_initialized[chan] = 0;
		return 0; //fix hotswapping with other devices
	}
	//FrameWait();
	u32 res = recv(chan);
	res = __builtin_bswap32(res);
	return res;
}

//Sends a higher-level command to the GBA.
//Commands must be supported from the GBA side.
u32 GBALINK_SendCommand(int chan, u4 commandType, u16 index, u16 packets, u4 extra) {
	//u4 commandType, u12 index, u14 packets, u2 reserved (zero)
	commandType &= 0xF;
	index &= 0xFFF;
	packets &= 0x3FFF;
	extra &= 0x3;
	u32 command = (commandType << 28) | (index << 16) | (packets << 2) | extra;
	return GBALINK_SendPacket(chan, command);
}

//Get GBA button input.
u32 GBALINK_GetInput(int chan) {
	//if (!isSendingFile) {
		gba_keys = GBALINK_SendCommand(chan, GBA_CMD_GET_INPUT, 0, 0, 0);
	//}
	return gba_keys;
}

//Get GBA cartridge gyro input.
u32 GBALINK_GetGyroInput(int chan) {
	return GBALINK_SendCommand(chan, GBA_CMD_GET_INPUT, 0, 0, 1);
}

//Get GBA cartridge tilt input.
u32 GBALINK_GetTiltInput(int chan) {
	return GBALINK_SendCommand(chan, GBA_CMD_GET_INPUT, 0, 0, 2);
}

//Get GBA cartridge light sensor input.
u32 GBALINK_GetLightInput(int chan) {
	return GBALINK_SendCommand(chan, GBA_CMD_GET_INPUT, 0, 0, 3);
}

//Set GBA cartridge rumble.
u32 GBALINK_Rumble(int chan, u32 command) {
	return GBALINK_SendCommand(chan, GBA_CMD_RUMBLE, 0, 0, (u4)command);
}

//Set multiple GBA palette entries.
//Indexes 0-255 are used for the background,
//while 256-511 are used for sprites.
//https://gbadev.net/gbadoc/memory.html#palette-ram
u32 GBALINK_SetPalette(int chan, int index, int count, u16* palette) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_PALETTE, (u16)index, (count + 1) / 2, count & 1);
	if (status) {
		for (int i = 0; i < (count + 1) / 2; ++i) {
			GBALINK_SendPacket(chan, palette[i * 2] | (palette[i * 2 + 1] << 16));
		}
	}
	return status;
}

//Set multiple GBA tile entries.
//Use mode to specify if in 256-bit or 512-bit mode. 0: 256-bit; 1: 512-bit.
//https://gbadev.net/gbadoc/sprites.html#sprite-tile-data
u32 GBALINK_SetTiles(int chan, int index, int count, u32* tiles, bool mode) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_TILES, (u16)index, count * (mode ? 8 : 16), mode);
	if (status) {
		for (int i = 0; i < count * (mode ? 8 : 16); ++i) {
			GBALINK_SendPacket(chan, tiles[i]);
		}
	}
	return status;
}

//Set GBA sprite attributes.
//Developers' Note: use lerp to move sprites across
//the screen without calling this function every frame.
//0: OFF (immediate); 1: Slow; 2: Medium; 3: Fast.
//https://gbadev.net/gbadoc/sprites.html
u32 GBALINK_SetSprite(int chan, int index, u64 sprite, int lerp) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_SPRITE, (u16)index, 2, (u4)lerp);
	if (status) {
		GBALINK_SendPacket(chan, sprite & 0xFFFFFFFF);
		GBALINK_SendPacket(chan, sprite >> 32);
	}
	return status;
}

//Set GBA background map.
//Mode 1 is used for tiled backgrounds.
//There are 3 available background slots, with the third
//slot being affine, meaning it can be rotated and scaled.
//sizeMode specifies the background size.
//0: 256x256; 1: 256x512; 2: 512x256; 3: 512x512.
//https://gbadev.net/gbadoc/backgrounds.html
u32 GBALINK_SetMap(int chan, int index, int count, u16* tile, int sizeMode) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_MAP, (u16)index, (u16)((count + 1) / 2), (u4)sizeMode);
	if (status) {
		for (int i = 0; i < (count + 1) / 2; ++i) {
			GBALINK_SendPacket(chan, tile[i * 2] | (tile[i * 2 + 1] << 16));
		}
	}
	return status;
}

//Store sound data to EWRAM. Available sound slots
//and allocated space are to be determined.
//https://gbadev.net/gbadoc/audio/directsound.html
/*u32 GBALINK_SetSound(int chan, int index, string soundSrc, bool isChiptune) {
	u32 packets = 0; //get filesize in bytes / 4
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_SOUND, (u16)index, packets, isChipTune);
	if (status) {
		for (int i = 0; i < packets; ++i) {
			//GBALINK_SendPacket(chan, GetHex(soundSrc, i*4));
		}
	}
	return status;
}*/

//Play stored sound data from EWRAM for specified duration.
//Available sound slots are to be determined.
u32 GBALINK_PlaySound(int chan, int index, int duration, bool isChiptune) {
	//*misusing packets as length to play sound
	return GBALINK_SendCommand(chan, GBA_CMD_SET_SOUND, (u16)index, (u16)duration, isChiptune + 2);
}

//Set a fullscreen background image.
//!! Splash Screen should use Mode 4 - 240x160, 256 colors.
//Splash bitmap + palette takes up 10,112 bytes, roughly 3.86% of the multiboot space.
//Splash Screen is automatially loaded into VRAM after being fully transferred.
//Use this tool to convert to GBA bitmap: https://github.com/felixjones/gfx2agb
/*u32 GBALINK_SetSplash(int chan, int index, string splashImgSrc, string splashPalSrc) {
	isSendingFile = true;
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_SPLASH, (u16)index, 10112, 0);

	if (status) {
		for (int i = 0; i < 128; ++i) {
			//GBALINK_SendPacket(chan, GetHex(splashPalSrc, i*4));
		}

		for (int i = 0; i < 9600; ++i) {
			//GBALINK_SendPacket(chan, GetHex(splashImgSrc, i*4));
		}
	}
	//gba_keys = recv();
	//gba_keys = __builtin_bswap32(res);
	return status;
}*/

u32 GBALINK_LoadSplash(int chan, int index) {
	return GBALINK_SendCommand(chan, GBA_CMD_SET_SPLASH, index, 0, 1);
}

//0: OFF; 1: ON; 2: Slow Flash
//Developers' Note: only call GBA_SetLookAtTv()
//if you are using the GBA for more than a splash screen,
//and you have nothing to display on the GBA for a short period.
u32 GBALINK_SetLookAtTv(int chan, int status) {
	return GBALINK_SendCommand(chan, GBA_CMD_SET_LOOK_AT_TV, 0, 0, status);
}

//Returns REGIN and REGOUT for the specified window.
//https://gbadev.net/gbadoc/windowing.html
u32 GBALINK_GetWindow(int chan, int index) {
	return GBALINK_SendCommand(chan, GBA_CMD_SET_WINDOW, (u16)index, 0, 0);
}

//Set a window for the GBA to display. Enable with GBA_EnableDisplay().
//Returns REGIN and REGOUT for the specified window.
u32 GBALINK_SetWindow(int chan, int index, int x, int y, int width, int height) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_WINDOW, (u16)index, 2, 0);

	GBALINK_SendPacket(chan, ((x + width) << 16) | x);
	GBALINK_SendPacket(chan, ((y + height) << 16) | y);
	return status;
}

u32 GBALINK_EnableDisplay(int chan, int index, u32 enable, u16 vCountTrigger) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_ENABLE_DISPLAY, (u16)index, (vCountTrigger == 0) ? 1 : 2, 0);
	//little tricky here - it's DISPCNT & DISPSTAT wrapped into one.
	//*note: bit 1 from enable switches between MODE 1 and MODE 4
	//DISPCNT bits (0 OR 2),4,5,6,7,8,9,A,B,C,D,E,F
	//DISPSTAT bits 3,4,5
	if (status) {
		GBALINK_SendPacket(chan, enable);
		if (vCountTrigger != 0) GBALINK_SendPacket(chan, vCountTrigger);
	}
	return 1;
}
//0: Scroll; 1: Scale; 2: Shear; 3: Rotate
u32 GBALINK_SetBkgScrollAffine(int chan, int index, u32 x, u32 y, int mode) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_BKG_SCROLL_AFFINE, (u16)index, 2, (u4)mode);
	if (status) {
		if (mode == 3) {
			GBALINK_SendPacket(chan, x);
			GBALINK_SendPacket(chan, y);
		}
		else {
			GBALINK_SendPacket(chan, (x << 16) | y);
		}
	}
	return status;
}

u32 GBALINK_SetMosaicEffect(int chan, int index, u32 mosaic) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_EFFECT, (u16)index, 1, 0);
	if (status) {
		GBALINK_SendPacket(chan, mosaic);
	}
	return status;
}

u32 GBALINK_SetBlendEffect(int chan, int index, u32 blend) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_EFFECT, (u16)index, 1, 1);
	if (status) {
		GBALINK_SendPacket(chan, blend);
	}
	return status;
}

u32 GBALINK_SetAlphaEffect(int chan, int index, u8 alphaSrcCoef, u8 alphaTgtCoef, u8 y) {
	u32 status = GBALINK_SendCommand(chan, GBA_CMD_SET_EFFECT, (u16)index, 1, 2);
	u32 data = (alphaSrcCoef << 10 & 0x1F) | (alphaTgtCoef << 5 & 0x1F) | (y & 0x1F);
	if (status) {
		GBALINK_SendPacket(chan, data);
	}
	return status;
}

u32 GBALINK_GetHardware(int chan) {
	return GBALINK_SendCommand(chan, GBA_CMD_GET_HARDWARE, 0, 0, 0);
}

#ifdef __cplusplus
}
#endif
