/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            keyboard.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"
#include "keymap.h"

PRIVATE KB_INPUT	kb_in;

PRIVATE	int	code_with_E0;
PRIVATE	int	shift_l;	/* l shift state */
PRIVATE	int	shift_r;	/* r shift state */
PRIVATE	int	alt_l;		/* l alt state	 */
PRIVATE	int	alt_r;		/* r left state	 */
PRIVATE	int	ctrl_l;		/* l ctrl state	 */
PRIVATE	int	ctrl_r;		/* l ctrl state	 */
PRIVATE	int	caps_lock;	/* Caps Lock	 */
PRIVATE	int	num_lock;	/* Num Lock	 */
PRIVATE	int	scroll_lock;	/* Scroll Lock	 */
PRIVATE	int	column;

PRIVATE int	caps_lock;	/* Caps Lock	 */
PRIVATE int	num_lock;	/* Num Lock	 */
PRIVATE int	scroll_lock;	/* Scroll Lock	 */

PRIVATE u8	get_byte_from_kbuf();
PRIVATE void    set_leds();
PRIVATE void    kb_wait();
PRIVATE void    kb_ack();

/*======================================================================*
                            keyboard_handler
 *======================================================================*/
PUBLIC void keyboard_handler(int irq)
{
	//从输入缓冲区中读取扫描码（1字节），KB_DATA=0x60
	u8 scan_code = in_byte(KB_DATA);

	//构建的键盘缓冲区大小：KB_IN_BYTES=32字节
	if (kb_in.count < KB_IN_BYTES) {
		//p_head：指向下一个空闲位置
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
			//如果已经达到缓冲区末尾，则将head指针移到开头
			kb_in.p_head = kb_in.buf;
		}
		//缓冲区内容长度++
		kb_in.count++;
	}
}


/*======================================================================*
                           init_keyboard
*======================================================================*/
PUBLIC void init_keyboard()
{
	//初始化KB_INPUT 键盘输入缓冲区
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	//初始化左右shift、alt、ctrl状态为0
	shift_l	= shift_r = 0;
	alt_l	= alt_r   = 0;
	ctrl_l	= ctrl_r  = 0;

	//初始化大写锁定、小键盘锁定、屏幕锁定的LED状态，0不亮，1亮
	caps_lock   = 0;
	num_lock    = 1;	//大多数时候默认使用小键盘数字
	scroll_lock = 0;
	//设置LED灯
	set_leds();

	//处理键盘中断
	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);/*设定键盘中断处理程序*/
	enable_irq(KEYBOARD_IRQ);                       /*开键盘中断*/
}


