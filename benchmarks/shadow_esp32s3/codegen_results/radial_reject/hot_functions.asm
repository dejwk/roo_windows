
/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c58 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c58:	004136        	entry	a1, 32
42038c5b:	15cc      	bnez.n	a5, 42038c60 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8>
42038c5d:	f01d      	retw.n
42038c5f:	94c200        	extui	a12, a0, 2, 10
42038c62:	93b200        	movnez	a11, a2, a0
42038c65:	a22000        	muluh	a2, a0, a0
42038c68:	20          	.byte	0x20
42038c69:	fffd25        	call8	42038c3c <roo_windows::Decoration::read(short, short) const>
42038c6c:	0066a2        	s32i	a10, a6, 0
42038c6f:	02c332        	addi	a3, a3, 2
42038c72:	442b      	addi.n	a4, a4, 2
42038c74:	550b      	addi.n	a5, a5, -1
42038c76:	664b      	addi.n	a6, a6, 4
42038c78:	fff7c6        	j	42038c5b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3>


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203312c <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
4203312c:	006136        	entry	a1, 48
4203312f:	002282        	l32i	a8, a2, 0
42033132:	20f770        	or	a15, a7, a7
42033135:	0b2882        	l32i	a8, a8, 44
42033138:	233380        	sext	a3, a3, 15
4203313b:	234480        	sext	a4, a4, 15
4203313e:	235580        	sext	a5, a5, 15
42033141:	236680        	sext	a6, a6, 15
42033144:	02ad      	mov.n	a10, a2
42033146:	31f9      	s32i.n	a15, a1, 12
42033148:	06ed      	mov.n	a14, a6
4203314a:	05dd      	mov.n	a13, a5
4203314c:	04cd      	mov.n	a12, a4
4203314e:	03bd      	mov.n	a11, a3
42033150:	0008e0        	callx8	a8
42033153:	027d      	mov.n	a7, a2
42033155:	0a2d      	mov.n	a2, a10
42033157:	facc      	bnez.n	a10, 4203316a <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
42033159:	31f8      	l32i.n	a15, a1, 12
4203315b:	06ed      	mov.n	a14, a6
4203315d:	05dd      	mov.n	a13, a5
4203315f:	04cd      	mov.n	a12, a4
42033161:	03bd      	mov.n	a11, a3
42033163:	07ad      	mov.n	a10, a7
42033165:	dc5da5        	call8	4200f740 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
42033168:	0a2d      	mov.n	a2, a10
4203316a:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42086564 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
42086564:	004136        	entry	a1, 32
42086567:	029d      	mov.n	a9, a2
42086569:	120c      	movi.n	a2, 1
4208656b:	832990        	moveqz	a2, a9, a9
4208656e:	4149b6        	bltui	a9, 4, 420865b3 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
42086571:	40f980        	nsau	a8, a9
42086574:	dfc882        	addi	a8, a8, -33
42086577:	608080        	neg	a8, a8
4208657a:	218180        	srai	a8, a8, 1
4208657d:	fec882        	addi	a8, a8, -2
42086580:	3a0c      	movi.n	a10, 3
42086582:	401800        	ssl	a8
42086585:	a1aa00        	sll	a10, a10
42086588:	c289a0        	quou	a8, a9, a10
4208658b:	88aa      	add.n	a8, a8, a10
4208658d:	418180        	srli	a8, a8, 1
42086590:	c2a980        	quou	a10, a9, a8
42086593:	aa8a      	add.n	a10, a10, a8
42086595:	41a1a0        	srli	a10, a10, 1
42086598:	c289a0        	quou	a8, a9, a10
4208659b:	88aa      	add.n	a8, a8, a10
4208659d:	1a0c      	movi.n	a10, 1
4208659f:	418180        	srli	a8, a8, 1
420865a2:	11aa00        	slli	a10, a10, 16
420865a5:	0518a7        	beq	a8, a10, 420865ae <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
420865a8:	82a880        	mull	a10, a8, a8
420865ab:	01b9a7        	bgeu	a9, a10, 420865b0 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
420865ae:	880b      	addi.n	a8, a8, -1
420865b0:	f42080        	extui	a2, a8, 0, 16
420865b3:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420863c4 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
420863c4:	004136        	entry	a1, 32
420863c7:	208230        	or	a8, a2, a3
420863ca:	0228d6        	bgez	a8, 420863f0 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
420863cd:	c02240        	sub	a2, a2, a4
420863d0:	40f290        	nsau	a9, a2
420863d3:	419590        	srli	a9, a9, 5
420863d6:	b28c      	beqz.n	a2, 420863e5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
420863d8:	c03340        	sub	a3, a3, a4
420863db:	180c      	movi.n	a8, 1
420863dd:	938930        	movnez	a8, a9, a3
420863e0:	ffa022        	movi	a2, 255
420863e3:	688c      	beqz.n	a8, 420863ed <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863e5:	1125c0        	slli	a2, a5, 4
420863e8:	252a      	add.n	a2, a5, a2
420863ea:	742020        	extui	a2, a2, 0, 8
420863ed:	f01d      	retw.n
420863ef:	222000        	orb	b2, b0, b0
420863f2:	3330d1        	l32r	a13, 420530b4 <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
420863f5:	823ad1        	l32r	a13, 42066ce0 <esp_littlefs_free_fd$isra$0+0x48> (880b3d12 <_rtc_reserved_end+0x27fb3d12>)
420863f8:	118880        	slli	a8, a8, 8
420863fb:	ffa022        	movi	a2, 255
420863fe:	eb48a6        	blti	a8, 4, 420863ed <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086401:	40f890        	nsau	a9, a8
42086404:	dfc992        	addi	a9, a9, -33
42086407:	609090        	neg	a9, a9
4208640a:	219190        	srai	a9, a9, 1
4208640d:	fec992        	addi	a9, a9, -2
42086410:	3a0c      	movi.n	a10, 3
42086412:	401900        	ssl	a9
42086415:	a1aa00        	sll	a10, a10
42086418:	c298a0        	quou	a9, a8, a10
4208641b:	99aa      	add.n	a9, a9, a10
4208641d:	419190        	srli	a9, a9, 1
42086420:	c2a890        	quou	a10, a8, a9
42086423:	aa9a      	add.n	a10, a10, a9
42086425:	41a1a0        	srli	a10, a10, 1
42086428:	c298a0        	quou	a9, a8, a10
4208642b:	99aa      	add.n	a9, a9, a10
4208642d:	1a0c      	movi.n	a10, 1
4208642f:	419190        	srli	a9, a9, 1
42086432:	11aa00        	slli	a10, a10, 16
42086435:	0519a7        	beq	a9, a10, 4208643e <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7a>
42086438:	82a990        	mull	a10, a9, a9
4208643b:	01b8a7        	bgeu	a8, a10, 42086440 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7c>
4208643e:	990b      	addi.n	a9, a9, -1
42086440:	1144c0        	slli	a4, a4, 4
42086443:	554a      	add.n	a5, a5, a4
42086445:	c05590        	sub	a5, a5, a9
42086448:	f48050        	extui	a8, a5, 0, 16
4208644b:	239580        	sext	a9, a5, 15
4208644e:	020c      	movi.n	a2, 0
42086450:	99e5f7        	bbsi	a5, 15, 420863ed <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086453:	fa0c      	movi.n	a10, 15
42086455:	ffa022        	movi	a2, 255
42086458:	912a97        	blt	a10, a9, 420863ed <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208645b:	1198c0        	slli	a9, a8, 4
4208645e:	889a      	add.n	a8, a8, a9
42086460:	742080        	extui	a2, a8, 0, 8
42086463:	ffe186        	j	420863ed <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033ea0 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033ea0:	00a136        	entry	a1, 80
42033ea3:	3169      	s32i.n	a6, a1, 12
42033ea5:	2159      	s32i.n	a5, a1, 8
42033ea7:	060c      	movi.n	a6, 0
42033ea9:	1cc172        	addi	a7, a1, 28
42033eac:	2188      	l32i.n	a8, a1, 8
42033eae:	029687        	bne	a6, a8, 42033eb4 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033eb1:	f01d      	retw.n
42033eb3:	938200        	movnez	a8, a2, a0
42033eb6:	22d200        	orb	b13, b2, b0
42033eb9:	018917        	bany	a9, a1, 42033ebe <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033ebc:	009482        	l16si	a8, a4, 0
42033ebf:	01b8      	l32i.n	a11, a1, 0
42033ec1:	1622a2        	l32i	a10, a2, 88
42033ec4:	08cd      	mov.n	a12, a8
42033ec6:	0067d2        	s32i	a13, a7, 0
42033ec9:	20e770        	or	a14, a7, a7
42033ecc:	016182        	s32i	a8, a1, 4
42033ecf:	201110        	or	a1, a1, a1
42033ed2:	fffa25        	call8	42033e74 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033ed5:	182282        	l32i	a8, a2, 96
42033ed8:	1922b2        	l32i	a11, a2, 100
42033edb:	8189      	s32i.n	a8, a1, 32
42033edd:	20c152        	addi	a5, a1, 32
42033ee0:	050b16        	beqz	a11, 42033f34 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033ee3:	0bc8      	l32i.n	a12, a11, 0
42033ee5:	24c1a2        	addi	a10, a1, 36
42033ee8:	201110        	or	a1, a1, a1
42033eeb:	ff3665        	call8	42033250 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033eee:	07d8      	l32i.n	a13, a7, 0
42033ef0:	11c8      	l32i.n	a12, a1, 4
42033ef2:	0021b2        	l32i	a11, a1, 0
42033ef5:	24c1a2        	addi	a10, a1, 36
42033ef8:	201110        	or	a1, a1, a1
42033efb:	fe1865        	call8	42032080 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033efe:	07a9      	s32i.n	a10, a7, 0
42033f00:	24c1a2        	addi	a10, a1, 36
42033f03:	201110        	or	a1, a1, a1
42033f06:	f894a5        	call8	4202c850 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f09:	1922b2        	l32i	a11, a2, 100
42033f0c:	24c1a2        	addi	a10, a1, 36
42033f0f:	002bc2        	l32i	a12, a11, 0
42033f12:	201110        	or	a1, a1, a1
42033f15:	ff33a5        	call8	42033250 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f18:	05d8      	l32i.n	a13, a5, 0
42033f1a:	0094c2        	l16si	a12, a4, 0
42033f1d:	0093b2        	l16si	a11, a3, 0
42033f20:	24c1a2        	addi	a10, a1, 36
42033f23:	201110        	or	a1, a1, a1
42033f26:	fe15a5        	call8	42032080 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f29:	05a9      	s32i.n	a10, a5, 0
42033f2b:	24c1a2        	addi	a10, a1, 36
42033f2e:	201110        	or	a1, a1, a1
42033f31:	f891e5        	call8	4202c850 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f34:	0094c2        	l16si	a12, a4, 0
42033f37:	0093b2        	l16si	a11, a3, 0
42033f3a:	07d8      	l32i.n	a13, a7, 0
42033f3c:	05ed      	mov.n	a14, a5
42033f3e:	a24b      	addi.n	a10, a2, 4
42033f40:	201110        	or	a1, a1, a1
42033f43:	04b7e5        	call8	42038ac0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f46:	3188      	l32i.n	a8, a1, 12
42033f48:	332b      	addi.n	a3, a3, 2
42033f4a:	a08680        	addx4	a8, a6, a8
42033f4d:	08a9      	s32i.n	a10, a8, 0
42033f4f:	661b      	addi.n	a6, a6, 1
42033f51:	442b      	addi.n	a4, a4, 2
42033f53:	ffd546        	j	42033eac <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f56:	c1a200        	mul16u	a10, a2, a0
42033f59:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f5c:	20          	.byte	0x20
42033f5d:	f88f25        	call8	4202c850 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f60:	02ad      	mov.n	a10, a2
42033f62:	201110        	or	a1, a1, a1
42033f65:	3cc8a5        	call8	42070bf0 <_Unwind_Resume>
42033f68:	fffac6        	j	42033f57 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038718 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
42038718:	006136        	entry	a1, 48
4203871b:	189192        	l16si	a9, a1, 48
4203871e:	3169      	s32i.n	a6, a1, 12
42038720:	f1cb      	addi.n	a15, a1, 12
42038722:	1a9182        	l16si	a8, a1, 52
42038725:	02ed      	mov.n	a14, a2
42038727:	74d070        	extui	a13, a7, 0, 8
4203872a:	02a927        	bge	a9, a2, 42038730 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
4203872d:	0030c6        	j	420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038730:	02a497        	bge	a4, a9, 42038736 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
42038733:	002f46        	j	420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038736:	02a837        	bge	a8, a3, 4203873c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
42038739:	002dc6        	j	420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203873c:	02a587        	bge	a5, a8, 42038742 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a>
4203873f:	002c46        	j	420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038742:	000fc2        	l8ui	a12, a15, 0
42038745:	3c9c      	beqz.n	a12, 4203875c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038747:	a2ca      	add.n	a10, a2, a12
42038749:	0f2a97        	blt	a10, a9, 4203875c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
4203874c:	b3ca      	add.n	a11, a3, a12
4203874e:	0a2b87        	blt	a11, a8, 4203875c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038751:	c0bb80        	sub	a11, a11, a8
42038754:	c0aa90        	sub	a10, a10, a9
42038757:	000706        	j	42038777 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x5f>
4203875a:	c20000        	quou	a0, a0, a0
4203875d:	1c16010f 	ee.vmulas.s16.qacc.ld.ip.qup	q0, a0, -240, q4, q0, q1, q6
42038761:	a4c002        	addi	a0, a0, -92
42038764:	c0          	.byte	0xc0
42038765:	1b29a7        	blt	a9, a10, 42038784 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038768:	80b3c0        	add	a11, a3, a12
4203876b:	152b87        	blt	a11, a8, 42038784 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
4203876e:	c0bb80        	sub	a11, a11, a8
42038771:	c0ac40        	sub	a10, a12, a4
42038774:	80aa90        	add	a10, a10, a9
42038777:	23bb80        	sext	a11, a11, 15
4203877a:	23aa80        	sext	a10, a10, 15
4203877d:	4dc465        	call8	420863c4 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
42038780:	0a2d      	mov.n	a2, a10
42038782:	f01d      	retw.n
42038784:	020fc2        	l8ui	a12, a15, 2
42038787:	2c9c      	beqz.n	a12, 4203879d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038789:	c0a4c0        	sub	a10, a4, a12
4203878c:	0d29a7        	blt	a9, a10, 4203879d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
4203878f:	c0a5c0        	sub	a10, a5, a12
42038792:	0728a7        	blt	a8, a10, 4203879d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038795:	c0bc50        	sub	a11, a12, a5
42038798:	bb8a      	add.n	a11, a11, a8
4203879a:	fff4c6        	j	42038771 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x59>
4203879d:	030fc2        	l8ui	a12, a15, 3
420387a0:	1c9c      	beqz.n	a12, 420387b5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387a2:	aeca      	add.n	a10, a14, a12
420387a4:	0d2a97        	blt	a10, a9, 420387b5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387a7:	c0b5c0        	sub	a11, a5, a12
420387aa:	0728b7        	blt	a8, a11, 420387b5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387ad:	c0bc50        	sub	a11, a12, a5
420387b0:	bb8a      	add.n	a11, a11, a8
420387b2:	ffe786        	j	42038754 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3c>
420387b5:	ea0c      	movi.n	a10, 14
420387b7:	ffa022        	movi	a2, 255
420387ba:	c43ad7        	bltu	a10, a13, 42038782 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387bd:	c0e9e0        	sub	a14, a9, a14
420387c0:	40fea0        	nsau	a10, a14
420387c3:	1b0c      	movi.n	a11, 1
420387c5:	41a5a0        	srli	a10, a10, 5
420387c8:	ce9c      	beqz.n	a14, 420387e8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387ca:	c09940        	sub	a9, a9, a4
420387cd:	83ab90        	moveqz	a10, a11, a9
420387d0:	014a56        	bnez	a10, 420387e8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387d3:	c03830        	sub	a3, a8, a3
420387d6:	40f390        	nsau	a9, a3
420387d9:	419590        	srli	a9, a9, 5
420387dc:	008316        	beqz	a3, 420387e8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387df:	c08850        	sub	a8, a8, a5
420387e2:	93b980        	movnez	a11, a9, a8
420387e5:	f99b16        	beqz	a11, 42038782 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387e8:	118dc0        	slli	a8, a13, 4
420387eb:	dd8a      	add.n	a13, a13, a8
420387ed:	7420d0        	extui	a2, a13, 0, 8
420387f0:	ffe386        	j	42038782 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387f3:	020c00        	andb	b0, b12, b0
420387f6:	ffe206        	j	42038782 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038ac0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038ac0:	008136        	entry	a1, 64
42038ac3:	2002a2        	l8ui	a10, a2, 32
42038ac6:	021292        	l16ui	a9, a2, 4
42038ac9:	0512d2        	l16ui	a13, a2, 10
42038acc:	0412c2        	l16ui	a12, a2, 8
42038acf:	0312b2        	l16ui	a11, a2, 6
42038ad2:	233380        	sext	a3, a3, 15
42038ad5:	234480        	sext	a4, a4, 15
42038ad8:	1149      	s32i.n	a4, a1, 4
42038ada:	0139      	s32i.n	a3, a1, 0
42038adc:	c0dda0        	sub	a13, a13, a10
42038adf:	c0cca0        	sub	a12, a12, a10
42038ae2:	baba      	add.n	a11, a10, a11
42038ae4:	2102f2        	l8ui	a15, a2, 33
42038ae7:	aa9a      	add.n	a10, a10, a9
42038ae9:	72e8      	l32i.n	a14, a2, 28
42038aeb:	23dd80        	sext	a13, a13, 15
42038aee:	23cc80        	sext	a12, a12, 15
42038af1:	23bb80        	sext	a11, a11, 15
42038af4:	23aa80        	sext	a10, a10, 15
42038af7:	7169      	s32i.n	a6, a1, 28
42038af9:	ffc1e5        	call8	42038718 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038afc:	ffa092        	movi	a9, 255
42038aff:	027d      	mov.n	a7, a2
42038b01:	0a6d      	mov.n	a6, a10
42038b03:	079a97        	bne	a10, a9, 42038b0e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x4e>
42038b06:	142292        	l32i	a9, a2, 80
42038b09:	052d      	mov.n	a2, a5
42038b0b:	0a1916        	beqz	a9, 42038bb0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038b0e:	04dd      	mov.n	a13, a4
42038b10:	03cd      	mov.n	a12, a3
42038b12:	3cc7b2        	addi	a11, a7, 60
42038b15:	28c7a2        	addi	a10, a7, 40
42038b18:	005565        	call8	42039070 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b1b:	200792        	l8ui	a9, a7, 32
42038b1e:	6169      	s32i.n	a6, a1, 24
42038b20:	012a80        	slli	a2, a10, 24
42038b23:	03c916        	beqz	a9, 42038b63 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038b26:	1149      	s32i.n	a4, a1, 4
42038b28:	0139      	s32i.n	a3, a1, 0
42038b2a:	67e8      	l32i.n	a14, a7, 24
42038b2c:	0597d2        	l16si	a13, a7, 10
42038b2f:	0497c2        	l16si	a12, a7, 8
42038b32:	0397b2        	l16si	a11, a7, 6
42038b35:	0297a2        	l16si	a10, a7, 4
42038b38:	ff0c      	movi.n	a15, 15
42038b3a:	ffbde5        	call8	42038718 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038b3d:	61a9      	s32i.n	a10, a1, 24
42038b3f:	0aac      	beqz.n	a10, 42038b63 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038b41:	7188      	l32i.n	a8, a1, 28
42038b43:	38cc      	bnez.n	a8, 42038b4a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x8a>
42038b45:	24c782        	addi	a8, a7, 36
42038b48:	7189      	s32i.n	a8, a1, 28
42038b4a:	7188      	l32i.n	a8, a1, 28
42038b4c:	02ad      	mov.n	a10, a2
42038b4e:	0898      	l32i.n	a9, a8, 0
42038b50:	6188      	l32i.n	a8, a1, 24
42038b52:	119980        	slli	a9, a9, 8
42038b55:	419890        	srli	a9, a9, 8
42038b58:	01b880        	slli	a11, a8, 24
42038b5b:	20b9b0        	or	a11, a9, a11
42038b5e:	cecee5        	call8	4200784c <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038b61:	0a2d      	mov.n	a2, a10
42038b63:	ffa092        	movi	a9, 255
42038b66:	4a9697        	bne	a6, a9, 42038bb4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf4>
42038b69:	fb7c      	movi.n	a11, -1
42038b6b:	01bb80        	slli	a11, a11, 24
42038b6e:	20b5b0        	or	a11, a5, a11
42038b71:	02ad      	mov.n	a10, a2
42038b73:	cecda5        	call8	4200784c <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038b76:	0a2d      	mov.n	a2, a10
42038b78:	1427a2        	l32i	a10, a7, 80
42038b7b:	031a16        	beqz	a10, 42038bb0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038b7e:	180a82        	l8ui	a8, a10, 24
42038b81:	053816        	beqz	a8, 42038bd8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x118>
42038b84:	20c440        	or	a12, a4, a4
42038b87:	03bd      	mov.n	a11, a3
42038b89:	ee2f65        	call8	42026e80 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
42038b8c:	6188      	l32i.n	a8, a1, 24
42038b8e:	75b8a0        	extui	a11, a10, 24, 8
42038b91:	d1bb80        	mul16s	a11, a11, a8
42038b94:	1f5381        	l32r	a8, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038b97:	11aa80        	slli	a10, a10, 8
42038b9a:	a2bb80        	muluh	a11, a11, a8
42038b9d:	01bbf0        	slli	a11, a11, 17
42038ba0:	75b8b0        	extui	a11, a11, 24, 8
42038ba3:	404800        	ssai	8
42038ba6:	81bba0        	src	a11, a11, a10
42038ba9:	02ad      	mov.n	a10, a2
42038bab:	ceca25        	call8	4200784c <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038bae:	0a2d      	mov.n	a2, a10
42038bb0:	f01d      	retw.n
42038bb2:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038bb5:	50fc06        	j	4204cfa9 <uart_set_word_length+0x65>
42038bb8:	7598      	l32i.n	a9, a5, 28
42038bba:	d19960        	mul16s	a9, a9, a6
42038bbd:	1f48a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038bc0:	7fc992        	addi	a9, a9, 127
42038bc3:	a299a0        	muluh	a9, a9, a10
42038bc6:	11b580        	slli	a11, a5, 8
42038bc9:	0199f0        	slli	a9, a9, 17
42038bcc:	759890        	extui	a9, a9, 24, 8
42038bcf:	404800        	ssai	8
42038bd2:	81b9b0        	src	a11, a9, a11
42038bd5:	ffe606        	j	42038b71 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb1>
42038bd8:	021a82        	l16ui	a8, a10, 4
42038bdb:	3ab8      	l32i.n	a11, a10, 12
42038bdd:	c03380        	sub	a3, a3, a8
42038be0:	031a82        	l16ui	a8, a10, 6
42038be3:	c04480        	sub	a4, a4, a8
42038be6:	740444        	mul.aa.ll	a4, a4
42038be9:	780334        	mula.aa.ll	a3, a3
42038bec:	031080        	rsr.acclo	a8
42038bef:	c0bb80        	sub	a11, a11, a8
42038bf2:	061be6        	bgei	a11, 1, 42038bfc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x13c>
42038bf5:	5aa8      	l32i.n	a10, a10, 20
42038bf7:	ffe446        	j	42038b8c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038bfa:	00          	.byte	00
42038bfb:	9a9200        	trunc.s	a9, f2, 0
42038bfe:	04          	.byte	0x4
42038bff:	3119a6        	blti	a9, 1, 42038c34 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c02:	899a      	add.n	a8, a9, a9
42038c04:	60a0c2        	movi	a12, 96
42038c07:	0f2b87        	blt	a11, a8, 42038c1a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038c0a:	261926        	beqi	a9, 1, 42038c34 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c0d:	c90b      	addi.n	a12, a9, -1
42038c0f:	23cc80        	sext	a12, a12, 15
42038c12:	11cce0        	slli	a12, a12, 2
42038c15:	0c2cb7        	blt	a12, a11, 42038c25 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x165>
42038c18:	0c4c      	movi.n	a12, 64
42038c1a:	4ab8      	l32i.n	a11, a10, 16
42038c1c:	5aa8      	l32i.n	a10, a10, 20
42038c1e:	4c2425        	call8	42084e60 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42038c21:	ffd9c6        	j	42038b8c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038c24:	00          	.byte	00
42038c25:	0b2926        	beqi	a9, 2, 42038c34 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c28:	889a      	add.n	a8, a8, a9
42038c2a:	888a      	add.n	a8, a8, a8
42038c2c:	f8c882        	addi	a8, a8, -8
42038c2f:	0c2c      	movi.n	a12, 32
42038c31:	e52b87        	blt	a11, a8, 42038c1a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038c34:	4aa8      	l32i.n	a10, a10, 16
42038c36:	ffd486        	j	42038b8c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c7c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038c7c:	00c136        	entry	a1, 96
42038c7f:	238680        	sext	a8, a6, 15
42038c82:	1189      	s32i.n	a8, a1, 4
42038c84:	142282        	l32i	a8, a2, 80
42038c87:	07fd      	mov.n	a15, a7
42038c89:	027d      	mov.n	a7, a2
42038c8b:	233380        	sext	a3, a3, 15
42038c8e:	234480        	sext	a4, a4, 15
42038c91:	235580        	sext	a5, a5, 15
42038c94:	204856        	bnez	a8, 42038e9c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038c97:	0288      	l32i.n	a8, a2, 0
42038c99:	11e8      	l32i.n	a14, a1, 4
42038c9b:	b888      	l32i.n	a8, a8, 44
42038c9d:	02ad      	mov.n	a10, a2
42038c9f:	a1f9      	s32i.n	a15, a1, 40
42038ca1:	05dd      	mov.n	a13, a5
42038ca3:	04cd      	mov.n	a12, a4
42038ca5:	03bd      	mov.n	a11, a3
42038ca7:	0008e0        	callx8	a8
42038caa:	0a2d      	mov.n	a2, a10
42038cac:	098a56        	bnez	a10, 42038d48 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcc>
42038caf:	1a9792        	l16si	a9, a7, 52
42038cb2:	249782        	l16si	a8, a7, 72
42038cb5:	ec0c      	movi.n	a12, 14
42038cb7:	538890        	max	a8, a8, a9
42038cba:	98fb      	addi.n	a9, a8, 15
42038cbc:	1ec882        	addi	a8, a8, 30
42038cbf:	b38990        	movgez	a8, a9, a9
42038cc2:	21b480        	srai	a11, a8, 4
42038cc5:	210782        	l8ui	a8, a7, 33
42038cc8:	1907a2        	l8ui	a10, a7, 25
42038ccb:	62cc80        	saltu	a12, a12, a8
42038cce:	180782        	l8ui	a8, a7, 24
42038cd1:	200792        	l8ui	a9, a7, 32
42038cd4:	7388a0        	maxu	a8, a8, a10
42038cd7:	1a07a2        	l8ui	a10, a7, 26
42038cda:	cc0b      	addi.n	a12, a12, -1
42038cdc:	7388a0        	maxu	a8, a8, a10
42038cdf:	1b07a2        	l8ui	a10, a7, 27
42038ce2:	60c0c0        	neg	a12, a12
42038ce5:	73a8a0        	maxu	a10, a8, a10
42038ce8:	89ca      	add.n	a8, a9, a12
42038cea:	99ba      	add.n	a9, a9, a11
42038cec:	538a80        	max	a8, a10, a8
42038cef:	029762        	l16si	a6, a7, 4
42038cf2:	0497a2        	l16si	a10, a7, 8
42038cf5:	99ca      	add.n	a9, a9, a12
42038cf7:	539980        	max	a9, a9, a8
42038cfa:	669a      	add.n	a6, a6, a9
42038cfc:	c09a90        	sub	a9, a10, a9
42038cff:	439950        	min	a9, a9, a5
42038d02:	3199      	s32i.n	a9, a1, 12
42038d04:	039792        	l16si	a9, a7, 6
42038d07:	536630        	max	a6, a6, a3
42038d0a:	998a      	add.n	a9, a9, a8
42038d0c:	99ba      	add.n	a9, a9, a11
42038d0e:	7199      	s32i.n	a9, a1, 28
42038d10:	059792        	l16si	a9, a7, 10
42038d13:	a1f8      	l32i.n	a15, a1, 40
42038d15:	c08980        	sub	a8, a9, a8
42038d18:	2189      	s32i.n	a8, a1, 8
42038d1a:	c08530        	sub	a8, a5, a3
42038d1d:	881b      	addi.n	a8, a8, 1
42038d1f:	4189      	s32i.n	a8, a1, 16
42038d21:	3188      	l32i.n	a8, a1, 12
42038d23:	881b      	addi.n	a8, a8, 1
42038d25:	5189      	s32i.n	a8, a1, 20
42038d27:	c08860        	sub	a8, a8, a6
42038d2a:	1188e0        	slli	a8, a8, 2
42038d2d:	8189      	s32i.n	a8, a1, 32
42038d2f:	4188      	l32i.n	a8, a1, 16
42038d31:	1128e0        	slli	a2, a8, 2
42038d34:	c08630        	sub	a8, a6, a3
42038d37:	1188e0        	slli	a8, a8, 2
42038d3a:	9189      	s32i.n	a8, a1, 36
42038d3c:	238680        	sext	a8, a6, 15
42038d3f:	6189      	s32i.n	a8, a1, 24
42038d41:	1188      	l32i.n	a8, a1, 4
42038d43:	03a847        	bge	a8, a4, 42038d4a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xce>
42038d46:	020c      	movi.n	a2, 0
42038d48:	f01d      	retw.n
42038d4a:	31a8      	l32i.n	a10, a1, 12
42038d4c:	038d      	mov.n	a8, a3
42038d4e:	0f9d      	mov.n	a9, a15
42038d50:	2b2a67        	blt	a10, a6, 42038d7f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x103>
42038d53:	0f8d      	mov.n	a8, a15
42038d55:	03bd      	mov.n	a11, a3
42038d57:	23c480        	sext	a12, a4, 15
42038d5a:	02ab67        	bge	a11, a6, 42038d60 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe4>
42038d5d:	0024c6        	j	42038df4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x178>
42038d60:	9188      	l32i.n	a8, a1, 36
42038d62:	61b8      	l32i.n	a11, a1, 24
42038d64:	9f8a      	add.n	a9, a15, a8
42038d66:	07ad      	mov.n	a10, a7
42038d68:	b1f9      	s32i.n	a15, a1, 44
42038d6a:	a199      	s32i.n	a9, a1, 40
42038d6c:	ffece5        	call8	42038c3c <roo_windows::Decoration::read(short, short) const>
42038d6f:	8188      	l32i.n	a8, a1, 32
42038d71:	a198      	l32i.n	a9, a1, 40
42038d73:	b1f8      	l32i.n	a15, a1, 44
42038d75:	898a      	add.n	a8, a9, a8
42038d77:	021897        	beq	a8, a9, 42038d7d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x101>
42038d7a:	0025c6        	j	42038e15 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x199>
42038d7d:	5188      	l32i.n	a8, a1, 20
42038d7f:	09dd      	mov.n	a13, a9
42038d81:	08bd      	mov.n	a11, a8
42038d83:	23c480        	sext	a12, a4, 15
42038d86:	0225b7        	blt	a5, a11, 42038d8c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x110>
42038d89:	0023c6        	j	42038e1c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a0>
42038d8c:	b51b      	addi.n	a11, a5, 1
42038d8e:	72ab80        	salt	a10, a11, a8
42038d91:	aa0b      	addi.n	a10, a10, -1
42038d93:	60a0a0        	neg	a10, a10
42038d96:	74a0a0        	extui	a10, a10, 0, 8
42038d99:	0c0c      	movi.n	a12, 0
42038d9b:	4a8c      	beqz.n	a10, 42038da3 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x127>
42038d9d:	c0bb80        	sub	a11, a11, a8
42038da0:	11cbe0        	slli	a12, a11, 2
42038da3:	71a8      	l32i.n	a10, a1, 28
42038da5:	99ca      	add.n	a9, a9, a12
42038da7:	72a4a0        	salt	a10, a4, a10
42038daa:	aa0b      	addi.n	a10, a10, -1
42038dac:	60a0a0        	neg	a10, a10
42038daf:	74a0a0        	extui	a10, a10, 0, 8
42038db2:	098d      	mov.n	a8, a9
42038db4:	4abc      	beqz.n	a10, 42038dec <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038db6:	21a8      	l32i.n	a10, a1, 8
42038db8:	72aa40        	salt	a10, a10, a4
42038dbb:	aa0b      	addi.n	a10, a10, -1
42038dbd:	60a0a0        	neg	a10, a10
42038dc0:	74a0a0        	extui	a10, a10, 0, 8
42038dc3:	5aac      	beqz.n	a10, 42038dec <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038dc5:	11a8      	l32i.n	a10, a1, 4
42038dc7:	21b8      	l32i.n	a11, a1, 8
42038dc9:	04ed      	mov.n	a14, a4
42038dcb:	43dab0        	min	a13, a10, a11
42038dce:	742ed7        	blt	a14, a13, 42038e46 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038dd1:	728d40        	salt	a8, a13, a4
42038dd4:	880b      	addi.n	a8, a8, -1
42038dd6:	608080        	neg	a8, a8
42038dd9:	748080        	extui	a8, a8, 0, 8
42038ddc:	00a0a2        	movi	a10, 0
42038ddf:	002816        	beqz	a8, 42038de5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x169>
42038de2:	c0ad40        	sub	a10, a13, a4
42038de5:	828a20        	mull	a8, a10, a2
42038de8:	4a4a      	add.n	a4, a10, a4
42038dea:	889a      	add.n	a8, a8, a9
42038dec:	441b      	addi.n	a4, a4, 1
42038dee:	08fd      	mov.n	a15, a8
42038df0:	ffd346        	j	42038d41 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xc5>
42038df3:	a77000        	ee.stf.64.xp	f7, f10, a0, a0
42038df6:	61f220        	xsr.ccompare2	a2
42038df9:	0d          	.byte	0xd
42038dfa:	0c6182        	s32i	a8, a1, 48
42038dfd:	b1c9      	s32i.n	a12, a1, 44
42038dff:	a1b9      	s32i.n	a11, a1, 40
42038e01:	ffe3a5        	call8	42038c3c <roo_windows::Decoration::read(short, short) const>
42038e04:	c188      	l32i.n	a8, a1, 48
42038e06:	a1b8      	l32i.n	a11, a1, 40
42038e08:	08a9      	s32i.n	a10, a8, 0
42038e0a:	d1f8      	l32i.n	a15, a1, 52
42038e0c:	b1c8      	l32i.n	a12, a1, 44
42038e0e:	bb1b      	addi.n	a11, a11, 1
42038e10:	884b      	addi.n	a8, a8, 4
42038e12:	ffd106        	j	42038d5a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xde>
42038e15:	09a9      	s32i.n	a10, a9, 0
42038e17:	994b      	addi.n	a9, a9, 4
42038e19:	ffd686        	j	42038d77 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xfb>
42038e1c:	20a770        	or	a10, a7, a7
42038e1f:	0f61f2        	s32i	a15, a1, 60
42038e22:	0e61d2        	s32i	a13, a1, 56
42038e25:	d189      	s32i.n	a8, a1, 52
42038e27:	c199      	s32i.n	a9, a1, 48
42038e29:	b1c9      	s32i.n	a12, a1, 44
42038e2b:	a1b9      	s32i.n	a11, a1, 40
42038e2d:	ffe0e5        	call8	42038c3c <roo_windows::Decoration::read(short, short) const>
42038e30:	e1d8      	l32i.n	a13, a1, 56
42038e32:	a1b8      	l32i.n	a11, a1, 40
42038e34:	0da9      	s32i.n	a10, a13, 0
42038e36:	f1f8      	l32i.n	a15, a1, 60
42038e38:	d188      	l32i.n	a8, a1, 52
42038e3a:	c198      	l32i.n	a9, a1, 48
42038e3c:	b1c8      	l32i.n	a12, a1, 44
42038e3e:	bb1b      	addi.n	a11, a11, 1
42038e40:	dd4b      	addi.n	a13, a13, 4
42038e42:	ffd006        	j	42038d86 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10a>
42038e45:	41a800        	srli	a10, a0, 8
42038e48:	201aa6        	blti	a10, 1, 42038e6c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038e4b:	254226        	beqi	a2, 4, 42038e74 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f8>
42038e4e:	0fbd      	mov.n	a11, a15
42038e50:	08ad      	mov.n	a10, a8
42038e52:	02cd      	mov.n	a12, a2
42038e54:	d199      	s32i.n	a9, a1, 52
42038e56:	c1e9      	s32i.n	a14, a1, 48
42038e58:	b1d9      	s32i.n	a13, a1, 44
42038e5a:	a1f9      	s32i.n	a15, a1, 40
42038e5c:	1ea281        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038e5f:	0008e0        	callx8	a8
42038e62:	a1f8      	l32i.n	a15, a1, 40
42038e64:	b1d8      	l32i.n	a13, a1, 44
42038e66:	c1e8      	l32i.n	a14, a1, 48
42038e68:	d198      	l32i.n	a9, a1, 52
42038e6a:	0a8d      	mov.n	a8, a10
42038e6c:	882a      	add.n	a8, a8, a2
42038e6e:	ee1b      	addi.n	a14, a14, 1
42038e70:	ffd686        	j	42038dce <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x152>
42038e73:	00          	.byte	00
42038e74:	0fa8      	l32i.n	a10, a15, 0
42038e76:	08a9      	s32i.n	a10, a8, 0
42038e78:	fffc06        	j	42038e6c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038e7b:	600000        	neg	a0, a0
42038e7e:	4020b6        	bltui	a0, 2, 42038ec2 <roo_windows::internal::MakeLookupShadow(roo_display::Box, roo_windows::BorderStyle::CornerRadii, int, bool)+0x16>
42038e81:	c4          	.byte	0xc4
42038e82:	a77020        	ee.stf.64.xp	f7, f10, a2, a0
42038e85:	db6520        	movt.s	f6, f5, b2
42038e88:	a9661bff 	ee.fft.cmul.s16.st.xp	q4, q6, q6, a15, a11, 2, 2, 0
42038e8c:	02          	.byte	0x2
42038e8d:	236680        	sext	a6, a6, 15
42038e90:	224b      	addi.n	a2, a2, 4
42038e92:	02fd      	mov.n	a15, a2
42038e94:	e5a567        	bge	a5, a6, 42038e7d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x201>
42038e97:	441b      	addi.n	a4, a4, 1
42038e99:	234480        	sext	a4, a4, 15
42038e9c:	1188      	l32i.n	a8, a1, 4
42038e9e:	02a847        	bge	a8, a4, 42038ea4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038ea1:	ffa846        	j	42038d46 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xca>
42038ea4:	0f2d      	mov.n	a2, a15
42038ea6:	036d      	mov.n	a6, a3
42038ea8:	fff986        	j	42038e92 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/radial_reject/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039070 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
42039070:	00a136        	entry	a1, 80
42039073:	238480        	sext	a8, a4, 15
42039076:	027d      	mov.n	a7, a2
42039078:	a189      	s32i.n	a8, a1, 40
4203907a:	0f0222        	l8ui	a2, a2, 15
4203907d:	238580        	sext	a8, a5, 15
42039080:	b189      	s32i.n	a8, a1, 44
42039082:	24f216        	beqz	a2, 420392d5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x265>
42039085:	009782        	l16si	a8, a7, 0
42039088:	0697f2        	l16si	a15, a7, 12
4203908b:	3189      	s32i.n	a8, a1, 12
4203908d:	3198      	l32i.n	a9, a1, 12
4203908f:	a188      	l32i.n	a8, a1, 40
42039091:	c08890        	sub	a8, a8, a9
42039094:	11c8c0        	slli	a12, a8, 4
42039097:	4189      	s32i.n	a8, a1, 16
42039099:	019782        	l16si	a8, a7, 2
4203909c:	cc8b      	addi.n	a12, a12, 8
4203909e:	5189      	s32i.n	a8, a1, 20
420390a0:	5198      	l32i.n	a9, a1, 20
420390a2:	b188      	l32i.n	a8, a1, 44
420390a4:	c08890        	sub	a8, a8, a9
420390a7:	6189      	s32i.n	a8, a1, 24
420390a9:	069392        	l16si	a9, a3, 12
420390ac:	1188c0        	slli	a8, a8, 4
420390af:	888b      	addi.n	a8, a8, 8
420390b1:	c0f8f0        	sub	a15, a8, a15
420390b4:	c08890        	sub	a8, a8, a9
420390b7:	9cca      	add.n	a9, a12, a12
420390b9:	7199      	s32i.n	a9, a1, 28
420390bb:	029792        	l16si	a9, a7, 4
420390be:	d88a      	add.n	a13, a8, a8
420390c0:	8199      	s32i.n	a9, a1, 32
420390c2:	1199c0        	slli	a9, a9, 4
420390c5:	2199      	s32i.n	a9, a1, 8
420390c7:	039792        	l16si	a9, a7, 6
420390ca:	2168      	l32i.n	a6, a1, 8
420390cc:	effa      	add.n	a14, a15, a15
420390ce:	9199      	s32i.n	a9, a1, 36
420390d0:	1199c0        	slli	a9, a9, 4
420390d3:	c04980        	sub	a4, a9, a8
420390d6:	72ae90        	salt	a10, a14, a9
420390d9:	72bd90        	salt	a11, a13, a9
420390dc:	c056c0        	sub	a5, a6, a12
420390df:	aa0b      	addi.n	a10, a10, -1
420390e1:	bb0b      	addi.n	a11, a11, -1
420390e3:	434480        	min	a4, a4, a8
420390e6:	7188      	l32i.n	a8, a1, 28
420390e8:	60a0a0        	neg	a10, a10
420390eb:	60b0b0        	neg	a11, a11
420390ee:	4355c0        	min	a5, a5, a12
420390f1:	c0c9f0        	sub	a12, a9, a15
420390f4:	74a0a0        	extui	a10, a10, 0, 8
420390f7:	74b0b0        	extui	a11, a11, 0, 8
420390fa:	43ccf0        	min	a12, a12, a15
420390fd:	6f2867        	blt	a8, a6, 42039170 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x100>
42039100:	642e97        	blt	a14, a9, 42039168 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf8>
42039103:	0a0782        	l8ui	a8, a7, 10
42039106:	6f1ab7        	beq	a10, a11, 42039179 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x109>
42039109:	7b2d97        	blt	a13, a9, 42039188 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x118>
4203910c:	0a0762        	l8ui	a6, a7, 10
4203910f:	23d880        	sext	a13, a8, 15
42039112:	11ddc0        	slli	a13, a13, 4
42039115:	02acd7        	bge	a12, a13, 4203911b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
42039118:	002106        	j	420391a0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x130>
4203911b:	23f680        	sext	a15, a6, 15
4203911e:	11ffc0        	slli	a15, a15, 4
42039121:	7b24f7        	blt	a4, a15, 420391a0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x130>
42039124:	43fc40        	min	a15, a12, a4
42039127:	752f57        	blt	a15, a5, 420391a0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x130>
4203912a:	a198      	l32i.n	a9, a1, 40
4203912c:	3188      	l32i.n	a8, a1, 12
4203912e:	21a8      	l32i.n	a10, a1, 8
42039130:	c08890        	sub	a8, a8, a9
42039133:	7198      	l32i.n	a9, a1, 28
42039135:	0829a7        	blt	a9, a10, 42039141 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd1>
42039138:	4188      	l32i.n	a8, a1, 16
4203913a:	8198      	l32i.n	a9, a1, 32
4203913c:	c08890        	sub	a8, a8, a9
4203913f:	881b      	addi.n	a8, a8, 1
42039141:	096c      	movi.n	a9, -32
42039143:	02a897        	bge	a8, a9, 42039149 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd9>
42039146:	006886        	j	420392ec <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27c>
42039149:	092c      	movi.n	a9, 32
4203914b:	02a987        	bge	a9, a8, 42039151 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe1>
4203914e:	006806        	j	420392f2 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x282>
42039151:	220b      	addi.n	a2, a2, -1
42039153:	902220        	addx2	a2, a2, a2
42039156:	1192a0        	slli	a9, a2, 6
42039159:	229a      	add.n	a2, a2, a9
4203915b:	1de691        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
4203915e:	898a      	add.n	a8, a9, a8
42039160:	882a      	add.n	a8, a8, a2
42039162:	200822        	l8ui	a2, a8, 32
42039165:	005b06        	j	420392d5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x265>
42039168:	090782        	l8ui	a8, a7, 9
4203916b:	ffe5c6        	j	42039106 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x96>
4203916e:	970000        	ee.stf.64.xp	f0, f9, a0, a0
42039171:	07820c2e 	ee.vmulas.s16.accx.ld.ip.qup	q6, a2, 0x1c0, q0, q1, q0, q2
42039175:	b70b      	addi.n	a11, a7, -1
42039177:	169a      	add.n	a1, a6, a9
42039179:	086d      	mov.n	a6, a8
4203917b:	ffe406        	j	4203910f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
4203917e:	820000        	mull	a0, a0, a0
42039181:	c60807        	bnone	a8, a0, 4203914b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdb>
42039184:	fffb      	addi.n	a15, a15, 15
42039186:	620000        	saltu	a0, a0, a0
42039189:	060907        	bnone	a9, a0, 42039193 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x123>
4203918c:	e0          	.byte	0xe0
4203918d:	970000ff 	ee.stf.128.ip	f14, f1, f0, f0, a15, 0
42039191:	052d      	mov.n	a2, a5
42039193:	0b0762        	l8ui	a6, a7, 11
42039196:	ffdd46        	j	4203910f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
42039199:	080762        	l8ui	a6, a7, 8
4203919c:	ffdbc6        	j	4203910f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
4203919f:	9ab700        	trunc.s	a11, f7, 0
420391a2:	868051        	l32r	a5, 4201aba4 <roo_display::SimpleStreamable<roo_display::ProgMemPtr, roo_display::internal::Indexed<(unsigned char)4>, roo_display::internal::RleStream4bppxBiased<roo_display::ProgMemPtr, roo_display::internal::Indexed<(unsigned char)4>, (signed char)4> >::createStream(roo_display::Box const&) const+0x3c> (31a0a0f4 <UserFrameTotalSize+0x31a09ff4>)
420391a5:	888053        	lsip	f5, a0, 0x220
420391a8:	88c023        	ssip	f2, a0, 0x220
420391ab:	258711        	l32r	a1, 420027c8 <VFSFileImpl::getNextFileName(bool*)+0xe8> (20c03e <UserFrameTotalSize+0x20bf3e>)
420391ae:	8c4045        	call0	41fc55b4 <_coredump_iram_end+0x1c3f1b4>
420391b1:	258753        	lsip	f5, a7, 148
420391b4:	8891a83f 	ee.ldf.128.xp	f0, f3, f10, f9, a3, a8
420391b8:	88a061        	l32r	a6, 4201b438 <roo_display::internal::SubRectangleStream<roo_display::RasterPixelStream<roo_io::UnsafeGenericMemoryIterable<std::byte const*>, roo_display::internal::Indexed<(unsigned char)4>, (roo_display::ColorPixelOrder)0, (roo_io::ByteOrder)1, 2> >::dskip(unsigned long)+0x8> (607070c0 <_rtc_reserved_end+0x6070c0>)
420391bb:	c0          	.byte	0xc0
420391bc:	881b      	addi.n	a8, a8, 1
420391be:	2a0c      	movi.n	a10, 2
420391c0:	08ae97        	bge	a14, a9, 420391cc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x15c>
420391c3:	5188      	l32i.n	a8, a1, 20
420391c5:	b198      	l32i.n	a9, a1, 44
420391c7:	1a0c      	movi.n	a10, 1
420391c9:	c08890        	sub	a8, a8, a9
420391cc:	096c      	movi.n	a9, -32
420391ce:	02a897        	bge	a8, a9, 420391d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x164>
420391d1:	0045c6        	j	420392ec <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27c>
420391d4:	092c      	movi.n	a9, 32
420391d6:	02a987        	bge	a9, a8, 420391dc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x16c>
420391d9:	004546        	j	420392f2 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x282>
420391dc:	220b      	addi.n	a2, a2, -1
420391de:	902220        	addx2	a2, a2, a2
420391e1:	22aa      	add.n	a2, a2, a10
420391e3:	1dc491        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420391e6:	11a2a0        	slli	a10, a2, 6
420391e9:	aa2a      	add.n	a10, a10, a2
420391eb:	898a      	add.n	a8, a9, a8
420391ed:	88aa      	add.n	a8, a8, a10
420391ef:	ffdbc6        	j	42039162 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf2>
420391f2:	000000        	ill
420391f5:	091792        	l16ui	a9, a7, 18
420391f8:	4385c0        	min	a8, a5, a12
420391fb:	60a090        	neg	a10, a9
420391fe:	020c      	movi.n	a2, 0
42039200:	5728a7        	blt	a8, a10, 4203925b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1eb>
42039203:	c0ad50        	sub	a10, a13, a5
42039206:	c08dc0        	sub	a8, a13, a12
42039209:	0222a7        	blt	a2, a10, 4203920f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x19f>
4203920c:	003206        	j	420392d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
4203920f:	022287        	blt	a2, a8, 42039215 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a5>
42039212:	003086        	j	420392d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
42039215:	828880        	mull	a8, a8, a8
42039218:	82aaa0        	mull	a10, a10, a10
4203921b:	21d9      	s32i.n	a13, a1, 8
4203921d:	a8aa      	add.n	a10, a8, a10
4203921f:	808d90        	add	a8, a13, a9
42039222:	01c882        	addi	a8, a8, 1
42039225:	828880        	mull	a8, a8, a8
42039228:	2fba87        	bgeu	a10, a8, 4203925b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1eb>
4203922b:	4d33a5        	call8	42086564 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
4203922e:	0221d2        	l32i	a13, a1, 8
42039231:	0e0792        	l8ui	a9, a7, 14
42039234:	c08ad0        	sub	a8, a10, a13
42039237:	c08890        	sub	a8, a8, a9
4203923a:	081792        	l16ui	a9, a7, 16
4203923d:	828890        	mull	a8, a8, a9
42039240:	1daf91        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
42039243:	889a      	add.n	a8, a8, a9
42039245:	218c80        	srai	a8, a8, 12
42039248:	ffa092        	movi	a9, 255
4203924b:	438890        	min	a8, a8, a9
4203924e:	090c      	movi.n	a9, 0
42039250:	538890        	max	a8, a8, a9
42039253:	1da991        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
42039256:	898a      	add.n	a8, a9, a8
42039258:	000822        	l8ui	a2, a8, 0
4203925b:	0913b2        	l16ui	a11, a3, 18
4203925e:	439540        	min	a9, a5, a4
42039261:	60a0b0        	neg	a10, a11
42039264:	080c      	movi.n	a8, 0
42039266:	5229a7        	blt	a9, a10, 420392bc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24c>
42039269:	236680        	sext	a6, a6, 15
4203926c:	1166c0        	slli	a6, a6, 4
4203926f:	c0a650        	sub	a10, a6, a5
42039272:	c09640        	sub	a9, a6, a4
42039275:	6ba8a7        	bge	a8, a10, 420392e4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x274>
42039278:	68a897        	bge	a8, a9, 420392e4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x274>
4203927b:	829990        	mull	a9, a9, a9
4203927e:	82aaa0        	mull	a10, a10, a10
42039281:	a9aa      	add.n	a10, a9, a10
42039283:	9b6a      	add.n	a9, a11, a6
42039285:	01c992        	addi	a9, a9, 1
42039288:	829990        	mull	a9, a9, a9
4203928b:	2dba97        	bgeu	a10, a9, 420392bc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24c>
4203928e:	4d2d65        	call8	42086564 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039291:	0e0392        	l8ui	a9, a3, 14
42039294:	c08a60        	sub	a8, a10, a6
42039297:	c08890        	sub	a8, a8, a9
4203929a:	081392        	l16ui	a9, a3, 16
4203929d:	828890        	mull	a8, a8, a9
420392a0:	1d9791        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
420392a3:	889a      	add.n	a8, a8, a9
420392a5:	218c80        	srai	a8, a8, 12
420392a8:	ffa092        	movi	a9, 255
420392ab:	438890        	min	a8, a8, a9
420392ae:	090c      	movi.n	a9, 0
420392b0:	538890        	max	a8, a8, a9
420392b3:	1d9391        	l32r	a9, 42000900 <_stext+0x8e0> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
420392b6:	808980        	add	a8, a9, a8
420392b9:	000882        	l8ui	a8, a8, 0
420392bc:	d19280        	mul16s	a9, a2, a8
420392bf:	1d88a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
420392c2:	7fc992        	addi	a9, a9, 127
420392c5:	b2a9a0        	mulsh	a10, a9, a10
420392c8:	282a      	add.n	a2, a8, a2
420392ca:	99aa      	add.n	a9, a9, a10
420392cc:	219790        	srai	a9, a9, 7
420392cf:	c02290        	sub	a2, a2, a9
420392d2:	742020        	extui	a2, a2, 0, 8
420392d5:	f01d      	retw.n
420392d7:	00          	.byte	00
420392d8:	022c57        	blt	a12, a5, 420392de <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x26e>
420392db:	ffd486        	j	42039231 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c1>
420392de:	08ad      	mov.n	a10, a8
420392e0:	ffd346        	j	42039231 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c1>
420392e3:	a45700        	extui	a5, a0, 7, 11
420392e6:	ada9      	s32i.n	a10, a13, 40
420392e8:	0609      	s32i.n	a0, a6, 0
420392ea:	ffe9      	s32i.n	a14, a15, 60
420392ec:	68a022        	movi	a2, 104
420392ef:	fff886        	j	420392d5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x265>
420392f2:	020c      	movi.n	a2, 0
420392f4:	fff746        	j	420392d5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x265>
