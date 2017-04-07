/*
 * NVMeDirect Device Driver
 *
 * Copyright (c) 2016 Computer Systems Laboratory, Sungkyunkwan University.
 * http://csl.skku.edu
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the therm and condotions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#ifndef _NVMED_MODULE_H
#define _NVMED_MODULE_H

#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/blkdev.h>

#include "./nvme.h"

#include "../include/nvmed.h"
#include "../include/nvme_hdr.h"

#define NVMED_ERR(string, args...) printk(KERN_ERR string, ##args)
#define NVMED_INFO(string, args...) printk(KERN_INFO string, ##args)
#define NVMED_DEBUG(string, args...) printk(KERN_DEBUG string, ##args)

#define PCI_CLASS_NVME	0x010802

#define KERNEL_VERSION_CODE	KERNEL_VERSION(KERNEL_VERSION_MAJOR, \
										KERNEL_VERSION_MINOR, 0)

#define	DEV_ENTRY_TO_DEVICE(dev_entry) &dev_entry->pdev->dev
#define NS_ENTRY_TO_DEV(ns_entry) ns_entry->dev_entry->dev

#define DEV_TO_ADMINQ(dev) dev->ctrl.admin_q
#define DEV_TO_INSTANCE(dev) dev->ctrl.instance
#define DEV_TO_HWSECTORS(dev) dev->ctrl.max_hw_sectors
#define DEV_TO_VWC(dev) dev->ctrl.vwc
#define DEV_TO_NS_LIST(dev) dev->ctrl.namespaces
#define DEV_TO_STRIPESIZE(dev) (dev->ctrl.max_hw_sectors << 8)

int (*nvmed_set_features_fn)(struct nvme_ctrl *dev, unsigned fid, unsigned dword11,
	dma_addr_t dma_addr, u32 *result) = NULL;

int (*nvmed_submit_cmd)(struct request_queue *q, struct nvme_command *cmd,
		void *buf, unsigned bufflen) = NULL;

#define TRUE	1
#define FALSE	0

unsigned char admin_timeout = 60;
#define ADMIN_TIMEOUT		(admin_timeout * HZ)

struct proc_dir_entry *NVMED_PROC_ROOT;

static LIST_HEAD(nvmed_dev_list);

struct async_cmd_info {
	struct kthread_work work;
	struct kthread_worker *worker;
	struct request *req;
	u32 result;
	int status;
	void *ctx;
};

struct nvme_queue {
	struct device *q_dmadev;
	struct nvme_dev *dev;
	spinlock_t q_lock;
	struct nvme_command *sq_cmds;
	struct nvme_command __iomem *sq_cmds_io;
	volatile struct nvme_completion *cqes;
	dma_addr_t sq_dma_addr;
	dma_addr_t cq_dma_addr;
	u32 __iomem *q_db;
	u16 q_depth;
	u16 sq_head;
	u16 sq_tail;
	u16 cq_head;
	u16 qid;
	u8 cq_phase;
	u8 cqe_seen;
};

typedef struct nvmed_user_quota_entry {
	kuid_t uid;
	unsigned int queue_max;
	unsigned int queue_used;

	struct list_head list;
} NVMED_USER_QUOTA_ENTRY;

typedef struct nvmed_dev_entry {
	struct nvme_dev *dev;
	struct pci_dev *pdev;

	spinlock_t ctrl_lock;

	unsigned int num_user_queue;
	DECLARE_BITMAP(queue_bmap, 256);

	struct list_head list;
	struct list_head ns_list;
} NVMED_DEV_ENTRY;

typedef struct nvmed_ns_entry {
	NVMED_DEV_ENTRY *dev_entry;

	struct nvme_ns *ns;
	
	struct proc_dir_entry *ns_proc_root;
	struct proc_dir_entry *proc_admin;
	struct proc_dir_entry *proc_sysfs_link;

	struct list_head list;

	struct list_head queue_list;
	struct list_head user_list;
	
	int partno;

	sector_t start_sect;
	sector_t nr_sects;
} NVMED_NS_ENTRY;

typedef struct nvmed_queue_entry {
	NVMED_NS_ENTRY *ns_entry;
	
	struct proc_dir_entry *queue_proc_root;
	struct proc_dir_entry *proc_sq;
	struct proc_dir_entry *proc_cq;
	struct proc_dir_entry *proc_db;

	struct nvme_queue* nvmeq;

	kuid_t owner;

	struct list_head list;
} NVMED_QUEUE_ENTRY;

#endif
