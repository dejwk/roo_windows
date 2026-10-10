
/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c54 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c54:	004136        	entry	a1, 32
42038c57:	15cc      	bnez.n	a5, 42038c5c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8>
42038c59:	f01d      	retw.n
42038c5b:	94c200        	extui	a12, a0, 2, 10
42038c5e:	93b200        	movnez	a11, a2, a0
42038c61:	a22000        	muluh	a2, a0, a0
42038c64:	20          	.byte	0x20
42038c65:	fffd25        	call8	42038c38 <roo_windows::Decoration::read(short, short) const>
42038c68:	0066a2        	s32i	a10, a6, 0
42038c6b:	02c332        	addi	a3, a3, 2
42038c6e:	442b      	addi.n	a4, a4, 2
42038c70:	550b      	addi.n	a5, a5, -1
42038c72:	664b      	addi.n	a6, a6, 4
42038c74:	fff7c6        	j	42038c57 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3>


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033128 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42033128:	006136        	entry	a1, 48
4203312b:	002282        	l32i	a8, a2, 0
4203312e:	20f770        	or	a15, a7, a7
42033131:	0b2882        	l32i	a8, a8, 44
42033134:	233380        	sext	a3, a3, 15
42033137:	234480        	sext	a4, a4, 15
4203313a:	235580        	sext	a5, a5, 15
4203313d:	236680        	sext	a6, a6, 15
42033140:	02ad      	mov.n	a10, a2
42033142:	31f9      	s32i.n	a15, a1, 12
42033144:	06ed      	mov.n	a14, a6
42033146:	05dd      	mov.n	a13, a5
42033148:	04cd      	mov.n	a12, a4
4203314a:	03bd      	mov.n	a11, a3
4203314c:	0008e0        	callx8	a8
4203314f:	027d      	mov.n	a7, a2
42033151:	0a2d      	mov.n	a2, a10
42033153:	facc      	bnez.n	a10, 42033166 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
42033155:	31f8      	l32i.n	a15, a1, 12
42033157:	06ed      	mov.n	a14, a6
42033159:	05dd      	mov.n	a13, a5
4203315b:	04cd      	mov.n	a12, a4
4203315d:	03bd      	mov.n	a11, a3
4203315f:	07ad      	mov.n	a10, a7
42033161:	dc5da5        	call8	4200f73c <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
42033164:	0a2d      	mov.n	a2, a10
42033166:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208648c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
4208648c:	004136        	entry	a1, 32
4208648f:	029d      	mov.n	a9, a2
42086491:	120c      	movi.n	a2, 1
42086493:	832990        	moveqz	a2, a9, a9
42086496:	4149b6        	bltui	a9, 4, 420864db <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
42086499:	40f980        	nsau	a8, a9
4208649c:	dfc882        	addi	a8, a8, -33
4208649f:	608080        	neg	a8, a8
420864a2:	218180        	srai	a8, a8, 1
420864a5:	fec882        	addi	a8, a8, -2
420864a8:	3a0c      	movi.n	a10, 3
420864aa:	401800        	ssl	a8
420864ad:	a1aa00        	sll	a10, a10
420864b0:	c289a0        	quou	a8, a9, a10
420864b3:	88aa      	add.n	a8, a8, a10
420864b5:	418180        	srli	a8, a8, 1
420864b8:	c2a980        	quou	a10, a9, a8
420864bb:	aa8a      	add.n	a10, a10, a8
420864bd:	41a1a0        	srli	a10, a10, 1
420864c0:	c289a0        	quou	a8, a9, a10
420864c3:	88aa      	add.n	a8, a8, a10
420864c5:	1a0c      	movi.n	a10, 1
420864c7:	418180        	srli	a8, a8, 1
420864ca:	11aa00        	slli	a10, a10, 16
420864cd:	0518a7        	beq	a8, a10, 420864d6 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
420864d0:	82a880        	mull	a10, a8, a8
420864d3:	01b9a7        	bgeu	a9, a10, 420864d8 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
420864d6:	880b      	addi.n	a8, a8, -1
420864d8:	f42080        	extui	a2, a8, 0, 16
420864db:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420862ec <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
420862ec:	004136        	entry	a1, 32
420862ef:	208230        	or	a8, a2, a3
420862f2:	0228d6        	bgez	a8, 42086318 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
420862f5:	c02240        	sub	a2, a2, a4
420862f8:	40f290        	nsau	a9, a2
420862fb:	419590        	srli	a9, a9, 5
420862fe:	b28c      	beqz.n	a2, 4208630d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
42086300:	c03340        	sub	a3, a3, a4
42086303:	180c      	movi.n	a8, 1
42086305:	938930        	movnez	a8, a9, a3
42086308:	ffa022        	movi	a2, 255
4208630b:	688c      	beqz.n	a8, 42086315 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208630d:	1125c0        	slli	a2, a5, 4
42086310:	252a      	add.n	a2, a5, a2
42086312:	742020        	extui	a2, a2, 0, 8
42086315:	f01d      	retw.n
42086317:	222000        	orb	b2, b0, b0
4208631a:	3330d1        	l32r	a13, 42052fdc <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
4208631d:	823ad1        	l32r	a13, 42066c08 <esp_littlefs_free_fd$isra$0+0x48> (880b3d12 <_rtc_reserved_end+0x27fb3d12>)
42086320:	118880        	slli	a8, a8, 8
42086323:	ffa022        	movi	a2, 255
42086326:	eb48a6        	blti	a8, 4, 42086315 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086329:	40f890        	nsau	a9, a8
4208632c:	dfc992        	addi	a9, a9, -33
4208632f:	609090        	neg	a9, a9
42086332:	219190        	srai	a9, a9, 1
42086335:	fec992        	addi	a9, a9, -2
42086338:	3a0c      	movi.n	a10, 3
4208633a:	401900        	ssl	a9
4208633d:	a1aa00        	sll	a10, a10
42086340:	c298a0        	quou	a9, a8, a10
42086343:	99aa      	add.n	a9, a9, a10
42086345:	419190        	srli	a9, a9, 1
42086348:	c2a890        	quou	a10, a8, a9
4208634b:	aa9a      	add.n	a10, a10, a9
4208634d:	41a1a0        	srli	a10, a10, 1
42086350:	c298a0        	quou	a9, a8, a10
42086353:	99aa      	add.n	a9, a9, a10
42086355:	1a0c      	movi.n	a10, 1
42086357:	419190        	srli	a9, a9, 1
4208635a:	11aa00        	slli	a10, a10, 16
4208635d:	0519a7        	beq	a9, a10, 42086366 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7a>
42086360:	82a990        	mull	a10, a9, a9
42086363:	01b8a7        	bgeu	a8, a10, 42086368 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7c>
42086366:	990b      	addi.n	a9, a9, -1
42086368:	1144c0        	slli	a4, a4, 4
4208636b:	554a      	add.n	a5, a5, a4
4208636d:	c05590        	sub	a5, a5, a9
42086370:	f48050        	extui	a8, a5, 0, 16
42086373:	239580        	sext	a9, a5, 15
42086376:	020c      	movi.n	a2, 0
42086378:	99e5f7        	bbsi	a5, 15, 42086315 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208637b:	fa0c      	movi.n	a10, 15
4208637d:	ffa022        	movi	a2, 255
42086380:	912a97        	blt	a10, a9, 42086315 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086383:	1198c0        	slli	a9, a8, 4
42086386:	889a      	add.n	a8, a8, a9
42086388:	742080        	extui	a2, a8, 0, 8
4208638b:	ffe186        	j	42086315 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033e9c <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033e9c:	00a136        	entry	a1, 80
42033e9f:	3169      	s32i.n	a6, a1, 12
42033ea1:	2159      	s32i.n	a5, a1, 8
42033ea3:	060c      	movi.n	a6, 0
42033ea5:	1cc172        	addi	a7, a1, 28
42033ea8:	2188      	l32i.n	a8, a1, 8
42033eaa:	029687        	bne	a6, a8, 42033eb0 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033ead:	f01d      	retw.n
42033eaf:	938200        	movnez	a8, a2, a0
42033eb2:	22d200        	orb	b13, b2, b0
42033eb5:	018917        	bany	a9, a1, 42033eba <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033eb8:	009482        	l16si	a8, a4, 0
42033ebb:	01b8      	l32i.n	a11, a1, 0
42033ebd:	1622a2        	l32i	a10, a2, 88
42033ec0:	08cd      	mov.n	a12, a8
42033ec2:	0067d2        	s32i	a13, a7, 0
42033ec5:	20e770        	or	a14, a7, a7
42033ec8:	016182        	s32i	a8, a1, 4
42033ecb:	201110        	or	a1, a1, a1
42033ece:	fffa25        	call8	42033e70 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033ed1:	182282        	l32i	a8, a2, 96
42033ed4:	1922b2        	l32i	a11, a2, 100
42033ed7:	8189      	s32i.n	a8, a1, 32
42033ed9:	20c152        	addi	a5, a1, 32
42033edc:	050b16        	beqz	a11, 42033f30 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033edf:	0bc8      	l32i.n	a12, a11, 0
42033ee1:	24c1a2        	addi	a10, a1, 36
42033ee4:	201110        	or	a1, a1, a1
42033ee7:	ff3665        	call8	4203324c <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033eea:	07d8      	l32i.n	a13, a7, 0
42033eec:	11c8      	l32i.n	a12, a1, 4
42033eee:	0021b2        	l32i	a11, a1, 0
42033ef1:	24c1a2        	addi	a10, a1, 36
42033ef4:	201110        	or	a1, a1, a1
42033ef7:	fe1865        	call8	4203207c <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033efa:	07a9      	s32i.n	a10, a7, 0
42033efc:	24c1a2        	addi	a10, a1, 36
42033eff:	201110        	or	a1, a1, a1
42033f02:	f894a5        	call8	4202c84c <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f05:	1922b2        	l32i	a11, a2, 100
42033f08:	24c1a2        	addi	a10, a1, 36
42033f0b:	002bc2        	l32i	a12, a11, 0
42033f0e:	201110        	or	a1, a1, a1
42033f11:	ff33a5        	call8	4203324c <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f14:	05d8      	l32i.n	a13, a5, 0
42033f16:	0094c2        	l16si	a12, a4, 0
42033f19:	0093b2        	l16si	a11, a3, 0
42033f1c:	24c1a2        	addi	a10, a1, 36
42033f1f:	201110        	or	a1, a1, a1
42033f22:	fe15a5        	call8	4203207c <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f25:	05a9      	s32i.n	a10, a5, 0
42033f27:	24c1a2        	addi	a10, a1, 36
42033f2a:	201110        	or	a1, a1, a1
42033f2d:	f891e5        	call8	4202c84c <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f30:	0094c2        	l16si	a12, a4, 0
42033f33:	0093b2        	l16si	a11, a3, 0
42033f36:	07d8      	l32i.n	a13, a7, 0
42033f38:	05ed      	mov.n	a14, a5
42033f3a:	a24b      	addi.n	a10, a2, 4
42033f3c:	201110        	or	a1, a1, a1
42033f3f:	04b7e5        	call8	42038abc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f42:	3188      	l32i.n	a8, a1, 12
42033f44:	332b      	addi.n	a3, a3, 2
42033f46:	a08680        	addx4	a8, a6, a8
42033f49:	08a9      	s32i.n	a10, a8, 0
42033f4b:	661b      	addi.n	a6, a6, 1
42033f4d:	442b      	addi.n	a4, a4, 2
42033f4f:	ffd546        	j	42033ea8 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f52:	c1a200        	mul16u	a10, a2, a0
42033f55:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f58:	20          	.byte	0x20
42033f59:	f88f25        	call8	4202c84c <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f5c:	02ad      	mov.n	a10, a2
42033f5e:	201110        	or	a1, a1, a1
42033f61:	3cbb65        	call8	42070b18 <_Unwind_Resume>
42033f64:	fffac6        	j	42033f53 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038714 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
42038714:	006136        	entry	a1, 48
42038717:	189192        	l16si	a9, a1, 48
4203871a:	3169      	s32i.n	a6, a1, 12
4203871c:	f1cb      	addi.n	a15, a1, 12
4203871e:	1a9182        	l16si	a8, a1, 52
42038721:	02ed      	mov.n	a14, a2
42038723:	74d070        	extui	a13, a7, 0, 8
42038726:	02a927        	bge	a9, a2, 4203872c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
42038729:	0030c6        	j	420387f0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203872c:	02a497        	bge	a4, a9, 42038732 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
4203872f:	002f46        	j	420387f0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038732:	02a837        	bge	a8, a3, 42038738 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
42038735:	002dc6        	j	420387f0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038738:	02a587        	bge	a5, a8, 4203873e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a>
4203873b:	002c46        	j	420387f0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203873e:	000fc2        	l8ui	a12, a15, 0
42038741:	3c9c      	beqz.n	a12, 42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038743:	a2ca      	add.n	a10, a2, a12
42038745:	0f2a97        	blt	a10, a9, 42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038748:	b3ca      	add.n	a11, a3, a12
4203874a:	0a2b87        	blt	a11, a8, 42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
4203874d:	c0bb80        	sub	a11, a11, a8
42038750:	c0aa90        	sub	a10, a10, a9
42038753:	000706        	j	42038773 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x5f>
42038756:	c20000        	quou	a0, a0, a0
42038759:	1c16010f 	ee.vmulas.s16.qacc.ld.ip.qup	q0, a0, -240, q4, q0, q1, q6
4203875d:	a4c002        	addi	a0, a0, -92
42038760:	c0          	.byte	0xc0
42038761:	1b29a7        	blt	a9, a10, 42038780 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038764:	80b3c0        	add	a11, a3, a12
42038767:	152b87        	blt	a11, a8, 42038780 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
4203876a:	c0bb80        	sub	a11, a11, a8
4203876d:	c0ac40        	sub	a10, a12, a4
42038770:	80aa90        	add	a10, a10, a9
42038773:	23bb80        	sext	a11, a11, 15
42038776:	23aa80        	sext	a10, a10, 15
42038779:	4db725        	call8	420862ec <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
4203877c:	0a2d      	mov.n	a2, a10
4203877e:	f01d      	retw.n
42038780:	020fc2        	l8ui	a12, a15, 2
42038783:	2c9c      	beqz.n	a12, 42038799 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038785:	c0a4c0        	sub	a10, a4, a12
42038788:	0d29a7        	blt	a9, a10, 42038799 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
4203878b:	c0a5c0        	sub	a10, a5, a12
4203878e:	0728a7        	blt	a8, a10, 42038799 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038791:	c0bc50        	sub	a11, a12, a5
42038794:	bb8a      	add.n	a11, a11, a8
42038796:	fff4c6        	j	4203876d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x59>
42038799:	030fc2        	l8ui	a12, a15, 3
4203879c:	1c9c      	beqz.n	a12, 420387b1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
4203879e:	aeca      	add.n	a10, a14, a12
420387a0:	0d2a97        	blt	a10, a9, 420387b1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387a3:	c0b5c0        	sub	a11, a5, a12
420387a6:	0728b7        	blt	a8, a11, 420387b1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387a9:	c0bc50        	sub	a11, a12, a5
420387ac:	bb8a      	add.n	a11, a11, a8
420387ae:	ffe786        	j	42038750 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3c>
420387b1:	ea0c      	movi.n	a10, 14
420387b3:	ffa022        	movi	a2, 255
420387b6:	c43ad7        	bltu	a10, a13, 4203877e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387b9:	c0e9e0        	sub	a14, a9, a14
420387bc:	40fea0        	nsau	a10, a14
420387bf:	1b0c      	movi.n	a11, 1
420387c1:	41a5a0        	srli	a10, a10, 5
420387c4:	ce9c      	beqz.n	a14, 420387e4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387c6:	c09940        	sub	a9, a9, a4
420387c9:	83ab90        	moveqz	a10, a11, a9
420387cc:	014a56        	bnez	a10, 420387e4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387cf:	c03830        	sub	a3, a8, a3
420387d2:	40f390        	nsau	a9, a3
420387d5:	419590        	srli	a9, a9, 5
420387d8:	008316        	beqz	a3, 420387e4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387db:	c08850        	sub	a8, a8, a5
420387de:	93b980        	movnez	a11, a9, a8
420387e1:	f99b16        	beqz	a11, 4203877e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387e4:	118dc0        	slli	a8, a13, 4
420387e7:	dd8a      	add.n	a13, a13, a8
420387e9:	7420d0        	extui	a2, a13, 0, 8
420387ec:	ffe386        	j	4203877e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387ef:	020c00        	andb	b0, b12, b0
420387f2:	ffe206        	j	4203877e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038abc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038abc:	008136        	entry	a1, 64
42038abf:	2002a2        	l8ui	a10, a2, 32
42038ac2:	021292        	l16ui	a9, a2, 4
42038ac5:	0512d2        	l16ui	a13, a2, 10
42038ac8:	0412c2        	l16ui	a12, a2, 8
42038acb:	0312b2        	l16ui	a11, a2, 6
42038ace:	233380        	sext	a3, a3, 15
42038ad1:	234480        	sext	a4, a4, 15
42038ad4:	1149      	s32i.n	a4, a1, 4
42038ad6:	0139      	s32i.n	a3, a1, 0
42038ad8:	c0dda0        	sub	a13, a13, a10
42038adb:	c0cca0        	sub	a12, a12, a10
42038ade:	baba      	add.n	a11, a10, a11
42038ae0:	2102f2        	l8ui	a15, a2, 33
42038ae3:	aa9a      	add.n	a10, a10, a9
42038ae5:	72e8      	l32i.n	a14, a2, 28
42038ae7:	23dd80        	sext	a13, a13, 15
42038aea:	23cc80        	sext	a12, a12, 15
42038aed:	23bb80        	sext	a11, a11, 15
42038af0:	23aa80        	sext	a10, a10, 15
42038af3:	7169      	s32i.n	a6, a1, 28
42038af5:	ffc1e5        	call8	42038714 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038af8:	ffa092        	movi	a9, 255
42038afb:	027d      	mov.n	a7, a2
42038afd:	0a6d      	mov.n	a6, a10
42038aff:	079a97        	bne	a10, a9, 42038b0a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x4e>
42038b02:	142292        	l32i	a9, a2, 80
42038b05:	052d      	mov.n	a2, a5
42038b07:	0a1916        	beqz	a9, 42038bac <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038b0a:	04dd      	mov.n	a13, a4
42038b0c:	03cd      	mov.n	a12, a3
42038b0e:	3cc7b2        	addi	a11, a7, 60
42038b11:	28c7a2        	addi	a10, a7, 40
42038b14:	005565        	call8	4203906c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b17:	200792        	l8ui	a9, a7, 32
42038b1a:	6169      	s32i.n	a6, a1, 24
42038b1c:	012a80        	slli	a2, a10, 24
42038b1f:	03c916        	beqz	a9, 42038b5f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038b22:	1149      	s32i.n	a4, a1, 4
42038b24:	0139      	s32i.n	a3, a1, 0
42038b26:	67e8      	l32i.n	a14, a7, 24
42038b28:	0597d2        	l16si	a13, a7, 10
42038b2b:	0497c2        	l16si	a12, a7, 8
42038b2e:	0397b2        	l16si	a11, a7, 6
42038b31:	0297a2        	l16si	a10, a7, 4
42038b34:	ff0c      	movi.n	a15, 15
42038b36:	ffbde5        	call8	42038714 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038b39:	61a9      	s32i.n	a10, a1, 24
42038b3b:	0aac      	beqz.n	a10, 42038b5f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038b3d:	7188      	l32i.n	a8, a1, 28
42038b3f:	38cc      	bnez.n	a8, 42038b46 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x8a>
42038b41:	24c782        	addi	a8, a7, 36
42038b44:	7189      	s32i.n	a8, a1, 28
42038b46:	7188      	l32i.n	a8, a1, 28
42038b48:	02ad      	mov.n	a10, a2
42038b4a:	0898      	l32i.n	a9, a8, 0
42038b4c:	6188      	l32i.n	a8, a1, 24
42038b4e:	119980        	slli	a9, a9, 8
42038b51:	419890        	srli	a9, a9, 8
42038b54:	01b880        	slli	a11, a8, 24
42038b57:	20b9b0        	or	a11, a9, a11
42038b5a:	cecee5        	call8	42007848 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038b5d:	0a2d      	mov.n	a2, a10
42038b5f:	ffa092        	movi	a9, 255
42038b62:	4a9697        	bne	a6, a9, 42038bb0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf4>
42038b65:	fb7c      	movi.n	a11, -1
42038b67:	01bb80        	slli	a11, a11, 24
42038b6a:	20b5b0        	or	a11, a5, a11
42038b6d:	02ad      	mov.n	a10, a2
42038b6f:	cecda5        	call8	42007848 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038b72:	0a2d      	mov.n	a2, a10
42038b74:	1427a2        	l32i	a10, a7, 80
42038b77:	031a16        	beqz	a10, 42038bac <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038b7a:	180a82        	l8ui	a8, a10, 24
42038b7d:	053816        	beqz	a8, 42038bd4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x118>
42038b80:	20c440        	or	a12, a4, a4
42038b83:	03bd      	mov.n	a11, a3
42038b85:	ee2f65        	call8	42026e7c <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
42038b88:	6188      	l32i.n	a8, a1, 24
42038b8a:	75b8a0        	extui	a11, a10, 24, 8
42038b8d:	d1bb80        	mul16s	a11, a11, a8
42038b90:	1f5481        	l32r	a8, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038b93:	11aa80        	slli	a10, a10, 8
42038b96:	a2bb80        	muluh	a11, a11, a8
42038b99:	01bbf0        	slli	a11, a11, 17
42038b9c:	75b8b0        	extui	a11, a11, 24, 8
42038b9f:	404800        	ssai	8
42038ba2:	81bba0        	src	a11, a11, a10
42038ba5:	02ad      	mov.n	a10, a2
42038ba7:	ceca25        	call8	42007848 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038baa:	0a2d      	mov.n	a2, a10
42038bac:	f01d      	retw.n
42038bae:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038bb1:	50fc06        	j	4204cfa5 <uart_set_stop_bits+0x7d>
42038bb4:	7598      	l32i.n	a9, a5, 28
42038bb6:	d19960        	mul16s	a9, a9, a6
42038bb9:	1f49a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038bbc:	7fc992        	addi	a9, a9, 127
42038bbf:	a299a0        	muluh	a9, a9, a10
42038bc2:	11b580        	slli	a11, a5, 8
42038bc5:	0199f0        	slli	a9, a9, 17
42038bc8:	759890        	extui	a9, a9, 24, 8
42038bcb:	404800        	ssai	8
42038bce:	81b9b0        	src	a11, a9, a11
42038bd1:	ffe606        	j	42038b6d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb1>
42038bd4:	021a82        	l16ui	a8, a10, 4
42038bd7:	3ab8      	l32i.n	a11, a10, 12
42038bd9:	c03380        	sub	a3, a3, a8
42038bdc:	031a82        	l16ui	a8, a10, 6
42038bdf:	c04480        	sub	a4, a4, a8
42038be2:	740444        	mul.aa.ll	a4, a4
42038be5:	780334        	mula.aa.ll	a3, a3
42038be8:	031080        	rsr.acclo	a8
42038beb:	c0bb80        	sub	a11, a11, a8
42038bee:	061be6        	bgei	a11, 1, 42038bf8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x13c>
42038bf1:	5aa8      	l32i.n	a10, a10, 20
42038bf3:	ffe446        	j	42038b88 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038bf6:	00          	.byte	00
42038bf7:	9a9200        	trunc.s	a9, f2, 0
42038bfa:	04          	.byte	0x4
42038bfb:	3119a6        	blti	a9, 1, 42038c30 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038bfe:	899a      	add.n	a8, a9, a9
42038c00:	60a0c2        	movi	a12, 96
42038c03:	0f2b87        	blt	a11, a8, 42038c16 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038c06:	261926        	beqi	a9, 1, 42038c30 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c09:	c90b      	addi.n	a12, a9, -1
42038c0b:	23cc80        	sext	a12, a12, 15
42038c0e:	11cce0        	slli	a12, a12, 2
42038c11:	0c2cb7        	blt	a12, a11, 42038c21 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x165>
42038c14:	0c4c      	movi.n	a12, 64
42038c16:	4ab8      	l32i.n	a11, a10, 16
42038c18:	5aa8      	l32i.n	a10, a10, 20
42038c1a:	4c16e5        	call8	42084d88 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42038c1d:	ffd9c6        	j	42038b88 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038c20:	00          	.byte	00
42038c21:	0b2926        	beqi	a9, 2, 42038c30 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c24:	889a      	add.n	a8, a8, a9
42038c26:	888a      	add.n	a8, a8, a8
42038c28:	f8c882        	addi	a8, a8, -8
42038c2b:	0c2c      	movi.n	a12, 32
42038c2d:	e52b87        	blt	a11, a8, 42038c16 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038c30:	4aa8      	l32i.n	a10, a10, 16
42038c32:	ffd486        	j	42038b88 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203906c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
4203906c:	006136        	entry	a1, 48
4203906f:	027d      	mov.n	a7, a2
42039071:	0f0222        	l8ui	a2, a2, 15
42039074:	234480        	sext	a4, a4, 15
42039077:	235580        	sext	a5, a5, 15
4203907a:	14f216        	beqz	a2, 420391cd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x161>
4203907d:	009782        	l16si	a8, a7, 0
42039080:	069392        	l16si	a9, a3, 12
42039083:	c04480        	sub	a4, a4, a8
42039086:	019782        	l16si	a8, a7, 2
42039089:	1144c0        	slli	a4, a4, 4
4203908c:	c08580        	sub	a8, a5, a8
4203908f:	a48b      	addi.n	a10, a4, 8
42039091:	1188c0        	slli	a8, a8, 4
42039094:	069742        	l16si	a4, a7, 12
42039097:	888b      	addi.n	a8, a8, 8
42039099:	0397c2        	l16si	a12, a7, 6
4203909c:	c04840        	sub	a4, a8, a4
4203909f:	c09890        	sub	a9, a8, a9
420390a2:	11ccc0        	slli	a12, a12, 4
420390a5:	244a      	add.n	a2, a4, a4
420390a7:	0297f2        	l16si	a15, a7, 4
420390aa:	599a      	add.n	a5, a9, a9
420390ac:	72e2c0        	salt	a14, a2, a12
420390af:	72d5c0        	salt	a13, a5, a12
420390b2:	c0bc40        	sub	a11, a12, a4
420390b5:	11ffc0        	slli	a15, a15, 4
420390b8:	ee0b      	addi.n	a14, a14, -1
420390ba:	dd0b      	addi.n	a13, a13, -1
420390bc:	60e0e0        	neg	a14, a14
420390bf:	60d0d0        	neg	a13, a13
420390c2:	c06fa0        	sub	a6, a15, a10
420390c5:	43bb40        	min	a11, a11, a4
420390c8:	8aaa      	add.n	a8, a10, a10
420390ca:	c04c90        	sub	a4, a12, a9
420390cd:	74e0e0        	extui	a14, a14, 0, 8
420390d0:	74d0d0        	extui	a13, a13, 0, 8
420390d3:	4366a0        	min	a6, a6, a10
420390d6:	434490        	min	a4, a4, a9
420390d9:	02a8f7        	bge	a8, a15, 420390df <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x73>
420390dc:	003e06        	j	420391d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x16c>
420390df:	02a2c7        	bge	a2, a12, 420390e5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x79>
420390e2:	003a86        	j	420391d0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x164>
420390e5:	0a0782        	l8ui	a8, a7, 10
420390e8:	029ed7        	bne	a14, a13, 420390ee <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x82>
420390eb:	003c86        	j	420391e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x175>
420390ee:	02a5c7        	bge	a5, a12, 420390f4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x88>
420390f1:	003ec6        	j	420391f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x184>
420390f4:	0a0722        	l8ui	a2, a7, 10
420390f7:	091792        	l16ui	a9, a7, 18
420390fa:	43a6b0        	min	a10, a6, a11
420390fd:	609090        	neg	a9, a9
42039100:	050c      	movi.n	a5, 0
42039102:	522a97        	blt	a10, a9, 42039158 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xec>
42039105:	238880        	sext	a8, a8, 15
42039108:	1188c0        	slli	a8, a8, 4
4203910b:	c0a860        	sub	a10, a8, a6
4203910e:	c098b0        	sub	a9, a8, a11
42039111:	0225a7        	blt	a5, a10, 42039117 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
42039114:	003c06        	j	42039208 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x19c>
42039117:	022597        	blt	a5, a9, 4203911d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb1>
4203911a:	003a86        	j	42039208 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x19c>
4203911d:	829990        	mull	a9, a9, a9
42039120:	82aaa0        	mull	a10, a10, a10
42039123:	3189      	s32i.n	a8, a1, 12
42039125:	a9aa      	add.n	a10, a9, a10
42039127:	4d3665        	call8	4208648c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
4203912a:	3188      	l32i.n	a8, a1, 12
4203912c:	0e0792        	l8ui	a9, a7, 14
4203912f:	c08a80        	sub	a8, a10, a8
42039132:	c08890        	sub	a8, a8, a9
42039135:	081792        	l16ui	a9, a7, 16
42039138:	828890        	mull	a8, a8, a9
4203913b:	1def91        	l32r	a9, 420008f8 <_stext+0x8d8> (80000 <UserFrameTotalSize+0x7ff00>)
4203913e:	889a      	add.n	a8, a8, a9
42039140:	218c80        	srai	a8, a8, 12
42039143:	ffa092        	movi	a9, 255
42039146:	438890        	min	a8, a8, a9
42039149:	00a092        	movi	a9, 0
4203914c:	538890        	max	a8, a8, a9
4203914f:	1de991        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1160 <roo_windows::internal::(anonymous namespace)::kKey>)
42039152:	808980        	add	a8, a9, a8
42039155:	000852        	l8ui	a5, a8, 0
42039158:	091382        	l16ui	a8, a3, 18
4203915b:	43a640        	min	a10, a6, a4
4203915e:	608080        	neg	a8, a8
42039161:	090c      	movi.n	a9, 0
42039163:	4d2a87        	blt	a10, a8, 420391b4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x148>
42039166:	237280        	sext	a7, a2, 15
42039169:	1177c0        	slli	a7, a7, 4
4203916c:	c08740        	sub	a8, a7, a4
4203916f:	c0a760        	sub	a10, a7, a6
42039172:	022987        	blt	a9, a8, 42039178 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10c>
42039175:	0026c6        	j	42039214 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a8>
42039178:	0229a7        	blt	a9, a10, 4203917e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x112>
4203917b:	002546        	j	42039214 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a8>
4203917e:	828880        	mull	a8, a8, a8
42039181:	82aaa0        	mull	a10, a10, a10
42039184:	a8aa      	add.n	a10, a8, a10
42039186:	4d3065        	call8	4208648c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039189:	0e0392        	l8ui	a9, a3, 14
4203918c:	c08a70        	sub	a8, a10, a7
4203918f:	c08890        	sub	a8, a8, a9
42039192:	081392        	l16ui	a9, a3, 16
42039195:	828890        	mull	a8, a8, a9
42039198:	1dd891        	l32r	a9, 420008f8 <_stext+0x8d8> (80000 <UserFrameTotalSize+0x7ff00>)
4203919b:	889a      	add.n	a8, a8, a9
4203919d:	218c80        	srai	a8, a8, 12
420391a0:	ffa092        	movi	a9, 255
420391a3:	438890        	min	a8, a8, a9
420391a6:	090c      	movi.n	a9, 0
420391a8:	538890        	max	a8, a8, a9
420391ab:	1dd491        	l32r	a9, 420008fc <_stext+0x8dc> (3c0b1060 <roo_windows::internal::(anonymous namespace)::kAmbient>)
420391ae:	808980        	add	a8, a9, a8
420391b1:	000892        	l8ui	a9, a8, 0
420391b4:	d18590        	mul16s	a8, a5, a9
420391b7:	1dcaa1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
420391ba:	7fc882        	addi	a8, a8, 127
420391bd:	b2a8a0        	mulsh	a10, a8, a10
420391c0:	295a      	add.n	a2, a9, a5
420391c2:	88aa      	add.n	a8, a8, a10
420391c4:	218780        	srai	a8, a8, 7
420391c7:	c02280        	sub	a2, a2, a8
420391ca:	742020        	extui	a2, a2, 0, 8
420391cd:	f01d      	retw.n
420391cf:	078200        	ee.stf.64.xp	f8, f0, a0, a2
420391d2:	4609      	s32i.n	a0, a6, 16
420391d4:	c4          	.byte	0xc4
420391d5:	c70000ff 	ee.vmulas.u16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
420391d9:	820c22        	l8ui	a2, a12, 130
420391dc:	d70b07        	bnone	a11, a0, 420391b7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x14b>
420391df:	082d169e 	ee.vmulas.s16.accx.ld.ip.qup	q1, a9, 0xfffffe60, q0, q2, q2, q5
420391e3:	ffc406        	j	420390f7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x8b>
420391e6:	820000        	mull	a0, a0, a0
420391e9:	c60807        	bnone	a8, a0, 420391b3 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x147>
420391ec:	fffb      	addi.n	a15, a15, 15
420391ee:	220000        	orb	b0, b0, b0
420391f1:	060907        	bnone	a9, a0, 420391fb <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x18f>
420391f4:	c0          	.byte	0xc0
420391f5:	c70000ff 	ee.vmulas.u16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
420391f9:	220525        	call8	4205b24c <lcd_com_register_device+0x24>
420391fc:	460b07        	bnone	a11, a0, 42039246 <roo_windows::(anonymous namespace)::PaintKeyContent(roo_windows::PaintContext&, roo_display::Drawable const&, roo_windows::Rect const&)+0x6>
420391ff:	bd          	.byte	0xbd
42039200:	080722ff 	ee.vmulas.s16.accx.ld.ip.qup	q0, a15, 0xfffffe20, q4, q4, q0, q7
42039204:	ffbbc6        	j	420390f7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x8b>
42039207:	2b6700        	oeq.s	b6, f7, f0
4203920a:	c74602        	s8i	a0, a6, 199
4203920d:	0609adff 	ee.vmulas.s16.accx.ld.ip.qup	q5, a15, 0x1d0, q6, q4, q0, q1
42039211:	00ffc6        	j	42039614 <roo_windows::KeyboardWidget::keyAt(int, int) const>
42039214:	022467        	blt	a4, a6, 4203921a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ae>
42039217:	ffdb86        	j	42039189 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11d>
4203921a:	08ad      	mov.n	a10, a8
4203921c:	ffda46        	j	42039189 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11d>


