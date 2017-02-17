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

#include <rdma/ibtrs.h>
#include <rdma/ibtrs_log.h>

inline void ibtrs_heartbeat_set_send_ts(struct ibtrs_heartbeat *h)
{
	struct timespec ts = CURRENT_TIME;

	atomic64_set(&h->send_ts_ms, timespec_to_ms(&ts));
}

inline void ibtrs_set_last_heartbeat(struct ibtrs_heartbeat *h)
{
	struct timespec ts = CURRENT_TIME;

	atomic64_set(&h->recv_ts_ms, timespec_to_ms(&ts));
}

inline u64 ibtrs_heartbeat_send_ts_diff_ms(const struct ibtrs_heartbeat *h)
{
	return timediff_cur_ms(atomic64_read(&h->send_ts_ms));
}

inline u64 ibtrs_recv_ts_ms_diff_ms(const struct ibtrs_heartbeat *h)
{
	return timediff_cur_ms(atomic64_read(&h->recv_ts_ms));
}

void ibtrs_set_heartbeat_timeout(struct ibtrs_heartbeat *h, u32 timeout_ms)
{
	h->timeout_ms = timeout_ms;
	h->warn_timeout_ms = (timeout_ms >> 1) + (timeout_ms >> 2);
}

void ibtrs_heartbeat_warn(const struct ibtrs_heartbeat *h)
{
	u64 diff = ibtrs_recv_ts_ms_diff_ms(h);

	DEB("last heartbeat message from %s was received %lu, %llums"
	    " ago\n", ibtrs_prefix(h), atomic64_read(&h->recv_ts_ms), diff);

	if (diff >= h->warn_timeout_ms)
		WRN(h, "Last Heartbeat message received %llums ago,"
		       " timeout: %ums\n", diff, h->timeout_ms);
}

bool ibtrs_heartbeat_timeout_is_expired(const struct ibtrs_heartbeat *h)
{
	u64 diff;

	if (h->timeout_ms == 0)
		return false;

	diff = ibtrs_recv_ts_ms_diff_ms(h);

	DEB("last heartbeat message from %s received %lu, %llums ago\n",
	    ibtrs_prefix(h), atomic64_read(&h->recv_ts_ms), diff);

	if (diff >= h->timeout_ms) {
		ERR(h, "Heartbeat timeout expired, no heartbeat received "
		       "for %llums, timeout: %ums\n", diff,
		       h->timeout_ms);
		return true;
	}

	return false;
}
