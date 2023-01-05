
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c 进程相关函数
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

/*======================================================================*
                              schedule	进程调度函数
 *======================================================================*/
PUBLIC void schedule()
{
	//时间片轮转调度，如果下一进程睡眠或者处于阻塞状态就跳过，向后轮询
	while (1){
		int t = get_ticks();	//当前总中断次数
		p_proc_ready++;			//切换至下一个进程
		//如果进程超出则指向进程表的第一个
		if (p_proc_ready >= proc_table + NR_TASKS)p_proc_ready = proc_table;
		//判断进程是否处于阻塞状态或睡眠状态
		if (p_proc_ready->pos == 0 && p_proc_ready->wake_ticks <= t) break;
	}
	//输出当前获取时间片的进程名，验证my_milli_seconds禁止了时间片分配
	//disp_str(p_proc_ready->p_name);
}

/*======================================================================*
					sys_get_ticks 通过系统调用访问时钟中断次数
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
					milli_sleep xx毫秒内不被分配时间片
 *======================================================================*/
PUBLIC void milli_sleep(int milli_sec){
		//设置当前进程的wake_ticks
		p_proc_ready->wake_ticks=(milli_sec * HZ /1000) + get_ticks();
		//当前进程休眠，调度下一进程
		schedule();
}


/*======================================================================*
                           p_semaphore 信号量P操作，S--，占据资源
 *======================================================================*/
PUBLIC void p_semaphore(SEMAPHORE* semaphore){
	//信号量--，占用一份资源
	semaphore->value--;
	//如果信号量小于0，则发生阻塞，将当前进程放入临界区
	if (semaphore->value < 0) {
		p_proc_ready->pos = 1;	//处于阻塞状态
		semaphore->list[semaphore->queue_end] = p_proc_ready;
		semaphore->queue_end = (semaphore->queue_end + 1) % SEMAPHORE_LIST_SIZE;	//更新临界区尾部
		schedule();		//轮询下一进程
	}

}

/*======================================================================*
                           v_semaphore 信号量V操作，S++，释放资源
 *======================================================================*/
PUBLIC void v_semaphore(SEMAPHORE* semaphore){
	//信号量++，释放一份资源
	semaphore->value++;
	//如果信号量不大于0，说明临界区不为空，释放一个进程
	if (semaphore->value <= 0) {
		PROCESS* p=semaphore->list[semaphore->queue_start];
		p->pos = 0;
		semaphore->queue_start = (semaphore->queue_start + 1) % SEMAPHORE_LIST_SIZE;	//更新临界区头
	}
}


