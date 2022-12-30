; 有初始值的变量
section .data
	string: db "Please input x+y or x*y:",0Ah,0h
	stringLength: equ $-string
	newline: db "", 0Ah,0h

	invalid: db "Invalid",0Ah,0h
	invalidLength: equ $-invalid


; --------------------------------------
; 无初始值的变量
section .bss
	num1: resw 20 
	num2: resw 20 
	len1: resd 1 ; num1长度
	len2: resd 1 ; num2长度
	index1: resd 1 ; num1 index
	index2: resd 1 ; num2 index

	digit: resb 255 ; 数字（十进制）

	addend1Address: resd 1 ; 加数1地址
	addend2Address: resd 1 ; 加数2地址
	addend1Index: resd 1 ; 加数1 index
	addend2Index: resd 1 ; 加数2 index
	addResult: resw 40 ; 加法结果
	addResultIndex: resd 1 ; 加法结果 index

	mulResultIndex: resd 1 ; 乘法结果位数
	mulOneDigit: resb 1 ; 一位乘数（十进制）
	tempMulDigit: resw 40 ; 多位x一位乘法结果
	mulDigit: resw 40 ; 多位x一位乘法结果（逆向）
	mulResult: resw 40 ; 乘法结果

	mulCarrier: resd 1 ; 乘法进位器
	mulIndexFrom0: resd 1 ; 从0开始的乘数index
	mulIndex: resd 1 ; 乘数index
	tempIndex: resd 1 ; add 0 to mulResult
	cntZero: resd 1 ; 0的个数
	nowLength: resd 1 
	fillZeroCnt: resd 1  ; 填充的0的个数

	printsize: resd 1
	printindex: resd 1
	print: resd 1

; --------------------------------------
; 代码区
section .text
global _start:

_start:
	; 打印提示信息"Please input x and y:"
	mov eax, 4 ; write系统调用号：4
	mov ebx, 1 ; 标准输出
	mov ecx, string ; 字符串
	mov edx, stringLength ; 字符串长度
	int 80h

cal:
    call getNumbers
    jmp cal
; --------------------------------------
; 系统终止
fin:
	mov eax, 1 ; SYS_EXIT的系统调用号：1
    mov ebx, 0 ; 0传递表示 No Error
    int 80h ; 触发OS中断
    
; --------------------------------------
; 处理异常
invalids:
    ; 打印提示信息"Invalid"
	mov eax, 4 ; write系统调用号：4
	mov ebx, 1 ; 标准输出
	mov ecx, invalid; 字符串
	mov edx, invalidLength ; 字符串长度
	int 80h

    flush:
        mov eax, 3 ; read系统调用号：3
        mov ebx, 0 ; 标准输入
        mov ecx, digit ; 输入存放在digit地址下（1byte）
        mov edx, 1 ; 读入1字节长度
        int 80h
        cmp byte[digit], 0Ah ; 读入回车符则结束
        je getNumberFinished ; 条件判断，跳出循环，恢复并RET
        jmp flush
; --------------------------------------
; 处理异常
getNumbers:
	pusha ; 全部压栈，保存寄存器状态
	
	initNum1:
		mov dword[len1], 0 ; 清空len1
		mov byte[digit], 0 ; 清空digit

	getNum1:
		mov eax, 3 ; read系统调用号：3
		mov ebx, 0 ; 标准输入
		mov ecx, digit ; 输入存放在digit地址下（1byte）
		mov edx, 1 ; 读入1字节长度
		int 80h

		cmp byte[digit], '+' ; 读入空格则结束
		je initNum2 ; 条件判断，跳出循环，读取num2

        cmp byte[digit], '*' ; 读入空格则结束
		je initNum3 ; 条件判断，跳出循环，读取num2

		cmp byte[digit], 'q' ; 读入q
		je fin ; 条件判断，跳出循环，读取num2

        cmp byte[digit], 48
        jl invalids

        cmp byte[digit], 57
        jg invalids

		mov eax, num1 ; eax存放num1起地址
		add eax, dword[len1] ; eax = num1起地址 + len1值（偏移量） 这里用len1表示当前digit在num1中的第几位
		mov dl, byte[digit] ; 将digit值存入dl（edx-dx-dh-dl）
		mov byte[eax], dl ; 将digit值存入num1对应位置
		inc dword[len1] ; len1++

		jmp getNum1 ; 循环读取每一位

