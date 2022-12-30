
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c	时钟相关函数
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
                           clock_handler	时钟中断处理程序
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	//每次时钟中断时监测屏幕是否占满，刷新屏幕
	if(disp_pos>80*50)clear_screen();
	
	ticks++;		//全局时钟中断次数++
	// p_proc_ready->ticks--;
	
	if (k_reenter != 0) {	//发生了中断重入，直接返回
		return;
	}
	
	// if (p_proc_ready->ticks > 0) {		//如果当前进程ticks还没变成0，其他进程就不能进入！
	// 	return;
	// }

	schedule();

}

/*======================================================================*
                              milli_delay	毫秒级延迟函数
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();
		//中断10ms发生一次，所以ticks也是10ms增加一次
        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {
			//参数milli_sec指的是过去了多少ms，10ms一个ticks（大概）
		}
}

