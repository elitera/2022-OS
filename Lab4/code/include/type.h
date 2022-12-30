
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            type.h  声明数据类型
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_ORANGES_TYPE_H_
#define	_ORANGES_TYPE_H_


typedef	unsigned int		u32;
typedef	unsigned short		u16;
typedef	unsigned char		u8;


typedef	void	(*int_handler)	();
typedef	void	(*task_f)	();
typedef	void	(*irq_handler)	(int irq);

typedef void*	system_call;  //无论系统调用何种函数，都不会编译时错误


#endif /* _ORANGES_TYPE_H_ */
