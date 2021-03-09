// SPDX-License-Identifier: GPL-2.0-only
#define pr_fmt(fmt) "ndtest :" fmt

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/genalloc.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/list_sort.h>
#include <linux/libnvdimm.h>
#include <linux/ndctl.h>
#include <nd-core.h>
#include <linux/printk.h>
#include <linux/seq_buf.h>

#include "../watermark.h"
#include "nfit_test.h"
#include "ndtest.h"

enum {
	DIMM_SIZE = SZ_32M,
	LABEL_SIZE = SZ_128K,
	NUM_INSTANCES = 2,
	NUM_DCR = 4,
	NDTEST_MAX_MAPPING = 6,
};

#define NDTEST_SCM_DIMM_CMD_MASK	   \
	((1ul << ND_CMD_GET_CONFIG_SIZE) | \
	 (1ul << ND_CMD_GET_CONFIG_DATA) | \
	 (1ul << ND_CMD_SET_CONFIG_DATA) | \
	 (1ul << ND_CMD_SMART_THRESHOLD) | \
	 (1uL << ND_CMD_SMART)           | \
	 (1ul << ND_CMD_CALL))

#define NFIT_DIMM_HANDLE(node, socket, imc, chan, dimm)			\
	(((node & 0xfff) << 16) | ((socket & 0xf) << 12)		\
	 | ((imc & 0xf) << 8) | ((chan & 0xf) << 4) | (dimm & 0xf))

static DEFINE_SPINLOCK(ndtest_lock);
static struct ndtest_priv *instances[NUM_INSTANCES];
static struct class *ndtest_dimm_class;
static struct gen_pool *ndtest_pool;
static struct workqueue_struct *ndtest_wq;

static const struct nd_papr_pdsm_health health_defaults = {
	.dimm_unarmed = 0,
	.dimm_bad_shutdown = 0,
	.dimm_health = PAPR_PDSM_DIMM_UNHEALTHY,
	.extension_flags = PDSM_DIMM_HEALTH_MEDIA_TEMPERATURE_VALID | PDSM_DIMM_HEALTH_ALARM_VALID |
			   PDSM_DIMM_HEALTH_CTRL_TEMPERATURE_VALID | PDSM_DIMM_HEALTH_SPARES_VALID |
			   PDSM_DIMM_HEALTH_RUN_GAUGE_VALID,
	.dimm_fuel_gauge = 95,
	.media_temperature = 23 * 16,
	.ctrl_temperature = 25 * 16,
	.spares = 75,
	.alarm_flags = ND_PAPR_HEALTH_SPARE_TRIP |
			ND_PAPR_HEALTH_TEMP_TRIP,
};

static struct ndtest_dimm dimm_group1[] = {
	{
		.size = DIMM_SIZE,
		.handle = NFIT_DIMM_HANDLE(0, 0, 0, 0, 0),
		.uuid_str = "1e5c75d2-b618-11ea-9aa3-507b9ddc0f72",
		.physical_id = 0,
		.num_formats = 2,
		.flags = PAPR_PMEM_HEALTH_NON_CRITICAL,
		.extension_flags = health_defaults.extension_flags,
		.dimm_fuel_gauge = health_defaults.dimm_fuel_gauge,
		.media_temperature = health_defaults.media_temperature,
		.ctrl_temperature = health_defaults.ctrl_temperature,
		.spares = health_defaults.spares,
		.alarm_flags = health_defaults.alarm_flags,
		.media_temperature_threshold = 40 * 16,
		.ctrl_temperature_threshold = 30 * 16,
		.spares_threshold = 5,
	},
	{
		.size = DIMM_SIZE,
		.handle = NFIT_DIMM_HANDLE(0, 0, 0, 0, 1),
		.uuid_str = "1c4d43ac-b618-11ea-be80-507b9ddc0f72",
		.physical_id = 1,
		.num_formats = 2,
		.flags = PAPR_PMEM_HEALTH_NON_CRITICAL,
		.extension_flags = health_defaults.extension_flags,
		.dimm_fuel_gauge = health_defaults.dimm_fuel_gauge,
		.media_temperature = health_defaults.media_temperature,
		.ctrl_temperature = health_defaults.ctrl_temperature,
		.spares = health_defaults.spares,
		.alarm_flags = health_defaults.alarm_flags,
		.media_temperature_threshold = 40 * 16,
		.ctrl_temperature_threshold = 30 * 16,
		.spares_threshold = 5,
	},
	{
		.size = DIMM_SIZE,
		.handle = NFIT_DIMM_HANDLE(0, 0, 1, 0, 0),
		.uuid_str = "a9f17ffc-b618-11ea-b36d-507b9ddc0f72",
		.physical_id = 2,
		.num_formats = 2,
		.flags = PAPR_PMEM_HEALTH_NON_CRITICAL,
		.extension_flags = health_defaults.extension_flags,
		.dimm_fuel_gauge = health_defaults.dimm_fuel_gauge,
		.media_temperature = health_defaults.media_temperature,
		.ctrl_temperature = health_defaults.ctrl_temperature,
		.spares = health_defaults.spares,
		.alarm_flags = health_defaults.alarm_flags,
		.media_temperature_threshold = 40 * 16,
		.ctrl_temperature_threshold = 30 * 16,
		.spares_threshold = 5,
	},
	{
		.size = DIMM_SIZE,
		.handle = NFIT_DIMM_HANDLE(0, 0, 1, 0, 1),
		.uuid_str = "b6b83b22-b618-11ea-8aae-507b9ddc0f72",
		.physical_id = 3,
		.num_formats = 2,
		.flags = PAPR_PMEM_HEALTH_NON_CRITICAL,
		.extension_flags = health_defaults.extension_flags,
		.dimm_fuel_gauge = health_defaults.dimm_fuel_gauge,
		.media_temperature = health_defaults.media_temperature,
		.ctrl_temperature = health_defaults.ctrl_temperature,
		.spares = health_defaults.spares,
		.alarm_flags = health_defaults.alarm_flags,
		.media_temperature_threshold = 40 * 16,
		.ctrl_temperature_threshold = 30 * 16,
		.spares_threshold = 5,
	},
	{
		.size = DIMM_SIZE,
		.handle = NFIT_DIMM_HANDLE(0, 1, 0, 0, 0),
		.uuid_str = "bf9baaee-b618-11ea-b181-507b9ddc0f72",
		.physical_id = 4,
		.num_formats = 2,
	},
};

static struct ndtest_dimm dimm_group2[] = {
	{
		.size = DIMM_SIZE,
		.handle = NFIT_DIMM_HANDLE(1, 0, 0, 0, 0),
		.uuid_str = "ca0817e2-b618-11ea-9db3-507b9ddc0f72",
		.physical_id = 0,
		.num_formats = 1,
		.flags = PAPR_PMEM_UNARMED | PAPR_PMEM_EMPTY |
			 PAPR_PMEM_SAVE_FAILED | PAPR_PMEM_SHUTDOWN_DIRTY |
			 PAPR_PMEM_HEALTH_FATAL,
	},
};

static struct ndtest_mapping region0_mapping[] = {
	{
		.dimm = 0,
		.position = 0,
		.start = 0,
		.size = SZ_16M,
	},
	{
		.dimm = 1,
		.position = 1,
		.start = 0,
		.size = SZ_16M,
	}
};

