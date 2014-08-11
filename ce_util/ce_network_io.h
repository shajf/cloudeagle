/*
 * =====================================================================================
 *
 *       Filename:  ce_network_io.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
*/

#ifndef CE_NETWORK_IO_H
#define CE_NETWORK_IO_H

/**
 * @file ce_network_io.h
 * @brief CE Network library
 */

#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_file.h"
#include "ce_errno.h"
#include "ce_time.h"
#ifdef __cplusplus
extern "c" {
#endif /* __cplusplus */

/* End System Headers */
#define POLLIN   1
#define POLLPRI  2
#define POLLOUT  4
#define POLLERR  8
#define POLLHUP  16
#define POLLNVAL 32

/** Maximum seconds to linger */
#define CE_MAX_SECS_TO_LINGER 30

/** Maximum hostname length */
#define CEMAXHOSTLEN 256

/** Default 'any' address */
#define CE_ANYADDR "0.0.0.0"

#define CE_SO_LINGER        1    /**< Linger */
#define CE_SO_KEEPALIVE     2    /**< Keepalive */
#define CE_SO_DEBUG         4    /**< Debug */
#define CE_SO_NONBLOCK      8    /**< Non-blocking IO */
#define CE_SO_REUSEADDR     16   /**< Reuse addresses */
#define CE_SO_SNDBUF        64   /**< Send buffer */
#define CE_SO_RCVBUF        128  /**< Receive buffer */
#define CE_SO_DISCONNECTED  256  /**< Disconnected */
#define CE_TCP_NODELAY      512  /**< For SCTP sockets, this is mapped
                                   * to STCP_NODELAY internally.
                                   */
#define CE_TCP_NOPUSH       1024 /**< No push */
#define CE_RESET_NODELAY    2048 /**< This flag is ONLY set internally
                                   * when we set CE_TCP_NOPUSH with
                                   * CE_TCP_NODELAY set to tell us that
                                   * CE_TCP_NODELAY should be turned on
                                   * again when NOPUSH is turned off
                                   */
#define CE_INCOMPLETE_READ 4096  /**< Set on non-blocking sockets
				   * (timeout != 0) on which the
				   * previous read() did not fill a buffer
				   * completely.  the next ce_socket_recv() 
                                   * will first call select()/poll() rather than
				   * going straight into read().  (Can also
				   * be set by an application to force a
				   * select()/poll() call before the next
				   * read, in cases where the app expects
				   * that an immediate read would fail.)
				   */
#define CE_INCOMPLETE_WRITE 8192 /**< like CE_INCOMPLETE_READ, but for write
                                   * @see CE_INCOMPLETE_READ
                                   */
#define CE_IPV6_V6ONLY     16384 /**< Don't accept IPv4 connections on an
                                   * IPv6 listening socket.
                                   */
#define CE_TCP_DEFER_ACCEPT 32768 /**< Delay accepting of new connections 
                                    * until data is available.
                                    * @see ce_socket_accept_filter
                                    */
#ifndef CE_ENOTIMPL
#define CE_ENOTIMPL -1
#endif
/** @} */

/** Define what type of socket shutdown should occur. */
typedef enum {
    CE_SHUTDOWN_READ,          /**< no longer allow read request */
    CE_SHUTDOWN_WRITE,         /**< no longer allow write requests */
    CE_SHUTDOWN_READWRITE      /**< no longer allow read or write requests */
} ce_shutdown_how_e;

#define CE_IPV4_ADDR_OK  0x01  /**< @see ce_sockaddr_info_get() */
#define CE_IPV6_ADDR_OK  0x02  /**< @see ce_sockaddr_info_get() */
/** @def CE_INADDR_NONE
 * Not all platforms have a real INADDR_NONE.  This macro replaces
 * INADDR_NONE on all platforms.
 */
#define CE_INADDR_NONE ((unsigned int) 0xffffffff)

/**
 * @def CE_INET
 * Not all platforms have these defined, so we'll define them here
 * The default values come from FreeBSD 4.1.1
 */
#define CE_INET     AF_INET
/** @def CE_UNSPEC
 * Let the system decide which address family to use
 */
#define CE_UNSPEC   AF_UNSPEC

#define CE_INET6    AF_INET6


/**
 * @defgroup IP_Proto IP Protocol Definitions for use when creating sockets
 * @{
 */
#define CE_PROTO_TCP       6   /**< TCP  */
#define CE_PROTO_UDP      17   /**< UDP  */
#define CE_PROTO_SCTP    132   /**< SCTP */
/** @} */

/**
 * Enum used to denote either the local and remote endpoint of a
 * connection.
 */
typedef enum {
    CE_LOCAL,   /**< Socket information for local end of connection */
    CE_REMOTE   /**< Socket information for remote end of connection */
} ce_interface_e;


#define ce_inet_addr    inet_addr

typedef long ce_interval_time_t;
typedef  socklen_t   ce_socklen_t;

typedef struct sock_userdata_t sock_userdata_t;

/** A structure to represent sockets */
typedef struct ce_socket_t     ce_socket_t;
/**
 * A structure to encapsulate headers and trailers for ce_socket_sendfile
 */
typedef struct ce_hdtr_t       ce_hdtr_t;
/** A structure to represent in_addr */
typedef struct in_addr          ce_in_addr_t;

/** @remark use ce_uint16_t just in case some system has a short that isn't 16 bits... */
typedef ce_uint16_t            ce_port_t;

/** @remark It's defined here as I think it should all be platform safe...
 * @see ce_sockaddr_t
 */
typedef struct ce_sockaddr_t ce_sockaddr_t;

struct sock_userdata_t 
{
	 sock_userdata_t *next;
	 const char *key;
	 void *data;
};

 struct ce_socket_t 
 {
	ce_pool_t *pool;
	int socketdes;
	int type;
	int protocol;
	ce_sockaddr_t *local_addr;
	ce_sockaddr_t *remote_addr;
	ce_interval_time_t timeout;
	int local_port_unknown;
	int local_interface_unknown;
	int remote_addr_unknown;
	ce_int32_t options;
	ce_int32_t inherit;
	sock_userdata_t *userdata;
	int connected;
	//ce_pollset_t *pollset;

 };
#define CE_SOCKET_T_SIZE \
	sizeof(ce_socket_t)

/**
 * CEs socket address type, used to ensure protocol independence
 */
struct ce_sockaddr_t {
    /** The pool to use... */
    ce_pool_t *pool;
    /** The hostname */
    char *hostname;
    /** Either a string of the port number or the service name for the port */
    char *servname;
    /** The numeric port */
    ce_port_t port;
    /** The family */
    ce_int32_t family;
    /** How big is the sockaddr we're using? */
    ce_socklen_t salen;
    /** How big is the ip address structure we're using? */
    int ipaddr_len;
    /** How big should the address buffer be?  16 for v4 or 46 for v6
     *  used in inet_ntop... */
    int addr_str_len;
    /** This points to the IP address structure within the appropriate
     *  sockaddr structure.  */
    void *ipaddr_ptr;
    /** If multiple addresses were found by ce_sockaddr_info_get(), this 
     *  points to a represecetion of the next address. */
    ce_sockaddr_t *next;
    /** Union of either IPv4 or IPv6 sockaddr. */
    union {
        /** IPv4 sockaddr structure */
        struct sockaddr_in sin;
#if CE_HAVE_IPV6
        /** IPv6 sockaddr structure */
        struct sockaddr_in6 sin6;
#endif
#if CE_HAVE_SA_STORAGE
        /** Placeholder to ensure that the size of this union is not
         * dependent on whether CE_HAVE_IPV6 is defined. */
        struct sockaddr_storage sas;
#endif
    } sa;
};
#define CE_SOCKADDR_T_SIZE \
	sizeof(ce_sockaddr_t)


/** A structure to encapsulate headers and trailers for ce_socket_sendfile */
struct ce_hdtr_t {
    /** An iovec to store the headers sent before the file. */
    struct iovec* headers;
    /** number of headers in the iovec */
    int numheaders;
    /** An iovec to store the trailers sent after the file. */
    struct iovec* trailers;
    /** number of trailers in the iovec */
    int numtrailers;
};
#define CE_HDTR_T_SIZE \
	sizeof(ce_hdtr_t)

/* function definitions */

extern const char *
ce_inet_ntop(int af, 
	       const void *src,
               char *dst,
               size_t size); 
extern int 
ce_inet_pton(int af, 
	      const char *src, 
	      void *dst);
extern void
ce_sockaddr_vars_set(ce_sockaddr_t *, 
		       int, 
		       ce_port_t);

#define ce_is_option_set(skt, option)  \
     (((skt)->options & (option)) == (option))

#define ce_set_option(skt, option, on)             \
       do				            \
       {                                            \
		if(on)                              \
			(skt)->options|=(option);   \
		else                                \
			(skt)->options&=~(option);  \
       }while(0) 

/**
 * Create a socket.
 * @param new_sock The new socket that has been set up.
 * @param family The address family of the socket (e.g., CE_INET).
 * @param type The type of the socket (e.g., SOCK_STREAM).
 * @param protocol The protocol of the socket (e.g., CE_PROTO_TCP).
 * @param cont The pool for the ce_socket_t and associated storage.
 */
extern ce_int_t 
ce_socket_create(ce_socket_t **new_sock, 
		   int family, 
	  	   int type,
		   int protocol,
		   ce_pool_t *cont);
extern ce_int_t
ce_sockdes_get(int *thesock,
		ce_socket_t *sock);

/**
 * Shutdown either reading, writing, or both sides of a socket.
 * @param thesocket The socket to close 
 * @param how How to shutdown the socket.  One of:
 * <PRE>
 *            CE_SHUTDOWN_READ         no longer allow read requests
 *            CE_SHUTDOWN_WRITE        no longer allow write requests
 *            CE_SHUTDOWN_READWRITE    no longer allow read or write requests 
 * </PRE>
 * @see ce_shutdown_how_e
 * @remark This does not actually close the socket descriptor, it just
 *      controls which calls are still valid on the socket.
 */
extern ce_int_t 
ce_socket_shutdown(ce_socket_t *thesocket,
		     ce_shutdown_how_e how);

/**
 * Close a socket.
 * @param thesocket The socket to close 
 */
extern ce_int_t
ce_socket_close(ce_socket_t *thesocket);

/**
 * Bind the socket to its associated port
 * @param sock The socket to bind 
 * @param sa The socket address to bind to
 * @remark This may be where we will find out if there is any other process
 *      using the selected port.
 */
extern ce_int_t 
ce_socket_bind(ce_socket_t *sock, 
                 ce_sockaddr_t *sa);

/**
 * Listen to a bound socket for connections.
 * @param sock The socket to listen on 
 * @param backlog The number of outstanding connections allowed in the sockets
 *                listen queue.  If this value is less than zero, the listen
 *                queue size is set to zero.  
 */
extern ce_int_t 
ce_socket_listen(ce_socket_t *sock, 
                   ce_int32_t backlog);

/**
 * Accept a new connection request
 * @param new_sock A copy of the socket that is connected to the socket that
 *                 made the connection request.  This is the socket which should
 *                 be used for all future communication.
 * @param sock The socket we are listening on.
 * @param connection_pool The pool for the new socket.
 */
extern ce_int_t 
ce_socket_accept(ce_socket_t **new_sock, 
	           ce_socket_t *sock,
	  	   ce_pool_t *connection_pool);

/**
 * Issue a connection request to a socket either on the same machine 
 * or a different one.
 * @param sock The socket we wish to use for our side of the connection 
 * @param sa The address of the machine we wish to connect to.
 */
extern ce_int_t 
ce_socket_connect(ce_socket_t *sock,
                    ce_sockaddr_t *sa);

/**
 * Determine whether the receive part of the socket has been closed by
 * the peer (such that a subsequent call to ce_socket_read would
 * return CE_EOF), if the socket's receive buffer is empty.  This
 * function does not block waiting for I/O.
 *
 * @param sock The socket to check
 * @param atreadeof If CE_SUCCESS is returned, *atreadeof is set to
 *                  non-zero if a subsequent read would return CE_EOF
 * @return an error is returned if it was not possible to determine the
 *         status, in which case *atreadeof is not changed.
 */
extern ce_int_t 
ce_socket_atreadeof(ce_socket_t *sock,
                      int *atreadeof);

/**
 * Create ce_sockaddr_t from hostname, address family, and port.
 * @param sa The new ce_sockaddr_t.
 * @param hostname The hostname or numeric address string to resolve/parse, or
 *               NULL to build an address that corresponds to 0.0.0.0 or ::
 * @param family The address family to use, or CE_UNSPEC if the system should 
 *               decide.
 * @param port The port number.
 * @param flags Special processing flags:
 * <PRE>
 *       CE_IPV4_ADDR_OK          first query for IPv4 addresses; only look
 *                                 for IPv6 addresses if the first query failed;
 *                                 only valid if family is CE_UNSPEC and hostname
 *                                 isn't NULL; mutually exclusive with
 *                                 CE_IPV6_ADDR_OK
 *       CE_IPV6_ADDR_OK          first query for IPv6 addresses; only look
 *                                 for IPv4 addresses if the first query failed;
 *                                 only valid if family is CE_UNSPEC and hostname
 *                                 isn't NULL and CE_HAVE_IPV6; mutually exclusive
 *                                 with CE_IPV4_ADDR_OK
 * </PRE>
 * @param p The pool for the ce_sockaddr_t and associated storage.
 */
extern ce_int_t 
ce_sockaddr_info_get(ce_sockaddr_t **sa,
			 char *hostname,
			ce_int32_t family,
			ce_port_t port,
			ce_int32_t flags,
			ce_pool_t *p);

/**
 * Look up the host name from an ce_sockaddr_t.
 * @param hostname The hostname.
 * @param sa The ce_sockaddr_t.
 * @param flags Special processing flags.
 */
extern ce_int_t 
ce_getnameinfo(char **hostname,
		ce_sockaddr_t *sa,
		ce_int32_t flags);

/**
 * Parse hostname/IP address with scope id and port.
 *
 * Any of the following strings are accepted:
 *   8080                  (just the port number)
 *   www.apache.org        (just the hostname)
 *   www.apache.org:8080   (hostname and port number)
 *   [fe80::1]:80          (IPv6 numeric address string only)
 *   [fe80::1%eth0]        (IPv6 numeric address string and scope id)
 *
 * Invalid strings:
 *                         (empty string)
 *   [abc]                 (not valid IPv6 numeric address string)
 *   abc:65536             (invalid port number)
 *
 * @param addr The new buffer coceining just the hostname.  On output, *addr 
 *             will be NULL if no hostname/IP address was specfied.
 * @param scope_id The new buffer coceining just the scope id.  On output, 
 *                 *scope_id will be NULL if no scope id was specified.
 * @param port The port number.  On output, *port will be 0 if no port was 
 *             specified.
 *             ### FIXME: 0 is a legal port (per RFC 1700). this should
 *             ### return something besides zero if the port is missing.
 * @param str The input string to be parsed.
 * @param p The pool from which *addr and *scope_id are allocated.
 * @remark If scope id shouldn't be allowed, check for scope_id != NULL in 
 *         addition to checking the return code.  If addr/hostname should be 
 *         required, check for addr == NULL in addition to checking the 
 *         return code.
 */
extern ce_int_t 
ce_parse_addr_port(char **addr,
          	     char **scope_id,
		     ce_port_t *port,
		     const char *str,
		     ce_pool_t *p);

/**
 * Get name of the current machine
 * @param buf A buffer to store the hostname in.
 * @param len The maximum length of the hostname that can be stored in the
 *            buffer provided.  The suggested length is CEMAXHOSTLEN + 1.
 * @param cont The pool to use.
 * @remark If the buffer was not large enough, an error will be returned.
 */
extern ce_int_t 
ce_gethostname(char *buf, 
		 size_t len, 
		 ce_pool_t *cont);

/**
 * Return the data associated with the current socket
 * @param data The user data associated with the socket.
 * @param key The key to associate with the user data.
 * @param sock The currently open socket.
 */
extern ce_int_t 
ce_socket_data_get(void **data,
		     const char *key,
		     ce_socket_t *sock);

/**
 * Set the data associated with the current socket.
 * @param sock The currently open socket.
 * @param data The user data to associate with the socket.
 * @param key The key to associate with the data.
 * @param cleanup The cleanup to call when the socket is destroyed.
 */
extern ce_int_t 
ce_socket_data_set(ce_socket_t *sock,
		     void *data,
		      char *key);

/**
 * Send data over a network.
 * @param sock The socket to send the data over.
 * @param buf The buffer which coceins the data to be sent. 
 * @param len On entry, the number of bytes to send; on exit, the number
 *            of bytes sent.
 * @remark
 * <PRE>
 * This functions acts like a blocking write by default.  To change 
 * this behavior, use ce_socket_timeout_set() or the CE_SO_NONBLOCK
 * socket option.
 *
 * It is possible for both bytes to be sent and an error to be returned.
 *
 * CE_EINTR is never returned.
 * </PRE>
 */
extern ce_int_t 
ce_socket_send(ce_socket_t *sock, 
		 const char *buf, 
		 size_t *len);

/**
 * Send multiple packets of data over a network.
 * @param sock The socket to send the data over.
 * @param vec The array of iovec structs coceining the data to send 
 * @param nvec The number of iovec structs in the array
 * @param len Receives the number of bytes actually written
 * @remark
 * <PRE>
 * This functions acts like a blocking write by default.  To change 
 * this behavior, use ce_socket_timeout_set() or the CE_SO_NONBLOCK
 * socket option.
 * The number of bytes actually sent is stored in argument 3.
 *
 * It is possible for both bytes to be sent and an error to be returned.
 *
 * CE_EINTR is never returned.
 * </PRE>
 */
ce_int_t 
ce_socket_sendv(ce_socket_t *sock, 
		  const struct iovec *vec,
		  ce_int32_t nvec,
		  size_t *len);

/**
 * @param sock The socket to send from
 * @param where The ce_sockaddr_t describing where to send the data
 * @param flags The flags to use
 * @param buf  The data to send
 * @param len  The length of the data to send
 */
extern ce_int_t 
ce_socket_sendto(ce_socket_t *sock, 
		   ce_sockaddr_t *where,
		   ce_int32_t flags,
		   const char *buf, 
		   size_t *len);

/**
 * Read data from a socket.  On success, the address of the peer from
 * which the data was sent is copied into the @a from parameter, and the
 * @a len parameter is updated to give the number of bytes written to
 * @a buf.
 *
 * @param from Updated with the address from which the data was received
 * @param sock The socket to use
 * @param flags The flags to use
 * @param buf  The buffer to use
 * @param len  The length of the available buffer
 */

extern ce_int_t 
ce_socket_recvfrom(ce_sockaddr_t *from, 
		     ce_socket_t *sock,
		     ce_int32_t flags,
		     char *buf, 
		     size_t *len);

/**
 * Read data from a network.
 * @param sock The socket to read the data from.
 * @param buf The buffer to store the data in. 
 * @param len On entry, the number of bytes to receive; on exit, the number
 *            of bytes received.
 * @remark
 * <PRE>
 * This functions acts like a blocking read by default.  To change 
 * this behavior, use ce_socket_timeout_set() or the CE_SO_NONBLOCK
 * socket option.
 * The number of bytes actually received is stored in argument 3.
 *
 * It is possible for both bytes to be received and an CE_EOF or
 * other error to be returned.
 *
 * CE_EINTR is never returned.
 * </PRE>
 */
extern ce_int_t
ce_socket_recv(ce_socket_t *sock, 
		 char *buf, 
		 size_t *len);

/**
 * Setup socket options for the specified socket
 * @param sock The socket to set up.
 * @param opt The option we would like to configure.  One of:
 * <PRE>
 *            CE_SO_DEBUG      --  turn on debugging information 
 *            CE_SO_KEEPALIVE  --  keep connections active
 *            CE_SO_LINGER     --  lingers on close if data is present
 *            CE_SO_NONBLOCK   --  Turns blocking on/off for socket
 *                                  When this option is enabled, use
 *                                  the CE_STATUS_IS_EAGAIN() macro to
 *                                  see if a send or receive function
 *                                  could not transfer data without
 *                                  blocking.
 *            CE_SO_REUSEADDR  --  The rules used in validating addresses
 *                                  supplied to bind should allow reuse
 *                                  of local addresses.
 *            CE_SO_SNDBUF     --  Set the SendBufferSize
 *            CE_SO_RCVBUF     --  Set the ReceiveBufferSize
 * </PRE>
 * @param on Value for the option.
 */
extern ce_int_t 
ce_socket_opt_set(ce_socket_t *sock,
		    ce_int32_t opt, 
		    ce_int32_t on);

/**
 * Setup socket timeout for the specified socket
 * @param sock The socket to set up.
 * @param t Value for the timeout.
 * <PRE>
 *   t > 0  -- read and write calls return CE_TIMEUP if specified time
 *             elapsess with no data read or written
 *   t == 0 -- read and write calls never block
 *   t < 0  -- read and write calls block
 * </PRE>
 */
extern ce_int_t 
ce_socket_timeout_set(ce_socket_t *sock,
			ce_interval_time_t t);

/**
 * Query socket options for the specified socket
 * @param sock The socket to query
 * @param opt The option we would like to query.  One of:
 * <PRE>
 *            CE_SO_DEBUG      --  turn on debugging information 
 *            CE_SO_KEEPALIVE  --  keep connections active
 *            CE_SO_LINGER     --  lingers on close if data is present
 *            CE_SO_NONBLOCK   --  Turns blocking on/off for socket
 *            CE_SO_REUSEADDR  --  The rules used in validating addresses
 *                                  supplied to bind should allow reuse
 *                                  of local addresses.
 *            CE_SO_SNDBUF     --  Set the SendBufferSize
 *            CE_SO_RCVBUF     --  Set the ReceiveBufferSize
 *            CE_SO_DISCONNECTED -- Query the disconnected state of the socket.
 *                                  (Currently only used on Windows)
 * </PRE>
 * @param on Socket option returned on the call.
 */
extern ce_int_t 
ce_socket_opt_get(ce_socket_t *sock, 
		    ce_int32_t opt,
		    ce_int32_t *on);

/**
 * Query socket timeout for the specified socket
 * @param sock The socket to query
 * @param t Socket timeout returned from the query.
 */
extern ce_int_t 
ce_socket_timeout_get(ce_socket_t *sock, 
			ce_interval_time_t *t);

/**
 * Query the specified socket if at the OOB/Urgent data mark
 * @param sock The socket to query
 * @param atmark Is set to true if socket is at the OOB/urgent mark,
 *               otherwise is set to false.
 */
extern ce_int_t 
ce_socket_atmark(ce_socket_t *sock, 
		   int *atmark);

/**
 * Return an address associated with a socket; either the address to
 * which the socket is bound locally or the the address of the peer
 * to which the socket is connected.
 * @param sa The returned ce_sockaddr_t.
 * @param which Whether to retrieve the local or remote address
 * @param sock The socket to use
 */
extern ce_int_t 
ce_socket_addr_get(ce_sockaddr_t **sa,
		     ce_interface_e which,
		     ce_socket_t *sock);
 
/**
 * Return the IP address (in numeric address string format) in
 * an CE socket address.  CE will allocate storage for the IP address 
 * string from the pool of the ce_sockaddr_t.
 * @param addr The IP address.
 * @param sockaddr The socket address to reference.
 */
extern ce_int_t 
ce_sockaddr_ip_get(char **addr, 
		     ce_sockaddr_t *sockaddr);

/**
 * Write the IP address (in numeric address string format) of the CE
 * socket address @a sockaddr into the buffer @a buf (of size @a buflen).
 * @param sockaddr The socket address to reference.
 */
extern ce_int_t 
ce_sockaddr_ip_getbuf(char *buf, 
			size_t buflen,
			ce_sockaddr_t *sockaddr);

/**
 * See if the IP addresses in two CE socket addresses are
 * equivalent.  Appropriate logic is present for comparing
 * IPv4-mapped IPv6 addresses with IPv4 addresses.
 *
 * @param addr1 One of the CE socket addresses.
 * @param addr2 The other CE socket address.
 * @remark The return value will be non-zero if the addresses
 * are equivalent.
 */
extern int 
ce_sockaddr_equal(const ce_sockaddr_t *addr1,
		    const ce_sockaddr_t *addr2);

/**
* Return the type of the socket.
* @param sock The socket to query.
* @param type The returned type (e.g., SOCK_STREAM).
*/
extern ce_int_t 
ce_socket_type_get(ce_socket_t *sock,
		     int *type);
 
/**
 * Given an ce_sockaddr_t and a service name, set the port for the service
 * @param sockaddr The ce_sockaddr_t that will have its port set
 * @param servname The name of the service you wish to use
 */
extern ce_int_t 
ce_getservbyname(ce_sockaddr_t *sockaddr, 
		   const char *servname);

/**
 * Return the protocol of the socket.
 * @param sock The socket to query.
 * @param protocol The returned protocol (e.g., CE_PROTO_TCP).
 */
extern ce_int_t 
ce_socket_protocol_get(ce_socket_t *sock,
			int *protocol);


/**
 * Join a Multicast Group
 * @param sock The socket to join a multicast group
 * @param join The address of the multicast group to join
 * @param iface Address of the interface to use.  If NULL is passed, the 
 *              default multicast interface will be used. (OS Dependent)
 * @param source Source Address to accept transmissions from (non-NULL 
 *               implies Source-Specific Multicast)
 */
extern ce_int_t 
ce_mcast_join(ce_socket_t *sock,
		ce_sockaddr_t *join,
		ce_sockaddr_t *iface,
		ce_sockaddr_t *source);

/**
 * Leave a Multicast Group.  All arguments must be the same as
 * ce_mcast_join.
 * @param sock The socket to leave a multicast group
 * @param addr The address of the multicast group to leave
 * @param iface Address of the interface to use.  If NULL is passed, the 
 *              default multicast interface will be used. (OS Dependent)
 * @param source Source Address to accept transmissions from (non-NULL 
 *               implies Source-Specific Multicast)
 */
extern ce_int_t 
ce_mcast_leave(ce_socket_t *sock,
		ce_sockaddr_t *addr,
		ce_sockaddr_t *iface,
		ce_sockaddr_t *source);

/**
 * Set the Multicast Time to Live (ttl) for a multicast transmission.
 * @param sock The socket to set the multicast ttl
 * @param ttl Time to live to Assign. 0-255, default=1
 * @remark If the TTL is 0, packets will only be seen by sockets on 
 * the local machine, and only when multicast loopback is enabled.
 */
extern ce_int_t 
ce_mcast_hops(ce_socket_t *sock,
		ce_byte_t ttl);

/**
 * Toggle IP Multicast Loopback
 * @param sock The socket to set multicast loopback
 * @param opt 0=disable, 1=enable
 */
extern ce_int_t 
ce_mcast_loopback(ce_socket_t *sock,
		    ce_byte_t opt);


/**
 * Set the Interface to be used for outgoing Multicast Transmissions.
 * @param sock The socket to set the multicast interface on
 * @param iface Address of the interface to use for Multicast
 */
extern ce_int_t 
ce_mcast_interface(ce_socket_t *sock,
		     ce_sockaddr_t *iface);


#ifdef __cplusplus
}
#endif

#endif  /* CE_NETWORK_IO_H */

