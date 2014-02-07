.section .itcm

.global Merge4Pixels
Merge4Pixels:
	push {r4-r7}

@(((a ^ b) & 0x7BDE) >> 1) + (a & b)

	ldr r7,= 0x7BDE

@r0 en r1
	eor r4, r0, r1
	and r4, r4, r7
	and r5, r0, r1
	add r4, r5, r4, lsr #1
@r2 en r3
	eor r6, r2, r3
	and r6, r6, r7
	and r5, r2, r3
	add r6, r5, r6, lsr #1
@r0r1 en r2r3
	eor r0, r4, r6
	and r0, r0, r7
	and r5, r4, r6
	add r0, r5, r0, lsr #1

	pop {r4-r7}
	bx lr

.extern readByte
.extern readU24LE

.extern writeRoomColor
.extern FILL_BITS_he

@void drawStripHE_asm(FILE* handle, LFLF_t* Room, byte *dst, int height, int width, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask)
@r0 = r4 = FILE* handle
@r1 = r5 = LFLF_t* Room
@r2 = r6 = byte *dst
@r3 = r7 = int height
.global drawStripHE_asm
drawStripHE_asm:
var_shift			= 4 * -3 + 12
var_color			= 4 * -2 + 12
var_data			= 4 * -1 + 12

