
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
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
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

PRIVATE int ESCModel;

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	//键盘被看作是TTY的一部分，所以在这里初始化
	init_keyboard();

	//初始化所有TTY
	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}

	//默认选择第一个console
	select_console(0);
	while (1) {
  

		//每个tty做读取和打印
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			if(FLAG && !ESCModel){
				FLAG=!FLAG;
				clean_console(p_tty->p_console);
			}
			tty_do_read(p_tty);		//tty_do_read->keyboard_read->in_process
			tty_do_write(p_tty);	//tty_do_write->out_char
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	//初始化ESCModel
	ESCModel = 0;
	//初始化缓冲区
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
	//指定对应的console
	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        //一个字符两个字节，低地址为内容，高地址为颜色
		char output[2] = {'\0', '\0'};

        if (!(key & FLAG_EXT)) {
			put_key(p_tty, key);	//打印型字符，直接存入缓冲区
        }
        else {
			int raw_code = key & MASK_RAW;	//不可打印字符对应的十进制数字
			switch(raw_code) {	//识别不同的功能型字符并存入缓冲区
			case ESC:
				put_key(p_tty,27);	//esc键的ascii码为27
				ESCModel=!ESCModel;	//ESCModel翻转
				break;
			case TAB:
				put_key(p_tty,'\t');
				break;
			case ENTER:
				put_key(p_tty, '\n');
				break;
			case BACKSPACE:
				put_key(p_tty, '\b');
				break;
			case UP:
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_DN);
				}
				break;
			case DOWN:
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_UP);
				}
				break;

			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
			case F8:
			case F9:
			case F10:
			case F11:
			case F12:
				/* Alt + F1~F12 */
				if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
					select_console(raw_code - F1);
				}
				break;
			//未实现pad小键盘上功能……
			default:
					break;
			}
        }
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)	//将key存入tty缓冲区
{
	//类似kb_in键盘输入缓冲区存储操作
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	//多个tty共用一个keyboard，所以需要判断是否是当前控制台
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)	//处理tty缓冲区中要显示的内容
{
	//类似kb_in键盘输入缓冲区读取操作
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;
		//打印相关字符
		out_char(p_tty->p_console, ch);
	}
}