initNum2:
    mov dword[len2], 0 ; 清空len2
    mov byte[digit], 0 ; 清空digit
    mov esi, 0 ;计数

getNum2:
    mov eax, 3 ; read系统调用号：3
    mov ebx, 0 ; 标准输入
    mov ecx, digit ; 输入存放在digit地址下（1byte）
    mov edx, 1 ; 读入1字节长度
    int 80h

    cmp byte[digit], 0Ah ; 读入回车符则结束
    je ad ; 条件判断，跳出循环，恢复并RET

    mov eax, num2 ; eax存放num1起地址
    add eax, dword[len2] ; 当前digit在num2中的第几位（根据len2进行偏移）
    mov dl, byte[digit] ; 将digit值存入dl
    mov byte[eax], dl ; 将digit值存入num1对应位置
    inc dword[len2] ; len2++
    inc esi

    jmp getNum2 ; 循环读取每一位

initNum3:
    mov dword[len2], 0 ; 清空len2
    mov byte[digit], 0 ; 清空digit
    mov esi, 0 ;计数

getNum3:
    mov eax, 3 ; read系统调用号：3
    mov ebx, 0 ; 标准输入
    mov ecx, digit ; 输入存放在digit地址下（1byte）
    mov edx, 1 ; 读入1字节长度
    int 80h

    cmp byte[digit], 0Ah ; 读入回车符则结束
    je mu ; 条件判断，跳出循环，恢复并RET

    mov eax, num2 ; eax存放num1起地址
    add eax, dword[len2] ; 当前digit在num2中的第几位（根据len2进行偏移）
    mov dl, byte[digit] ; 将digit值存入dl
    mov byte[eax], dl ; 将digit值存入num1对应位置
    inc dword[len2] ; len2++
    inc esi

    jmp getNum3 ; 循环读取每一位


getNumberFinished:
	popa ; 恢复所有寄存器的值
	ret ; 恢复返回地址，返回调用处

; --------------------------------------
; 加法执行
ad:
    cmp esi, 0
    je invalids
	pusha
	mov dword[addend1Address], num1 ; 存入num1地址
	mov dword[addend2Address], num2 ; 存入num2地址
	mov eax, dword[len1] ; num1位数
	mov dword[addend1Index], eax ; 存入num1位数
	mov eax, dword[len2] ; num2位数
	mov dword[addend2Index], eax ; 存入num2位数

	call addition ; 调用addition处理加法

	mov dword[addend1Address], 0
	mov dword[addend2Address], 0
	mov dword[addend1Index], 0
	mov dword[addend2Index], 0
	popa

	; 加法结果输出
	pusha
	mov eax, addResult ; eax存放addResult（此时还是逆序）
	mov ebx, dword[addResultIndex]
	mov dword[printsize], ebx ; printsize存放addResult位数
	call reverseOutput ; 调用reverseOutput逆向输出addResult
	popa
    jmp getNumberFinished

; --------------------------------------
; 乘法执行
mu:
    cmp esi, 0
    je invalids
	call multiply ; 调用multiply处理乘法

	pusha
	mov eax, mulResult
	mov ebx, dword[nowLength]
	mov dword[printsize], ebx
	call strOutput
	popa
    jmp getNumberFinished

; --------------------------------------
; 加法函数
addition:
	mov ebx, 0 ; 是否需要进位的flag

	; index1存放num1的index（从低位开始）
	mov eax, dword[addend1Index]
	dec eax
	mov dword[index1], eax

	; index2存放num2的index（从低位开始）
	mov eax, dword[addend2Index]
	dec eax
	mov dword[index2], eax

	; addResultIndex为加法结果的位数
	mov dword[addResultIndex], 0