static struct ndtest_mapping region1_mapping[] = {
	{
		.dimm = 0,
		.position = 0,
		.start = SZ_16M,
		.size = SZ_16M,
	},
	{
		.dimm = 1,
		.position = 1,
		.start = SZ_16M,
		.size = SZ_16M,
	},
	{
		.dimm = 2,
		.position = 2,
		.start = SZ_16M,
		.size = SZ_16M,
	},
	{
		.dimm = 3,
		.position = 3,
		.start = SZ_16M,
		.size = SZ_16M,
	},
};

static struct ndtest_mapping region2_mapping[] = {
	{
		.dimm = 0,
		.position = 0,
		.start = 0,
		.size = DIMM_SIZE,
	},
};

static struct ndtest_mapping region3_mapping[] = {
	{
		.dimm = 1,
		.start = 0,
		.size = DIMM_SIZE,
	}
};

static struct ndtest_mapping region4_mapping[] = {
	{
		.dimm = 2,
		.start = 0,
		.size = DIMM_SIZE,
	}
};

static struct ndtest_mapping region5_mapping[] = {
	{
		.dimm = 3,
		.start = 0,
		.size = DIMM_SIZE,
	}
};

static struct ndtest_region bus0_regions[] = {
	{
		.type = ND_DEVICE_NAMESPACE_PMEM,
		.num_mappings = ARRAY_SIZE(region0_mapping),
		.mapping = region0_mapping,
		.size = DIMM_SIZE,
		.range_index = 1,
	},
	{
		.type = ND_DEVICE_NAMESPACE_PMEM,
		.num_mappings = ARRAY_SIZE(region1_mapping),
		.mapping = region1_mapping,
		.size = DIMM_SIZE * 2,
		.range_index = 2,
	},
	{
		.type = ND_DEVICE_NAMESPACE_BLK,
		.num_mappings = ARRAY_SIZE(region2_mapping),
		.mapping = region2_mapping,
		.size = DIMM_SIZE,
		.range_index = 3,
	},
	{
		.type = ND_DEVICE_NAMESPACE_BLK,
		.num_mappings = ARRAY_SIZE(region3_mapping),
		.mapping = region3_mapping,
		.size = DIMM_SIZE,
		.range_index = 4,
	},
	{
		.type = ND_DEVICE_NAMESPACE_BLK,
		.num_mappings = ARRAY_SIZE(region4_mapping),
		.mapping = region4_mapping,
		.size = DIMM_SIZE,
		.range_index = 5,
	},
	{
		.type = ND_DEVICE_NAMESPACE_BLK,
		.num_mappings = ARRAY_SIZE(region5_mapping),
		.mapping = region5_mapping,
		.size = DIMM_SIZE,
		.range_index = 6,
	},
};

static struct ndtest_mapping region6_mapping[] = {
	{
		.dimm = 0,
		.position = 0,
		.start = 0,
		.size = DIMM_SIZE,
	},
};

static struct ndtest_region bus1_regions[] = {
	{
		.type = ND_DEVICE_NAMESPACE_IO,
		.num_mappings = ARRAY_SIZE(region6_mapping),
		.mapping = region6_mapping,
		.size = DIMM_SIZE,
		.range_index = 1,
	},
};

static struct ndtest_config bus_configs[NUM_INSTANCES] = {
	/* bus 1 */
	{
		.dimm_start = 0,
		.dimm_count = ARRAY_SIZE(dimm_group1),
		.dimms = dimm_group1,
		.regions = bus0_regions,
		.num_regions = ARRAY_SIZE(bus0_regions),
	},
	/* bus 2 */
	{
		.dimm_start = ARRAY_SIZE(dimm_group1),
		.dimm_count = ARRAY_SIZE(dimm_group2),
		.dimms = dimm_group2,
		.regions = bus1_regions,
		.num_regions = ARRAY_SIZE(bus1_regions),
	},
};

static inline struct ndtest_priv *to_ndtest_priv(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return container_of(pdev, struct ndtest_priv, pdev);
}

static int ndtest_config_get(struct ndtest_dimm *p, unsigned int buf_len,
			     struct nd_cmd_get_config_data_hdr *hdr)
{
	unsigned int len;

	if ((hdr->in_offset + hdr->in_length) > LABEL_SIZE)
		return -EINVAL;

	hdr->status = 0;
	len = min(hdr->in_length, LABEL_SIZE - hdr->in_offset);
	memcpy(hdr->out_buf, p->label_area + hdr->in_offset, len);

	return buf_len - len;
}

static int ndtest_config_set(struct ndtest_dimm *p, unsigned int buf_len,
			     struct nd_cmd_set_config_hdr *hdr)
{
	unsigned int len;

	if ((hdr->in_offset + hdr->in_length) > LABEL_SIZE)
		return -EINVAL;

	len = min(hdr->in_length, LABEL_SIZE - hdr->in_offset);
	memcpy(p->label_area + hdr->in_offset, hdr->in_buf, len);

	return buf_len - len;
}

static int ndtest_get_config_size(struct ndtest_dimm *dimm, unsigned int buf_len,
				  struct nd_cmd_get_config_size *size)
{
	size->status = 0;
	size->max_xfer = 8;
	size->config_size = dimm->config_size;

	return 0;
}

static int ndtest_pdsm_health(struct ndtest_dimm *dimm,
			union nd_pdsm_payload *payload,
			unsigned int buf_len)
{
	struct nd_papr_pdsm_health *health = &payload->health;

	if (buf_len < sizeof(health))
		return -EINVAL;

	health->extension_flags = 0;
	health->dimm_unarmed = !!(dimm->flags & PAPR_PMEM_UNARMED_MASK);
	health->dimm_bad_shutdown = !!(dimm->flags & PAPR_PMEM_BAD_SHUTDOWN_MASK);
	health->dimm_bad_restore = !!(dimm->flags & PAPR_PMEM_BAD_RESTORE_MASK);
	health->dimm_health = PAPR_PDSM_DIMM_HEALTHY;

	if (dimm->flags & PAPR_PMEM_HEALTH_FATAL)
		health->dimm_health = PAPR_PDSM_DIMM_FATAL;
	else if (dimm->flags & PAPR_PMEM_HEALTH_CRITICAL)
		health->dimm_health = PAPR_PDSM_DIMM_CRITICAL;
	else if (dimm->flags & PAPR_PMEM_HEALTH_UNHEALTHY ||
		 dimm->flags & PAPR_PMEM_HEALTH_NON_CRITICAL)
		health->dimm_health = PAPR_PDSM_DIMM_UNHEALTHY;

	health->extension_flags = 0;
	if (dimm->extension_flags & PDSM_DIMM_HEALTH_RUN_GAUGE_VALID) {
		health->dimm_fuel_gauge = dimm->dimm_fuel_gauge;
		health->extension_flags |= PDSM_DIMM_HEALTH_RUN_GAUGE_VALID;
	}
	if (dimm->extension_flags & PDSM_DIMM_HEALTH_MEDIA_TEMPERATURE_VALID) {
		health->media_temperature = dimm->media_temperature;
		health->extension_flags |= PDSM_DIMM_HEALTH_MEDIA_TEMPERATURE_VALID;
	}
	if (dimm->extension_flags & PDSM_DIMM_HEALTH_CTRL_TEMPERATURE_VALID) {
		health->ctrl_temperature = dimm->ctrl_temperature;
		health->extension_flags |= PDSM_DIMM_HEALTH_CTRL_TEMPERATURE_VALID;
	}
	if (dimm->extension_flags & PDSM_DIMM_HEALTH_SPARES_VALID) {
		health->spares = dimm->spares;
		health->extension_flags |= PDSM_DIMM_HEALTH_SPARES_VALID;
	}
	if (dimm->extension_flags & PDSM_DIMM_HEALTH_ALARM_VALID) {
		health->alarm_flags = dimm->alarm_flags;
		health->extension_flags |= PDSM_DIMM_HEALTH_ALARM_VALID;
	}

	return 0;
}

