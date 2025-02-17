/*
 * Copyright (C) 2025 breadcodes
 */

#include "GBATransfer.h"

volatile int frame = 0;
volatile u32 addr = GBA_EWRAM_START;
volatile u32 data_index = 0;
volatile u32 chunk_count = 0;
volatile u8 data_agb[MAX_CHUNK_SIZE];
volatile u8 data_status[MAX_CHUNK_SIZE];
volatile u8 data_out[MAX_CHUNK_SIZE];

u8 *resbuf = NULL;
u8 *cmdbuf = NULL;

volatile u32 transfer_status = 0;
volatile u32 resval = 0;
const u32 logodat[39] = {
    0x24FFAE51, 0x699AA221, 0x3D84820A, 0x84E409AD, 0x11248B98, 0xC0817F21, 0xA352BE19, 0x9309CE20,
    0x10464A4A, 0xF82731EC, 0x58C7E833, 0x82E3CEBF, 0x85F4DF94, 0xCE4B09C1, 0x94568AC0, 0x1372A7FC,
    0x9F844D73, 0xA3CA9A61, 0x5897A327, 0xFC039876, 0x231DC761, 0x0304AE56, 0xBF388400, 0x40A70EFD,
    0xFF52FE03, 0x6F9530F1, 0x97FBC085, 0x60D68025, 0xA963BE03, 0x014E38E2, 0xF9A234FF, 0xBB3E0344,
    0x780090CB, 0x88113A94, 0x65C07C63, 0x87F03CAF, 0xD625E48B, 0x380AAC72, 0x21D4F807};

void callback(s32 chan, u32 ret)
{
    transfer_status = 1;
}
void acallback(s32 res, u32 val)
{
    resval = val;
}
uint calckey(uint size)
{
    uint ret = 0;
    size = (size - 0x200) >> 3;
    int res1 = (size & 0x3F80) << 1;
    res1 |= (size & 0x4000) << 2;
    res1 |= (size & 0x7F);
    res1 |= 0x380000;
    int res2 = res1;
    res1 = res2 >> 0x10;
    int res3 = res2 >> 8;
    res3 += res1;
    res3 += res2;
    res3 <<= 24;
    res3 |= res2;
    res3 |= 0x80808080;
    if ((res3 & 0x200) == 0)
    {
        ret |= (((res3) & 0xFF) ^ 0x4B) << 24;
        ret |= (((res3 >> 8) & 0xFF) ^ 0x61) << 16;
        ret |= (((res3 >> 16) & 0xFF) ^ 0x77) << 8;
        ret |= (((res3 >> 24) & 0xFF) ^ 0x61);
    }
    else
    {
        ret |= (((res3) & 0xFF) ^ 0x73) << 24;
        ret |= (((res3 >> 8) & 0xFF) ^ 0x65) << 16;
        ret |= (((res3 >> 16) & 0xFF) ^ 0x64) << 8;
        ret |= (((res3 >> 24) & 0xFF) ^ 0x6F);
    }
    return ret;
}

s32 recv(int chan)
{
    SI_GetTypeAsync(chan, acallback);
	if (resval && !(resval & SI_GBA))
	{
		printf("No GBA\n");
	}
    
    memset(resbuf, 0, 32);
    cmdbuf[0] = 0x14; // read
    transfer_status = 0;
    SI_Transfer(chan, cmdbuf, 1, resbuf, 5, callback, SI_TRANS_DELAY);
    while (transfer_status == 0)
        ;
    return *(vu32 *)resbuf;
}
u8 send(int chan, u32 msg)
{
    SI_GetTypeAsync(chan, acallback);
    if (resval && !(resval & SI_GBA))
    {
        printf("No GBA\n");
        return 0;
    }
    
    cmdbuf[0] = 0x15;
    cmdbuf[1] = (msg >> 0) & 0xFF;
    cmdbuf[2] = (msg >> 8) & 0xFF;
    cmdbuf[3] = (msg >> 16) & 0xFF;
    cmdbuf[4] = (msg >> 24) & 0xFF;
    transfer_status = 0;
    resbuf[0] = 0;
    SI_Transfer(chan, cmdbuf, 5, resbuf, 1, callback, SI_TRANS_DELAY);
    while (transfer_status == 0)
        ;

    return 1;
}
unsigned int docrc(u32 crc, u32 val)
{
    int i;
    for (i = 0; i < 0x20; i++)
    {
        if ((crc ^ val) & 1)
        {
            crc >>= 1;
            crc ^= 0xa1c1;
        }
        else
            crc >>= 1;
        val >>= 1;
    }
    return crc;
}
void doreset(int chan)
{
    cmdbuf[0] = 0xFF; // reset
    transfer_status = 0;
    SI_Transfer(chan, cmdbuf, 1, resbuf, 3, callback, SI_TRANS_DELAY);
    while (transfer_status == 0)
        ;
}
void getstatus(int chan)
{
    cmdbuf[0] = 0; // status
    transfer_status = 0;
    SI_Transfer(chan, cmdbuf, 1, resbuf, 3, callback, SI_TRANS_DELAY);
    while (transfer_status == 0)
        ;
}