/tmp/roo-shadow-review/codegen/core_baseline/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c78 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038c78:	00c136        	entry	a1, 96
42038c7b:	238680        	sext	a8, a6, 15
42038c7e:	1189      	s32i.n	a8, a1, 4
42038c80:	142282        	l32i	a8, a2, 80
42038c83:	07fd      	mov.n	a15, a7
42038c85:	027d      	mov.n	a7, a2
42038c87:	233380        	sext	a3, a3, 15
42038c8a:	234480        	sext	a4, a4, 15
42038c8d:	235580        	sext	a5, a5, 15
42038c90:	204856        	bnez	a8, 42038e98 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038c93:	0288      	l32i.n	a8, a2, 0
42038c95:	11e8      	l32i.n	a14, a1, 4
42038c97:	b888      	l32i.n	a8, a8, 44
42038c99:	02ad      	mov.n	a10, a2
42038c9b:	a1f9      	s32i.n	a15, a1, 40
42038c9d:	05dd      	mov.n	a13, a5
42038c9f:	04cd      	mov.n	a12, a4
42038ca1:	03bd      	mov.n	a11, a3
42038ca3:	0008e0        	callx8	a8
42038ca6:	0a2d      	mov.n	a2, a10
42038ca8:	098a56        	bnez	a10, 42038d44 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcc>
42038cab:	1a9792        	l16si	a9, a7, 52
42038cae:	249782        	l16si	a8, a7, 72
42038cb1:	ec0c      	movi.n	a12, 14
42038cb3:	538890        	max	a8, a8, a9
42038cb6:	98fb      	addi.n	a9, a8, 15
42038cb8:	1ec882        	addi	a8, a8, 30
42038cbb:	b38990        	movgez	a8, a9, a9
42038cbe:	21b480        	srai	a11, a8, 4
42038cc1:	210782        	l8ui	a8, a7, 33
42038cc4:	1907a2        	l8ui	a10, a7, 25
42038cc7:	62cc80        	saltu	a12, a12, a8
42038cca:	180782        	l8ui	a8, a7, 24
42038ccd:	200792        	l8ui	a9, a7, 32
42038cd0:	7388a0        	maxu	a8, a8, a10
42038cd3:	1a07a2        	l8ui	a10, a7, 26
42038cd6:	cc0b      	addi.n	a12, a12, -1
42038cd8:	7388a0        	maxu	a8, a8, a10
42038cdb:	1b07a2        	l8ui	a10, a7, 27
42038cde:	60c0c0        	neg	a12, a12
42038ce1:	73a8a0        	maxu	a10, a8, a10
42038ce4:	89ca      	add.n	a8, a9, a12
42038ce6:	99ba      	add.n	a9, a9, a11
42038ce8:	538a80        	max	a8, a10, a8
42038ceb:	029762        	l16si	a6, a7, 4
42038cee:	0497a2        	l16si	a10, a7, 8
42038cf1:	99ca      	add.n	a9, a9, a12
42038cf3:	539980        	max	a9, a9, a8
42038cf6:	669a      	add.n	a6, a6, a9
42038cf8:	c09a90        	sub	a9, a10, a9
42038cfb:	439950        	min	a9, a9, a5
42038cfe:	3199      	s32i.n	a9, a1, 12
42038d00:	039792        	l16si	a9, a7, 6
42038d03:	536630        	max	a6, a6, a3
42038d06:	998a      	add.n	a9, a9, a8
42038d08:	99ba      	add.n	a9, a9, a11
42038d0a:	7199      	s32i.n	a9, a1, 28
42038d0c:	059792        	l16si	a9, a7, 10
42038d0f:	a1f8      	l32i.n	a15, a1, 40
42038d11:	c08980        	sub	a8, a9, a8
42038d14:	2189      	s32i.n	a8, a1, 8
42038d16:	c08530        	sub	a8, a5, a3
42038d19:	881b      	addi.n	a8, a8, 1
42038d1b:	4189      	s32i.n	a8, a1, 16
42038d1d:	3188      	l32i.n	a8, a1, 12
42038d1f:	881b      	addi.n	a8, a8, 1
42038d21:	5189      	s32i.n	a8, a1, 20
42038d23:	c08860        	sub	a8, a8, a6
42038d26:	1188e0        	slli	a8, a8, 2
42038d29:	8189      	s32i.n	a8, a1, 32
42038d2b:	4188      	l32i.n	a8, a1, 16
42038d2d:	1128e0        	slli	a2, a8, 2
42038d30:	c08630        	sub	a8, a6, a3
42038d33:	1188e0        	slli	a8, a8, 2
42038d36:	9189      	s32i.n	a8, a1, 36
42038d38:	238680        	sext	a8, a6, 15
42038d3b:	6189      	s32i.n	a8, a1, 24
42038d3d:	1188      	l32i.n	a8, a1, 4
42038d3f:	03a847        	bge	a8, a4, 42038d46 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xce>
42038d42:	020c      	movi.n	a2, 0
42038d44:	f01d      	retw.n
42038d46:	31a8      	l32i.n	a10, a1, 12
42038d48:	038d      	mov.n	a8, a3
42038d4a:	0f9d      	mov.n	a9, a15
42038d4c:	2b2a67        	blt	a10, a6, 42038d7b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x103>
42038d4f:	0f8d      	mov.n	a8, a15
42038d51:	03bd      	mov.n	a11, a3
42038d53:	23c480        	sext	a12, a4, 15
42038d56:	02ab67        	bge	a11, a6, 42038d5c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe4>
42038d59:	0024c6        	j	42038df0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x178>
42038d5c:	9188      	l32i.n	a8, a1, 36
42038d5e:	61b8      	l32i.n	a11, a1, 24
42038d60:	9f8a      	add.n	a9, a15, a8
42038d62:	07ad      	mov.n	a10, a7
42038d64:	b1f9      	s32i.n	a15, a1, 44
42038d66:	a199      	s32i.n	a9, a1, 40
42038d68:	ffece5        	call8	42038c38 <roo_windows::Decoration::read(short, short) const>
42038d6b:	8188      	l32i.n	a8, a1, 32
42038d6d:	a198      	l32i.n	a9, a1, 40
42038d6f:	b1f8      	l32i.n	a15, a1, 44
42038d71:	898a      	add.n	a8, a9, a8
42038d73:	021897        	beq	a8, a9, 42038d79 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x101>
42038d76:	0025c6        	j	42038e11 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x199>
42038d79:	5188      	l32i.n	a8, a1, 20
42038d7b:	09dd      	mov.n	a13, a9
42038d7d:	08bd      	mov.n	a11, a8
42038d7f:	23c480        	sext	a12, a4, 15
42038d82:	0225b7        	blt	a5, a11, 42038d88 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x110>
42038d85:	0023c6        	j	42038e18 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a0>
42038d88:	b51b      	addi.n	a11, a5, 1
42038d8a:	72ab80        	salt	a10, a11, a8
42038d8d:	aa0b      	addi.n	a10, a10, -1
42038d8f:	60a0a0        	neg	a10, a10
42038d92:	74a0a0        	extui	a10, a10, 0, 8
42038d95:	0c0c      	movi.n	a12, 0
42038d97:	4a8c      	beqz.n	a10, 42038d9f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x127>
42038d99:	c0bb80        	sub	a11, a11, a8
42038d9c:	11cbe0        	slli	a12, a11, 2
42038d9f:	71a8      	l32i.n	a10, a1, 28
42038da1:	99ca      	add.n	a9, a9, a12
42038da3:	72a4a0        	salt	a10, a4, a10
42038da6:	aa0b      	addi.n	a10, a10, -1
42038da8:	60a0a0        	neg	a10, a10
42038dab:	74a0a0        	extui	a10, a10, 0, 8
42038dae:	098d      	mov.n	a8, a9
42038db0:	4abc      	beqz.n	a10, 42038de8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038db2:	21a8      	l32i.n	a10, a1, 8
42038db4:	72aa40        	salt	a10, a10, a4
42038db7:	aa0b      	addi.n	a10, a10, -1
42038db9:	60a0a0        	neg	a10, a10
42038dbc:	74a0a0        	extui	a10, a10, 0, 8
42038dbf:	5aac      	beqz.n	a10, 42038de8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038dc1:	11a8      	l32i.n	a10, a1, 4
42038dc3:	21b8      	l32i.n	a11, a1, 8
42038dc5:	04ed      	mov.n	a14, a4
42038dc7:	43dab0        	min	a13, a10, a11
42038dca:	742ed7        	blt	a14, a13, 42038e42 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038dcd:	728d40        	salt	a8, a13, a4
42038dd0:	880b      	addi.n	a8, a8, -1
42038dd2:	608080        	neg	a8, a8
42038dd5:	748080        	extui	a8, a8, 0, 8
42038dd8:	00a0a2        	movi	a10, 0
42038ddb:	002816        	beqz	a8, 42038de1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x169>
42038dde:	c0ad40        	sub	a10, a13, a4
42038de1:	828a20        	mull	a8, a10, a2
42038de4:	4a4a      	add.n	a4, a10, a4
42038de6:	889a      	add.n	a8, a8, a9
42038de8:	441b      	addi.n	a4, a4, 1
42038dea:	08fd      	mov.n	a15, a8
42038dec:	ffd346        	j	42038d3d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xc5>
42038def:	a77000        	ee.stf.64.xp	f7, f10, a0, a0
42038df2:	61f220        	xsr.ccompare2	a2
42038df5:	0d          	.byte	0xd
42038df6:	0c6182        	s32i	a8, a1, 48
42038df9:	b1c9      	s32i.n	a12, a1, 44
42038dfb:	a1b9      	s32i.n	a11, a1, 40
42038dfd:	ffe3a5        	call8	42038c38 <roo_windows::Decoration::read(short, short) const>
42038e00:	c188      	l32i.n	a8, a1, 48
42038e02:	a1b8      	l32i.n	a11, a1, 40
42038e04:	08a9      	s32i.n	a10, a8, 0
42038e06:	d1f8      	l32i.n	a15, a1, 52
42038e08:	b1c8      	l32i.n	a12, a1, 44
42038e0a:	bb1b      	addi.n	a11, a11, 1
42038e0c:	884b      	addi.n	a8, a8, 4
42038e0e:	ffd106        	j	42038d56 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xde>
42038e11:	09a9      	s32i.n	a10, a9, 0
42038e13:	994b      	addi.n	a9, a9, 4
42038e15:	ffd686        	j	42038d73 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xfb>
42038e18:	20a770        	or	a10, a7, a7
42038e1b:	0f61f2        	s32i	a15, a1, 60
42038e1e:	0e61d2        	s32i	a13, a1, 56
42038e21:	d189      	s32i.n	a8, a1, 52
42038e23:	c199      	s32i.n	a9, a1, 48
42038e25:	b1c9      	s32i.n	a12, a1, 44
42038e27:	a1b9      	s32i.n	a11, a1, 40
42038e29:	ffe0e5        	call8	42038c38 <roo_windows::Decoration::read(short, short) const>
42038e2c:	e1d8      	l32i.n	a13, a1, 56
42038e2e:	a1b8      	l32i.n	a11, a1, 40
42038e30:	0da9      	s32i.n	a10, a13, 0
42038e32:	f1f8      	l32i.n	a15, a1, 60
42038e34:	d188      	l32i.n	a8, a1, 52
42038e36:	c198      	l32i.n	a9, a1, 48
42038e38:	b1c8      	l32i.n	a12, a1, 44
42038e3a:	bb1b      	addi.n	a11, a11, 1
42038e3c:	dd4b      	addi.n	a13, a13, 4
42038e3e:	ffd006        	j	42038d82 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10a>
42038e41:	41a800        	srli	a10, a0, 8
42038e44:	201aa6        	blti	a10, 1, 42038e68 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038e47:	254226        	beqi	a2, 4, 42038e70 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f8>
42038e4a:	0fbd      	mov.n	a11, a15
42038e4c:	08ad      	mov.n	a10, a8
42038e4e:	02cd      	mov.n	a12, a2
42038e50:	d199      	s32i.n	a9, a1, 52
42038e52:	c1e9      	s32i.n	a14, a1, 48
42038e54:	b1d9      	s32i.n	a13, a1, 44
42038e56:	a1f9      	s32i.n	a15, a1, 40
42038e58:	1ea381        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038e5b:	0008e0        	callx8	a8
42038e5e:	a1f8      	l32i.n	a15, a1, 40
42038e60:	b1d8      	l32i.n	a13, a1, 44
42038e62:	c1e8      	l32i.n	a14, a1, 48
42038e64:	d198      	l32i.n	a9, a1, 52
42038e66:	0a8d      	mov.n	a8, a10
42038e68:	882a      	add.n	a8, a8, a2
42038e6a:	ee1b      	addi.n	a14, a14, 1
42038e6c:	ffd686        	j	42038dca <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x152>
42038e6f:	00          	.byte	00
42038e70:	0fa8      	l32i.n	a10, a15, 0
42038e72:	08a9      	s32i.n	a10, a8, 0
42038e74:	fffc06        	j	42038e68 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038e77:	600000        	neg	a0, a0
42038e7a:	4020b6        	bltui	a0, 2, 42038ebe <roo_windows::internal::MakeLookupShadow(roo_display::Box, roo_windows::BorderStyle::CornerRadii, int, bool)+0x16>
42038e7d:	c4          	.byte	0xc4
42038e7e:	a77020        	ee.stf.64.xp	f7, f10, a2, a0
42038e81:	db6520        	movt.s	f6, f5, b2
42038e84:	a9661bff 	ee.fft.cmul.s16.st.xp	q4, q6, q6, a15, a11, 2, 2, 0
42038e88:	02          	.byte	0x2
42038e89:	236680        	sext	a6, a6, 15
42038e8c:	224b      	addi.n	a2, a2, 4
42038e8e:	02fd      	mov.n	a15, a2
42038e90:	e5a567        	bge	a5, a6, 42038e79 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x201>
42038e93:	441b      	addi.n	a4, a4, 1
42038e95:	234480        	sext	a4, a4, 15
42038e98:	1188      	l32i.n	a8, a1, 4
42038e9a:	02a847        	bge	a8, a4, 42038ea0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038e9d:	ffa846        	j	42038d42 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xca>
42038ea0:	0f2d      	mov.n	a2, a15
42038ea2:	036d      	mov.n	a6, a3
42038ea4:	fff986        	j	42038e8e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>
