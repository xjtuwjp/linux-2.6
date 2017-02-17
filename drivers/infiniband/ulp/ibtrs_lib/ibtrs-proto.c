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

#include <linux/errno.h>
#include <linux/printk.h>
#include <rdma/ibtrs.h>
#include <rdma/ibtrs_log.h>

static int
ibtrs_validate_msg_sess_open_resp(const struct ibtrs_msg_sess_open_resp *msg)
{
	static const int min_bufs = 1;

	if (unlikely(msg->hdr.tsize !=
				IBTRS_MSG_SESS_OPEN_RESP_LEN(msg->cnt))) {
		ERR_NP("Session open resp msg received with unexpected length"
		       " %dB instead of %luB\n", msg->hdr.tsize,
		       IBTRS_MSG_SESS_OPEN_RESP_LEN(msg->cnt));

		return -EINVAL;
	}

	if (msg->max_inflight_msg < min_bufs) {
		ERR_NP("Sess Open msg received with invalid max_inflight_msg %d"
		       " expected >= %d\n", msg->max_inflight_msg, min_bufs);
		return -EINVAL;
	}

	if (unlikely(msg->cnt != msg->max_inflight_msg)) {
		ERR_NP("Session open msg received with invalid cnt %d"
		       " expected %d (queue_depth)\n", msg->cnt,
		       msg->max_inflight_msg);
		return -EINVAL;
	}

	if (msg->ver != IBTRS_VERSION) {
		WRN_NP("Sess open resp version mismatch: client version %d,"
		       " server version: %d\n", IBTRS_VERSION, msg->ver);
	}

	return 0;
}

static int
ibtrs_validate_msg_user(const struct ibtrs_msg_user *msg)
{
	/* keep as place holder */
	return 0;
}

static int
ibtrs_validate_msg_rdma_write(const struct ibtrs_msg_rdma_write *msg,
			      u16 queue_depth)
{
	if (unlikely(msg->hdr.tsize <= sizeof(*msg))) {
		ERR_NP("RDMA-Write msg received with invalid length %d"
		       " expected > %lu\n", msg->hdr.tsize, sizeof(*msg));
		return -EINVAL;
	}

	return 0;
}

static int
ibtrs_validate_msg_req_rdma_write(const struct ibtrs_msg_req_rdma_write *msg,
				  u16 queue_depth)
{
	if (unlikely(msg->hdr.tsize <= sizeof(*msg))) {
		ERR_NP("Request-RDMA-Write msg request received with invalid"
		       " length %d expected > %lu\n", msg->hdr.tsize,
		       sizeof(*msg));
		return -EINVAL;
	}

	return 0;
}

static int
ibtrs_validate_msg_con_open(const struct ibtrs_msg_con_open *msg)
{
	if (unlikely(msg->hdr.tsize != sizeof(*msg))) {
		ERR_NP("Con Open msg received with invalid length: %d"
		       " expected %lu\n", msg->hdr.tsize, sizeof(*msg));
		return -EINVAL;
	}

	return 0;
}

static int
ibtrs_validate_msg_sess_open(const struct ibtrs_msg_sess_open *msg)
{
	if (msg->hdr.tsize != sizeof(*msg)) {
		ERR_NP("Sess open msg received with invalid length: %d"
		       " expected %lu\n", msg->hdr.tsize, sizeof(*msg));
		return -EPROTONOSUPPORT;
	}

	if (msg->ver != IBTRS_VERSION) {
		WRN_NP("Sess open msg version mismatch: client version %d,"
		       " server version: %d\n", msg->ver, IBTRS_VERSION);
	}

	return 0;
}

static int ibtrs_validate_msg_sess_info(const struct ibtrs_msg_sess_info *msg)
{
	if (msg->hdr.tsize != sizeof(*msg)) {
		ERR_NP("Error message received with invalid length: %d,"
		       " expected %lu\n", msg->hdr.tsize, sizeof(*msg));
		return -EPROTONOSUPPORT;
	}

	return 0;
}

static int ibtrs_validate_msg_error(const struct ibtrs_msg_error *msg)
{
	if (msg->hdr.tsize != sizeof(*msg)) {
		ERR_NP("Error message received with invalid length: %d,"
		       " expected %lu\n", msg->hdr.tsize, sizeof(*msg));
		return -EPROTONOSUPPORT;
	}

	return 0;
}

int ibtrs_validate_message(u16 queue_depth, const void *data)
{
	const struct ibtrs_msg_hdr *hdr = data;

	switch (hdr->type) {
	case IBTRS_MSG_RDMA_WRITE: {
		const struct ibtrs_msg_rdma_write *msg = data;

		return ibtrs_validate_msg_rdma_write(msg, queue_depth);
	}
	case IBTRS_MSG_REQ_RDMA_WRITE: {
		const struct ibtrs_msg_req_rdma_write *req = data;

		return ibtrs_validate_msg_req_rdma_write(req, queue_depth);
	}
	case IBTRS_MSG_SESS_OPEN_RESP: {
		const struct ibtrs_msg_sess_open_resp *msg = data;

		return ibtrs_validate_msg_sess_open_resp(msg);
	}
	case IBTRS_MSG_SESS_INFO: {
		const struct ibtrs_msg_sess_info *msg = data;

		return ibtrs_validate_msg_sess_info(msg);
	}
	case IBTRS_MSG_USER: {
		const struct ibtrs_msg_user *msg = data;

		return ibtrs_validate_msg_user(msg);
	}
	case IBTRS_MSG_CON_OPEN: {
		const struct ibtrs_msg_con_open *msg = data;

		return ibtrs_validate_msg_con_open(msg);
	}
	case IBTRS_MSG_SESS_OPEN: {
		const struct ibtrs_msg_sess_open *msg = data;

		return ibtrs_validate_msg_sess_open(msg);
	}
	case IBTRS_MSG_ERROR: {
		const struct ibtrs_msg_error *msg = data;

		return ibtrs_validate_msg_error(msg);
	}
	default:
		ERR_NP("Received IBTRS message with unknown type\n");
		return -EINVAL;
	}
	return 0;
}

void fill_ibtrs_msg_sess_open(struct ibtrs_msg_sess_open *msg, u8 con_cnt,
			      const uuid_le *uuid)
{
	msg->hdr.type		= IBTRS_MSG_SESS_OPEN;
	msg->hdr.tsize		= sizeof(*msg);
	msg->ver		= IBTRS_VERSION;
	msg->con_cnt		= con_cnt;

	memcpy(msg->uuid, uuid->b, IBTRS_UUID_SIZE);
}

void fill_ibtrs_msg_con_open(struct ibtrs_msg_con_open *msg,
			     const uuid_le *uuid)
{
	msg->hdr.type		= IBTRS_MSG_CON_OPEN;
	msg->hdr.tsize		= sizeof(*msg);
	memcpy(msg->uuid, uuid->b, IBTRS_UUID_SIZE);
}

void fill_ibtrs_msg_sess_info(struct ibtrs_msg_sess_info *msg,
			      const char *hostname) {
	msg->hdr.type		= IBTRS_MSG_SESS_INFO;
	msg->hdr.tsize		= sizeof(*msg);
	memcpy(msg->hostname, hostname, sizeof(msg->hostname));
}
