#ifndef _SNDPTEST_CORE_H_
#define _SNDPTEST_CORE_H_

void sound_rec_path_set(char *buf, unsigned int size);
void sound_rec_path_release(void);
void sound_play_path_set(char *buf, unsigned int size);
void sound_play_path_release(void);
void sound_loopback_audioic_start(void);
void sound_loopback_audioic_stop(void);
void sound_loopback_spuv_start(void);
void sound_loopback_spuv_stop(void);
void record_sound_callback(unsigned int size);

#endif /* _SNDPTEST_CORE_H_ */