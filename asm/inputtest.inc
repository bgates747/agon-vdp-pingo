
; ============ KEYBOARD INPUT ============
do_input:
; get user keyboard input
	; MOSCALL mos_getkbmap
    ld ix,keyboard_masks

; ------ QUIT ------
; 113 Escape
    bit 0,(ix+14)
	jp nz,main_end

; ; ------ TEST ROUTINE ------
; ; 99 Space do testing routine
;     bit 2,(ix+12)
;     jp z,no_move
;     RUN_TESTS sid
    jp rendbmp

app_special_init:
; clear the screen twice to set both buffers to background color
    call vdu_clg
    call vdu_flip
    call vdu_clg
    ret

app_special_render:
    RUN_TESTS sid
    call vdu_flip
    ret

sid: equ 100
mid: equ 1
oid: equ 1
obj_scale: equ 256
objbmid: equ 256
tgtbmid: equ 257

; full screen 10 fps
cstw: equ 320
csth: equ 240

; ; wolf3d target 20 fps
; cstw: equ 240
; csth: equ 160

; ; fsim target 17 fps
; cstw: equ 320
; csth: equ 148

cstx: equ 0
csty: equ 0

camd: equ 32*1 ; 32767/256 * bar
camx: dl  0*camd
camy: dl  0*camd
camz: dl  0*camd

camdx: dl 0x000000
camdy: dl 0x000000
camdz: dl 0x000000

camdr: equ 91*5 ; 32767/360*foo
camrx: dl 0x000000
camry: dl 0x000000
camrz: dl 0x000000

camdrx: dl 0x000000
camdry: dl 0x000000
camdrz: dl 0x000000

objdr: equ 91*5 ; 32767/360*foo
objdrx: dl 0
objdry: dl 0
objdrz: dl 0

objrx: dl 0
objry: dl 0
objrz: dl 0

objd: equ 32*1 ; 32767/256 * bar
objx: dl 0*objd
objy: dl 0 ; -42 ; -1/3*objd
objz: dl -20*objd

objdx: dl 0x000000
objdy: dl 0x000000
objdz: dl 0x000000

dithering_type: db 0x00 ; 0=none, 1=bayer ordered matrix, 2=floyd-steinberg