additionloop:
	; 如果index1先到-1，num2比较长
	cmp dword[index1], -1
	je index2Longer

	; 如果index2先到-1，num1比较长
	cmp dword[index2], -1
	je index1Longer

	; 将num1的index1位存放到dl中
	mov eax, dword[addend1Address]
	add eax, dword[index1]
	mov dl, byte[eax]
	sub dl, 48 ; ascii码，char-'0'=实际的数字

	; 将num2的index2位存放到dh中
	mov eax, dword[addend2Address]
	add eax, dword[index2]
	mov dh, byte[eax]
	sub dh, 48 ; ascii码，char-'0'=实际的数字

	; 是否需要进位的flag
	cmp ebx, 0
	je carry

	; 高位+1，ebx归0
	inc dh
	mov ebx, 0

	carry:
		; add，判断是否需要进位
		add dh, dl
		cmp dh, 10
		jl reserveAddResultByByte

		; 和大于10，ebx置为1，dh-10
		mov ebx, 1
		sub dh, 10

	reserveAddResultByByte:
		add dh, 48 ; ascii码，num+'0'=char

		; 对应位的和存入结果（从高位开始），所以最后需要倒转结果
		mov eax, addResult
		add eax, dword[addResultIndex]
		mov byte[eax], dh
		inc dword[addResultIndex] ; addResultIndex++

		; cnt各前移一位，继续加法循环
		dec dword[index1]
		dec dword[index2]
		jmp additionloop

	index1Longer:
		; index1=-1，加法结束
		cmp dword[index1], -1
		je additionFinished

		; num1比较长
		mov eax, dword[addend1Address]
		add eax, dword[index1]
		mov dl, byte[eax]
		
		; 无进位，直接将剩下位写入result
		cmp ebx, 0
		je index1LongerReserveAddResultByByte
		; 有进位，处理进位，dl++
		mov ebx, 0
		inc dl

		; 不产生新进位
		cmp dl, 58 ; ascii 码比较（只有+1可能所以不需要转换为数字本身了）
		jl index1LongerReserveAddResultByByte
		; 产生新进位
		sub dl, 10
		mov ebx, 1

		index1LongerReserveAddResultByByte:
			; 对应位的和存入addResult
			mov eax, addResult
			add eax, dword[addResultIndex]
			mov byte[eax], dl
			inc dword[addResultIndex]

			; index1前移一位，继续index1Longer循环
			dec dword[index1]
			jmp index1Longer

	index2Longer:
		; index2=-1，加法结束
		cmp dword[index2], -1
		je additionFinished

		; num2比较长
		mov eax, dword[addend2Address]
		add eax, dword[index2]
		mov dl, byte[eax]

		; 无进位，直接将剩下位写入result
		cmp ebx, 0
		je index2LongerReserveAddResultByByte
		; 有进位，处理进位，dl++
		mov ebx, 0
		inc dl

		; 不产生新进位
		cmp dl, 58 ; ascii 码比较（只有+1可能所以不需要转换为数字本身了）
		jl index2LongerReserveAddResultByByte
		; 产生新进位
		sub dl, 10
		mov ebx, 1

		index2LongerReserveAddResultByByte:
			; 对应位的和存入addResult
			mov eax, addResult
			add eax, dword[addResultIndex]
			mov byte[eax], dl
			inc dword[addResultIndex]

			; index2前移一位，继续index2Longer循环
			dec dword[index2]
			jmp index2Longer

additionFinished:
	; 是否有最高位进位
	cmp ebx, 0
	je notOverflow

	; 最高位进位1
	mov eax, addResult
	add eax, dword[addResultIndex]
	mov byte[eax], 31h ; 16进制，31h=49='1'
	inc dword[addResultIndex]

	notOverflow:
		mov dword[addend1Address], 0
		mov dword[addend2Address], 0
		ret

