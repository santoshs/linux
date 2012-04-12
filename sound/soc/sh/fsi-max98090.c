#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <sound/sh_fsi.h>
//#include <../sound/soc/codecs/max98090.h>
#include <sound/soundpath/soundpath.h>

extern struct snd_soc_dai_driver max98090_dai_driver[];

static struct snd_soc_dai_link fsi_dai_link[] = {
	{
		.name		= "fsia max98090",
		.stream_name	= "max98090 hifi",
		.cpu_dai_name	= "fsia-dai",
		.codec_dai_name	= "max98090-hifi",
		.platform_name	= "sh_fsi2.0",
		.codec_name	= "max98090.6-0010",
	},
	{
		.name		= "fsib max98090",
		.stream_name	= "max98090 fm",
		.cpu_dai_name	= "fsib-dai",
		.codec_dai_name	= "max98090-fm",
		.platform_name	= "sh_fsi2.1",
		.codec_name	= "max98090.6-0010",
	},
};

static struct snd_soc_card fsi_soc_card = {
	.name = "FSI",
	.dai_link = fsi_dai_link,
	.num_links = ARRAY_SIZE(fsi_dai_link),
};

static struct platform_device *fsi_snd_device;

static int __init fsi_max98090_init(void)
{
	int ret = -ENOMEM;
printk(KERN_INFO "@@ %s\n", __func__);

	ret = sndp_init(fsi_soc_dai, max98090_dai_driver, &fsi_soc_platform);
	if(ret)
		goto out;

	fsi_snd_device = platform_device_alloc("soc-audio", -1);
	if(!fsi_snd_device)
		goto out;

	platform_set_drvdata(fsi_snd_device, &fsi_soc_card);
	ret = platform_device_add(fsi_snd_device);

	if(ret)
		platform_device_put(fsi_snd_device);

out:
printk(KERN_INFO "@@ %s [%d]\n", __func__, ret);
	return ret;

}

static void __exit fsi_max98090_exit(void)
{
//	sndp_exit();
	platform_device_unregister(fsi_snd_device);
}

module_init(fsi_max98090_init);
module_exit(fsi_max98090_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic SH4 FSI-MAX98090 sound card");
MODULE_AUTHOR("");




