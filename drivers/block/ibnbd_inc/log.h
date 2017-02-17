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

#ifndef __IBNBD_LOG_H__
#define __IBNBD_LOG_H__

#define ERR_NP(fmt, ...) pr_err("ibnbd L%d ERR: " fmt, \
				__LINE__, ##__VA_ARGS__)
#define ERR_NP_RL(fmt, ...) pr_err_ratelimited("ibnbd L%d ERR: " fmt, \
				__LINE__, ##__VA_ARGS__)

#define WRN_NP(fmt, ...) pr_warn("ibnbd L%d WARN: " fmt, \
				__LINE__, ##__VA_ARGS__)
#define WRN_NP_RL(fmt, ...) pr_warn_ratelimited("ibnbd L%d WARN: " fmt,\
				__LINE__, ##__VA_ARGS__)

#define INFO_NP(fmt, ...)  pr_info("ibnbd: " fmt, ##__VA_ARGS__)
#define INFO_NP_RL(fmt, ...) pr_info_ratelimited("ibnbd: " fmt, ##__VA_ARGS__)

#define DEB(fmt, ...) pr_debug("ibnbd L%d " fmt, __LINE__, ##__VA_ARGS__)

#define ibnbd_prefix(dev) ((dev->sess->hostname[0] != '\0') ? \
			    dev->sess->hostname : dev->sess->str_addr)

#endif /*__IBNBD_LOG_H__*/
