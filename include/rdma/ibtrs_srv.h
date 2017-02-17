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

#ifndef _IBTRS_SRV_H
#define _IBTRS_SRV_H

#include <linux/socket.h>

struct ibtrs_session;
struct ibtrs_ops_id;

enum ibtrs_srv_rdma_ev {
	IBTRS_SRV_RDMA_EV_RECV,
	IBTRS_SRV_RDMA_EV_WRITE_REQ,
};

/**
 * enum ibtrs_srv_sess_ev - Session events
 * @IBTRS_SRV_SESS_EV_CONNECTED:	Connection from client established
 * @IBTRS_SRV_SESS_EV_DISCONNECTING:	Connection is currently disconnected,
 *					sending data through the connection may
 *					fail, but could still recv messages.
 * @IBTRS_SRV_SESS_EV_DISCONNECTED:	Connection was disconnected, all
 *					connection IBTRS resources were freed.
 */

enum ibtrs_srv_sess_ev {
	IBTRS_SRV_SESS_EV_CONNECTED,
	IBTRS_SRV_SESS_EV_DISCONNECTING,
	IBTRS_SRV_SESS_EV_DISCONNECTED,
};

/**
 * &struct ibtrs_srv_ops - Callbacks for ibtrs_server
 * @owner:		module that uses ibtrs_server
 * @rdma_ev:		Event notification for RDMA operations
 *			If the callback returns a value != 0, an error message
 *			for the data transfer will be sent to the client.

 *	@sess:		Session
 *	@priv:		Private data from user
 *	@id:		internal IBTRS id
 *	@ev:		Event
 *	@data:		Data received by the client. The message of the user of
 *			ibtrs_client is allocated at the end of the buffer.
 *			Before the message the data of the ibtrs_client is
 *			located.
 *			If the event is %IBTRS_SRV_RDMA_EV_WRITE_REQ, the user
 *			can write his response into @data. When
 *			ibtrs_srv_resp_rdma() is called, this @data will be
 *			transferred to the client.
 *	@len:		length of data in @data

 * @sess_ev:		Events about connective state changes
 *			If the callback returns != 0 and the event
 *			%IBTRS_SRV_SESS_EV_CONNECTED the corresponding session
 *			was destroyed.
 *	@sess:		Session
 *	@ev:		event
 *	@priv:		Private data from user if previously set with
 *			ibtrs_srv_set_sess_priv()

 * @recv:		Event notification for infiniband message receival
 *	@sess:		Session
 *	@priv:		Private data from user if previously set with
 *			ibtrs_srv_set_sess_priv()
 *	@msg:		Received message
 *	@len:		length of @msg
 */

typedef int (rdma_ev_fn)(struct ibtrs_session *sess, void *priv,
			 struct ibtrs_ops_id *id, enum ibtrs_srv_rdma_ev ev,
			 void *data, size_t len);
typedef int (sess_ev_fn)(struct ibtrs_session *sess, enum ibtrs_srv_sess_ev ev,
			 void *priv);
typedef void (recv_fn)(struct ibtrs_session *sess, void *priv, const void *msg,
		       size_t len);

struct ibtrs_srv_ops {
	struct module *owner;

	rdma_ev_fn	*rdma_ev;
	sess_ev_fn	*sess_ev;
	recv_fn		*recv;
};

/**
 * ibtrs_srv_register() - register a module with ibtrs_server
 * @ops:		callback functions
 *
 * Registers a module with the ibtrs_server. The user module passes the function
 * pointers, that ibtrs_server can call to communicate with it.
 *
 * Return:
 * 0:		Success
 * <0:		Error
 */
int ibtrs_srv_register(const struct ibtrs_srv_ops *ops);

/**
 * ibtrs_srv_unregister - unregister a module with ibtrs_server
 * @ops: the struct that was passed to ibtrs_srv_register() before
 *
 * Unregisters a module from the ibtrs_server. All open connections will be
 * terminated.
 */
void ibtrs_srv_unregister(const struct ibtrs_srv_ops *ops);

/**
 * ibtrs_srv_resp_rdma() - Finish an RDMA request
 *
 * @id:		Internal IBTRS operation identifier
 * @errno:	Response Code send to the other side for this operation.
 *		0 = success, <=0 error
 * Return:
 *  0:		Success
 * <0:		Error
 *
 * Finish a RDMA operation. A message is sent to the client and the
 * corresponding memory areas will be released.
 */
int ibtrs_srv_resp_rdma(struct ibtrs_ops_id *id, int errno);

/**
 * ibtrs_srv_send() - Send data to the ibtrs_server with an infiniband message.
 * @sess	Session
 * @vec:	Data send to the server
 * @nr:		Length of @vec
 *
 * Return:
 * 0:		Success
 * <0:		Error
 * -EINVAL:	@len is too big
 */
int ibtrs_srv_send(struct ibtrs_session *sess, const struct kvec *vec,
		   size_t nr);

/**
 * ibtrs_srv_set_sess_priv() - Set private pointer in ibtrs_session.
 * @sess	Session
 * @priv:	The private pointer that is associated with the session.
 */
void ibtrs_srv_set_sess_priv(struct ibtrs_session *sess, void *priv);

/**
 * ibtrs_srv_get_sess_qdepth() - Get ibtrs_session qdepth.
 * @sess	Session
 */
int ibtrs_srv_get_sess_qdepth(struct ibtrs_session *sess);

/**
 * ibtrs_srv_get_sess_addr() - Get ibtrs_session address.
 * @sess	Session
 */
const char *ibtrs_srv_get_sess_addr(struct ibtrs_session *sess);

/**
 * ibtrs_srv_get_sess_hostname() - Get ibtrs_session peer hostname.
 * @sess	Session
 */
const char *ibtrs_srv_get_sess_hostname(struct ibtrs_session *sess);

#endif
