
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h  函数声明
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
//  ADD:
void B_Reader();
void C_Reader();
void D_Reader();
void E_Writer();
void F_Writer();
void A_Process();
void Reader();
void Writer();
void clear_screen();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);


/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
//ADD
PUBLIC  void    milli_sleep(int milli_sec);
PUBLIC  void    p_semaphore(SEMAPHORE* semaphore);
PUBLIC  void    v_semaphore(SEMAPHORE* semaphore);


/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
//ADD
PUBLIC  void    my_milli_seconds(int milli_sec);
PUBLIC  void    sys_my_milli_seconds(int milli_sec);
PUBLIC  void    my_disp_str(char* str,int color);
PUBLIC  void    sys_my_disp_str(char* str,int color);
PUBLIC  void    my_p_opera(SEMAPHORE* semaphore);
PUBLIC  void    sys_my_p_opera(SEMAPHORE* semaphore);
PUBLIC  void    my_v_opera(SEMAPHORE* semaphore);
PUBLIC  void    sys_my_v_opera(SEMAPHORE* semaphore);
