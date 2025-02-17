/*
 * Copyright (C) 2025 NewGBAXL
 */

#ifndef _GBALINK_H_
#define _GBALINK_H_

#include "GBATransfer.h"

//#include <cstring>

#define GBA_CMD_RESERVED1 0
#define GBA_CMD_RESERVED2 1
#define GBA_CMD_GET_INPUT 2
#define GBA_CMD_RUMBLE 3
#define GBA_CMD_SET_PALETTE 4
#define GBA_CMD_SET_TILES 5
#define GBA_CMD_SET_SPRITE 6
#define GBA_CMD_SET_MAP 7
#define GBA_CMD_SET_SOUND 8
#define GBA_CMD_SET_SPLASH 9
#define GBA_CMD_SET_LOOK_AT_TV 10
#define GBA_CMD_SET_WINDOW 11
#define GBA_CMD_ENABLE_DISPLAY 12
#define GBA_CMD_SET_BKG_SCROLL_AFFINE 13
#define GBA_CMD_SET_EFFECT 14
#define GBA_CMD_GET_HARDWARE 15

#define CARTTYPE_TILT 0x02
#define CARTTYPE_GYRO 0x04
#define CARTTYPE_RUMBLE 0x08
#define CARTTYPE_LIGHT 0x10

#ifdef __cplusplus
extern "C" {
#endif

static s8 __gba_initialized[4] = { 0, 0, 0, 0 };

typedef unsigned int u4;

u8 GBALINK_Init(int chan, char* image, u32 size);
u32 GBALINK_SendCommand(int chan, u4 commandType, u16 index, u16 packets, u4 data);
u32 GBALINK_SendPacket(int chan, u32 data);
u32 GBALINK_GetInput(int chan);
u32 GBALINK_GetGyroInput(int chan);
u32 GBALINK_GetTiltInput(int chan);
u32 GBALINK_GetLightInput(int chan);
u32 GBALINK_Rumble(int chan, u32 command);
u32 GBALINK_SetPalette(int chan, int index, int count, u16* palette);
u32 GBALINK_SetTiles(int chan, int index, int count, u32* tiles, bool mode);
u32 GBALINK_SetSprite(int chan, int index, u64 sprite, int lerp);
u32 GBALINK_SetMap(int chan, int index, int count, u16* tile, int sizeMode);
//u32 GBALINK_SetSound(int chan, int index, string soundSrc, bool isChiptune);
//u32 GBALINK_SetSplash(int chan, int index, string splashImgSrc, string splashPalSrc);
u32 GBALINK_SetLookAtTv(int chan, int status);
u32 GBALINK_GetWindow(int chan, int index);
u32 GBALINK_SetWindow(int chan, int index, int x, int y, int width, int height);
u32 GBALINK_EnableDisplay(int chan, int index, u32 enable, u16 vCountTrigger);
u32 GBALINK_SetBkgScrollAffine(int chan, int index, u32 x, u32 y, int mode);
u32 GBALINK_SetMosaicEffect(int chan, int index, u32 mosaic);
u32 GBALINK_SetBlendEffect(int chan, int index, u32 blend);
u32 GBALINK_GetHardware(int chan);

#ifdef __cplusplus
}
#endif

#endif