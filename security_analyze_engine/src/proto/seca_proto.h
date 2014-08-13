/*
 * =====================================================================================
 *
 *       Copyright (C) 2014 jianfeng sha
 *
 *       Filename:  seca_proto.h
 *
 *       Description:  
 *
 *       Created:  08/10/14 11:55:45
 *
 *       Author:  jianfeng sha , csp001314@163.com
 *
 * =====================================================================================
 */
#ifndef SECA_PROTO_H
#define SECA_PROTO_H

typedef struct seca_proto_pool_t        seca_proto_pool_t;
typedef struct seca_proto_t             seca_proto_t;
typedef struct seca_proto_request_t     seca_proto_request_t;
typedef struct seca_proto_ops_t         seca_proto_ops_t;


typedef enum{
    PROTO_NONE,
    PROTO_HTTP,
    PROTO_MAIL,
    PROTO_FTP,
    PROTO_NUM
}proto_e;


struct seca_proto_pool_t{
    
    ce_pool_t           *pool;
    ce_log_t            *log;
    seca_proto_t *protos[PROTO_NUM]; 
};

struct seca_proto_t {
    
    ce_pool_t           *pool;
    ce_log_t            *log;
    seca_proto_ops_t    *proto_ops; 
    
    seca_proto_queue_t  *pre_queue;
    seca_proto_queue_t  *post_queue;

};

struct seca_proto_ops_t {

    typedef seca_proto_t           *(*proto_create)(ce_pool_t *pool,ce_log_t *log);
    typedef void                    (*proto_destroy)(seca_proto_t *proto);
    typedef ce_int_t                (*proto_fetch_data_push)(seca_proto_t *poto,seca_fetch_data_t *data);
    
};

#ifdef __cplusplus 
extern "C" {
#endif /*__cplusplus*/

SECA_EXPORT seca_proto_pool_t* seca_proto_pool_create(ce_pool_t *pool,ce_log_t *log);

SECA_EXPORT seca_proto_pool_destroy(seca_proto_pool_t *pp);

SECA_EXPORT ce_int_t seca_proto_fetch_data_push(seca_proto_pool_t *pp,seca_fetch_data_t *data);

SECA_EXPORT seca_proto_request_t * seca_proto_request_pop(seca_proto_pool_t *pp,proto_t proto_type);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /* SECA_PROTO_H */

