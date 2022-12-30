
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


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

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

PRIVATE int ESC_start;
PRIVATE int ESC_finding;
PRIVATE u8* ESC_start_pos;

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)	//初始化tty对应的console
{
	//初始化ESC
	ESC_start = 0;
	ESC_finding = 0;
	
	//tty_table和console_table对应
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;	//一个字符占两个字节
		disp_pos = 0;
	}
	else {
		//其他tty光标设置未实现，只是打印说明tty变化了
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	//清屏，清除启动时的输出
	clean_console(p_tty->p_console);
	
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);	//当前显示地址的pos

	if(ESC_finding){
		if(ch==27){
			backnomal(p_con);
			ESC_start = !ESC_start;
			ESC_finding = !ESC_finding;
			flush(p_con);
		}
		return;
	}


	switch(ch) {
	case 27:		//esc
		if(!ESC_start){	//第一个ESC
			ESC_start_pos = p_vmem;	//记录esc进入的pos
			ESC_start = !ESC_start;
		}
		else{	//已经读到过一个ESC了
			ESC_start = !ESC_start;  //能进入这个条件的是还没回车就按了第二个ESC
			backnomal(p_con);
			flush(p_con);
		}
		break;
	case '\t':		//tab
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 4) {
			int i;
			for(i=0;i<4;i++){
				*p_vmem++ = '\t';
				*p_vmem++ = DEFAULT_UNSHOW_COLOR;
				p_con->cursor++;
			}
			
		}
		break;
	case '\n':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {	//如果光标位置下一行存在
			if(ESC_start){
				ESC_finding=1;
				findmodel(p_con);
			}
			else {
				u32 newline = p_con->original_addr + SCREEN_WIDTH * 
							((p_con->cursor - p_con->original_addr) /
							SCREEN_WIDTH + 1);
				while(p_con->cursor<newline){	//将当前光标与下一行开头之前全部填充为'\n'
					*p_vmem++ = '\n';
					*p_vmem++ = DEFAULT_UNSHOW_COLOR;
					p_con->cursor++;
				}
			}
			
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {	//如果当前tty中有内容
			if(*(p_vmem-2)=='\t'){	//如果是tab，四个空格一起删除
				int i;
				for(i=0;i<4;i++){
					p_con->cursor--;
					*(p_vmem-2) = ' ';
					*(p_vmem-1) = DEFAULT_CHAR_COLOR;
					p_vmem-=2;
				}
			}
			else if(*(p_vmem-2)=='\n'){	//如果之前有回车，删除到所有回车符之前
				while(*(p_vmem-2)=='\n'){
					p_con->cursor--;
					*(p_vmem-2) = ' ';
					*(p_vmem-1) = DEFAULT_CHAR_COLOR;
					p_vmem-=2;
				}
			}
			else {		//如果只是普通的删除
				p_con->cursor--;
				*(p_vmem-2) = ' ';
				*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			}
		}
		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			if(ESC_start){*p_vmem++ = DEFAULT_REDCHAR_COLOR;}
			else {*p_vmem++ = DEFAULT_CHAR_COLOR;}
			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {	//如果光标超出当前界面，自动滚屏
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)	//p_con更新
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)	//设置光标位置
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)	//设置当前屏幕开始地址
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

/*======================================================================*
			   clean_console
 *======================================================================*/
PUBLIC void clean_console(CONSOLE* p_con){	//console清屏操作
	p_con->cursor = p_con->original_addr;	//光标设置为起始位置
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);	//当前光标所在pos
	while(p_vmem<V_MEM_BASE + (p_con->original_addr + p_con->v_mem_limit) * 2) {
		*p_vmem++ = ' ';
		*p_vmem++ = DEFAULT_CHAR_COLOR;
	}
	set_cursor(p_con->cursor);
}

/*======================================================================*
			   findmodel
 *======================================================================*/
PUBLIC void findmodel(CONSOLE* p_con){	//搜索并标红
	u8* p = (u8*)(V_MEM_BASE + p_con->original_addr * 2);	//起始位置
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);	//当前显示地址的pos
	while(p<ESC_start_pos){
		int same=1;
		int i=0;
		while(ESC_start_pos+i<p_vmem){
			if(*(p+i)!=*(ESC_start_pos+i)){
				same=0;
				break;
			}
			i+=2;
		}
		if(same){
			i=0;
			while(ESC_start_pos+i<p_vmem){
				if(*p!='\t'){
					*(p+1) = DEFAULT_REDCHAR_COLOR;
				}
				p = p+2;
				i+=2;
			}
		}
		else{
			p = p+2;
		}
	}
}

/*======================================================================*
			   backnomal
 *======================================================================*/
PUBLIC void backnomal(CONSOLE* p_con){	//搜索结束，恢复颜色，删除搜索字段
	u8* p = (u8*)(V_MEM_BASE + p_con->original_addr * 2);	//起始位置
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);	//当前显示地址的pos
	while(p<ESC_start_pos){
		if(*p!='\t'&&*p!='\n'){
			*(p+1) = DEFAULT_CHAR_COLOR;
		}
		p = p+2;
	}
	while(p<p_vmem){
		*p++ = ' ';
		*p++ = DEFAULT_CHAR_COLOR;
		p_con->cursor--;
	}
}