static void smart_notify(struct ndtest_dimm *dimm)
{
	struct device *bus = dimm->dev->parent;

	if (((dimm->alarm_flags & ND_PAPR_HEALTH_SPARE_TRIP) &&
	      dimm->spares <= dimm->spares_threshold) ||
	    ((dimm->alarm_flags & ND_PAPR_HEALTH_TEMP_TRIP) &&
	      dimm->media_temperature >= dimm->media_temperature_threshold) ||
	    ((dimm->alarm_flags & ND_PAPR_HEALTH_CTEMP_TRIP) &&
	     dimm->ctrl_temperature >= dimm->ctrl_temperature_threshold) ||
	    !(dimm->flags & PAPR_PMEM_HEALTH_NON_CRITICAL) ||
	    (dimm->flags & PAPR_PMEM_BAD_SHUTDOWN_MASK)) {
		device_lock(bus);
		/* send smart notification */
		if (dimm->notify_handle)
			sysfs_notify_dirent(dimm->notify_handle);
		device_unlock(bus);
	}
}

static int ndtest_pdsm_health_inject(struct ndtest_dimm *dimm,
				union nd_pdsm_payload *payload,
				unsigned int buf_len)
{
	struct nd_papr_pdsm_health_inject *inj = &payload->inject;

	if (buf_len < sizeof(inj))
		return -EINVAL;

	if (inj->flags & ND_PAPR_HEALTH_INJECT_MTEMP) {
		if (inj->mtemp_enable)
			dimm->media_temperature = inj->media_temperature;
		else
			dimm->media_temperature = health_defaults.media_temperature;
	}
	if (inj->flags & ND_PAPR_HEALTH_INJECT_SPARE) {
		if (inj->spares_enable)
			dimm->spares = inj->spares;
		else
			dimm->spares = health_defaults.spares;
	}
	if (inj->flags & ND_PAPR_HEALTH_INJECT_FATAL) {
		if (inj->fatal_enable)
			dimm->flags |= PAPR_PMEM_HEALTH_FATAL;
		else
			dimm->flags &= ~PAPR_PMEM_HEALTH_FATAL;
	}
	if (inj->flags & ND_PAPR_HEALTH_INJECT_SHUTDOWN) {
		if (inj->unsafe_shutdown_enable)
			dimm->flags |= PAPR_PMEM_SHUTDOWN_DIRTY;
		else
			dimm->flags &= ~PAPR_PMEM_SHUTDOWN_DIRTY;
	}
	smart_notify(dimm);
	inj->status = 0;

	return 0;
}

static int ndtest_pdsm_health_threshold(struct ndtest_dimm *dimm,
			union nd_pdsm_payload *payload,
			unsigned int buf_len)
{
	struct nd_papr_pdsm_health_threshold *threshold = &payload->threshold;

	if (buf_len < sizeof(threshold))
		return -EINVAL;

	threshold->media_temperature = dimm->media_temperature_threshold;
	threshold->ctrl_temperature = dimm->ctrl_temperature_threshold;
	threshold->spares = dimm->spares_threshold;
	threshold->alarm_control = dimm->alarm_flags;

	return 0;
}

static int ndtest_pdsm_health_set_threshold(struct ndtest_dimm *dimm,
			union nd_pdsm_payload *payload,
			unsigned int buf_len)
{
	struct nd_papr_pdsm_health_threshold *threshold = &payload->threshold;

	if (buf_len < sizeof(threshold))
		return -EINVAL;

	dimm->media_temperature_threshold = threshold->media_temperature;
	dimm->ctrl_temperature_threshold = threshold->ctrl_temperature;
	dimm->spares_threshold = threshold->spares;
	dimm->alarm_flags = threshold->alarm_control;

	smart_notify(dimm);

	return 0;
}

static void ars_complete_all(struct ndtest_priv *p)
{
	int i;

	for (i = 0; i < p->config->num_regions; i++) {
		struct ndtest_region *region = &p->config->regions[i];

		if (region->region)
			nvdimm_region_notify(region->region,
					     NVDIMM_REVALIDATE_POISON);
	}
}

static void ndtest_scrub(struct work_struct *work)
{
	struct ndtest_priv *p = container_of(work, typeof(struct ndtest_priv),
					     dwork.work);
	struct badrange_entry *be;
	int rc, i = 0;

	spin_lock(&p->badrange.lock);
	list_for_each_entry(be, &p->badrange.list, list) {
		rc = nvdimm_bus_add_badrange(p->bus, be->start, be->length);
		if (rc)
			dev_err(&p->pdev.dev, "Failed to process ARS records\n");
		else
			i++;
	}
	spin_unlock(&p->badrange.lock);

	if (i == 0) {
		queue_delayed_work(ndtest_wq, &p->dwork, HZ);
		return;
	}

	ars_complete_all(p);
	p->scrub_count++;

	mutex_lock(&p->ars_lock);
	sysfs_notify_dirent(p->scrub_state);
	clear_bit(ARS_BUSY, &p->scrub_flags);
	clear_bit(ARS_POLL, &p->scrub_flags);
	set_bit(ARS_VALID, &p->scrub_flags);
	mutex_unlock(&p->ars_lock);

}

static int ndtest_scrub_notify(struct ndtest_priv *p)
{
	if (!test_and_set_bit(ARS_BUSY, &p->scrub_flags))
		queue_delayed_work(ndtest_wq, &p->dwork, HZ);

	return 0;
}

static int ndtest_ars_inject(struct ndtest_priv *p,
			     struct nd_cmd_ars_err_inj *inj,
			     unsigned int buf_len)
{
	int rc;

	if (buf_len != sizeof(*inj)) {
		dev_dbg(&p->bus->dev, "buflen: %u, inj size: %lu\n",
			buf_len, sizeof(*inj));
		rc = -EINVAL;
		goto err;
	}

	rc =  badrange_add(&p->badrange, inj->err_inj_spa_range_base,
			   inj->err_inj_spa_range_length);

	if (inj->err_inj_options & (1 << ND_ARS_ERR_INJ_OPT_NOTIFY))
		ndtest_scrub_notify(p);

	inj->status = 0;

	return 0;

err:
	inj->status = NFIT_ARS_INJECT_INVALID;
	return rc;
}

static int ndtest_ars_inject_clear(struct ndtest_priv *p,
				   struct nd_cmd_ars_err_inj_clr *inj,
				   unsigned int buf_len)
{
	int rc;

	if (buf_len != sizeof(*inj)) {
		rc = -EINVAL;
		goto err;
	}

	if (inj->err_inj_clr_spa_range_length <= 0) {
		rc = -EINVAL;
		goto err;
	}

	badrange_forget(&p->badrange, inj->err_inj_clr_spa_range_base,
			inj->err_inj_clr_spa_range_length);

	inj->status = 0;
	return 0;

err:
	inj->status = NFIT_ARS_INJECT_INVALID;
	return rc;
}

