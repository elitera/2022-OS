
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h		声明进程相关结构和宏
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//作为进程表中存放寄存器值的结构
typedef struct s_stackframe {/* proc_ptr points here	↑ Low		*/
	u32	gs;			/* ┓								│			*/
	u32	fs;			/* ┃								│			*/
	u32	es;			/* ┃								│			*/
	u32	ds;			/* ┃								│			*/
	u32	edi;		/* ┃								│			*/
	u32	esi;		/* ┣ pushed by save() 中断处理程序	 │			 */
	u32	ebp;		/* ┃								│			*/
	u32	kernel_esp;	/* <- 'popad' will ignore it		│			*/
	u32	ebx;		/* ┃								↑栈从高地址往低地址增长*/		
	u32	edx;		/* ┃								│			*/
	u32	ecx;		/* ┃								│			*/
	u32	eax;		/* ┛								│			*/
	u32	retaddr;	/* return address for assembly code save()	│	*/
	u32	eip;		/*  ┓								│			*/
	u32	cs;			/*  ┃								│			*/
	u32	eflags;		/*  ┣ 由CPU压栈，在ring0->ring1时	 │			 */
	u32	esp;		/*  ┃								│			*/
	u32	ss;			/*  ┛								┷High		*/
}STACK_FRAME;

//进程表结构，开头的regs在栈结构中存放相关寄存器值
typedef struct s_proc {
	STACK_FRAME regs;          /* 进程寄存器保存在stack frame中 */

	u16 ldt_sel;               /* GDT的段选择子：LDT的基址和段界限，gdt selector giving ldt base and limit */
	DESCRIPTOR ldts[LDT_SIZE]; /* LDT局部描述符表，存放进程私有的段描述符们 */

        int ticks;                 /* 可用的中断次数，每次发生中断就递减 */
        //int priority;			   /* 优先级，固定的值，当所有ticks都变为0后，在把各自的优先数赋值给各自的ticks */
		int wake_ticks;    		   /* 用于定时唤醒，表示预期的醒来时间 */
		int pos;				/* 进程状态，0-非阻塞，1-阻塞 */

	u32 pid;                   /* MM内存中传递的进程id */
	char p_name[16];           /* 进程名字 */
}PROCESS;

//任务结构
typedef struct s_task {
	task_f	initial_eip;		//进程起始地址
	int	stacksize;				//堆栈大小
	char	name[32];
}TASK;

#define SEMAPHORE_LIST_SIZE 32
//信号量定义，包含可用资源值、一个临界区数组和头尾索引
typedef struct s_semaphore
{
	int value;
	PROCESS *list[SEMAPHORE_LIST_SIZE];
	int queue_start;
	int queue_end;
}SEMAPHORE;


/* Number of tasks  任务数量 */ 
#define NR_TASKS	6

/* stacks of tasks */
// #define STACK_SIZE_TESTA	0x8000
// #define STACK_SIZE_TESTB	0x8000
// #define STACK_SIZE_TESTC	0x8000
//六个进程定义
#define STACK_SIZE_A	0x8000
#define STACK_SIZE_B	0x8000
#define STACK_SIZE_C	0x8000
#define STACK_SIZE_D	0x8000
#define STACK_SIZE_E	0x8000
#define STACK_SIZE_F	0x8000

#define STACK_SIZE_TOTAL	(	STACK_SIZE_A + \
								STACK_SIZE_B + \
								STACK_SIZE_C + \
								STACK_SIZE_D + \
								STACK_SIZE_E + \
								STACK_SIZE_F)