s8 init_transfer_rom(int chan, char* gba_rom, u32 gba_rom_size)
{
    u32 i = 0;

    cmdbuf = memalign(32, 32);
    resbuf = memalign(32, 32);

    resval = 0;
    SI_GetTypeAsync(chan, acallback);
    
    while (!resval);
    if (!(resval & SI_GBA))
    {
        printf("No GBA\n");
        free(cmdbuf);
        free(resbuf);
        return -1;
    }

    while (1)
    {
        if (resval)
        {
            if (resval == 0x80 || resval & 8)
            {
                resval = 0;
                SI_GetTypeAsync(chan, acallback);
            }
            else if (resval)
            {
                break;
            }
        }
    }

    if (memcmp(gba_rom + 4, logodat, sizeof(logodat)) != 0)
    {
        memcpy(gba_rom + 4, logodat, sizeof(logodat));
    }
    if (gba_rom[0xB2] != 0x96)
    {
        gba_rom[0xB2] = 0x96;
    }
    u8 chk = 0;
    for (i = 0xA0; i < 0xBD; i++)
        chk -= gba_rom[i];
    chk -= 0x19;
    if (gba_rom[0xBD] != chk)
    {
        gba_rom[0xBD] = chk;
    }
    if (*(u32 *)(gba_rom + 0xE4) == 0x0010A0E3 && *(u32 *)(gba_rom + 0xEC) == 0xC010A0E3 &&
        *(u32 *)(gba_rom + 0x100) == 0xFCFFFF1A && *(u32 *)(gba_rom + 0x118) == 0x040050E3 &&
        *(u32 *)(gba_rom + 0x11C) == 0xFBFFFF1A && *(u32 *)(gba_rom + 0x12C) == 0x020050E3 &&
        *(u32 *)(gba_rom + 0x130) == 0xFBFFFF1A && *(u32 *)(gba_rom + 0x140) == 0xFEFFFF1A)
    {
        // jump over joyboot handshake
        *(u32 *)(gba_rom + 0xE0) = 0x170000EA;
    }

    printf("It's dangerous to go alone...\n");
    resbuf[2] = 0;
    int attempts = 0;
    while (!(resbuf[2] & 0x10))
    {
        doreset(chan);
        getstatus(chan);
		if (++attempts > 100)
		{
			printf("Failed to reset GBA\n");
			free(cmdbuf);
			free(resbuf);
			return -2;
		}
    }
    printf("...\n");
    unsigned int sendsize = (((gba_rom_size) + 7) & ~7);
    unsigned int ourkey = calckey(sendsize);
    u32 sessionkeyraw = recv(chan);
    u32 sessionkey = __builtin_bswap32(sessionkeyraw ^ 0x7365646F);
    send(chan, __builtin_bswap32(ourkey));
    unsigned int fcrc = 0x15a0;
    for (i = 0; i < 0xC0; i += 4)
        send(chan, __builtin_bswap32(*(vu32 *)(gba_rom + i)));
    for (i = 0xC0; i < sendsize; i += 4)
    {
        u32 enc = ((gba_rom[i + 3] << 24) | (gba_rom[i + 2] << 16) | (gba_rom[i + 1] << 8) | (gba_rom[i]));
        fcrc = docrc(fcrc, enc);
        sessionkey = (sessionkey * 0x6177614B) + 1;
        enc ^= sessionkey;
        enc ^= ((~(i + (0x20 << 20))) + 1);
        enc ^= 0x20796220;
        send(chan, enc);
    }
    fcrc |= (sendsize << 16);
    sessionkey = (sessionkey * 0x6177614B) + 1;
    fcrc ^= sessionkey;
    fcrc ^= ((~(i + (0x20 << 20))) + 1);
    fcrc ^= 0x20796220;
    send(chan, (u32)fcrc);
    recv(chan);
    printf("Take this!\n");
    VIDEO_WaitVSync();
    VIDEO_WaitVSync();
    //sleep(3);
    
    free(cmdbuf);
    free(resbuf);
    return 1;
}