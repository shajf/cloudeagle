/*
 *@file: entryTimer.c
 *@brief: TODO
 *
 *@description: TODO
 *
 *@author: zhoujunfeng
 *@version: v4.5.88
 *@created: 2013-7-11 下午05:10:11
 *@comment:
 *@copyrigtht: (C) 2000-2013  Corporation. All rights Reserved
 */
#include "ce_timer.h"

static void timer_thrd_cleanup(void * arg)
{
	timer_ds_t * timer_ds = (timer_ds_t*)arg;

	/*取消注册*/
	if (timer_ds->beat_info.beat_unregister != NULL)
	{
		timer_ds->beat_info.beat_unregister(timer_ds->tid);
	}
}

/**
 * @brief run_timer的包裹函数
 * @param arg 和run_timer的参数相同
 */
static void * ce_run_timer_wrap(void * arg)
{
	timer_ds_t * timer_ds = (timer_ds_t*) arg;
	pthread_t tid = pthread_self();

	int err;
	struct timeval timer_val;

	timer_val.tv_sec = timer_ds->timer.tv_sec;
	timer_val.tv_usec = timer_ds->timer.tv_usec;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //允许退出线程
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置立即取消

	if (timer_ds->beat_info.beat_register != NULL)
	{
		timer_ds->beat_info.beat_register(tid);
	}

	/// 注册线程退出清理函数
	pthread_cleanup_push(timer_thrd_cleanup, (void*)timer_ds);

	while (1)
	{
		if (timer_ds->beat_info.beat != NULL)
		{
			timer_ds->beat_info.beat(tid);
		}

		/// 脑子抽筋啊?
		err = select(0, NULL, NULL, NULL, &timer_val);
		if (0 > err)
		{
			fprintf(stderr, "select: %s \n", strerror(err));
			free(timer_ds);
			break;
		}

		timer_ds->timer_trigger(timer_ds->timer_trigger_param);
		timer_val.tv_sec = timer_ds->timer.tv_sec;
		timer_val.tv_usec = timer_ds->timer.tv_usec;
	}

	/*以return方式返回时，需要手动pop*/
	pthread_cleanup_pop(1);
	return 	NULL;
}

/**
 * @brief 设置定时器时间
 * @param seconds 定时器设置秒
 * @param mseconds 定时器设置微妙
 * @param timer 定时器
 */
static void ce_timer_set(int seconds, int mseconds, timer_type_t* timer)
{
	timer->tv_sec = seconds;
	timer->tv_usec = mseconds;
}

/**
 * @brief 绑定定时器触发函数，定时器定时运行绑定的触发函数
 * @param timer 定时器
 * @param timer_trigger 定时器绑定的触发函数
 * @param timer_trigger_param 触发函数参数
 * @param timer_ds 定时器及触发函数的绑定结构体
 */
static void ce_timer_bind(timer_type_t * timer, timer_trigger_t timer_trigger,
                void * timer_trigger_param, timer_ds_t *timer_ds)
{
	memcpy(&(timer_ds->timer), timer, sizeof(timer_type_t));
	
	timer_ds->timer_trigger = timer_trigger;
	timer_ds->timer_trigger_param = (void *) timer_trigger_param;
}

/**
 * @brief 创建一个线程运行定时器绑定的触发函数
 * @param timer_ds 定时器及触发函数的绑定结构体
 * @param tid 线程id
 * @return CE_OK:创建成功，RET_FAIL:创建失败
 */
static int ce_timer_run(timer_ds_t * timer_ds, pthread_t * tid)
{
	int err;
	int ret_val = CE_OK;
	pthread_t new_thread;
	pthread_attr_t new_attr;
	pthread_attr_init(&new_attr);

	pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_JOINABLE);
	err = pthread_create(&new_thread, &new_attr, ce_run_timer_wrap,
	        (void *) timer_ds);
	pthread_attr_destroy(&new_attr);
	*tid = new_thread;
	if (0 != err)
	{
		fprintf(stderr, "pthread_create: %s \n", strerror(err));
		ret_val = CE_ERROR;
	}
	return ret_val;
}

/**
 * @brief 停止定时器的运行
 * @param tid 线程id
 * @return CE_OK:停止成功，RET_FAIL:停止失败
 */
int ce_timer_stop(void * timer)
{
	int err;
	timer_ds_t * timer_ds = (timer_ds_t *)timer;

	if (NULL == timer_ds)
	{
		fprintf(stderr, "timer is NULL when stop.");
		return CE_ERROR;
	}

	err = pthread_cancel(timer_ds->tid);
	if (0 != err)
	{
		fprintf(stderr, "pthread_cancel: %s \n", strerror(err));
		return CE_ERROR;
	}

	err = pthread_join(timer_ds->tid, NULL);
	if (0 != err)
	{
		fprintf(stderr, "pthread_join: %s \n", strerror(err));
		return CE_ERROR;
	}

	free(timer_ds);
	return CE_OK;
}

/**
 * @brief 创建一个线程，在线程中启动定时器运行绑定的触发函数
 * @param timer_trigger 定时器触发函数
 * @param param	定时器触发函数参数
 * @param period 定时器定时时间，单位为秒
 * @return CE_OK:创建成功，RET_FAIL:创建失败
 */
void* ce_timer_start(timer_trigger_t timer_trigger, void * param, int period)
{
	pthread_t tid = 0;
	int ret_val;
	timer_type_t timer;

	timer_ds_t * timer_ds = (timer_ds_t*)ce_alloc(sizeof(timer_ds_t));
	if (NULL == timer_ds)
	{		
		return NULL;
	}

	memset(timer_ds, 0, sizeof(timer_ds_t));

	ce_timer_set(period, 0, &timer);

	ce_timer_bind(&timer, timer_trigger, param, timer_ds);

	ret_val = ce_timer_run(timer_ds, &tid);
	if (CE_ERROR == ret_val)
	{		
		free(timer_ds);
		return NULL;
	}

	timer_ds->tid = tid;
	return timer_ds;
}


/**
 * @brief 创建一个线程，在线程中启动定时器运行绑定的触发函数
 * @param timer_trigger 定时器触发函数
 * @param param	定时器触发函数参数
 * @param period 定时器定时时间，单位为秒
 * @return CE_OK:创建成功，RET_FAIL:创建失败
 */
void* ce_timer_start_with_beat(timer_trigger_t timer_trigger,
		void * param,
		int period,
		timer_beat_t *beat_info)
{
	pthread_t tid = 0;
	int ret_val;
	timer_type_t timer;

	timer_ds_t * timer_ds = (timer_ds_t*)ce_alloc(sizeof(timer_ds_t));
	if (NULL == timer_ds)
	{
		return NULL;
	}

	memset(timer_ds, 0, sizeof(timer_ds_t));

	ce_timer_set(period, 0, &timer);

	ce_timer_bind(&timer, timer_trigger, param, timer_ds);
	memcpy(&timer_ds->beat_info, beat_info, sizeof(timer_ds->beat_info));

	ret_val = ce_timer_run(timer_ds, &tid);
	if (CE_ERROR == ret_val)
	{
		free(timer_ds);
		return NULL;
	}

	timer_ds->tid = tid;
	return timer_ds;
}

