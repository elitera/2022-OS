
SECTION .data
message1 db "Please input x and y: ", 0h
message3 db "The sum of these two numbers is: ", 0h
message4 db "The product of these two numbers is: ", 0h

SECTION .bss
input_string: resb 255

first_string: resb 255
second_string: resb 255
add_result: resb 255
mul_result: resb 255
add_ptr: resb 1

SECTION .text
global main
strlen:
    push ebx
    mov ebx, eax
.next:
    cmp BYTE[ebx], 0
    jz .finish
    inc ebx
    jmp .next
.finish:
    sub ebx, eax
    mov eax, ebx
    pop ebx
    ret

endl:
    push eax
    mov eax, 0Ah
    call putchar
    pop eax
    ret

puts:
    push edx
    push ecx
    push ebx
    push eax

    mov ecx, eax
    call strlen
    mov edx, eax
    mov ebx, 1
    mov eax, 4
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx
    ret

putchar:
    push edx
    push ecx
    push ebx
    push eax

    mov eax, 4
    mov ebx, 1
    mov ecx, esp
    mov edx, 1
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx

    ret

getline:
    push edx
    push ecx
    push ebx
    push eax

    mov edx, ebx
    mov ecx, eax
    mov ebx, 1
    mov eax, 3
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx

    ret

sub_digit:
    mov ecx, 1
    sub al, 10
    ret

parse_input:
.loop:
    cmp BYTE[ecx], 32
    jz .rett
    cmp BYTE[ecx], 10
    jz .rett
    mov dl, BYTE[ecx]
    mov BYTE[eax], dl
    inc eax
    inc ecx
    jmp .loop

.rett:
    inc ecx
    ret

main:
    mov eax, message1
    call puts

    mov eax, input_string
    mov ebx, 255
    call getline

    mov ecx, input_string

    mov eax, first_string
    call parse_input

    mov eax, second_string
    call parse_input

.after_input:
    mov ecx, 0
    mov edx, add_result
    add edx, 255
    xor eax, eax
    mov al, 10
    mov BYTE[edx], al

    mov eax, first_string       ; get len of the first number
    call strlen                 ; esi: ptr of num1
    mov esi, eax
    add esi, first_string
    sub esi, 1

    mov eax, second_string      ; get len of the second number
    call strlen                 ; edi: ptr of num2
    mov edi, eax
    add edi, second_string
    sub edi, 1

.loopAdd:
    cmp esi, first_string
    jl .rest_second_digits
    cmp edi, second_string
    jl .rest_first_digits
    xor eax, eax
    add al, BYTE[esi]
    sub al, 48
    add al, BYTE[edi]           ; add by digits
    add al, cl                  ; add carry
    mov ecx, 0                  ; reset carry
    dec esi                     ; move ptr1
    dec edi                     ; move ptr2
    dec edx                     ; move result ptr
    cmp al, 57                  ; check if overflow occurs
    mov BYTE[edx], al
    jle .loopAdd                ; if not continue the loop
    call sub_digit              ; if so, call sub digit
    mov BYTE[edx], al
    jmp .loopAdd

.rest_first_digits:
    cmp esi, first_string
    jl .after_add
    xor eax, eax
    add al, BYTE[esi]           ; add by digits
    add al, cl                 ; add carry
    mov ecx, 0
    dec esi                     ; move ptr2
    dec edx                     ; move result ptr
    mov BYTE[edx], al
    cmp al, 57
    jle .rest_first_digits
    call sub_digit
    mov BYTE[edx], al
    jmp .rest_first_digits

.rest_second_digits:
    cmp edi, second_string
    jl .after_add
    xor eax, eax
    add al, BYTE[edi]           ; add by digits
    add al, cl                  ; add carry
    mov ecx, 0
    dec edi                     ; move ptr2
    dec edx                     ; move result ptr
    mov BYTE[edx], al
    cmp al, 57
    jle .rest_second_digits
    call sub_digit
    mov BYTE[edx], al
    jmp .rest_second_digits

.add_carry:
    mov al, 49
    dec edx
    mov BYTE[edx], al
    jmp .output_add

.after_add:
    cmp ecx, 1
    jz .add_carry
    jmp .output_add

.output_add:
    mov eax, message3
    call puts

    mov eax, edx
    call puts

.start_mul:
    mov edx, mul_result
    mov ecx, 0
    add edx, 255                ; edx: ptr of result
    xor eax, eax
    mov al, 10
    mov BYTE[edx], al
    dec edx

    mov eax, first_string       ; get len of the first number
    call strlen                 ; esi: ptr of num1
    mov esi, eax
    add esi, first_string
    sub esi, 1

    mov eax, second_string      ; get len of the second number
    call strlen                 ; edi: ptr of num2
    mov edi, eax
    add edi, second_string
    sub edi, 1                  ; from tail

.outter_loopMul:
.loop:
    cmp edi, second_string       ; mul by digits
    jl .mul_output
    call inner_loopMul
    dec edi
    dec edx
    jmp .loop

.mul_output:
    mov edx, eax
    mov eax, message4
    call puts
    call format

.output_loop:
    cmp BYTE[edx], 48
    jnz .print_mul
    xor ecx, ecx
    add ecx, mul_result
    add ecx, 254
    cmp edx, ecx
    je .print_mul
    add edx, 1
    jmp .output_loop

.print_mul:
    mov eax, edx
    call puts

format:
    push edx
    push eax
.loop:
    mov eax, mul_result
    add eax, 255
    cmp edx, eax                ;need to fomat
    jge .finish
    add BYTE[edx], 48
    inc edx
    jmp .loop
.finish:
    pop eax
    pop edx 
    ret

inner_loopMul:
    push esi
    push ebx
    push edx

.loop:
    cmp esi, first_string       ; mul by digits
    jl .finish
    xor eax, eax
    xor ebx, ebx
    add al, BYTE[esi]
    sub al, 48
    add bl, BYTE[edi]           ; add by digits
    sub bl, 48
    mul bl
    add BYTE[edx], al
    mov al, BYTE[edx]
    mov ah, 0
    mov bl, 10
    div bl
    mov BYTE[edx], ah
    dec esi                     ; move ptr1
    dec edx                     ; move result ptr
    add BYTE[edx], al
    jmp .loop

.finish:
    mov eax, edx
    pop edx
    pop ebx
    pop esi
    ret


    