\
/*
 * Tegra Graphics Init for T186 Architecture Chips
 *
 * T186 host1x compatibility layer for the 5.10 NVIDIA nvhost driver.
 */

#include <linux/slab.h>
#include <linux/io.h>
#include <linux/version.h>

#include "platform.h"
#include "dev.h"
#include "chip_support.h"
#include "t186.h"
#include "hardware_t186.h"
#include "host1x/host1x.h"

static struct host1x_device_info host1x04_info = {
	.nb_channels = T186_NVHOST_NUMCHANNELS,
	.ch_base = 0,
	.ch_limit = T186_NVHOST_NUMCHANNELS,
	.nb_mlocks = NV_HOST1X_NB_MLOCKS,
	.initialize_chip_support = nvhost_init_t186_support,
	.nb_hw_pts = NV_HOST1X_SYNCPT_NB_PTS,
	.nb_pts = NV_HOST1X_SYNCPT_NB_PTS,
	.pts_base = 0,
	.pts_limit = NV_HOST1X_SYNCPT_NB_PTS,
	.nb_syncpt_irqs = 1,
	.syncpt_policy = SYNCPT_PER_CHANNEL_INSTANCE,
	.channel_policy = MAP_CHANNEL_ON_SUBMIT,
	.resources = {
		"vm",
		"hypervisor",
	},
	.nb_resources = 2,
};

struct nvhost_device_data t18_host1x_info = {
	.clocks = {
		{"host1x", 102000000},
		{"actmon", UINT_MAX},
	},
	.can_powergate = false,
	.autosuspend_delay = 50,
	.private_data = &host1x04_info,
	.finalize_poweron = nvhost_host1x_finalize_poweron,
	.prepare_poweroff = nvhost_host1x_prepare_poweroff,
	.isolate_contexts = true,
};

/*
 * The 5.10 nvhost core uses the T194-style host1x implementation.
 * Build it against the T186 host1x5 register definitions above.
 */
#include "host1x/host1x_channel_t194.c"
#include "host1x/host1x_cdma_t194.c"
#include "host1x/host1x_syncpt.c"
#include "host1x/host1x_syncpt_prot_t194.c"
#include "host1x/host1x_intr_t194.c"
#include "host1x/host1x_debug_t194.c"
#include "host1x/host1x_vm_t194.c"

static void t186_set_nvhost_chanops(struct nvhost_channel *ch)
{
	if (ch)
		ch->ops = host1x_channel_ops;
}

static int nvhost_init_t186_channel_support(struct nvhost_master *host,
					    struct nvhost_chip_support *op)
{
	op->nvhost_dev.set_nvhost_chanops = t186_set_nvhost_chanops;
	return 0;
}

static void t186_remove_support(struct nvhost_chip_support *op)
{
	kfree(op->priv);
	op->priv = NULL;
}

int nvhost_init_t186_support(struct nvhost_master *host,
			     struct nvhost_chip_support *op)
{
	int err;

	op->soc_name = "tegra18x";

	err = nvhost_init_t186_channel_support(host, op);
	if (err)
		return err;

	op->cdma = host1x_cdma_ops;
	op->push_buffer = host1x_pushbuffer_ops;
	op->debug = host1x_debug_ops;

	host->sync_aperture = host->aperture;
	op->syncpt = host1x_syncpt_ops;
	op->intr = host1x_intr_ops;
	op->vm = host1x_vm_ops;

	op->syncpt.reset = t194_syncpt_reset;
	op->syncpt.mark_used = t194_syncpt_mark_used;
	op->syncpt.mark_unused = t194_syncpt_mark_unused;
	op->syncpt.mutex_owner = t194_syncpt_mutex_owner;

	op->remove_support = t186_remove_support;

	return 0;
}
