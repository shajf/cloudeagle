/*
 * =====================================================================================
 *
 *       Copyright (C) 2014 jianfeng sha
 *
 *       Filename:  proto.h
 *
 *       Description:  
 *
 *       Created:  08/10/14 11:55:45
 *
 *       Author:  jianfeng sha , csp001314@163.com
 *
 * =====================================================================================
 */
#ifndef PROTO_H
#define PROTO_H

typedef struct proto_pool_t              proto_pool_t;
typedef struct proto_t                   proto_t;
typedef struct proto_request_t           proto_request_t;
typedef struct proto_ops_t               proto_ops_t;

typedef enum{
    PROTO_NONE,
    PROTO_HTTP,
    PROTO_MAIL,
    PROTO_FTP,
    PROTO_NUM
}proto_e;


struct proto_pool_t{
    
    ce_pool_t           *pool;
    ce_log_t            *log;
    proto_t             *protos[PROTO_NUM]; 
};

struct proto_proto_t {
    
    ce_pool_t           *pool;
    ce_log_t            *log;
    proto_ops_t         *proto_ops; 
    
    proto_queue_t       *pre_queue;
    proto_queue_t       *post_queue;

};

struct proto_ops_t {

    typedef proto_t                 *(*proto_create)(ce_pool_t *pool,ce_log_t *log);
    typedef void                    (*proto_destroy)(proto_t *proto);
    typedef ce_int_t                (*proto_fetch_data_push)(proto_t *poto,proto_fetch_data_t *data);
    
};

#ifdef __cplusplus 
extern "C" {
#endif /*__cplusplus*/

SECA_EXPORT  proto_pool_t* proto_pool_create(ce_pool_t *pool,ce_log_t *log);

SECA_EXPORT proto_pool_destroy(proto_pool_t *pp);

SECA_EXPORT ce_int_t proto_fetch_data_push(proto_pool_t *pp,proto_fetch_data_t *data);

SECA_EXPORT proto_request_t * proto_request_pop(proto_pool_t *pp,proto_e proto_type);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /* PROTO_PROTO_H */

