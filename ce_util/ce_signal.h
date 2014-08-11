/*
 * =====================================================================================
 *
 *       Filename:  ce_signal.h
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

#ifndef CE_SIGNAL_H
#define CE_SIGNAL_H

/**
 * @file ce_signal.h
 * @brief CE Signal Handling
 */

#include "ce_basicdefs.h"
#include "ce_palloc.h"

/* @defgroup ce_signal Signal Handling
 */


#undef SIG_DFL
#undef SIG_IGN
#undef SIG_ERR
#define SIG_DFL (void (*)(int))0
#define SIG_IGN (void (*)(int))1
#define SIG_ERR (void (*)(int))-1


/** Function prototype for signal handlers */
typedef void ce_sigfunc_t(int);

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**
 * Set the signal handler function for a given signal
 * @param signo The signal (eg... SIGWINCH)
 * @param func the function to get called
 */
extern ce_sigfunc_t * ce_signal(int signo, ce_sigfunc_t * func);

/**
 * Get the description for a specific signal number
 * @param signum The signal number
 * @return The description of the signal
 */
extern const char * ce_signal_description_get(int signum);

/**
 * CE-private function for initializing the signal package
 * @internal
 * @param pglobal The internal, global pool
 */
extern void ce_signal_init(ce_pool_t *pglobal);

/**
 * Block the delivery of a particular signal
 * @param signum The signal number
 * @return status
 */
extern  ce_int_t ce_signal_block(int signum);

/**
 * Enable the delivery of a particular signal
 * @param signum The signal number
 * @return status
 */
extern  ce_int_t ce_signal_unblock(int signum);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_SIGNAL_H */
