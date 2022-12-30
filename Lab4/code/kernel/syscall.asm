
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm 系统调用
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

; ADD:引入符号
extern milli_sleep
extern disp_color_str
extern p_semaphore
extern v_semaphore

; ADD:定义常量
_NR_get_ticks       	equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_my_milli_seconds    equ 1 ; xx毫秒内不被分配时间片
_NR_my_disp_str         equ 2 ; 打印字符串，接受char*参数
_NR_my_p_opera       	equ 3 ; 信号量P操作，S--，占据资源
_NR_my_v_opera       	equ 4 ; 信号量V操作，S++，释放资源
INT_VECTOR_SYS_CALL 	equ 0x90  ; 中断号，到protect.c中初始化中断门

; 导出符号
global	get_ticks
; ADD:系统调用入口
global	my_milli_seconds
global	my_disp_str
global	my_p_opera
global	my_v_opera
; ADD:内核函数
global sys_my_milli_seconds
global sys_my_disp_str
global sys_my_p_opera
global sys_my_v_opera

bits 32
[section .text]

; ====================================================================
;                              get_ticks 获取时钟中断次数
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              my_milli_seconds xx毫秒内不被分配时间片
; ====================================================================
my_milli_seconds:
	mov	eax, _NR_my_milli_seconds
	push ebx					; 先将ebx内容压栈
	mov ebx, [esp+8]			; 此时的栈：函数参数（从右到左）->返回地址->ebx[栈顶在这]
	int	INT_VECTOR_SYS_CALL		; 系统中断门，此时的ebx中保存有第一个参数
	pop ebx						; 恢复ebx值
	ret							; 函数调用返回

sys_my_milli_seconds:
	push ebx					; 将ebx保存的第一个参数压入栈顶
	call milli_sleep			; 函数调用，会访问压入的参数
	pop ebx						; ebx首先出栈，注意push和pop一一对应
	ret

; ====================================================================
;                              my_disp_str 打印字符串，接受char*参数
; ====================================================================
my_disp_str:
	mov	eax, _NR_my_disp_str
	push ebx					; 先将ebx内容压栈
	push ecx
	mov ebx, [esp+12]			; 此时的栈：函数参数（从右到左）->返回地址->ebx->ecx[栈顶在这]
	mov ecx, [esp+16]
	int	INT_VECTOR_SYS_CALL		; 系统中断门，此时的ebx中保存有参数起始位置
	pop ecx
	pop ebx						; 恢复ebx值
	ret							; 函数调用返回

sys_my_disp_str:
	pusha						; 调用前保存所有寄存器值
	push ecx					; 将ebx保存的第一个参数压入栈顶
	push ebx				; 将ebx保存的第一个参数压入栈顶
	call disp_color_str			; 函数调用，会访问压入的参数
	pop ebx						; ebx首先出栈，注意push和pop一一对应
	pop ecx
	popa						; 调用后恢复所有寄存器值
	ret

; ====================================================================
;                              my_p_opera 信号量P操作，S--，占据资源
; ====================================================================
my_p_opera:
	mov	eax, _NR_my_p_opera
	push ebx					; 先将ebx内容压栈
	mov ebx, [esp+8]			; 此时的栈：函数参数（从右到左）->返回地址->ebx[栈顶在这]
	int	INT_VECTOR_SYS_CALL		; 系统中断门，此时的ebx中保存有第一个参数
	pop ebx						; 恢复ebx值
	ret							; 函数调用返回

sys_my_p_opera:
	push ebx					; 将ebx保存的第一个参数压入栈顶
	call p_semaphore			; 函数调用，会访问压入的参数
	pop ebx						; ebx首先出栈，注意push和pop一一对应
	ret

; ====================================================================
;                              my_v_opera 信号量V操作，S++，释放资源
; ====================================================================
my_v_opera:
	mov	eax, _NR_my_v_opera
	push ebx					; 先将ebx内容压栈
	mov ebx, [esp+8]			; 此时的栈：函数参数（从右到左）->返回地址->ebx[栈顶在这]
	int	INT_VECTOR_SYS_CALL		; 系统中断门，此时的ebx中保存有第一个参数
	pop ebx						; 恢复ebx值
	ret							; 函数调用返回

sys_my_v_opera:
	push ebx					; 将ebx保存的第一个参数压入栈顶
	call v_semaphore			; 函数调用，会访问压入的参数
	pop ebx						; ebx首先出栈，注意push和pop一一对应
	ret