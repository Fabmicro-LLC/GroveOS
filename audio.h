#ifndef ___AUDIO_H___
#define ___AUDIO_H___

#define FRAME_SIZE_SAMPLES 		(160*4)
#define FRAME_SIZE_BYTES 		(FRAME_SIZE_SAMPLES*2)

#define DAC_BUF_SIZE 			(FRAME_SIZE_BYTES*2)

extern char dac_buf[DAC_BUF_SIZE];
extern char* play_buf;
extern int play_buf_len;
extern int play_buf_offset;
extern int play_buf_empty; 
extern int play_dma_ht_counter;
extern int play_dma_tc_counter;
extern int error_play_buf_underrun;


void play_start(unsigned char *Buffer, int Len);
void play_stop();
void play_init();
void play_tc();
void play_ht();


void enable_audio_amp(int enable);

#endif
