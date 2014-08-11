/*
 *By shajianfeng
 * */
#ifndef LINE_CONF_FILE_H
#define LINE_CONF_FILE_H
#include "ce_basicdefs.h"
#include "ce_file.h"
#include "ce_string.h"
#include "ce_palloc.h"

typedef struct {
    /**< an apr_file_getc()-like function */
    ce_int_t (*getch) (char *ch, void *param);
    /**< an apr_file_gets()-like function */
    ce_int_t (*getstr) (void *buf, size_t bufsiz, void *param);
    /**< a close handler function */
    ce_int_t (*close) (void *param);
    /**< the argument passed to getch/getstr/close */
    void *param;
    /**< the filename / description */
    const char *name;
    /**< current line number, starting at 1 */
    unsigned line_number;
}ce_configfile_t;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern int ce_cfg_closefile(ce_configfile_t *cfp); 

/* Open a ce_configfile_t as FILE, return open ce_configfile_t struct pointer */
extern ce_int_t ce_pcfg_openfile(ce_configfile_t **ret_cfg,ce_pool_t *p, const char *name);

extern ce_configfile_t * ce_pcfg_open_custom(
            ce_pool_t *p, const char *descr, void *param,
            ce_int_t (*getc_func) (char *ch, void *param),
            ce_int_t (*gets_func) (void *buf, size_t bufsize, void *param),
            ce_int_t (*close_func) (void *param));

/* Read one character from a configfile_t */
extern ce_int_t ce_cfg_getc(char *ch, ce_configfile_t *cfp);

extern ce_int_t ce_cfg_getline(char *buf, size_t bufsize,
                                        ce_configfile_t *cfp);

#include "ce_varbuf.h"

extern ce_int_t ce_varbuf_cfg_getline(struct ce_varbuf *vb,
                                          ce_configfile_t *cfg, 
                                          size_t max_len);


#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*LINE_CONF_FILE_H*/
