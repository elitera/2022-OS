
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c		主程序入口
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
                            PV操作相关信号量全局变量、初始化函数
 *======================================================================*/
SEMAPHORE x,y,z;
SEMAPHORE rmutex,wmutex;
SEMAPHORE max_reader;
int readcount,writercount;
int priority;  //优先级：0-读者优先，1-写者优先
init_semophore(SEMAPHORE* semaphore,int value){
	semaphore->value=value;
	semaphore->queue_start = semaphore->queue_end =0;
}

int time_piece=1000;
int reader_num=0;		//记录读者数量
int flag[5] = {0, 0, 0, 0, 0};  //判断进程状态，0休息，1正在，2等待

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	//作为最后一部分被执行代码
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;	//任务表
	PROCESS*	p_proc		= proc_table;	//进程表
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;	//任务栈
	u16		selector_ldt	= SELECTOR_LDT_FIRST;	//LDTR 16位，对应GDT中LDT描述符的段选择子
	int i;
	for (i = 0; i < NR_TASKS; i++) {		//初始化每一个进程
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;						// pid

		p_proc->ldt_sel = selector_ldt;			// LDTR

		//LDT包含两个描述符，分别初始化为内核代码段和内核数据段
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;		//改变DPL优先级
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;	//改变DPL优先级
		
		//寄存器初始化，除了cs指向LDT中第一个描述符，ds、es、fs、ss都指向LDT中第二个描述符，gs指向显存只是RPL变化
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;
		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		//堆栈从高到低生长
		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	//初始化各进程的优先级，由于是循环读写模式，区分优先级，所以不使用进程优先级
	// proc_table[0].ticks = proc_table[0].priority =  15;
	// proc_table[1].ticks = proc_table[1].priority =  5;
	// proc_table[2].ticks = proc_table[2].priority =  3;

	k_reenter = 0;	//全局中断嵌套次数，因为restart中自减了该变量，所以初始化为0
	ticks = 0;		//全局时钟中断次数

	//初始化各信号量
	init_semophore(&x,1);
	init_semophore(&y,1);
	init_semophore(&z,1);
	init_semophore(&rmutex,1);
	init_semophore(&wmutex,1);
	init_semophore(&max_reader,1);   //同时读书的最大读者数量，1、2、3
	readcount=0;
	writercount=0;
	priority=0;				//优先级：0-读者优先，1-写者优先，2-公平竞争

	//当前进程赋值
	p_proc_ready	= proc_table;

        /* 初始化 8253 PIT */ 
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));	// 中断 10ms发生一次

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	clear_screen();	//清屏函数调用
	milli_delay(2000);		//防止清除输出文本

	restart();		//进程从ring0-ring1

	while(1){}
}

/*======================================================================*
                               六个进程
			B、C、D为读者进程，E、F为写者进程，A 为普通进程
				∗ B阅读消耗2个时间片
				∗ C、D阅读消耗3个时间片
				∗ E写消耗3个时间片
				∗ F写消耗4个时间片
 *======================================================================*/
int count = 1;

void A_Process(){
	my_milli_seconds(time_piece);
	while(1){
		if(count < 21){
			if(count < 10){
				char ch[2];
				ch[0] = count + '0';
				ch[1] = '\0';
				my_disp_str(ch,0x0F);
			}
			else if(count < 20){
				char ch[3];
				ch[0] = '1';
				ch[1] = (count - 10) + '0';
				ch[2] = '\0';
				my_disp_str(ch,0x0F);
			}
			else{
				char ch[3];
				ch[0] = '2';
				ch[1] = '0';
				ch[2] = '\0';
				my_disp_str(ch,0x0F);
			}
			for(int j=0;j<5;j++){
				if(flag[j] == 0){
					my_disp_str(" Z",0x09);
				}
				else if(flag[j] == 1){
					my_disp_str(" O",0x0A);
				}
				else{
					my_disp_str(" X",0x0C);
				}
			}
			my_disp_str("\n",0x0F);
		}
		count++;
		my_milli_seconds(time_piece);
	}
}

void B_Reader(){Reader("B",0x0A,2,0);}

void C_Reader(){Reader("C",0x0C,3,1);}

void D_Reader(){Reader("D",0x0C,3,2);}

void E_Writer(){Writer("E",0x0D,3,3);}

void F_Writer(){Writer("F",0x0E,4,4);}

/*======================================================================*
                               读者进程
 *======================================================================*/