static int ndtest_ars_inject_status(struct ndtest_priv *p,
				    struct nd_cmd_ars_err_inj_stat *stat,
				    unsigned int buf_len)
{
	struct badrange_entry *be;
	int max = SZ_4K / sizeof(struct nd_error_stat_query_record);
	int i = 0;

	stat->status = 0;
	spin_lock(&p->badrange.lock);
	list_for_each_entry(be, &p->badrange.list, list) {
		stat->record[i].err_inj_stat_spa_range_base = be->start;
		stat->record[i].err_inj_stat_spa_range_length = be->length;
		i++;
		if (i > max)
			break;
	}
	spin_unlock(&p->badrange.lock);
	stat->inj_err_rec_count = i;

	return 0;
}

static int ndtest_dimm_cmd_call(struct ndtest_dimm *dimm, unsigned int buf_len,
			   void *buf)
{
	struct nd_cmd_pkg *call_pkg = buf;
	unsigned int len = call_pkg->nd_size_in + call_pkg->nd_size_out;
	struct nd_pkg_pdsm *pdsm = (struct nd_pkg_pdsm *) call_pkg->nd_payload;
	union nd_pdsm_payload *payload = &(pdsm->payload);
	unsigned int func = call_pkg->nd_command;

	switch (func) {
	case PAPR_PDSM_HEALTH:
		return ndtest_pdsm_health(dimm, payload, len);
	case PAPR_PDSM_HEALTH_INJECT:
		return ndtest_pdsm_health_inject(dimm, payload, len);
	case PAPR_PDSM_HEALTH_THRESHOLD:
		return ndtest_pdsm_health_threshold(dimm, payload, len);
	case PAPR_PDSM_HEALTH_THRESHOLD_SET:
		return ndtest_pdsm_health_set_threshold(dimm, payload, len);
	}

	return 0;
}

static int ndtest_bus_cmd_call(struct nvdimm_bus_descriptor *nd_desc, void *buf,
			       unsigned int buf_len, int *cmd_rc)
{
	struct nd_cmd_pkg *pkg = buf;
	struct ndtest_priv *p = container_of(nd_desc, struct ndtest_priv,
					     bus_desc);
	void *payload = pkg->nd_payload;
	unsigned int func = pkg->nd_command;
	unsigned int len = pkg->nd_size_in + pkg->nd_size_out;

	switch (func) {
	case PAPR_PDSM_INJECT_SET:
		return ndtest_ars_inject(p, payload, len);
	case PAPR_PDSM_INJECT_CLEAR:
		return ndtest_ars_inject_clear(p, payload, len);
	case PAPR_PDSM_INJECT_GET:
		return ndtest_ars_inject_status(p, payload, len);
	}

	return -ENOTTY;
}

static int ndtest_cmd_ars_cap(struct ndtest_priv *p, struct nd_cmd_ars_cap *cmd,
			      unsigned int buf_len)
{
	int ars_recs;

	if (buf_len < sizeof(*cmd))
		return -EINVAL;

	/* for testing, only store up to n records that fit within a page */
	ars_recs = SZ_4K / sizeof(struct nd_ars_record);

	cmd->max_ars_out = sizeof(struct nd_cmd_ars_status)
		+ ars_recs * sizeof(struct nd_ars_record);
	cmd->status = (ND_ARS_PERSISTENT | ND_ARS_VOLATILE) << 16;
	cmd->clear_err_unit = 256;
	p->max_ars = cmd->max_ars_out;

	return 0;
}

static void post_ars_status(struct ars_state *state,
			    struct badrange *badrange, u64 addr, u64 len)
{
	struct nd_cmd_ars_status *status;
	struct nd_ars_record *record;
	struct badrange_entry *be;
	u64 end = addr + len - 1;
	int i = 0;

	state->deadline = jiffies + 1*HZ;
	status = state->ars_status;
	status->status = 0;
	status->address = addr;
	status->length = len;
	status->type = ND_ARS_PERSISTENT;

	spin_lock(&badrange->lock);
	list_for_each_entry(be, &badrange->list, list) {
		u64 be_end = be->start + be->length - 1;
		u64 rstart, rend;

		/* skip entries outside the range */
		if (be_end < addr || be->start > end)
			continue;

		rstart = (be->start < addr) ? addr : be->start;
		rend = (be_end < end) ? be_end : end;
		record = &status->records[i];
		record->handle = 0;
		record->err_address = rstart;
		record->length = rend - rstart + 1;
		i++;
	}
	spin_unlock(&badrange->lock);

	status->num_records = i;
	status->out_length = sizeof(struct nd_cmd_ars_status)
		+ i * sizeof(struct nd_ars_record);
}

#define NFIT_ARS_STATUS_BUSY (1 << 16)
#define NFIT_ARS_START_BUSY 6

static int ndtest_cmd_ars_start(struct ndtest_priv *priv,
				struct nd_cmd_ars_start *start,
				unsigned int buf_len, int *cmd_rc)
{
	if (buf_len < sizeof(*start))
		return -EINVAL;

	spin_lock(&priv->state.lock);
	if (time_before(jiffies, priv->state.deadline)) {
		start->status = NFIT_ARS_START_BUSY;
		*cmd_rc = -EBUSY;
	} else {
		start->status = 0;
		start->scrub_time = 1;
		post_ars_status(&priv->state, &priv->badrange,
				start->address, start->length);
		*cmd_rc = 0;
	}
	spin_unlock(&priv->state.lock);

	return 0;
}

static int ndtest_cmd_ars_status(struct ndtest_priv *priv,
				 struct nd_cmd_ars_status *status,
				 unsigned int buf_len, int *cmd_rc)
{
	if (buf_len < priv->state.ars_status->out_length)
		return -EINVAL;

	spin_lock(&priv->state.lock);
	if (time_before(jiffies, priv->state.deadline)) {
		memset(status, 0, buf_len);
		status->status = NFIT_ARS_STATUS_BUSY;
		status->out_length = sizeof(*status);
		*cmd_rc = -EBUSY;
	} else {
		memcpy(status, priv->state.ars_status,
		       priv->state.ars_status->out_length);
		*cmd_rc = 0;
	}
	spin_unlock(&priv->state.lock);

	return 0;
}

static int ndtest_cmd_clear_error(struct ndtest_priv *priv,
				     struct nd_cmd_clear_error *inj,
				     unsigned int buf_len, int *cmd_rc)
{
	const u64 mask = 255;

	if (buf_len < sizeof(*inj))
		return -EINVAL;

	if ((inj->address & mask) || (inj->length & mask))
		return -EINVAL;

	badrange_forget(&priv->badrange, inj->address, inj->length);
	inj->status = 0;
	inj->cleared = inj->length;
	*cmd_rc = 0;

	return 0;
}

