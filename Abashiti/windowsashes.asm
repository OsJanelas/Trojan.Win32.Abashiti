[bits 16]
[org 0x7c00]

start:
    ; Configura o modo de vídeo 13h (320x200, 256 cores)
    mov ax, 0x0013
    int 0x10

    ; Aponta ES para o segmento de memória de vídeo
    push 0xa000
    pop es

draw_fractal:
    xor di, di          ; Começa no topo da tela (offset 0)
    xor dx, dx          ; DX será nossa coordenada Y

.y_loop:
    xor cx, cx          ; CX será nossa coordenada X

.x_loop:
    ; Algoritmo XOR Fractal: cor = x XOR y
    mov ax, cx
    xor ax, dx
    
    ; Escreve o pixel na memória de vídeo (ES:DI)
    stosb               ; [ES:DI] = AL, DI++
    
    inc cx
    cmp cx, 320         ; Fim da largura?
    jne .x_loop

    inc dx
    cmp dx, 200         ; Fim da altura?
    jne .y_loop

draw_text:
    ; Configura cursor para escrever o texto
    mov ah, 0x02        ; Função de setar cursor
    mov bh, 0x00        ; Página 0
    mov dh, 10          ; Linha (centro aprox)
    mov dl, 11          ; Coluna
    int 0x10

    mov si, msg         ; Aponta para nossa mensagem
    mov bl, 0x0F        ; Cor branca (High Intensity)

.print_char:
    lodsb               ; Carrega AL com o char em SI, incrementa SI
    test al, al         ; Chegou no zero (null terminator)?
    jz hang
    mov ah, 0x0e        ; Função Teletype (escrever char)
    int 0x10
    jmp .print_char

hang:
    jmp $               ; Loop infinito

msg db "Oh, u are gay LOL", 0

; Preenchimento da MBR
times 510-($-$$) db 0
dw 0xaa55               ; Assinatura de boot obrigatória