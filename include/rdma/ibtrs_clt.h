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

#if !defined(IBTRS_CLIENT_H)
#define IBTRS_CLIENT_H

#include <linux/scatterlist.h>

struct ibtrs_session;

/**
 * ibtrs_clt_open() - Open a session to a ibtrs_server
 * @addr: The IPv4, IPv6 or GID address of the peer
 * @pdu_sz: Size of extra payload which can be accessed after tag allocation.
 * @priv: Pointer passed back on &ibtrs_clt_ops->sess_ev() invocation
 * @max_inflight_msg: Max. number of parallel inflight messages for the session
 * @max_segments: Max. number of segments per IO request
 * @reconnect_delay_sec: time between reconnect tries
 * @max_reconnect_attempts: Number of times to reconnect on error before giving
 *			    up, 0 for * disabled, -1 for forever
 *
 * Starts session establishment with the ibtrs_server. The function can block
 * up to ~2000ms until it returns.
 *
 * Return a valid pointer on success otherwise PTR_ERR.
 * -EINVAL:	The provided addr could not be resolved to an Infiniband
 *		address, the route to the host could not be resolved or
 *		ibtrs_clt_register() was not called before.
 */
struct ibtrs_session *ibtrs_clt_open(const struct sockaddr_storage *addr,
				     size_t pdu_sz, void *priv,
				     u8 reconnect_delay_sec, u16 max_segments,
				     s16 max_reconnect_attempts);

/**
 * ibtrs_clt_close() - Close a session
 * @sess: Session handler, is freed on return
 */
int ibtrs_clt_close(struct ibtrs_session *sess);

/**
 * enum ibtrs_clt_rdma_ev - Events related to RDMA transfer operations
 */
enum ibtrs_clt_rdma_ev {
	IBTRS_CLT_RDMA_EV_RDMA_REQUEST_WRITE_COMPL,
	IBTRS_CLT_RDMA_EV_RDMA_WRITE_COMPL,
};

/**
 * enum ibtrs_sess_ev - Events about connectivity state of a session
 * @IBTRS_CLT_SESS_EV_RECONNECT		The session was reconnected.
 * @IBTRS_CLT_SESS_EV_DISCONNECTED	The session was disconnected.
 * @IBTRS_CLT_SESS_EV_MAX_RECONN_EXCEEDED Reconect attempts stopped because
 *					  max. number of reconnect attempts
 *					  are reached.
 */
enum ibtrs_clt_sess_ev {
	IBTRS_CLT_SESS_EV_RECONNECT,
	IBTRS_CLT_SESS_EV_DISCONNECTED,
	IBTRS_CLT_SESS_EV_MAX_RECONN_EXCEEDED,
};

/**
 * struct ibtrs_clt_ops - Callback functions of the user module
 * @owner:		module that uses ibtrs_server
 * @rdma_ev:		Event notifications for RDMA operations,
 *      Context: in interrupt(soft irq). The function should be relatively fast.
 *	@priv:			user supplied data that was passed to
 *				ibtrs_clt_request_rdma_write() or
 *				ibtrs_clt_rdma_write() before
 *	@ev:			Occurred event
 *	@errno:			Result of corresponding operation,
 *				0 on success or negative ERRNO code on error
 * @sess_ev:		Event notification for connection state changes
 *	@priv:			user supplied data that was passed to
 *				ibtrs_clt_open()
 *	@ev:			Occurred event
 *	@errno:			Result of corresponding operation,
 *				0 on success or negative ERRNO code on error
 * @recv:		Event notification for infiniband message receival
 *	@priv:			user supplied data that was passed to
 *				ibtrs_clt_open()
 *	@msg			Received data
 *	@len.			Size of the @msg buffer
 *
 * The @recv and @rdma_ev are running on the same CPU that requested the RDMA
 * operation before.
 */

typedef void (rdma_clt_ev_fn)(void *priv, enum ibtrs_clt_rdma_ev ev, int errno);
typedef void (sess_clt_ev_fn)(void *priv, enum ibtrs_clt_sess_ev ev, int errno);
typedef void (recv_clt_fn)(void *priv, const void *msg, size_t len);

struct ibtrs_clt_ops {
	struct module		*owner;
	rdma_clt_ev_fn		*rdma_ev;
	sess_clt_ev_fn		*sess_ev;
	recv_clt_fn		*recv;
};

/**
 * ibtrs_clt_register() - register a user module with ibtrs_client
 * @ops:	callback functions to register
 *
 * Return:
 * 0:		Success
 * -ENOTSUPP:	Registration failed, max. number of supported user modules
		reached
 */
int ibtrs_clt_register(const struct ibtrs_clt_ops *ops);

/**
 * ibtrs_clt_unregister() - unregister a module at ibtrs_client
 * @ops:	struct that was passed before to ibtrs_clt_register()
 *
 * ibtrs_clt_unregister() must only be called after all session that were
 * created by the user module were closed.
 */
void ibtrs_clt_unregister(const struct ibtrs_clt_ops *ops);

enum {
	IBTRS_TAG_NOWAIT = 0,
	IBTRS_TAG_WAIT   = 1,
};

/**
 * ibtrs_tag - tags the memory allocation for future RDMA operation
 */