static int ndtest_ctl(struct nvdimm_bus_descriptor *nd_desc,
		      struct nvdimm *nvdimm, unsigned int cmd, void *buf,
		      unsigned int buf_len, int *cmd_rc)
{
	struct ndtest_dimm *dimm;
	int _cmd_rc;

	if (!cmd_rc)
		cmd_rc = &_cmd_rc;

	*cmd_rc = 0;

	if (!nvdimm) {
		struct ndtest_priv *priv;

		if (!nd_desc)
			return -ENOTTY;

		priv = container_of(nd_desc, struct ndtest_priv, bus_desc);
		switch (cmd) {
		case ND_CMD_CALL:
			return ndtest_bus_cmd_call(nd_desc, buf, buf_len,
						   cmd_rc);
		case ND_CMD_ARS_CAP:
			return ndtest_cmd_ars_cap(priv, buf, buf_len);
		case ND_CMD_ARS_START:
			return ndtest_cmd_ars_start(priv, buf, buf_len, cmd_rc);
		case ND_CMD_ARS_STATUS:
			return ndtest_cmd_ars_status(priv, buf, buf_len,
						     cmd_rc);
		case ND_CMD_CLEAR_ERROR:
			return ndtest_cmd_clear_error(priv, buf, buf_len,
						      cmd_rc);
		default:
			dev_dbg(&priv->pdev.dev, "Invalid command\n");
			return -ENOTTY;
		}
	}

	dimm = nvdimm_provider_data(nvdimm);
	if (!dimm)
		return -EINVAL;

	switch (cmd) {
	case ND_CMD_GET_CONFIG_SIZE:
		*cmd_rc = ndtest_get_config_size(dimm, buf_len, buf);
		break;
	case ND_CMD_GET_CONFIG_DATA:
		*cmd_rc = ndtest_config_get(dimm, buf_len, buf);
		break;
	case ND_CMD_SET_CONFIG_DATA:
		*cmd_rc = ndtest_config_set(dimm, buf_len, buf);
		break;
	case ND_CMD_CALL:
		*cmd_rc = ndtest_dimm_cmd_call(dimm, buf_len, buf);
		break;
	default:
		return -EINVAL;
	}

	/* Failures for a DIMM can be injected using fail_cmd and
	 * fail_cmd_code, see the device attributes below
	 */
	if ((1 << cmd) & dimm->fail_cmd)
		return dimm->fail_cmd_code ? dimm->fail_cmd_code : -EIO;

	return 0;
}

static int ndtest_blk_do_io(struct nd_blk_region *ndbr, resource_size_t dpa,
		void *iobuf, u64 len, int rw)
{
	struct ndtest_dimm *dimm = ndbr->blk_provider_data;
	struct ndtest_blk_mmio *mmio = dimm->mmio;
	struct nd_region *nd_region = &ndbr->nd_region;
	unsigned int lane;

	if (!mmio)
		return -ENOMEM;

	lane = nd_region_acquire_lane(nd_region);
	if (rw)
		memcpy(mmio->base + dpa, iobuf, len);
	else {
		memcpy(iobuf, mmio->base + dpa, len);
		arch_invalidate_pmem(mmio->base + dpa, len);
	}

	nd_region_release_lane(nd_region, lane);

	return 0;
}

static int ndtest_blk_region_enable(struct nvdimm_bus *nvdimm_bus,
				    struct device *dev)
{
	struct nd_blk_region *ndbr = to_nd_blk_region(dev);
	struct nvdimm *nvdimm;
	struct ndtest_dimm *dimm;
	struct ndtest_blk_mmio *mmio;

	nvdimm = nd_blk_region_to_dimm(ndbr);
	dimm = nvdimm_provider_data(nvdimm);

	nd_blk_region_set_provider_data(ndbr, dimm);
	dimm->blk_region = to_nd_region(dev);

	mmio = devm_kzalloc(dev, sizeof(struct ndtest_blk_mmio), GFP_KERNEL);
	if (!mmio)
		return -ENOMEM;

	mmio->base = (void __iomem *) devm_nvdimm_memremap(
		dev, dimm->address, 12, nd_blk_memremap_flags(ndbr));
	if (!mmio->base) {
		dev_err(dev, "%s failed to map blk dimm\n", nvdimm_name(nvdimm));
		return -ENOMEM;
	}
	mmio->size = dimm->size;
	mmio->base_offset = 0;

	dimm->mmio = mmio;

	return 0;
}

static struct nfit_test_resource *ndtest_resource_lookup(resource_size_t addr)
{
	int i;

	for (i = 0; i < NUM_INSTANCES; i++) {
		struct nfit_test_resource *n, *nfit_res = NULL;
		struct ndtest_priv *t = instances[i];

		if (!t)
			continue;
		spin_lock(&ndtest_lock);
		list_for_each_entry(n, &t->resources, list) {
			if (addr >= n->res.start && (addr < n->res.start
						+ resource_size(&n->res))) {
				nfit_res = n;
				break;
			} else if (addr >= (unsigned long) n->buf
					&& (addr < (unsigned long) n->buf
						+ resource_size(&n->res))) {
				nfit_res = n;
				break;
			}
		}
		spin_unlock(&ndtest_lock);
		if (nfit_res)
			return nfit_res;
	}

	pr_warn("Failed to get resource\n");

	return NULL;
}

static void ndtest_release_resource(void *data)
{
	struct nfit_test_resource *res  = data;

	spin_lock(&ndtest_lock);
	list_del(&res->list);
	spin_unlock(&ndtest_lock);

	if (resource_size(&res->res) >= DIMM_SIZE)
		gen_pool_free(ndtest_pool, res->res.start,
				resource_size(&res->res));
	vfree(res->buf);
	kfree(res);
}

static void *ndtest_alloc_resource(struct ndtest_priv *p, size_t size,
				   dma_addr_t *dma)
{
	dma_addr_t __dma;
	void *buf;
	struct nfit_test_resource *res;
	struct genpool_data_align data = {
		.align = SZ_128M,
	};

	res = kzalloc(sizeof(*res), GFP_KERNEL);
	if (!res)
		return NULL;

	buf = vmalloc(size);
	if (!buf)
		return NULL;

	if (size >= DIMM_SIZE)
		__dma = gen_pool_alloc_algo(ndtest_pool, size,
					    gen_pool_first_fit_align, &data);
	else
		__dma = (unsigned long) buf;

	if (!__dma)
		goto buf_err;

	INIT_LIST_HEAD(&res->list);
	res->dev = &p->pdev.dev;
	res->buf = buf;
	res->res.start = __dma;
	res->res.end = __dma + size - 1;
	res->res.name = "NFIT";
	spin_lock_init(&res->lock);
	INIT_LIST_HEAD(&res->requests);
	spin_lock(&ndtest_lock);
	list_add(&res->list, &p->resources);
	spin_unlock(&ndtest_lock);

	if (dma)
		*dma = __dma;

	if (!devm_add_action(&p->pdev.dev, ndtest_release_resource, res))
		return res->buf;

buf_err:
	if (__dma && size >= DIMM_SIZE)
		gen_pool_free(ndtest_pool, __dma, size);
	if (buf)
		vfree(buf);
	kfree(res);

	return NULL;
}

static ssize_t range_index_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct nd_region *nd_region = to_nd_region(dev);
	struct ndtest_region *region = nd_region_provider_data(nd_region);

	return sprintf(buf, "%d\n", region->range_index);
}
static DEVICE_ATTR_RO(range_index);

static struct attribute *ndtest_region_attributes[] = {
	&dev_attr_range_index.attr,
	NULL,
};

static const struct attribute_group ndtest_region_attribute_group = {
	.name = "papr",
	.attrs = ndtest_region_attributes,
};

static const struct attribute_group *ndtest_region_attribute_groups[] = {
	&ndtest_region_attribute_group,
	NULL,
};

static int ndtest_create_region(struct ndtest_priv *p,
				struct ndtest_region *region)
{
	struct nd_mapping_desc mappings[NDTEST_MAX_MAPPING];
	struct nd_blk_region_desc ndbr_desc;
	struct nd_interleave_set *nd_set;
	struct nd_region_desc *ndr_desc;
	struct resource res;
	int i, ndimm = region->mapping[0].dimm;
	u64 uuid[2];

