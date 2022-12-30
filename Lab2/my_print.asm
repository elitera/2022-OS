section .data
color_red: db      1Bh, '[31;1m', 0
.len    equ     $ - color_red
; white
color_default:  db      1Bh, '[37;1m', 0
.len    equ     $ - color_default

global my_print
section .text
my_print:
    ; eax points to the str
    ; ebx is the color
    ; ebx=0 => default
    ; ebx=1 => red
    mov eax, [esp+4]
    mov ebx, [esp+8]
    cmp ebx, 1
    jne Set_Default_Color
        ; ebx = 1, color red
        call set_color_red
        jmp Print_Asm_Str
    Set_Default_Color:
        ; ebx = 0, color default(white)
        call set_color_default
    Print_Asm_Str:
        call sprint
        ret


set_color_red:
    push edx
    push ecx
    push ebx
    push eax

    mov eax, 4
    mov ebx, 1
    mov ecx, color_red
    mov edx, color_red.len
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx
    ret

set_color_default:
    push edx
    push ecx
    push ebx
    push eax

    mov eax, 4
    mov ebx, 1
    mov ecx, color_default
    mov edx, color_default.len
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx
    ret

strlen:
    ; eax stores the msg
    ; ebx stores the length
    push eax
    mov ebx, eax
Next_Chat:
    cmp byte[ebx], 0
    jz Finished
    inc ebx
    jmp Next_Chat
Finished:
    sub ebx, eax
    pop eax
    ret

sprint:
    push edx
    push ecx
    push ebx
    push eax

    call strlen     ; ebx is the length
    mov edx, ebx    ; ecx store the length
    mov ecx, eax    ; edx store the msg
    mov eax, 4
    mov ebx, 1
    int 80h

    pop eax
    pop ebx
    pop ecx
    pop edx
    ret