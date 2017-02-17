/*
 * InfiniBand Transport Layer
 *
 * Copyright (c) 2014 - 2017 ProfitBricks GmbH. All rights reserved.
 * Authors: Fabian Holler <mail@fholler.de>
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

#ifndef __IBTRS_LOG_H__
#define __IBTRS_LOG_H__
#include "ibtrs.h"

#define DEB(fmt, ...) pr_debug("ibtrs L%d " fmt, __LINE__, ##__VA_ARGS__)
#define DEB_RL(fmt, ...) pr_debug_ratelimited("ibtrs L%d " fmt, \
					      __LINE__, ##__VA_ARGS__)
static inline void ibtrs_deb_msg_hdr(const char *prep,
				     const struct ibtrs_msg_hdr *hdr)
{
	DEB("%sibtrs msg hdr:\n"
	    "\ttype: %d\n"
	    "\ttsize: %d\n", prep, hdr->type, hdr->tsize);
}

#define ERR_NP(fmt, ...) pr_err("ibtrs L%d ERR: " fmt, \
				__LINE__, ##__VA_ARGS__)

#define WRN_NP(fmt, ...) pr_warn("ibtrs L%d WARN: " fmt, \
				__LINE__, ##__VA_ARGS__)
#define INFO_NP(fmt, ...)  pr_info("ibtrs: " fmt, ##__VA_ARGS__)

#define INFO_NP_RL(fmt, ...) pr_info_ratelimited("ibtrs: " fmt, ##__VA_ARGS__)

#define ibtrs_prefix(sess) ((sess->hostname[0] != '\0') ? sess->hostname : \
							  sess->addr)

#define ERR(sess, fmt, ...) pr_err("ibtrs L%d <%s> ERR: " fmt, \
				__LINE__, ibtrs_prefix(sess), ##__VA_ARGS__)
#define ERR_RL(sess, fmt, ...) pr_err_ratelimited("ibtrs L%d <%s> ERR: " fmt, \
				__LINE__, ibtrs_prefix(sess), ##__VA_ARGS__)

#define WRN(sess, fmt, ...) pr_warn("ibtrs L%d <%s> WARN: " fmt, \
				__LINE__, ibtrs_prefix(sess), ##__VA_ARGS__)
#define WRN_RL(sess, fmt, ...) pr_warn_ratelimited("ibtrs L%d <%s> WARN: " \
			fmt, __LINE__, ibtrs_prefix(sess), ##__VA_ARGS__)

#define INFO(sess, fmt, ...) pr_info("ibtrs <%s>: " fmt, \
				    ibtrs_prefix(sess), ##__VA_ARGS__)
#define INFO_RL(sess, fmt, ...) pr_info_ratelimited("ibtrs <%s>: " fmt, \
					ibtrs_prefix(sess), ##__VA_ARGS__)
#endif /*__IBTRS_LOG_H__*/