	memset(&res, 0, sizeof(res));
	memset(&mappings, 0, sizeof(mappings));
	memset(&ndbr_desc, 0, sizeof(ndbr_desc));
	ndr_desc = &ndbr_desc.ndr_desc;

	if (!ndtest_alloc_resource(p, region->size, &res.start))
		return -ENOMEM;

	res.end = res.start + region->size - 1;
	ndr_desc->mapping = mappings;
	ndr_desc->res = &res;
	ndr_desc->provider_data = region;
	ndr_desc->attr_groups = ndtest_region_attribute_groups;

	if (uuid_parse(p->config->dimms[ndimm].uuid_str, (uuid_t *)uuid)) {
		pr_err("failed to parse UUID\n");
		return -ENXIO;
	}

	nd_set = devm_kzalloc(&p->pdev.dev, sizeof(*nd_set), GFP_KERNEL);
	if (!nd_set)
		return -ENOMEM;

	nd_set->cookie1 = cpu_to_le64(uuid[0]);
	nd_set->cookie2 = cpu_to_le64(uuid[1]);
	nd_set->altcookie = nd_set->cookie1;
	ndr_desc->nd_set = nd_set;

	if (region->type == ND_DEVICE_NAMESPACE_BLK) {
		mappings[0].start = 0;
		mappings[0].size = DIMM_SIZE;
		mappings[0].nvdimm = p->config->dimms[ndimm].nvdimm;

		ndr_desc->mapping = &mappings[0];
		ndr_desc->num_mappings = 1;
		ndr_desc->num_lanes = 1;
		ndbr_desc.enable = ndtest_blk_region_enable;
		ndbr_desc.do_io = ndtest_blk_do_io;
		region->region = nvdimm_blk_region_create(p->bus, ndr_desc);

		goto done;
	}

	for (i = 0; i < region->num_mappings; i++) {
		ndimm = region->mapping[i].dimm;
		mappings[i].start = region->mapping[i].start;
		mappings[i].size = region->mapping[i].size;
		mappings[i].position = region->mapping[i].position;
		mappings[i].nvdimm = p->config->dimms[ndimm].nvdimm;
	}

	ndr_desc->num_mappings = region->num_mappings;
	region->region = nvdimm_pmem_region_create(p->bus, ndr_desc);

done:
	if (!region->region) {
		dev_err(&p->pdev.dev, "Error registering region %pR\n",
			ndr_desc->res);
		return -ENXIO;
	}

	return 0;
}

static int ndtest_init_regions(struct ndtest_priv *p)
{
	int i, ret = 0;

	for (i = 0; i < p->config->num_regions; i++) {
		ret = ndtest_create_region(p, &p->config->regions[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static void put_dimms(void *data)
{
	struct ndtest_priv *p = data;
	int i;

	for (i = 0; i < p->config->dimm_count; i++)
		if (p->config->dimms[i].dev) {
			device_unregister(p->config->dimms[i].dev);
			p->config->dimms[i].dev = NULL;
		}
}

static ssize_t handle_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct ndtest_dimm *dimm = dev_get_drvdata(dev);

	return sprintf(buf, "%#x\n", dimm->handle);
}
static DEVICE_ATTR_RO(handle);

static ssize_t fail_cmd_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct ndtest_dimm *dimm = dev_get_drvdata(dev);

	return sprintf(buf, "%#x\n", dimm->fail_cmd);
}

static ssize_t fail_cmd_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct ndtest_dimm *dimm = dev_get_drvdata(dev);
	unsigned long val;
	ssize_t rc;

	rc = kstrtol(buf, 0, &val);
	if (rc)
		return rc;

	dimm->fail_cmd = val;

	return size;
}
static DEVICE_ATTR_RW(fail_cmd);

static ssize_t fail_cmd_code_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct ndtest_dimm *dimm = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", dimm->fail_cmd_code);
}

static ssize_t fail_cmd_code_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct ndtest_dimm *dimm = dev_get_drvdata(dev);
	unsigned long val;
	ssize_t rc;

	rc = kstrtol(buf, 0, &val);
	if (rc)
		return rc;

	dimm->fail_cmd_code = val;
	return size;
}
static DEVICE_ATTR_RW(fail_cmd_code);

static struct attribute *dimm_attributes[] = {
	&dev_attr_handle.attr,
	&dev_attr_fail_cmd.attr,
	&dev_attr_fail_cmd_code.attr,
	NULL,
};

static struct attribute_group dimm_attribute_group = {
	.attrs = dimm_attributes,
};

static const struct attribute_group *dimm_attribute_groups[] = {
	&dimm_attribute_group,
	NULL,
};

static ssize_t phys_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);

	return sprintf(buf, "%#x\n", dimm->physical_id);
}
static DEVICE_ATTR_RO(phys_id);

static ssize_t vendor_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x1234567\n");
}
static DEVICE_ATTR_RO(vendor);

static ssize_t id_show(struct device *dev,
		       struct device_attribute *attr, char *buf)
{
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);

	return sprintf(buf, "%04x-%02x-%04x-%08x", 0xabcd,
		       0xa, 2016, ~(dimm->handle));
}
static DEVICE_ATTR_RO(id);

static ssize_t nvdimm_handle_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);

	return sprintf(buf, "%#x\n", dimm->handle);
}

static struct device_attribute dev_attr_nvdimm_show_handle =  {
	.attr	= { .name = "handle", .mode = 0444 },
	.show	= nvdimm_handle_show,
};

static ssize_t subsystem_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%04x\n", 0);
}
static DEVICE_ATTR_RO(subsystem_vendor);

static ssize_t dirty_shutdown_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", 42);
}
static DEVICE_ATTR_RO(dirty_shutdown);

static ssize_t formats_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);

	return sprintf(buf, "%d\n", dimm->num_formats);
}
static DEVICE_ATTR_RO(formats);

static ssize_t format_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);

	if (dimm->num_formats > 1)
		return sprintf(buf, "0x201\n");

	return sprintf(buf, "0x101\n");
}
static DEVICE_ATTR_RO(format);

static ssize_t format1_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	return sprintf(buf, "0x301\n");
}
static DEVICE_ATTR_RO(format1);

static umode_t ndtest_nvdimm_attr_visible(struct kobject *kobj,
					struct attribute *a, int n)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);

	if (a == &dev_attr_format1.attr && dimm->num_formats <= 1)
		return 0;

	return a->mode;
}

static ssize_t flags_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct nvdimm *nvdimm = to_nvdimm(dev);
	struct ndtest_dimm *dimm = nvdimm_provider_data(nvdimm);
	struct seq_buf s;
	u64 flags;

	flags = dimm->flags;

	seq_buf_init(&s, buf, PAGE_SIZE);
	if (flags & PAPR_PMEM_UNARMED_MASK)
		seq_buf_printf(&s, "not_armed ");

	if (flags & PAPR_PMEM_BAD_SHUTDOWN_MASK)
		seq_buf_printf(&s, "flush_fail ");

	if (flags & PAPR_PMEM_BAD_RESTORE_MASK)
		seq_buf_printf(&s, "restore_fail ");

	if (flags & PAPR_PMEM_SAVE_MASK)
		seq_buf_printf(&s, "save_fail ");

	if (flags & PAPR_PMEM_SMART_EVENT_MASK)
		seq_buf_printf(&s, "smart_notify ");


	if (seq_buf_used(&s))
		seq_buf_printf(&s, "\n");

	return seq_buf_used(&s);
}
static DEVICE_ATTR_RO(flags);