; --------------------------------------
; 乘法函数
multiply:
	pusha

	; mulIndex保存乘数位数
	mov eax, dword[len2] ;
	mov dword[mulIndex], eax
	mov dword[mulIndexFrom0], 0
	mov dword[tempIndex], 0
	mov dword[mulResult], 0
	mov dword[nowLength], 0

	; 变量初始化全部填充0
	pusha
	mov eax, mulResult
	mov ebx, 40
	call fillZero
	popa

	pusha
	mov eax, addResult
	mov ebx, 40
	call fillZero
	popa

	pusha
	mov eax, mulDigit
	mov ebx, 40
	call fillZero
	popa

	pusha
	mov eax, tempMulDigit
	mov ebx, 40
	call fillZero
	popa

	multiplyLoop:
		; 乘数位数遍历完后结束乘法，跳出循环
		cmp dword[mulIndex], 0
		je multiplyFinished

		; ebx指向乘数最低位
		mov ebx, num2
		add ebx, dword[mulIndex]
		dec ebx

		; 一位乘数的乘法
		mov ecx, dword[ebx]
		sub ecx, 48
		mov byte[mulOneDigit], cl ;ecx的低8位，1byte
		call multiplyOneDigit ; 多位 x 一位乘法函数调用
		
		; 反转一位乘法的结果，存放于tempMulDigit中
		mov eax, 0
		reverseOneDigit:
			; 需要反转的长度=mulResultIndex
			cmp eax, dword[mulResultIndex]
			je mulAddition

			; ebx从头向后（mulDigit）
			mov ebx, mulDigit
			add ebx, eax
			; ecx从后向头（tempMulDigit）
			mov ecx, tempMulDigit
			add ecx, dword[mulResultIndex]
			dec ecx
			sub ecx, eax
			; 通过dl传递每一位的值
			mov dl, byte[ebx]
			mov byte[ecx], dl

			; eax++，循环继续
			inc eax
			jmp reverseOneDigit

		mulAddition:
			; tempMulDigit+mulResult
			pusha
			mov dword[addend1Address], tempMulDigit
			mov dword[addend2Address], mulResult
			mov eax, dword[mulResultIndex]
			mov dword[addend1Index], eax
			mov eax, dword[nowLength]
			mov dword[addend2Index], eax

			call addition ; 加法调用

			mov dword[addend1Address], 0
			mov dword[addend2Address], 0
			mov dword[addend1Index], 0
			mov dword[addend2Index], 0
			popa

			; 加法结果的长度存入nowLength和tempIndex
			mov eax, dword[addResultIndex]
			mov dword[nowLength], eax
			mov dword[tempIndex], eax
			
		refreshMulResult:
			; addResult替换到mulResult，遍历addResult
			cmp dword[tempIndex], -1
			je refreshMulResultFinished

			; eax存放mulResult偏移量（从前到后）
			mov eax, mulResult 
			sub eax, dword[tempIndex]
			add eax, dword[nowLength]
			sub eax, 1
			; ebx存放addResult偏移量（从后到前）
			mov ebx, addResult
			add ebx, dword[tempIndex]
			; cl做中转，传递每一位
			mov cl, byte[ebx]
			mov byte[eax], cl

			; tempIndex--，继续遍历addResult
			dec dword[tempIndex]
			jmp refreshMulResult

		refreshMulResultFinished:
			; 清空tempIndex和mulResultIndex，避免脏位
			mov dword[tempIndex], 0
			mov dword[mulResultIndex], 0
			; mulIndex--,mulIndexFrom0++
			dec dword[mulIndex]
			inc dword[mulIndexFrom0]
			; 遍历乘数的每一位
			jmp multiplyLoop

multiplyFinished:
	popa
	ret

; --------------------------------------
; 多位 x 一位乘法函数
multiplyOneDigit:
	pusha

	; index1存放num1位数
	mov eax, dword[len1]
	mov dword[index1], eax

	mov dword[mulCarrier], 0
	mov dword[mulResultIndex], 0 ; 乘法结果长度
	mov dword[mulDigit], 0 ; 乘法结果

	; cntZero保存mulIndexFrom0（0的个数）
	mov eax, dword[mulIndexFrom0]
	mov dword[cntZero], eax

	insert0:
		; 对应位后的0都添加了，跳入乘法
		cmp dword[cntZero], 0
		je multiplyOneDigitLoop

		; 在mulDigit偏移量对应位置置为0
		mov eax, mulDigit
		add eax, dword[mulResultIndex]
		mov byte[eax], 48

		; mulResultIndex++，cntZero--，继续循环
		inc dword[mulResultIndex]
		dec dword[cntZero]
		jmp insert0

	multiplyOneDigitLoop:
		; num1遍历完结束，跳出循环
		cmp dword[index1], 0
		je multiplyOneDigitFinished

		; 按位执行乘法（从num1的最低位地址开始）
		mov ebx, num1
		add ebx, dword[index1]
		dec ebx

		; eax=eax*mulOneDigit
		mov eax, 0
		mov al, byte[ebx]
		sub eax, 48
		mul byte[mulOneDigit]

		mov ecx, eax
		add ecx, dword[mulCarrier]
		mov dword[mulCarrier], 0

		mulOverflow:
			; 按位乘法0~81，乘法结果：个位-ecx，十位-mulCarrier
			cmp ecx, 10
			jb overflowFinished

			sub ecx, 10
			inc dword[mulCarrier]
			jmp mulOverflow
		
		overflowFinished:	
			; 本次乘法结果存放入mulDigit
			mov eax, mulDigit ;
			add eax, dword[mulResultIndex] ;
			add cl, 48
			mov byte[eax], cl
			
			; mulDigit长度mulResultIndex++，遍历num1计数器index1--，继续循环
			inc dword[mulResultIndex]
			dec dword[index1]
			jmp multiplyOneDigitLoop

