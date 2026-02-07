    bits 64
    extern malloc, puts, printf, fflush, abort, free
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8

    section   .text
;;; print_int proc
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf

    xor rdi, rdi
    call fflush

    mov rsp, rbp
    pop rbp
    ret

;;; is_odd proc
is_odd:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element proc
add_element:
    push rbp
    push rbx
    push r14
    mov rbp, rsp
    sub rsp, 16

    mov r14, rdi
    mov rbx, rsi

    mov rdi, 16
    call malloc
    test rax, rax
    jz abort

    mov [rax], r14
    mov [rax + 8], rbx

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; map proc
map:    
    push rbp
    mov rbp, rsp
    sub rsp, 16

    test rdi, rdi
    jz .out
    
    push rbp
    push rbx
.loop:
    mov rbx, rdi
    mov rbp, rsi

    mov rdi, [rdi]
    call rsi
    
    mov rsi, rbp
    mov rdi, [rbx + 8]
    test rdi, rdi
    jnz .loop

    pop rbx
    pop rbp
.out:
    mov rsp, rbp
    pop rbp
    ret


;;; filter proc
filter:
    push rbp
    mov rbp, rsp
    test rdi, rdi
    jz .out

    push rbx
    push r12
    push r13
    mov r12, 0
.loop: 
    mov rbx, rdi ; source_ptr
    mov r13, rdx ; is_odd
    mov rdi, [rdi]
    call rdx
    test rax, rax
    jz .next

    mov rdi, [rbx]
    mov rsi, r12
    call add_element
    mov r12, rax
.next: 
    mov rdi, [rbx + 8]
    mov rdx, r13
    test rdi, rdi
    jnz .loop 

    mov rax, r12
    pop r13
    pop r12
    pop rbx
.out: 
    pop rbp
    ret

;;; clear proc
clear:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    
    test rdi, rdi
    jz .out
    
    push rbx
.loop:
    mov rbx, [rdi + 8]  
    call free            
    mov rdi, rbx         
    test rdi, rdi
    jnz .loop
    pop rbx
.out:
    mov rsp, rbp
    pop rbp
    ret

;;; main proc
main:
    push rbx
    push r12
    push r13

    xor rax, rax
    mov rbx, data_length
adding_loop:
    mov rdi, [data - 8 + rbx * 8]
    mov rsi, rax
    call add_element
    dec rbx
    jnz adding_loop

    mov rbx, rax
    mov r12, rax
    mov rdi, rax
    mov rsi, print_int
    call map

    mov rdi, empty_str
    call puts

    mov rdx, is_odd
    mov rdi, rbx
    call filter

    mov r13, rax
    mov rdi, rax
    mov rsi, print_int
    call map

    mov rdi, empty_str
    call puts

    mov rdi, r12
    call clear
    mov rdi, r13
    call clear

    pop r13
    pop r12
    pop rbx

    xor rax, rax
    ret