/*======================================================================*
                           keyboard_read
*======================================================================*/
PUBLIC void keyboard_read(TTY* p_tty)	//键盘读取并解析的函数
{
	u8	scan_code;	//扫描码，1字节
	int	make;	/*标记是否是makeCode  1: make;  0: break. */

	u32	key = 0;/* 用一个整型来表示一个键。比如，如果 Home 被按下，
			 * 则 key 值将为定义在 keymap.h 中的 'HOME'。
			 * 恰好对应keymap中的数组下标*/
	u32*	keyrow;	/* 指向 keymap[] 的某一行 */

	if(kb_in.count > 0){
		code_with_E0 = 0;	//初始化E0结尾状态，0为不是

		scan_code = get_byte_from_kbuf();	//将从缓冲区中读取一个字节抽象为函数

		/* 下面开始解析扫描码 */
		if (scan_code == 0xE1) {
			//处理E1结尾，Control Pad
			int i;
			u8 pausebrk_scode[] = {0xE1, 0x1D, 0x45,
					       0xE1, 0x9D, 0xC5};
			int is_pausebreak = 1;
			for(i=1;i<6;i++){
				if (get_byte_from_kbuf() != pausebrk_scode[i]) {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak) {
				key = PAUSEBREAK;
			}
		}
		else if (scan_code == 0xE0) {
			//处理E0结尾
			scan_code = get_byte_from_kbuf();

			/* PrintScreen 被按下 */
			if (scan_code == 0x2A) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			/* PrintScreen 被释放 */
			if (scan_code == 0xB7) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			/* 不是PrintScreen, 此时scan_code为0xE0紧跟的那个值. */
			if (key == 0) {
				code_with_E0 = 1;
			}
		}
		if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
			/* 首先判断Make Code 还是 Break Code */
			make = (scan_code & FLAG_BREAK ? 0 : 1);	//make & 0x80 =break

			/* 先定位到 keymap 中的行 */
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];

			column = 0;	//下面开始判断属于第几列（原字符、加了shift、E0结尾）

			int caps = shift_l || shift_r;	//如果前一个是shift被按住
			if (caps_lock) {	//处理大写锁定
				if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
					caps = !caps;	//此时按下shift反而应该小写
				}
			}
			if (caps) {	//大写标记
				column = 1;
			}

			if (code_with_E0) {	//E0结尾标记
				column = 2;
			}

			key = keyrow[column];	//key值为对应的map下标索引

			switch(key) {		//key检查特殊键，如果是break扫描码就把特殊键标记归0了！
			case SHIFT_L:
				shift_l = make;
				break;
			case SHIFT_R:
				shift_r = make;
				break;
			case CTRL_L:
				ctrl_l = make;
				break;
			case CTRL_R:
				ctrl_r = make;
				break;
			case ALT_L:
				alt_l = make;
				break;
			case ALT_R:
				alt_l = make;
				break;
			//三种锁定不需要考虑按住
			case CAPS_LOCK:
				if (make) {	
					caps_lock   = !caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if (make) {
					num_lock    = !num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if (make) {
					scroll_lock = !scroll_lock;
					set_leds();
				}
				break;
			default:
				break;
			}

			if (make) { /* 忽略 Break Code */
				int pad = 0;

				/* 首先处理小键盘 */
				if ((key >= PAD_SLASH) && (key <= PAD_9)) {
					pad = 1;
					switch(key) {
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (num_lock &&
						    (key >= PAD_0) &&
						    (key <= PAD_9)) {
							key = key - PAD_0 + '0';
						}
						else if (num_lock &&
							 (key == PAD_DOT)) {
							key = '.';
						}
						else{
							switch(key) {
							case PAD_HOME:
								key = HOME;
								break;
							case PAD_END:
								key = END;
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}

				//处理特殊键的key值
				key |= shift_l	? FLAG_SHIFT_L	: 0;
				key |= shift_r	? FLAG_SHIFT_R	: 0;
				key |= ctrl_l	? FLAG_CTRL_L	: 0;
				key |= ctrl_r	? FLAG_CTRL_R	: 0;
				key |= alt_l	? FLAG_ALT_L	: 0;
				key |= alt_r	? FLAG_ALT_R	: 0;
				key |= pad      ? FLAG_PAD      : 0;

				//如果都不是，key就是可显示字符

				in_process(p_tty, key);	//在对应的tty下处理读取的字符
			}
		}
	}
}

/*======================================================================*
			    get_byte_from_kbuf
 *======================================================================*/
PRIVATE u8 get_byte_from_kbuf()       /* 从键盘缓冲区中读取下一个字节 */
{
        u8 scan_code;

        while (kb_in.count <= 0) {
			

		}   /* 等待下一个字节到来 */

        disable_int();	//关中断
        scan_code = *(kb_in.p_tail);
        kb_in.p_tail++;
        if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
                kb_in.p_tail = kb_in.buf;
        }
        kb_in.count--;
        enable_int();	//开中断

	return scan_code;
}

/*======================================================================*
				 kb_wait
 *======================================================================*/
PRIVATE void kb_wait()	/* 等待 8042 的输入缓冲区空 */
{
	u8 kb_stat;

	do {
		kb_stat = in_byte(KB_CMD);
	} while (kb_stat & 0x02);
}


/*======================================================================*
				 kb_ack
 *======================================================================*/
PRIVATE void kb_ack()	//等待ACK
{
	u8 kb_read;

	do {
		kb_read = in_byte(KB_DATA);
	} while (kb_read =! KB_ACK);
}

/*======================================================================*
				 set_leds
 *======================================================================*/
PRIVATE void set_leds()	//设置LED
{	
	//LED参数字节的前三位分别是屏幕、数字、大写锁定，调整对齐
	u8 leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

	kb_wait();	//等待8042输入缓冲区为空
	out_byte(KB_DATA, LED_CODE);	//向0x60写0xED命令，以设置LED
	kb_ack();	//接收键盘回复的ACK

	kb_wait();	//等待8042输入缓冲区为空
	out_byte(KB_DATA, leds);	//向0x60写LED参数
	kb_ack();	//等待第二个ACK
}

