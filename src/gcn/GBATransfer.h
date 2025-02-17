/*
 * Copyright (C) 2025 breadcodes
 */

#ifndef GBATRANSFER_H
#define GBATRANSFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/si.h>

#define JOY_CMD_RESET 0x0000FF00
#define JOY_CMD_STATUS 0x0000FF00

#define GBA_EWRAM_START 0x02000000
#define MAX_CHUNK_SIZE 32

#define SI_TRANS_DELAY 50

// Change these to extern declarations
extern volatile int frame;
extern volatile u32 addr;
extern volatile u32 data_index;
extern volatile u32 chunk_count;
extern volatile u8 data_agb[MAX_CHUNK_SIZE];
extern volatile u8 data_status[MAX_CHUNK_SIZE];
extern volatile u8 data_out[MAX_CHUNK_SIZE];

extern u8 *resbuf, *cmdbuf;

extern volatile u32 transfer_status;
extern volatile u32 resval;
extern const u32 logodat[39];

void callback(s32 chan, u32 ret);
void acallback(s32 res, u32 val);
uint calckey(uint size);
s32 recv(int chan);
u8 send(int chan, u32 msg);
unsigned int docrc(u32 crc, u32 val);
void doreset();
void getstatus();

s8 init_transfer_rom(int chan, char* gba_rom, u32 gba_rom_size);

#endif // TRANSFER_ROM_H