PUBLIC void Reader(char* name,int color,int proc_ticks,int num){
	if(priority==0){		//读者优先
		while(1){
				//开始读
				// my_disp_str(name,color);
				// my_disp_str(" come for reading!   ",color);
			flag[num] = 0;	

			my_p_opera(&rmutex);
			if(readcount==0)my_p_opera(&wmutex);  	//如果之前没有人在读，就将wmutex--
			readcount++;				//读者数++
			flag[num] = 2;
			my_v_opera(&rmutex);
			my_p_opera(&max_reader);	//max_reader信号量控制最多可读人数

				reader_num++;
				//读文件
				// my_disp_str(name,color);
				// my_disp_str(" is reading!   ",color);
				//int t=ticks;
				//while(get_ticks()-t<proc_ticks){}
				flag[num] = 1;
				milli_delay(proc_ticks * time_piece);		//自定义时间片长度
				reader_num--;
			
			my_v_opera(&max_reader);	//当前进程结束读，max_reader++
			my_p_opera(&rmutex);
			readcount--;				//读完成，读者数--
			if(readcount==0) my_v_opera(&wmutex);	//如果没有人在读就释放wmutex++
			my_v_opera(&rmutex);

				//读完成
				// my_disp_str(name,color);
				// my_disp_str(" finish reading!   ",color);
				flag[num] = 0;
			
			// my_milli_seconds(10000);		//饥饿问题
		} 
	}
	else if(priority==1){				//写者优先
		while(1){
				//开始读
				// my_disp_str(name,color);
				// my_disp_str(" come for reading!   ",color);
			flag[num] = 0;

			my_p_opera(&z);
			my_p_opera(&rmutex);
			my_p_opera(&x);
			if(readcount==0) my_p_opera(&wmutex);		//读时不能写，wmutex--
			readcount++;
			flag[num] = 2;
			my_v_opera(&x);
			my_v_opera(&rmutex);
			my_v_opera(&z);
			my_p_opera(&max_reader);	//max_reader信号量控制最多可读人数
			
				reader_num++;
				//读文件
				// my_disp_str(name,color);
				// my_disp_str(" is reading!   ",color);
				//int t=ticks;
				//while(get_ticks()-t<proc_ticks){}
				flag[num] = 1;
				milli_delay(proc_ticks * time_piece);		//自定义时间片长度
				reader_num--;
			
			my_v_opera(&max_reader);	//当前进程结束读，max_reader++
			my_p_opera(&x);
			readcount--;
			if(readcount==0) my_v_opera(&wmutex);		//没有读者，可以写，wmutex++
			my_v_opera(&x);

				//读完成
				flag[num] = 0;
				// my_disp_str(name,color);
				// my_disp_str(" finish reading!   ",color);
		}
	}
	else{
		while (1){
			flag[num] = 0;
			my_p_opera(&x);
			my_p_opera(&rmutex);
			if (readcount==0) my_p_opera(&wmutex);
			readcount++;
			flag[num] = 2;
			my_v_opera(&x);
			my_v_opera(&rmutex);
			my_p_opera(&max_reader);
			flag[num] = 1;
			milli_delay(proc_ticks * time_piece);
			reader_num--;
			my_v_opera(&max_reader);
			my_p_opera(&rmutex);
			readcount--;
			if(readcount==0) my_v_opera(&wmutex);
			my_v_opera(&rmutex);
			flag[num] = 0;
		}
		
	}
	
}

/*======================================================================*
                              写者进程
 *======================================================================*/
PUBLIC void Writer(char* name,int color,int proc_ticks,int num){
	if(priority==0){		//读者优先
		while(1){
				//开始写
				// my_disp_str(name,color);
				// my_disp_str(" come for writing!   ",color);
			flag[num] = 0;
			my_p_opera(&wmutex);
			writercount++;
			flag[num] = 2;

				//写文件
				// my_disp_str(name,color);
				// my_disp_str(" is writing!   ",color);
				//int t=ticks;
				//while(get_ticks()-t<proc_ticks){}
				flag[num] = 1;
				milli_delay(proc_ticks * time_piece);		//自定义时间片长度
			
			my_v_opera(&wmutex);
			writercount--;

				//写完成
				// my_disp_str(name,color);
				// my_disp_str(" finish writing!   ",color);
				flag[num] = 0;
		}
	}
	else if(priority==1){				//写者优先
		while(1){
				//开始写
				// my_disp_str(name,color);
				// my_disp_str(" come for writing!   ",color);
			flag[num] = 0;
			my_p_opera(&y);
			writercount++;
			flag[num] = 2;
			if(writercount==1) my_p_opera(&rmutex);		//如果没有人在写，把读互斥量rmutex--
			my_v_opera(&y);

			my_p_opera(&wmutex);
				//写文件
				// my_disp_str(name,color);
				// my_disp_str(" is writing!   ",color);
				//int t=ticks;
				//while(get_ticks()-t<proc_ticks){}
				flag[num] = 1;
				milli_delay(proc_ticks * time_piece);		//自定义时间片长度
			my_v_opera(&wmutex);

			my_p_opera(&y);
			writercount--;
			if(writercount==0) my_v_opera(&rmutex);
			my_v_opera(&y);

				//写完成
				// my_disp_str(name,color);
				// my_disp_str(" finish writing!   ",color);
				flag[num] = 0;
	
			//my_milli_seconds(10000);		//饥饿问题
		}
	}
	else{
		while(1){
			flag[num] = 0;
			my_p_opera(&x);
			my_p_opera(&wmutex);
			writercount++;
			flag[num] = 2;
			if(writercount==1) my_p_opera(&rmutex);
			flag[num] = 1;
			milli_delay(proc_ticks * time_piece);
			my_v_opera(&wmutex);
			writercount--;
			if(writercount==0) my_v_opera(&rmutex);
			my_v_opera(&x);
			flag[num] = 0;
		}
	}
}

/*======================================================================*
                               清屏函数
 *======================================================================*/
PUBLIC void clear_screen(){
	disp_pos=0;
	for(int i=0;i< 80*25;i++){
		disp_color_str(" ",0x07);
	}
	disp_pos=0;
}