var_width			= 4 * 9 + 12
var_dstPitch		= 4 * 10 + 12
var_transpColor		= 4 * 11 + 12
var_decomp_shr		= 4 * 12 + 12
var_decomp_mask		= 4 * 13 + 12
	push {r4-r11, lr}
	sub sp, sp, #12

	mov	r4, r0
	mov r5, r1
	mov r6, r2
	mov r7, r3
	mov r11, #0

	str r11, [sp, #var_color]
	str r11, [sp, #var_data]

	add r1, sp, #var_color
	bl readByte						@readByte(handle, &color);

	mov r0, r4
	add r1, sp, #var_data
	bl readU24LE					@readU24LE(handle, &data);

	mov r8, #24						@shift = 24;

	ldr r9, [sp, #var_width]		
	mov r11, r9						@x = width;

drawStripHE_loop:								@while (1) {
	mov r0, r5
	mov r1, r6
	ldrb r2, [sp, #var_color]
	ldr r3, [sp, #var_transpColor]
	cmp r2, r3
	blne writeRoomColor				@	writeRoomColor(Room, dst, color);

	add	r6, r6, #2					@	dst += 2;

	subs r11, r11, #1				@	if (--x == 0)
	bne drawStripHE_non_zero		@	{
	mov r11, r9						@		x = width;

	ldr r10, [sp, #var_dstPitch]
	sub r10, r10, r9, lsl #1
	add r6, r6, r10					@		dst += dstPitch - width * 2;

	subs r7, r7, #1					@		if (--height == 0)
	addeq sp, sp, #12
	popeq {r4-r11, pc}				@			return;
									@	}
drawStripHE_non_zero:
	@mov r0, r4
	add r0, sp, #var_data
	str r8, [sp, #var_shift]
	add r1, sp, #var_shift
	mov r2, #1
	bl FILL_BITS_he					@	FILL_BITS(&data, &shift, 1);
	ldr r8, [sp, #var_shift]

@READ_BIT start
@shift--
	sub r8, r8, #1
@dataBit = data & 1
	ldr r1, [sp, #var_data]
	ands r0, r1, #1
@data >>= 1
	lsr r1, r1, #1
	str r1, [sp, #var_data]
@READ_BIT end						@	if (READ_BIT)
	beq drawStripHE_loop			@	{
	@mov r0, r4
	add r0, sp, #var_data
	str r8, [sp, #var_shift]
	add r1, sp, #var_shift
	mov r2, #1
	bl FILL_BITS_he					@		FILL_BITS(&data, &shift, 1);
	ldr r8, [sp, #var_shift]

@READ_BIT start
@shift--
	sub r8, r8, #1
@dataBit = data & 1
	ldr r1, [sp, #var_data]
	ands r0, r1, #1
@data >>= 1
	lsr r1, r1, #1
	str r1, [sp, #var_data]
@READ_BIT end						@		if (READ_BIT)
	beq drawStripHE_use_decomp		@		{
						
	@mov r0, r4
	add r0, sp, #var_data
	str r8, [sp, #var_shift]
	add r1, sp, #var_shift
	mov r2, #3
	bl FILL_BITS_he					@			FILL_BITS(&data, &shift, 3);
	ldr r8, [sp, #var_shift]

	ldrb r0, [sp, #var_color]
	ldr r1,= delta_color
	ldr r2, [sp, #var_data]
	and r2, r2, #7
	ldrsb r3, [r1, r2]
	add r1, r0, r3
	strb r1, [sp, #var_color]		@			color += delta_color[data & 7];

	sub r8, r8, #3					@			shift -= 3;

	ldr r2, [sp, #var_data]
	lsr r2, r2, #3
	str r2, [sp, #var_data]			@			data >>= 3;
	b drawStripHE_loop				@		}
drawStripHE_use_decomp:				@		else {
	@mov r0, r4
	add r0, sp, #var_data
	str r8, [sp, #var_shift]
	add r1, sp, #var_shift
	ldr r10, [sp, #var_decomp_shr]
	mov r2, r10
	bl FILL_BITS_he					@			FILL_BITS(&data, &shift, _decomp_shr);
	ldr r8, [sp, #var_shift]

	ldr r1, [sp, #var_data]
	ldr r2, [sp, #var_decomp_mask]
	and r1, r1, r2
	strb r1, [sp, #var_color]		@			color = data & _decomp_mask;

	sub r8, r8, r10					@			shift -= _decomp_shr;

	ldr r2, [sp, #var_data]
	lsr r2, r2, r10
	str r2, [sp, #var_data]			@			data >>= _decomp_shr;
									@		}
									@	}
	b drawStripHE_loop				@}

delta_color:
	.byte -4, -3, -2, -1, 1, 2, 3, 4

.extern clearByteBuffer
.extern readByteBuffer

.extern fseek

.extern HE1_File

@.extern writePaletteColor

@void writePaletteColor_asm(void* Palette, byte *dst, byte color)
.global writePaletteColor_asm
writePaletteColor_asm:
	add r2, r2, r2, lsl #1
	add r0, r0, r2
	ldrb r2, [r0, #0]
	ldrb r3, [r0, #1]
	ldrb r0, [r0, #2]
	mov r2, r2, lsr #3
	mov r3, r3, lsr #3
	mov r0, r0, lsr #3
	orr r2, r2, r3, lsl #5
	orr r2, r2, r0, lsl #10
	strh r2, [r1]
	bx lr

@void DecompressAKOSCodec1_asm(uint8_t* Dst, void* Palette, void* Colors, uint32_t Width, uint32_t Height, uint32_t DataOffset, uint32_t PaletteLength)
@r0 = r4 = uint8_t* Dst
@r1 = r5 = void* Palette
@r2 = r6 = void* Colors
@r3 = r7 = uint32_t Width
.global DecompressAKOSCodec1_asm
DecompressAKOSCodec1_asm:
SEEK_SET = 0

var_mask			= 4 * -1 + 4

var_height			= 4 * 9 + 4
var_dataoffset		= 4 * 10 + 4
var_palettelength	= 4 * 11 + 4
	push {r4-r11, lr}
	sub sp, #4

	mov r4, r0
	mov r5, r1
	mov r6, r2
	mov r7, r3

	@bl clearByteBuffer				@buffersize = 0;

	ldr r0, [sp, #var_palettelength]
	cmp r0, #32						@if (PaletteLength == 32)
	moveq r8, #3					@	ColorShift = 3;
	beq AKOSCodec1_cont
	cmp r0, #64						@else if (PaletteLength == 64)
	moveq r8, #2					@	ColorShift = 2;
	beq AKOSCodec1_cont
	mov r8, #4						@else ColorShift = 4;
AKOSCodec1_cont:
	@mov r0, #0xFF
	@lsr r0, r0, r8
	@mvn r9, r0, lsl r8				@RepeatMask = (byte)~((0xFF >> ColorShift) << ColorShift);
	
	@str r9, [sp, #var_mask]

	ldr r9, [sp, #var_height]

	ldr r0,= HE1_File
	ldr r0, [r0]
	ldr r1, [sp, #var_dataoffset]
	mov r2, #SEEK_SET
	bl fseek						@fseek(HE1_File, DataOffset, SEEK_SET);

	mov r10, #0						@x = 0;
	mov r11, #0						@y = 0;
AKOSCodec1_loop:					@while (true) {
	bl readByteBuffer				@	readByteBuffer(HE1_File, &d);
	@ldrb r1, [sp, #var_mask]
	mov r1, #1
	rsb r1, r1, r1, lsl r8
	ands r2, r0, r1					@	repeat = d & RepeatMask;
	lsr r3, r0, r8					@	color = d >> ColorShift;
	bne AKOSCodec1_col				@	if (repeat == 0) {
	push {r3}
	bl readByteBuffer				@		readByteBuffer(HE1_File, &d);
	pop {r3}
	mov r2, r0						@		repeat = d;
AKOSCodec1_col:						@	}
	cmp r3, #0
	moveq r3, #0x8000
	beq AKOSCodec1_rep
	ldrb r1, [r5, r3] 
	add r1, r1, r1, lsl #1
	add r0, r6, r1
	ldrb r1, [r0, #0]
	ldrb r12, [r0, #1]
	ldrb r0, [r0, #2]
	mov r3, r1, lsr #3
	mov r12, r12, lsr #3
	mov r0, r0, lsr #3
	orr r3, r3, r12, lsl #5
	orr r3, r3, r0, lsl #10
AKOSCodec1_rep:						@	for (int j = 0; j < repeat; j++) {
	cmp r3, #0x8000					@		if (color != 0)
	strneh r3, [r4]					@			writePaletteColor_asm(Colors, Dst + (y * Width * 2 + x * 2), ((uint8_t*)Palette)[color]);

	add r4, r7, lsl #1
	add r11, r11, #1				@		y++;
	cmp r11, r9						@		if (y == Height)
	bne AKOSCodec1_forl				@		{
	mul r0, r9, r7
	sub r0, r0, #1
	sub r4, r4, r0, lsl #1
	mov r11, #0						@			y = 0;
	add r10, r10, #1				@			x++;
	cmp r10, r7						@			if (x == Width)
	addeq sp, #4	
	popeq {r4-r11, pc}				@				return;
AKOSCodec1_forl:					@		}
	subs r2, r2, #1
	bne AKOSCodec1_rep				@	}
	b AKOSCodec1_loop				@}