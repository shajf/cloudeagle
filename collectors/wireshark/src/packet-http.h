/* packet-http.h
 *
 * $Id$
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __PACKET_HTTP_H__
#define __PACKET_HTTP_H__

#include <epan/packet.h>
#include "ws_symbol_export.h"

WS_DLL_PUBLIC
void http_dissector_add(guint32 port, dissector_handle_t handle);
WS_DLL_PUBLIC
void http_port_add(guint32 port);

/* Used for HTTP statistics */
typedef struct _http_info_value_t {
	guint32 framenum;
	gchar	*request_method;
	guint	 response_code;
	gchar   *http_host;
	const gchar   *request_uri;
} http_info_value_t;

/* Used for HTTP Export Object feature */
typedef struct _http_eo_t {
	guint32  pkt_num;
	gchar   *hostname;
	gchar   *filename;
	gchar   *content_type;
	guint32  payload_len;
	const guint8 *payload_data;
} http_eo_t;

/** information about a request and response on a HTTP conversation. */
typedef struct _http_req_res_t {
	/** the running number on the conversation */
	guint32 number;
	/** frame number of the request */
	guint32 req_framenum;
	/** frame number of the corresponding response */
	guint32 res_framenum;
	/** timestamp of the request */
	nstime_t req_ts;
	/** pointer to the next element in the linked list, NULL for the tail node */
	struct _http_req_res_t *next;
	/** pointer to the previous element in the linked list, NULL for the head node */
	struct _http_req_res_t *prev;
} http_req_res_t;

/** Conversation data of a HTTP connection. */
typedef struct _http_conv_t {
	guint    response_code;
	gchar   *http_host;
	gchar   *request_method;
	gchar   *request_uri;
	guint8   upgrade;
	guint32	startframe;	/* First frame of proxied connection */
	/** the tail node of req_res */
	http_req_res_t *req_res_tail;
	/** the number of requests on the conversation. */
	guint32 req_res_num;
} http_conv_t;

WS_DLL_PUBLIC int hf_http_notification;
WS_DLL_PUBLIC int hf_http_response;
WS_DLL_PUBLIC int hf_http_request;
WS_DLL_PUBLIC int hf_http_basic;
WS_DLL_PUBLIC int hf_http_request_method;
WS_DLL_PUBLIC int hf_http_request_uri;
WS_DLL_PUBLIC int hf_http_request_full_uri;
WS_DLL_PUBLIC int hf_http_version;
WS_DLL_PUBLIC int hf_http_response_code;
WS_DLL_PUBLIC int hf_http_response_phrase;
WS_DLL_PUBLIC int hf_http_authorization;
WS_DLL_PUBLIC int hf_http_proxy_authenticate;
WS_DLL_PUBLIC int hf_http_proxy_authorization;
WS_DLL_PUBLIC int hf_http_proxy_connect_host;
WS_DLL_PUBLIC int hf_http_proxy_connect_port;
WS_DLL_PUBLIC int hf_http_www_authenticate;
WS_DLL_PUBLIC int hf_http_content_type;
WS_DLL_PUBLIC int hf_http_content_length_header;
WS_DLL_PUBLIC int hf_http_content_length;
WS_DLL_PUBLIC int hf_http_content_encoding;
WS_DLL_PUBLIC int hf_http_transfer_encoding;
WS_DLL_PUBLIC int hf_http_upgrade;
WS_DLL_PUBLIC int hf_http_user_agent;
WS_DLL_PUBLIC int hf_http_host;
WS_DLL_PUBLIC int hf_http_connection;
WS_DLL_PUBLIC int hf_http_cookie;
WS_DLL_PUBLIC int hf_http_accept;
WS_DLL_PUBLIC int hf_http_referer;
WS_DLL_PUBLIC int hf_http_accept_language;
WS_DLL_PUBLIC int hf_http_accept_encoding;
WS_DLL_PUBLIC int hf_http_date;
WS_DLL_PUBLIC int hf_http_cache_control;
WS_DLL_PUBLIC int hf_http_server;
WS_DLL_PUBLIC int hf_http_location;
WS_DLL_PUBLIC int hf_http_sec_websocket_accept;
WS_DLL_PUBLIC int hf_http_sec_websocket_extensions;
WS_DLL_PUBLIC int hf_http_sec_websocket_key;
WS_DLL_PUBLIC int hf_http_sec_websocket_protocol;
WS_DLL_PUBLIC int hf_http_sec_websocket_version;
WS_DLL_PUBLIC int hf_http_set_cookie;
WS_DLL_PUBLIC int hf_http_last_modified;
WS_DLL_PUBLIC int hf_http_x_forwarded_for;
WS_DLL_PUBLIC int hf_http_request_in;
WS_DLL_PUBLIC int hf_http_response_in;
WS_DLL_PUBLIC int hf_http_next_request_in;
WS_DLL_PUBLIC int hf_http_next_response_in;
WS_DLL_PUBLIC int hf_http_prev_request_in;
WS_DLL_PUBLIC int hf_http_prev_response_in;
WS_DLL_PUBLIC int hf_http_time;
#endif /* __PACKET_HTTP_H__ */
