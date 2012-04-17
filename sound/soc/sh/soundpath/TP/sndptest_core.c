#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/scuw_extern.h>
#include <sound/soundpath/fsi_extern.h>
#include <sound/soundpath/clkgen_extern.h>
#include <sound/soundpath/max98090_extern.h>
#include "sndptest_core.h"
#ifndef TEST
#define TEST
#endif

void sound_rec_path_set(char *buf, unsigned int size)
{
	int ret = 0;
#ifdef TEST
	printk(KERN_WARNING "sound_rec_path_set(%d, [%s])\n", size,  buf);
#endif
	// Audio IC API call
	ret = max98090_set_device(MAX98090_DEV_CAPTURE_MIC);
	if (0 != ret)
		printk(KERN_WARNING "MAXIM ERROR\n");
	// soundpath init, fsi_hdmi.c ni syori wo ire tara hazusu
	sndp_path_test_sndp_init();
	// Pm_runtime get
	sndp_path_test_pm_runtime_get_sync();
	// FSI start
	ret = fsi_rec_test_start_a(buf, size);
	if (0 != ret)
		return;
	// CLKGEN start
	clkgen_rec_test_start_a();

#ifdef NO_INTURRUPT
	// Temp process
	fsi_test_fifo_read_a();
#endif // NO_INTURRUPT
}

void sound_rec_path_release(void)
{
	int size = 0;
#ifdef TEST
	printk(KERN_WARNING "sound_rec_path_release()\n");
#endif
	// FSI stop and get record size
	size = fsi_rec_test_stop_a();
	record_sound_callback(size);//dummy
	// CLKGEN stop
	clkgen_rec_test_stop_a();
	// Pm_runtime put
	sndp_path_test_pm_runtime_put_sync();
	// Audio IC API call
	
}

void sound_play_path_set(char *buf, unsigned int size)
{
	int ret = 0;
#ifdef TEST
	printk(KERN_WARNING "sound_play_path_set(%d,[%s])\n", size, buf);
#endif
	// Audio IC API call
	ret = max98090_set_device(MAX98090_DEV_PLAYBACK_HEADPHONES);
	if (0 != ret)
		printk(KERN_WARNING "MAXIM ERROR\n");
	// soundpath init, fsi_hdmi.c ni syori wo ire tara hazusu
	sndp_path_test_sndp_init();
	// Pm_runtime get
	sndp_path_test_pm_runtime_get_sync();
	// FSI start
	ret = fsi_play_test_start_a(buf, size);
	if (0 != ret)
		return;
	// CLKGEN start
	clkgen_play_test_start_a();

#ifdef NO_INTURRUPT
	// Temp process
	fsi_test_fifo_write_a();
#endif // NO_INTURRUPT
}

void sound_play_path_release(void)
{
#ifdef TEST
	printk(KERN_WARNING "sound_play_path_release()\n");
#endif
	// FSI stop
	fsi_play_test_stop_a();
	// CLKGEN stop
	clkgen_play_test_stop_a();
	// Pm_runtime put
	sndp_path_test_pm_runtime_put_sync();
}

void sound_loopback_audioic_start(void)
{
#ifdef TEST
	printk(KERN_WARNING "soudn_loopback_audioic_start()\n");
#endif
	// Audio IC API call
	
}

void sound_loopback_audioic_stop(void)
{
#ifdef TEST
	printk(KERN_WARNING "sound_loopback_audioic_stop()\n");
#endif
	// Audio IC API call
	
}

void sound_loopback_spuv_start(void)
{
	int ret;
	int dev = MAX98090_DEV_PLAYBACK_HEADPHONES;
#ifdef TEST
	printk(KERN_WARNING "sound_loopback_spuv_start()\n");
#endif
	// Audio IC API call
	dev |= MAX98090_DEV_CAPTURE_MIC;
	ret = max98090_set_device(dev);
	if (0 != ret)
		printk(KERN_WARNING "MAXIM ERROR\n");
	// soundpath init, fsi_hdmi.c ni syori wo ire tara hazusu
	sndp_path_test_sndp_init();
	// Pm_runtime get
	sndp_path_test_pm_runtime_get_sync();
	// SCUW start
	scuw_voice_test_start_a();
	// FSI start
	ret = fsi_voice_test_start_a();
	if (0 != ret) {
		scuw_voice_test_stop_a();
		return;
	}
	// CLKGEN start
	clkgen_voice_test_start_a();
}

void sound_loopback_spuv_stop(void)
{
#ifdef TEST
	printk(KERN_WARNING "sound_loopback_spuv_stop()\n");
#endif
	// FSI stop
	scuw_voice_test_stop_a();
	// FSI stop
	fsi_voice_test_stop_a();
	// CLKGEN stop
	clkgen_voice_test_stop_a();
	// Pm_runtime put
	sndp_path_test_pm_runtime_put_sync();
}

