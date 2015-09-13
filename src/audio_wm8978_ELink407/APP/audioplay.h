#ifndef __AUDIOPLAY_H
#define __AUDIOPLAY_H
#include "sys.h"
#include "ff.h"
#include "wavplay.h"
///////////////////////////////////////////////////////////////////// 	
     

						  
#define I2S_TX_DMA_BUF_SIZE    8192		//定义TX DMA 数组大小(播放192Kbps@24bit的时候,需要设置8192大才不会卡)


//音乐播放控制器
typedef __packed struct
{  
	//2个I2S解码的BUF
	u8 *i2sbuf1;
	u8 *i2sbuf2; 
	u8 *tbuf;				//零时数组,仅在24bit解码的时候需要用到
	FIL *file;				//音频文件指针
	
	u8 status;				//bit0:0,暂停播放;1,继续播放
							//bit1:0,结束播放;1,开启播放 
}__audiodev; 
extern __audiodev audiodev;	//音乐播放控制器


void wav_i2s_dma_callback(void);

void audio_start(void);
void audio_stop(void);
u16 audio_get_tnum(u8 *path);
void audio_index_show(u16 index,u16 total);
void audio_msg_show(u32 totsec,u32 cursec,u32 bitrate);
void audio_play(void);
u8 audio_play_song(u8* fname);

 
#endif












