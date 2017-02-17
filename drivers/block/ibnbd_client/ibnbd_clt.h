/*
 * InfiniBand Network Block Driver
 *
 * Copyright (c) 2014 - 2017 ProfitBricks GmbH. All rights reserved.
 * Authors: Fabian Holler < mail@fholler.de>
 *          Jack Wang <jinpu.wang@profitbricks.com>
 *   	    Kleber Souza <kleber.souza@profitbricks.com>
 * 	    Danil Kipnis <danil.kipnis@profitbricks.com>
 *   	    Roman Pen <roman.penyaev@profitbricks.com>
 *          Milind Dumbare <Milind.dumbare@gmail.com>
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 */

#ifndef _IBNBD_CLT_H
#define _IBNBD_CLT_H
#include <linux/blkdev.h>
#include <linux/wait.h>			/* for wait_queue_head_t */
#include <linux/in.h>			/* for sockaddr_in */
#include <linux/inet.h>			/* for sockaddr_in */
#include <linux/blk-mq.h>
#include "ibnbd_clt_log.h"
#include "../ibnbd_inc/ibnbd.h"
#include "../ibnbd_inc/ibnbd-proto.h"	/* ibnbd protocol messages */
#include <rdma/ibtrs_clt.h>	/* for ibtrs api */
#include <rdma/ibtrs.h>

#define IP_PREFIX "ip:"
#define IP_PREFIX_LEN strlen(IP_PREFIX)
#define GID_PREFIX "gid:"
#define GID_PREFIX_LEN strlen(GID_PREFIX)

#define BMAX_SEGMENTS 31
#define RECONNECT_DELAY 30
#define MAX_RECONNECTS -1

enum ibnbd_dev_state {
	DEV_STATE_INIT,
	DEV_STATE_INIT_CLOSED,
	DEV_STATE_CLOSED,
	DEV_STATE_UNMAPPED,
	DEV_STATE_OPEN
};

enum ibnbd_queue_mode {
	BLK_MQ,
	BLK_RQ
};

struct ibnbd_iu {
	struct request		*rq;
	struct ibtrs_tag	*tag;
	struct ibnbd_dev	*dev;
	struct ibnbd_msg_io	msg;
	int			errno;
	struct scatterlist	sglist[BMAX_SEGMENTS];
};

struct ibnbd_cpu_qlist {
	struct list_head	requeue_list;
	spinlock_t		requeue_lock;
	unsigned int		cpu;
};

enum sess_state {
	SESS_STATE_READY,
	SESS_STATE_DISCONNECTED,
	SESS_STATE_DESTROYED,
};

struct ibnbd_session {
	struct list_head        list;
	struct ibtrs_session    *sess;
	struct ibnbd_cpu_qlist	__percpu
				*cpu_queues;
	DECLARE_BITMAP(cpu_queues_bm, NR_CPUS);
	int	__percpu	*cpu_rr; /* per-cpu var for CPU round-robin */
	atomic_t		busy;
	int			queue_depth;
	u32			max_io_size;
	struct blk_mq_tag_set	tag_set;
	struct mutex		lock; /* protects state and devs_list */
	struct list_head        devs_list; /* list of struct ibnbd_dev */
	struct kref		refcount;
	struct sockaddr_storage addr;
	char			str_addr[IBTRS_ADDRLEN];
	char			hostname[MAXHOSTNAMELEN];
	enum sess_state		state;
	u8			ver; /* protocol version */
	struct completion	*sess_info_compl;
};

struct ibnbd_work {
	struct work_struct	work;
	struct ibnbd_session	*sess;
};

/**
 * Submission queues.
 */
struct ibnbd_queue {
	struct list_head	requeue_list;
	unsigned long		in_list;
	struct ibnbd_dev	*dev;
	struct blk_mq_hw_ctx	*hctx;
};

struct ibnbd_dev {
	struct list_head        g_list;
	struct ibnbd_session	*sess;
	struct request_queue	*queue;
	struct ibnbd_queue	*hw_queues;
	struct delayed_work	rq_delay_work;
	u32			device_id;
	u32			clt_device_id;
	struct completion	*open_compl;	/* completion for open msg */
	int			open_errno;
	struct mutex		lock;
	enum ibnbd_dev_state	dev_state;
	enum ibnbd_queue_mode	queue_mode;
	enum ibnbd_io_mode	io_mode; /* user requested */
	enum ibnbd_io_mode	remote_io_mode; /* server really used */
	/* local Idr index - used to track minor number allocations. */
	char			pathname[NAME_MAX];
	enum ibnbd_access_mode	access_mode;
	bool			read_only;
	bool			rotational;
	u32			max_hw_sectors;
	u32			max_write_same_sectors;
	u32			max_discard_sectors;
	u32			discard_zeroes_data;
	u32			discard_granularity;
	u32			discard_alignment;
	u16			secure_discard;
	u16			physical_block_size;
	u16			logical_block_size;
	u16			max_segments;
	size_t			nsectors;
	u64			size;		/* device size in bytes */
	struct list_head        list;
	struct gendisk		*gd;
	struct kobject		kobj;
	char			blk_symlink_name[NAME_MAX];
	struct completion	*close_compl;
	atomic_t		refcount;
};

static inline const char *ibnbd_queue_mode_str(enum ibnbd_queue_mode mode)
{
	switch (mode) {
	case BLK_RQ:
		return "rq";
	case BLK_MQ:
		return "mq";
	default:
		return "unknown";
	}
}

int ibnbd_close_device(struct ibnbd_dev *dev, bool force);
struct ibnbd_session *ibnbd_create_session(const struct sockaddr_storage *addr);
struct ibnbd_session *ibnbd_clt_find_sess(const struct sockaddr_storage *addr);
void ibnbd_clt_sess_release(struct kref *ref);
struct ibnbd_dev *ibnbd_client_add_device(struct ibnbd_session *sess,
					  const char *pathname,
					  enum ibnbd_access_mode access_mode,
					  enum ibnbd_queue_mode queue_mode,
					  enum ibnbd_io_mode io_mode);
void ibnbd_destroy_gen_disk(struct ibnbd_dev *dev);
int ibnbd_addr_to_str(const struct sockaddr_storage *addr,
		      char *buf, size_t len);
bool ibnbd_clt_dev_is_open(struct ibnbd_dev *dev);
bool ibnbd_clt_dev_is_mapped(const char *pathname);
int open_remote_device(struct ibnbd_dev *dev);

const char *ibnbd_clt_get_io_mode(const struct ibnbd_dev *dev);

#define ERR_DEVS(sess, fmt, ...)	\
({	struct ibnbd_dev *dev;		\
					\
	mutex_lock(&sess->lock);	\
	list_for_each_entry(dev, &sess->devs_list, list) \
		pr_err("ibnbd L%d <%s@%s> ERR:" fmt, \
			__LINE__, dev->pathname, dev->sess->str_addr,\
			##__VA_ARGS__); \
	mutex_unlock(&sess->lock);	\
})

#define INFO_DEVS(sess, fmt, ...)	\
({	struct ibnbd_dev *dev;		\
					\
	mutex_lock(&sess->lock);	\
	list_for_each_entry(dev, &sess->devs_list, list) \
		pr_info("ibnbd <%s@%s> ERR:" fmt, \
			dev->pathname, dev->sess->str_addr,\
			##__VA_ARGS__);	\
	mutex_unlock(&sess->lock);	\
})
#endif /* _IBNBD_CLT_H */