struct ibtrs_tag {
	unsigned int cpu_id;
	unsigned int mem_id;
	unsigned int mem_id_mask;
};

static inline struct ibtrs_tag *ibtrs_tag_from_pdu(void *pdu)
{
	return pdu - sizeof(struct ibtrs_tag);
}

static inline void *ibtrs_tag_to_pdu(struct ibtrs_tag *tag)
{
	return tag + 1;
}

/**
 * ibtrs_get_tag() - allocates tag for future RDMA operation
 * @sess:	Current session
 * @cpu_id:	cpu_id to run
 * @nr_bytes:	Number of bytes to consume per tag
 * @wait:	Wait type
 *
 * Description:
 *    Allocates tag for the following RDMA operation.  Tag is used
 *    to preallocate all resources and to propagate memory pressure
 *    up earlier.
 *
 * Context:
 *    Can sleep if @wait == IBTRS_TAG_WAIT
 */
struct ibtrs_tag *ibtrs_get_tag(struct ibtrs_session *sess, int cpu_id,
				size_t nr_bytes, int wait);

/**
 * ibtrs_put_tag() - puts allocated tag
 * @sess:	Current session
 * @tag:	Tag to be freed
 *
 * Context:
 *    Does not matter
 */
void ibtrs_put_tag(struct ibtrs_session *sess, struct ibtrs_tag *tag);

/**
 * ibtrs_clt_rdma_write() - Transfer data to the server via RDMA.
 * @sess:	Session
 * @tag:	Preallocated tag
 * @priv:	User provided data, passed back on corresponding
 *		@ibtrs_clt_ops->rdma_ev() event
 * @vec:	User module message to transfer together with @sg.
 *		Sum of len of all @vec elements limited to <= IO_MSG_SIZE
 * @nr:		Number of elements in @vec.
 * @data_len:	Size of data in @sg
 * @sg:		data to transferred, 512B aligned in the receivers memory
 * @sg_len:	number of elements in @sg array
 *
 * Return:
 * 0:		Success
 * <0:		Error
 *
 * On completion of the operation a %IBTRS_CLT_RDMA_EV_RDMA_WRITE_COMPL is
 * generated. If an error happened on IBTRS layer for this operation a
 * %IBTRS_CLT_RDMA_EV_ERROR is generated.
 */
int ibtrs_clt_rdma_write(struct ibtrs_session *sess, struct ibtrs_tag *tag,
			 void *priv, const struct kvec *vec, size_t nr,
			 size_t data_len, struct scatterlist *sg,
			 unsigned int sg_len);

/**
 * ibtrs_clt_request_rdma_write() - Request data transfer from server via RDMA.
 *
 * @sess:	Session
 * @tag:	Preallocated tag
 * @priv:	User provided data, passed back on corresponding
 *		@ibtrs_clt_ops->rdma_ev() event
 * @vec:	Message that is send to server together with the request.
 *		Sum of len of all @vec elements limited to <= IO_MSG_SIZE.
 * @nr:		Number of elements in @vec.
 * @result_len: Max. length of data that ibtrs_server will send back
 * @recv_sg:	Pages in which the response of the server will be stored.
 * @recv_sg_len: Number of elements in the @recv_sg
 *
 * Return:
 * 0:		Success
 * <0:		Error
 *
 * IBTRS Client will request a data transfer from Server to Client via RDMA.
 * The data that the server will respond with will be stored in @recv_sg when
 * the user receives an %IBTRS_CLT_RDMA_EV_RDMA_REQUEST_WRITE_COMPL event.
 * If an error occurred on the IBTRS layer a %IBTRS_CLT_RDMA_EV_ERROR is
 * generated instead
 */
int ibtrs_clt_request_rdma_write(struct ibtrs_session *sess,
				 struct ibtrs_tag *tag, void *priv,
				 const struct kvec *vec, size_t nr,
				 size_t result_len,
				 struct scatterlist *recv_sg,
				 unsigned int recv_sg_len);

/**
 * ibtrs_clt_send() - Send data to server via an infiniband message.
 * @sess:	Session
 * @vec:	Data to transfer
 * @nr:		Number of elements in @vec
 *
 * Return:
 * 0:		Success
 * <0:		Error:
 *		-ECOMM		no connection to the server
 *		-EINVAL		message size too big (500 bytes max)
 *		-EAGAIN		run out of tx buffers - try again later
 *		-<IB ERROR>	see mlx doc
 *
 * The operation is not confirmed. It is the responsibility of the user on the
 * other side to send an acknowledgment if required.
 */
int ibtrs_clt_send(struct ibtrs_session *sess, const struct kvec *vec,
		   size_t nr);

/**
 * ibtrs_attrs - IBTRS session attributes
 */
struct ibtrs_attrs {
	u32	queue_depth;
	u64	mr_page_mask;
	u32	mr_page_size;
	u32	mr_max_size;
	u32	max_pages_per_mr;
	u32	max_sge;
	u32	max_io_size;
	u8	hostname[MAXHOSTNAMELEN];
};

/**
 * ibtrs_clt_query() - queries IBTRS session attributes
 *
 * Returns:
 *    0 on success
 *    -ECOMM		no connection to the server
 */
int ibtrs_clt_query(struct ibtrs_session *sess, struct ibtrs_attrs *attr);

#endif
