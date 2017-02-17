/*
 * InfiniBand Transport Layer
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

#include <linux/slab.h>
#include <rdma/ibtrs.h>

/*
 * Return an IU  to the free pool
 */
inline void ibtrs_iu_put(struct list_head *head, struct ibtrs_iu *iu)
{
	list_add(&iu->list, head);
}

/*
 * Get an IU from the free pool, need lock to protect list
 */
struct ibtrs_iu *ibtrs_iu_get(struct list_head *head)
{
	struct ibtrs_iu *iu;

	if (list_empty(head))
		return NULL;

	iu = list_first_entry(head, struct ibtrs_iu, list);
	list_del(&iu->list);
	return iu;
}

struct ibtrs_iu *ibtrs_iu_alloc(u32 tag, size_t size, gfp_t gfp_mask,
				struct ib_device *dma_dev,
				enum dma_data_direction direction, bool is_msg)
{
	struct ibtrs_iu *iu;

	iu = kmalloc(sizeof(*iu), gfp_mask);
	if (!iu)
		return NULL;

	iu->buf = kzalloc(size, gfp_mask);
	if (!iu->buf)
		goto err1;

	iu->dma_addr = ib_dma_map_single(dma_dev, iu->buf, size, direction);
	if (ib_dma_mapping_error(dma_dev, iu->dma_addr))
		goto err2;

	iu->size      = size;
	iu->direction = direction;
	iu->tag       = tag;
	iu->is_msg     = is_msg;
	return iu;

err2:
	kfree(iu->buf);
err1:
	kfree(iu);
	return NULL;
}

void ibtrs_iu_free(struct ibtrs_iu *iu, enum dma_data_direction dir,
		   struct ib_device *ib_dev)
{
	if (WARN_ON(!iu))
		return;

	ib_dma_unmap_single(ib_dev, iu->dma_addr, iu->size, dir);
	kfree(iu->buf);
	kfree(iu);
}
