\
#ifndef NVHOST_T186_H
#define NVHOST_T186_H

#define T186_NVHOST_NUMCHANNELS 63

struct nvhost_device_data;
struct nvhost_master;
struct nvhost_chip_support;

extern struct nvhost_device_data t18_host1x_info;

extern int nvhost_init_t186_support(struct nvhost_master *host,
				    struct nvhost_chip_support *op);

#endif