multiplyOneDigitFinished:
	; 处理最后一次乘法是否有进位
	cmp dword[mulCarrier], 0
	je multiplyOneDigitReslt

	; 增加一位，增加mulDigit长度mulResultIndex
	mov eax, mulDigit
	add eax, dword[mulResultIndex]
	mov cl, byte[mulCarrier]
	add cl, 48
	mov byte[eax], cl
	inc dword[mulResultIndex]

	multiplyOneDigitReslt:
		; 乘法进位器归零，函数返回
		mov dword[mulCarrier], 0
		
		popa
		ret

; --------------------------------------
; 逆向输出加法结果
reverseOutput:
	; eax存放addResult地址（逆序状态）
	; printsize存放addResult位数
	add eax, dword[printsize] ; addResult地址+位数偏移量（最后一位的下一bit）
	push eax
	mov dword[printindex], 0 ; printindex:正向index
	mov dword[print], 0 ; print: 打印位
	dec dword[printsize] ; index应当为length-1

	reverseNextchar:
		pop eax
		dec eax ; 指针移动到待输出str的最后一位
		push eax ; 压栈保存地址

		mov dword[print], eax ; 读进一位并系统调用输出
		mov eax, 4 ; write系统调用号：4
		mov ebx, 1 ; 标准输出
		mov ecx, dword[print] ; 字符串
		mov edx, 1 ; 长度
		int 80h

		; 比较printindex与length-1
		mov eax, dword[printsize]
		cmp dword[printindex], eax
		je reverseOutputFinished

		; printindex++，继续逆向输出下一位
		inc dword[printindex]
		jmp reverseNextchar

reverseOutputFinished:
	pop eax ; 释放出栈
	; 打印换行
	mov eax, 4
	mov ebx, 1
	mov ecx, newline
	mov edx, 1
	int 80h

	ret

; --------------------------------------
; 输出字符串
strOutput:
	; eax: Result地址
	dec eax ;地址--，第一位地址前1byte
	push eax
	mov dword[printindex], 0 ; printindex:正向index
	mov dword[print], 0 ; print: 打印位
	dec dword[printsize] ; index应当为length-1

	strNextchar:
		pop eax
		inc eax ; eax地址指向第一位
		push eax

		mov dword[print], eax ; 读进一位并系统调用输出
		mov eax, 4 ; write系统调用号：4
		mov ebx, 1 ; 标准输出
		mov ecx, dword[print] ; 字符串
		mov edx, 1 ; 长度
		int 80h

		; 比较printindex与length-1
		mov eax, dword[printsize]
		cmp dword[printindex], eax
		je strOutputFinished

		; printindex++，继续输出下一位
		inc dword[printindex]
		jmp strNextchar

strOutputFinished:
	pop eax ; 释放出栈
	; 打印换行
	mov eax, 4
	mov ebx, 1
	mov ecx, newline
	mov edx, 1
	int 80h

	ret

; --------------------------------------
; 填充0
fillZero:
	; eax : 需要填充0的地址
	; ebx : 需要填充的位数
	mov dword[fillZeroCnt], 0 ; fillZeroCnt:填充的index，从0开始
	mov ecx, eax

	zeroDigit:
		; 如果位数填充完则跳出循环
		cmp dword[fillZeroCnt], ebx
		je fillZeroFinished

		; 填充1位0
		mov eax, ecx
		add eax, dword[fillZeroCnt] ; 起地址+偏移量
		mov byte[eax], 48 ; 填充48='0'

		; 填充index++，继续循环
		inc dword[fillZeroCnt]
		jmp zeroDigit

fillZeroFinished:
	; 填充index归零，避免污染，函数返回
	mov dword[fillZeroCnt], 0
	ret