#define PAPR_PMEM_DIMM_CMD_MASK				\
	 ((1U << PAPR_PDSM_HEALTH)			\
	 | (1U << PAPR_PDSM_HEALTH_INJECT)		\
	 | (1U << PAPR_PDSM_HEALTH_THRESHOLD)		\
	 | (1U << PAPR_PDSM_HEALTH_THRESHOLD_SET))


static ssize_t dsm_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%#x\n", PAPR_PMEM_DIMM_CMD_MASK);
}
static DEVICE_ATTR_RO(dsm_mask);

static struct attribute *ndtest_nvdimm_attributes[] = {
	&dev_attr_nvdimm_show_handle.attr,
	&dev_attr_vendor.attr,
	&dev_attr_id.attr,
	&dev_attr_phys_id.attr,
	&dev_attr_subsystem_vendor.attr,
	&dev_attr_dirty_shutdown.attr,
	&dev_attr_formats.attr,
	&dev_attr_format.attr,
	&dev_attr_format1.attr,
	&dev_attr_flags.attr,
	&dev_attr_dsm_mask.attr,
	NULL,
};

static const struct attribute_group ndtest_nvdimm_attribute_group = {
	.name = "papr",
	.attrs = ndtest_nvdimm_attributes,
	.is_visible = ndtest_nvdimm_attr_visible,
};

static const struct attribute_group *ndtest_nvdimm_attribute_groups[] = {
	&ndtest_nvdimm_attribute_group,
	NULL,
};

static int ndtest_dimm_register(struct ndtest_priv *priv,
				struct ndtest_dimm *dimm, int id)
{
	struct device *dev = &priv->pdev.dev;
	unsigned long dimm_flags = dimm->flags;
	struct kernfs_node *papr_kernfs;

	if (dimm->num_formats > 1) {
		set_bit(NDD_ALIASING, &dimm_flags);
		set_bit(NDD_LABELING, &dimm_flags);
	}

	if (dimm->flags & PAPR_PMEM_UNARMED_MASK)
		set_bit(NDD_UNARMED, &dimm_flags);

	dimm->nvdimm = nvdimm_create(priv->bus, dimm,
				    ndtest_nvdimm_attribute_groups, dimm_flags,
				    NDTEST_SCM_DIMM_CMD_MASK, 0, NULL);
	if (!dimm->nvdimm) {
		dev_err(dev, "Error creating DIMM object for %pOF\n", priv->dn);
		return -ENXIO;
	}

	dimm->dev = device_create_with_groups(ndtest_dimm_class,
					     &priv->pdev.dev,
					     0, dimm, dimm_attribute_groups,
					     "test_dimm%d", id);
	if (!dimm->dev) {
		pr_err("Could not create dimm device attributes\n");
		return -ENOMEM;
	}

	nd_synchronize();

	papr_kernfs = sysfs_get_dirent(nvdimm_kobj(dimm->nvdimm)->sd, "papr");
	if (!papr_kernfs) {
		pr_err("Could not initialize the notifier handle\n");
		return 0;
	}

	dimm->notify_handle = sysfs_get_dirent(papr_kernfs, "flags");
	sysfs_put(papr_kernfs);
	if (!dimm->notify_handle) {
		pr_err("Could not initialize the notifier handle\n");
		return 0;
	}
	return 0;
}

static int ndtest_nvdimm_init(struct ndtest_priv *p)
{
	struct ndtest_dimm *d;
	void *res;
	int i, id;

	for (i = 0; i < p->config->dimm_count; i++) {
		d = &p->config->dimms[i];
		d->id = id = p->config->dimm_start + i;
		res = ndtest_alloc_resource(p, LABEL_SIZE, NULL);
		if (!res)
			return -ENOMEM;

		d->label_area = res;
		sprintf(d->label_area, "label%d", id);
		d->config_size = LABEL_SIZE;

		if (!ndtest_alloc_resource(p, d->size,
					   &p->dimm_dma[id]))
			return -ENOMEM;

		if (!ndtest_alloc_resource(p, LABEL_SIZE,
					   &p->label_dma[id]))
			return -ENOMEM;

		if (!ndtest_alloc_resource(p, LABEL_SIZE,
					   &p->dcr_dma[id]))
			return -ENOMEM;

		d->address = p->dimm_dma[id];

		ndtest_dimm_register(p, d, id);
	}

	return 0;
}

static ssize_t compatible_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "nvdimm_test");
}
static DEVICE_ATTR_RO(compatible);

static struct attribute *of_node_attributes[] = {
	&dev_attr_compatible.attr,
	NULL
};

static const struct attribute_group of_node_attribute_group = {
	.name = "of_node",
	.attrs = of_node_attributes,
};

#define PAPR_PMEM_BUS_DSM_MASK				\
	((1U << PAPR_PDSM_INJECT_SET)			\
	 | (1U << PAPR_PDSM_INJECT_GET)			\
	 | (1U << PAPR_PDSM_INJECT_CLEAR))

static ssize_t bus_dsm_mask_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%#x\n", PAPR_PMEM_BUS_DSM_MASK);
}
static struct device_attribute dev_attr_bus_dsm_mask = {
	.attr	= { .name = "dsm_mask", .mode = 0444 },
	.show	= bus_dsm_mask_show,
};

static ssize_t scrub_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct nvdimm_bus_descriptor *nd_desc;
	struct ndtest_priv *p;
	ssize_t rc = -ENXIO;
	bool busy = 0;

	device_lock(dev);
	nd_desc = dev_get_drvdata(dev);
	if (!nd_desc) {
		device_unlock(dev);
		return rc;
	}

	p = container_of(nd_desc, struct ndtest_priv, bus_desc);

	mutex_lock(&p->ars_lock);
	busy = test_bit(ARS_BUSY, &p->scrub_flags) &&
		!test_bit(ARS_CANCEL, &p->scrub_flags);
	rc = sprintf(buf, "%d%s", p->scrub_count, busy ? "+\n" : "\n");
	if (busy && capable(CAP_SYS_RAWIO) &&
	    !test_and_set_bit(ARS_POLL, &p->scrub_flags))
		mod_delayed_work(ndtest_wq, &p->dwork, HZ);

	mutex_unlock(&p->ars_lock);

	device_unlock(dev);
	return rc;
}

static ssize_t scrub_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t size)
{
	struct nvdimm_bus_descriptor *nd_desc;
	struct ndtest_priv *p;
	ssize_t rc = 0;
	long val;

	rc = kstrtol(buf, 0, &val);
	if (rc)
		return rc;
	if (val != 1)
		return -EINVAL;
	device_lock(dev);
	nd_desc = dev_get_drvdata(dev);
	if (nd_desc) {
		p = container_of(nd_desc, struct ndtest_priv, bus_desc);

		ndtest_scrub_notify(p);
	}
	device_unlock(dev);

	return size;
}
static DEVICE_ATTR_RW(scrub);

static struct attribute *ndtest_attributes[] = {
	&dev_attr_bus_dsm_mask.attr,
	&dev_attr_scrub.attr,
	NULL,
};

