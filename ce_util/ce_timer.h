/*
 *@file: entryTimer.h
 *@brief: TODO
 *
 *@description: TODO
 *
 *@author: zhoujunfeng
 *@version: v4.5.88
 *@created: 2013-7-11 下午05:09:31
 *@comment:
 *@copyrigtht: (C) 2000-2013  Corporation. All rights Reserved
 */

#ifndef ENTRYTIMER_H_
#define ENTRYTIMER_H_


#include "ce_basicdefs.h"
#include "ce_thread_proc.h"

typedef struct timeval timer_type_t;

typedef void *(*timer_trigger_t)(void *);

typedef struct timer_beat_info
{
	void (*beat_register)(pthread_t tid);
	void (*beat)(pthread_t tid);
	void (*beat_unregister)(pthread_t tid);
}timer_beat_t;

typedef struct timer_ds
{
	timer_type_t timer;
	timer_trigger_t timer_trigger;
	void * timer_trigger_param;

	pthread_t tid;

	timer_beat_t beat_info;
} timer_ds_t;


/**
 * @brief 停止定时器的运行
 * @param tid 线程id
 * @return CE_OK:停止成功，RET_FAIL:停止失败
 */
int ce_timer_stop(void * timer);

/**
 * @brief 创建一个线程，在线程中启动定时器运行绑定的触发函数
 * @param timer_trigger 定时器触发函数
 * @param param	定时器触发函数参数
 * @param period 定时器定时时间，单位为秒
 * @return CE_OK:创建成功，RET_FAIL:创建失败
 */
void* ce_timer_start(timer_trigger_t timer_trigger, void * param, int period);

void* ce_timer_start_with_beat(timer_trigger_t timer_trigger,
		void * param,
		int period,
		timer_beat_t *beat_info);


#endif /* ENTRYTIMER_H_ */