static const struct attribute_group ndtest_attribute_group = {
	.name = "papr",
	.attrs = ndtest_attributes,
};

static const struct attribute_group *ndtest_attribute_groups[] = {
	&of_node_attribute_group,
	&ndtest_attribute_group,
	NULL,
};

#define PAPR_PMEM_BUS_CMD_MASK				   \
	(1UL << ND_CMD_ARS_CAP				   \
	 | 1UL << ND_CMD_ARS_START			   \
	 | 1UL << ND_CMD_ARS_STATUS			   \
	 | 1UL << ND_CMD_CLEAR_ERROR			   \
	 | 1UL << ND_CMD_CALL)

static int ndtest_bus_register(struct ndtest_priv *p)
{
	p->config = &bus_configs[p->pdev.id];

	p->bus_desc.ndctl = ndtest_ctl;
	p->bus_desc.module = THIS_MODULE;
	p->bus_desc.provider_name = NULL;
	p->bus_desc.cmd_mask = PAPR_PMEM_BUS_CMD_MASK;
	p->bus_desc.attr_groups = ndtest_attribute_groups;
	p->bus_desc.bus_family_mask = NVDIMM_FAMILY_PAPR;

	set_bit(NVDIMM_FAMILY_PAPR, &p->bus_desc.dimm_family_mask);

	p->bus = nvdimm_bus_register(&p->pdev.dev, &p->bus_desc);
	if (!p->bus) {
		dev_err(&p->pdev.dev, "Error creating nvdimm bus %pOF\n", p->dn);
		return -ENOMEM;
	}

	return 0;
}

static int ndtest_remove(struct platform_device *pdev)
{
	struct ndtest_priv *p = to_ndtest_priv(&pdev->dev);

	nvdimm_bus_unregister(p->bus);
	return 0;
}

static int ndtest_init_ars(struct ndtest_priv *p)
{
	struct kernfs_node *papr_node;
	struct device *bus_dev;

	p->state.ars_status = devm_kzalloc(
		&p->pdev.dev, sizeof(struct nd_cmd_ars_status) + SZ_4K,
		GFP_KERNEL);
	if (!p->state.ars_status)
		return -ENOMEM;

	bus_dev = to_nvdimm_bus_dev(p->bus);
	papr_node = sysfs_get_dirent(bus_dev->kobj.sd, "papr");
	if (!papr_node) {
		dev_err(&p->pdev.dev, "sysfs_get_dirent 'papr' failed\n");
		return -ENOENT;
	}

	p->scrub_state = sysfs_get_dirent(papr_node, "scrub");
	if (!p->scrub_state) {
		dev_err(&p->pdev.dev, "sysfs_get_dirent 'scrub' failed\n");
		return -ENOENT;
	}

	return 0;
}

static int ndtest_probe(struct platform_device *pdev)
{
	struct ndtest_priv *p;
	int rc;

	p = to_ndtest_priv(&pdev->dev);
	if (ndtest_bus_register(p))
		return -ENOMEM;

	p->dcr_dma = devm_kcalloc(&p->pdev.dev, NUM_DCR,
				 sizeof(dma_addr_t), GFP_KERNEL);
	p->label_dma = devm_kcalloc(&p->pdev.dev, NUM_DCR,
				   sizeof(dma_addr_t), GFP_KERNEL);
	p->dimm_dma = devm_kcalloc(&p->pdev.dev, NUM_DCR,
				  sizeof(dma_addr_t), GFP_KERNEL);

	rc = ndtest_nvdimm_init(p);
	if (rc)
		goto err;

	rc = ndtest_init_regions(p);
	if (rc)
		goto err;

	rc = ndtest_init_ars(p);
	if (rc)
		goto err;

	rc = devm_add_action_or_reset(&pdev->dev, put_dimms, p);
	if (rc)
		goto err;

	platform_set_drvdata(pdev, p);

	return 0;

err:
	pr_err("%s:%d Failed nvdimm init\n", __func__, __LINE__);
	return rc;
}

static const struct platform_device_id ndtest_id[] = {
	{ KBUILD_MODNAME },
	{ },
};

static struct platform_driver ndtest_driver = {
	.probe = ndtest_probe,
	.remove = ndtest_remove,
	.driver = {
		.name = KBUILD_MODNAME,
	},
	.id_table = ndtest_id,
};

static void ndtest_release(struct device *dev)
{
	struct ndtest_priv *p = to_ndtest_priv(dev);

	kfree(p);
}

static void cleanup_devices(void)
{
	int i;

	for (i = 0; i < NUM_INSTANCES; i++)
		if (instances[i])
			platform_device_unregister(&instances[i]->pdev);

	nfit_test_teardown();

	if (ndtest_pool)
		gen_pool_destroy(ndtest_pool);

	destroy_workqueue(ndtest_wq);

	if (ndtest_dimm_class)
		class_destroy(ndtest_dimm_class);
}

static __init int ndtest_init(void)
{
	int rc, i;

	pmem_test();
	libnvdimm_test();
	device_dax_test();
	dax_pmem_test();
	dax_pmem_core_test();
#ifdef CONFIG_DEV_DAX_PMEM_COMPAT
	dax_pmem_compat_test();
#endif

	nfit_test_setup(ndtest_resource_lookup, NULL);

	ndtest_wq = create_singlethread_workqueue("nfit");
	if (!ndtest_wq)
		return -ENOMEM;

	ndtest_dimm_class = class_create(THIS_MODULE, "nfit_test_dimm");
	if (IS_ERR(ndtest_dimm_class)) {
		rc = PTR_ERR(ndtest_dimm_class);
		goto err_register;
	}

	ndtest_pool = gen_pool_create(ilog2(SZ_4M), NUMA_NO_NODE);
	if (!ndtest_pool) {
		rc = -ENOMEM;
		goto err_register;
	}

	if (gen_pool_add(ndtest_pool, SZ_4G, SZ_4G, NUMA_NO_NODE)) {
		rc = -ENOMEM;
		goto err_register;
	}

	/* Each instance can be taken as a bus, which can have multiple dimms */
	for (i = 0; i < NUM_INSTANCES; i++) {
		struct ndtest_priv *priv;
		struct platform_device *pdev;

		priv = kzalloc(sizeof(*priv), GFP_KERNEL);
		if (!priv) {
			rc = -ENOMEM;
			goto err_register;
		}

		INIT_LIST_HEAD(&priv->resources);
		badrange_init(&priv->badrange);
		pdev = &priv->pdev;
		pdev->name = KBUILD_MODNAME;
		pdev->id = i;
		pdev->dev.release = ndtest_release;
		rc = platform_device_register(pdev);
		if (rc) {
			put_device(&pdev->dev);
			goto err_register;
		}
		get_device(&pdev->dev);

		instances[i] = priv;

		/* Everything about ARS here */
		INIT_DELAYED_WORK(&priv->dwork, ndtest_scrub);
		mutex_init(&priv->ars_lock);
		spin_lock_init(&priv->state.lock);
	}

	rc = platform_driver_register(&ndtest_driver);
	if (rc)
		goto err_register;

	return 0;

err_register:
	pr_err("Error registering platform device\n");
	cleanup_devices();

	return rc;
}

static __exit void ndtest_exit(void)
{
	flush_workqueue(ndtest_wq);
	cleanup_devices();
	platform_driver_unregister(&ndtest_driver);
}

module_init(ndtest_init);
module_exit(ndtest_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("IBM Corporation");
