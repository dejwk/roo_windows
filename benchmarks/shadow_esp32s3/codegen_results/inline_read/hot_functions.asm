
/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c3c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c3c:	004136        	entry	a1, 32
42038c3f:	15cc      	bnez.n	a5, 42038c44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8>
42038c41:	f01d      	retw.n
42038c43:	94c200        	extui	a12, a0, 2, 10
42038c46:	93b200        	movnez	a11, a2, a0
42038c49:	22d200        	orb	b13, b2, b0
42038c4c:	a0e205        	call0	41fd9a70 <_coredump_iram_end+0x1c53670>
42038c4f:	a22000        	muluh	a2, a0, a0
42038c52:	e6e520        	ee.ldf.64.xp	f14, f14, a2, a5
42038c55:	2b06a9ff 	ee.vmulas.s8.accx.ld.ip.qup	q6, a15, 0xfffffe90, q6, q4, q0, q6
42038c59:	33          	.byte	0x33
42038c5a:	442b      	addi.n	a4, a4, 2
42038c5c:	550b      	addi.n	a5, a5, -1
42038c5e:	664b      	addi.n	a6, a6, 4
42038c60:	fff6c6        	j	42038c3f <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3>


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


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


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208654c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
4208654c:	004136        	entry	a1, 32
4208654f:	029d      	mov.n	a9, a2
42086551:	120c      	movi.n	a2, 1
42086553:	832990        	moveqz	a2, a9, a9
42086556:	4149b6        	bltui	a9, 4, 4208659b <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
42086559:	40f980        	nsau	a8, a9
4208655c:	dfc882        	addi	a8, a8, -33
4208655f:	608080        	neg	a8, a8
42086562:	218180        	srai	a8, a8, 1
42086565:	fec882        	addi	a8, a8, -2
42086568:	3a0c      	movi.n	a10, 3
4208656a:	401800        	ssl	a8
4208656d:	a1aa00        	sll	a10, a10
42086570:	c289a0        	quou	a8, a9, a10
42086573:	88aa      	add.n	a8, a8, a10
42086575:	418180        	srli	a8, a8, 1
42086578:	c2a980        	quou	a10, a9, a8
4208657b:	aa8a      	add.n	a10, a10, a8
4208657d:	41a1a0        	srli	a10, a10, 1
42086580:	c289a0        	quou	a8, a9, a10
42086583:	88aa      	add.n	a8, a8, a10
42086585:	1a0c      	movi.n	a10, 1
42086587:	418180        	srli	a8, a8, 1
4208658a:	11aa00        	slli	a10, a10, 16
4208658d:	0518a7        	beq	a8, a10, 42086596 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
42086590:	82a880        	mull	a10, a8, a8
42086593:	01b9a7        	bgeu	a9, a10, 42086598 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
42086596:	880b      	addi.n	a8, a8, -1
42086598:	f42080        	extui	a2, a8, 0, 16
4208659b:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420863ac <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
420863ac:	004136        	entry	a1, 32
420863af:	208230        	or	a8, a2, a3
420863b2:	0228d6        	bgez	a8, 420863d8 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
420863b5:	c02240        	sub	a2, a2, a4
420863b8:	40f290        	nsau	a9, a2
420863bb:	419590        	srli	a9, a9, 5
420863be:	b28c      	beqz.n	a2, 420863cd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
420863c0:	c03340        	sub	a3, a3, a4
420863c3:	180c      	movi.n	a8, 1
420863c5:	938930        	movnez	a8, a9, a3
420863c8:	ffa022        	movi	a2, 255
420863cb:	688c      	beqz.n	a8, 420863d5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863cd:	1125c0        	slli	a2, a5, 4
420863d0:	252a      	add.n	a2, a5, a2
420863d2:	742020        	extui	a2, a2, 0, 8
420863d5:	f01d      	retw.n
420863d7:	222000        	orb	b2, b0, b0
420863da:	3330d1        	l32r	a13, 4205309c <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
420863dd:	823ad1        	l32r	a13, 42066cc8 <esp_littlefs_free_fd$isra$0+0x48> (880b3d12 <_rtc_reserved_end+0x27fb3d12>)
420863e0:	118880        	slli	a8, a8, 8
420863e3:	ffa022        	movi	a2, 255
420863e6:	eb48a6        	blti	a8, 4, 420863d5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863e9:	40f890        	nsau	a9, a8
420863ec:	dfc992        	addi	a9, a9, -33
420863ef:	609090        	neg	a9, a9
420863f2:	219190        	srai	a9, a9, 1
420863f5:	fec992        	addi	a9, a9, -2
420863f8:	3a0c      	movi.n	a10, 3
420863fa:	401900        	ssl	a9
420863fd:	a1aa00        	sll	a10, a10
42086400:	c298a0        	quou	a9, a8, a10
42086403:	99aa      	add.n	a9, a9, a10
42086405:	419190        	srli	a9, a9, 1
42086408:	c2a890        	quou	a10, a8, a9
4208640b:	aa9a      	add.n	a10, a10, a9
4208640d:	41a1a0        	srli	a10, a10, 1
42086410:	c298a0        	quou	a9, a8, a10
42086413:	99aa      	add.n	a9, a9, a10
42086415:	1a0c      	movi.n	a10, 1
42086417:	419190        	srli	a9, a9, 1
4208641a:	11aa00        	slli	a10, a10, 16
4208641d:	0519a7        	beq	a9, a10, 42086426 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7a>
42086420:	82a990        	mull	a10, a9, a9
42086423:	01b8a7        	bgeu	a8, a10, 42086428 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7c>
42086426:	990b      	addi.n	a9, a9, -1
42086428:	1144c0        	slli	a4, a4, 4
4208642b:	554a      	add.n	a5, a5, a4
4208642d:	c05590        	sub	a5, a5, a9
42086430:	f48050        	extui	a8, a5, 0, 16
42086433:	239580        	sext	a9, a5, 15
42086436:	020c      	movi.n	a2, 0
42086438:	99e5f7        	bbsi	a5, 15, 420863d5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208643b:	fa0c      	movi.n	a10, 15
4208643d:	ffa022        	movi	a2, 255
42086440:	912a97        	blt	a10, a9, 420863d5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086443:	1198c0        	slli	a9, a8, 4
42086446:	889a      	add.n	a8, a8, a9
42086448:	742080        	extui	a2, a8, 0, 8
4208644b:	ffe186        	j	420863d5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


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
42033f65:	3cc725        	call8	42070bd8 <_Unwind_Resume>
42033f68:	fffac6        	j	42033f57 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


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
4203877d:	4dc2e5        	call8	420863ac <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
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


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


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
42038b18:	0053e5        	call8	42039058 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
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
42038bb5:	50fc06        	j	4204cfa9 <uart_get_word_length+0x1>
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
42038c1e:	4c22a5        	call8	42084e48 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
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


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c64 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038c64:	00c136        	entry	a1, 96
42038c67:	078d      	mov.n	a8, a7
42038c69:	069d      	mov.n	a9, a6
42038c6b:	086d      	mov.n	a6, a8
42038c6d:	238980        	sext	a8, a9, 15
42038c70:	3189      	s32i.n	a8, a1, 12
42038c72:	142282        	l32i	a8, a2, 80
42038c75:	027d      	mov.n	a7, a2
42038c77:	233380        	sext	a3, a3, 15
42038c7a:	234480        	sext	a4, a4, 15
42038c7d:	235580        	sext	a5, a5, 15
42038c80:	200856        	bnez	a8, 42038e84 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038c83:	0288      	l32i.n	a8, a2, 0
42038c85:	31e8      	l32i.n	a14, a1, 12
42038c87:	b888      	l32i.n	a8, a8, 44
42038c89:	02ad      	mov.n	a10, a2
42038c8b:	06fd      	mov.n	a15, a6
42038c8d:	05dd      	mov.n	a13, a5
42038c8f:	04cd      	mov.n	a12, a4
42038c91:	03bd      	mov.n	a11, a3
42038c93:	0008e0        	callx8	a8
42038c96:	0a2d      	mov.n	a2, a10
42038c98:	09ca56        	bnez	a10, 42038d38 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd4>
42038c9b:	249792        	l16si	a9, a7, 72
42038c9e:	1a9782        	l16si	a8, a7, 52
42038ca1:	ec0c      	movi.n	a12, 14
42038ca3:	538890        	max	a8, a8, a9
42038ca6:	98fb      	addi.n	a9, a8, 15
42038ca8:	1ec882        	addi	a8, a8, 30
42038cab:	b38990        	movgez	a8, a9, a9
42038cae:	21b480        	srai	a11, a8, 4
42038cb1:	210782        	l8ui	a8, a7, 33
42038cb4:	1907a2        	l8ui	a10, a7, 25
42038cb7:	62cc80        	saltu	a12, a12, a8
42038cba:	180782        	l8ui	a8, a7, 24
42038cbd:	200792        	l8ui	a9, a7, 32
42038cc0:	7388a0        	maxu	a8, a8, a10
42038cc3:	1a07a2        	l8ui	a10, a7, 26
42038cc6:	cc0b      	addi.n	a12, a12, -1
42038cc8:	7388a0        	maxu	a8, a8, a10
42038ccb:	1b07a2        	l8ui	a10, a7, 27
42038cce:	60c0c0        	neg	a12, a12
42038cd1:	73a8a0        	maxu	a10, a8, a10
42038cd4:	89ca      	add.n	a8, a9, a12
42038cd6:	99ba      	add.n	a9, a9, a11
42038cd8:	538a80        	max	a8, a10, a8
42038cdb:	99ca      	add.n	a9, a9, a12
42038cdd:	0297a2        	l16si	a10, a7, 4
42038ce0:	539980        	max	a9, a9, a8
42038ce3:	aa9a      	add.n	a10, a10, a9
42038ce5:	53aa30        	max	a10, a10, a3
42038ce8:	11a9      	s32i.n	a10, a1, 4
42038cea:	0497a2        	l16si	a10, a7, 8
42038ced:	c09a90        	sub	a9, a10, a9
42038cf0:	439950        	min	a9, a9, a5
42038cf3:	5199      	s32i.n	a9, a1, 20
42038cf5:	039792        	l16si	a9, a7, 6
42038cf8:	998a      	add.n	a9, a9, a8
42038cfa:	99ba      	add.n	a9, a9, a11
42038cfc:	9199      	s32i.n	a9, a1, 36
42038cfe:	059792        	l16si	a9, a7, 10
42038d01:	c08980        	sub	a8, a9, a8
42038d04:	4189      	s32i.n	a8, a1, 16
42038d06:	c08530        	sub	a8, a5, a3
42038d09:	881b      	addi.n	a8, a8, 1
42038d0b:	6189      	s32i.n	a8, a1, 24
42038d0d:	5188      	l32i.n	a8, a1, 20
42038d0f:	1198      	l32i.n	a9, a1, 4
42038d11:	881b      	addi.n	a8, a8, 1
42038d13:	7189      	s32i.n	a8, a1, 28
42038d15:	c08890        	sub	a8, a8, a9
42038d18:	1188e0        	slli	a8, a8, 2
42038d1b:	a189      	s32i.n	a8, a1, 40
42038d1d:	6188      	l32i.n	a8, a1, 24
42038d1f:	1188e0        	slli	a8, a8, 2
42038d22:	2189      	s32i.n	a8, a1, 8
42038d24:	c08930        	sub	a8, a9, a3
42038d27:	1188e0        	slli	a8, a8, 2
42038d2a:	b189      	s32i.n	a8, a1, 44
42038d2c:	029182        	l16si	a8, a1, 4
42038d2f:	8189      	s32i.n	a8, a1, 32
42038d31:	3188      	l32i.n	a8, a1, 12
42038d33:	03a847        	bge	a8, a4, 42038d3a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd6>
42038d36:	020c      	movi.n	a2, 0
42038d38:	f01d      	retw.n
42038d3a:	5198      	l32i.n	a9, a1, 20
42038d3c:	11a8      	l32i.n	a10, a1, 4
42038d3e:	038d      	mov.n	a8, a3
42038d40:	062d      	mov.n	a2, a6
42038d42:	03bd      	mov.n	a11, a3
42038d44:	23c480        	sext	a12, a4, 15
42038d47:	0229a7        	blt	a9, a10, 42038d4d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038d4a:	002406        	j	42038dde <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x17a>
42038d4d:	029d      	mov.n	a9, a2
42038d4f:	08bd      	mov.n	a11, a8
42038d51:	23c480        	sext	a12, a4, 15
42038d54:	0225b7        	blt	a5, a11, 42038d5a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf6>
42038d57:	002b46        	j	42038e08 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a4>
42038d5a:	a51b      	addi.n	a10, a5, 1
42038d5c:	729a80        	salt	a9, a10, a8
42038d5f:	990b      	addi.n	a9, a9, -1
42038d61:	609090        	neg	a9, a9
42038d64:	749090        	extui	a9, a9, 0, 8
42038d67:	0b0c      	movi.n	a11, 0
42038d69:	498c      	beqz.n	a9, 42038d71 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10d>
42038d6b:	c0aa80        	sub	a10, a10, a8
42038d6e:	11bae0        	slli	a11, a10, 2
42038d71:	9198      	l32i.n	a9, a1, 36
42038d73:	22ba      	add.n	a2, a2, a11
42038d75:	729490        	salt	a9, a4, a9
42038d78:	990b      	addi.n	a9, a9, -1
42038d7a:	609090        	neg	a9, a9
42038d7d:	749090        	extui	a9, a9, 0, 8
42038d80:	028d      	mov.n	a8, a2
42038d82:	79bc      	beqz.n	a9, 42038dbd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038d84:	4198      	l32i.n	a9, a1, 16
42038d86:	729940        	salt	a9, a9, a4
42038d89:	990b      	addi.n	a9, a9, -1
42038d8b:	609090        	neg	a9, a9
42038d8e:	749090        	extui	a9, a9, 0, 8
42038d91:	89ac      	beqz.n	a9, 42038dbd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038d93:	3198      	l32i.n	a9, a1, 12
42038d95:	41a8      	l32i.n	a10, a1, 16
42038d97:	04dd      	mov.n	a13, a4
42038d99:	4399a0        	min	a9, a9, a10
42038d9c:	02ad97        	bge	a13, a9, 42038da2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x13e>
42038d9f:	0022c6        	j	42038e2e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038da2:	728940        	salt	a8, a9, a4
42038da5:	880b      	addi.n	a8, a8, -1
42038da7:	608080        	neg	a8, a8
42038daa:	748080        	extui	a8, a8, 0, 8
42038dad:	0a0c      	movi.n	a10, 0
42038daf:	188c      	beqz.n	a8, 42038db4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x150>
42038db1:	c0a940        	sub	a10, a9, a4
42038db4:	2188      	l32i.n	a8, a1, 8
42038db6:	44aa      	add.n	a4, a4, a10
42038db8:	828a80        	mull	a8, a10, a8
42038dbb:	882a      	add.n	a8, a8, a2
42038dbd:	441b      	addi.n	a4, a4, 1
42038dbf:	086d      	mov.n	a6, a8
42038dc1:	ffdb06        	j	42038d31 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcd>
42038dc4:	0527d2        	l32i	a13, a7, 20
42038dc7:	00a0e2        	movi	a14, 0
42038dca:	20a770        	or	a10, a7, a7
42038dcd:	d1c9      	s32i.n	a12, a1, 52
42038dcf:	c1b9      	s32i.n	a11, a1, 48
42038dd1:	ffcee5        	call8	42038ac0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038dd4:	c1b8      	l32i.n	a11, a1, 48
42038dd6:	d1c8      	l32i.n	a12, a1, 52
42038dd8:	02a9      	s32i.n	a10, a2, 0
42038dda:	bb1b      	addi.n	a11, a11, 1
42038ddc:	224b      	addi.n	a2, a2, 4
42038dde:	012182        	l32i	a8, a1, 4
42038de1:	df2b87        	blt	a11, a8, 42038dc4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x160>
42038de4:	0b2182        	l32i	a8, a1, 44
42038de7:	0527d2        	l32i	a13, a7, 20
42038dea:	81b8      	l32i.n	a11, a1, 32
42038dec:	0e0c      	movi.n	a14, 0
42038dee:	07ad      	mov.n	a10, a7
42038df0:	268a      	add.n	a2, a6, a8
42038df2:	ffcce5        	call8	42038ac0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038df5:	a188      	l32i.n	a8, a1, 40
42038df7:	828a      	add.n	a8, a2, a8
42038df9:	049827        	bne	a8, a2, 42038e01 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x19d>
42038dfc:	7188      	l32i.n	a8, a1, 28
42038dfe:	ffd2c6        	j	42038d4d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038e01:	02a9      	s32i.n	a10, a2, 0
42038e03:	224b      	addi.n	a2, a2, 4
42038e05:	fffc06        	j	42038df9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x195>
42038e08:	0527d2        	l32i	a13, a7, 20
42038e0b:	00a0e2        	movi	a14, 0
42038e0e:	20a770        	or	a10, a7, a7
42038e11:	f199      	s32i.n	a9, a1, 60
42038e13:	e189      	s32i.n	a8, a1, 56
42038e15:	d1c9      	s32i.n	a12, a1, 52
42038e17:	c1b9      	s32i.n	a11, a1, 48
42038e19:	ffca65        	call8	42038ac0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038e1c:	f198      	l32i.n	a9, a1, 60
42038e1e:	c1b8      	l32i.n	a11, a1, 48
42038e20:	09a9      	s32i.n	a10, a9, 0
42038e22:	e188      	l32i.n	a8, a1, 56
42038e24:	d1c8      	l32i.n	a12, a1, 52
42038e26:	bb1b      	addi.n	a11, a11, 1
42038e28:	994b      	addi.n	a9, a9, 4
42038e2a:	ffc986        	j	42038d54 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf0>
42038e2d:	00          	.byte	00
42038e2e:	61a8      	l32i.n	a10, a1, 24
42038e30:	1a1aa6        	blti	a10, 1, 42038e4e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038e33:	21a8      	l32i.n	a10, a1, 8
42038e35:	1f4a26        	beqi	a10, 4, 42038e58 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f4>
42038e38:	0acd      	mov.n	a12, a10
42038e3a:	06bd      	mov.n	a11, a6
42038e3c:	08ad      	mov.n	a10, a8
42038e3e:	d1d9      	s32i.n	a13, a1, 52
42038e40:	c199      	s32i.n	a9, a1, 48
42038e42:	1ea881        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038e45:	0008e0        	callx8	a8
42038e48:	c198      	l32i.n	a9, a1, 48
42038e4a:	d1d8      	l32i.n	a13, a1, 52
42038e4c:	0a8d      	mov.n	a8, a10
42038e4e:	21a8      	l32i.n	a10, a1, 8
42038e50:	dd1b      	addi.n	a13, a13, 1
42038e52:	88aa      	add.n	a8, a8, a10
42038e54:	ffd106        	j	42038d9c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x138>
42038e57:	06a800        	ee.ldf.64.xp	f10, f0, a0, a8
42038e5a:	08a9      	s32i.n	a10, a8, 0
42038e5c:	fffb86        	j	42038e4e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038e5f:	57d800        	ee.stf.64.xp	f13, f5, a0, a8
42038e62:	02bd      	mov.n	a11, a2
42038e64:	0e0c      	movi.n	a14, 0
42038e66:	04cd      	mov.n	a12, a4
42038e68:	07ad      	mov.n	a10, a7
42038e6a:	1189      	s32i.n	a8, a1, 4
42038e6c:	ffc525        	call8	42038ac0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038e6f:	1188      	l32i.n	a8, a1, 4
42038e71:	221b      	addi.n	a2, a2, 1
42038e73:	08a9      	s32i.n	a10, a8, 0
42038e75:	232280        	sext	a2, a2, 15
42038e78:	884b      	addi.n	a8, a8, 4
42038e7a:	086d      	mov.n	a6, a8
42038e7c:	e0a527        	bge	a5, a2, 42038e60 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1fc>
42038e7f:	441b      	addi.n	a4, a4, 1
42038e81:	234480        	sext	a4, a4, 15
42038e84:	3188      	l32i.n	a8, a1, 12
42038e86:	02a847        	bge	a8, a4, 42038e8c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038e89:	ffaa46        	j	42038d36 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd2>
42038e8c:	068d      	mov.n	a8, a6
42038e8e:	032d      	mov.n	a2, a3
42038e90:	fff986        	j	42038e7a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/inline_read/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039058 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
42039058:	00a136        	entry	a1, 80
4203905b:	238480        	sext	a8, a4, 15
4203905e:	027d      	mov.n	a7, a2
42039060:	a189      	s32i.n	a8, a1, 40
42039062:	0f0222        	l8ui	a2, a2, 15
42039065:	238580        	sext	a8, a5, 15
42039068:	b189      	s32i.n	a8, a1, 44
4203906a:	243216        	beqz	a2, 420392b1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
4203906d:	009782        	l16si	a8, a7, 0
42039070:	0697f2        	l16si	a15, a7, 12
42039073:	3189      	s32i.n	a8, a1, 12
42039075:	3198      	l32i.n	a9, a1, 12
42039077:	a188      	l32i.n	a8, a1, 40
42039079:	c08890        	sub	a8, a8, a9
4203907c:	11c8c0        	slli	a12, a8, 4
4203907f:	4189      	s32i.n	a8, a1, 16
42039081:	019782        	l16si	a8, a7, 2
42039084:	cc8b      	addi.n	a12, a12, 8
42039086:	5189      	s32i.n	a8, a1, 20
42039088:	5198      	l32i.n	a9, a1, 20
4203908a:	b188      	l32i.n	a8, a1, 44
4203908c:	c08890        	sub	a8, a8, a9
4203908f:	6189      	s32i.n	a8, a1, 24
42039091:	069392        	l16si	a9, a3, 12
42039094:	1188c0        	slli	a8, a8, 4
42039097:	888b      	addi.n	a8, a8, 8
42039099:	c0f8f0        	sub	a15, a8, a15
4203909c:	c08890        	sub	a8, a8, a9
4203909f:	9cca      	add.n	a9, a12, a12
420390a1:	7199      	s32i.n	a9, a1, 28
420390a3:	029792        	l16si	a9, a7, 4
420390a6:	d88a      	add.n	a13, a8, a8
420390a8:	8199      	s32i.n	a9, a1, 32
420390aa:	1199c0        	slli	a9, a9, 4
420390ad:	2199      	s32i.n	a9, a1, 8
420390af:	039792        	l16si	a9, a7, 6
420390b2:	2168      	l32i.n	a6, a1, 8
420390b4:	11b9c0        	slli	a11, a9, 4
420390b7:	effa      	add.n	a14, a15, a15
420390b9:	c04b80        	sub	a4, a11, a8
420390bc:	9199      	s32i.n	a9, a1, 36
420390be:	72adb0        	salt	a10, a13, a11
420390c1:	729eb0        	salt	a9, a14, a11
420390c4:	c056c0        	sub	a5, a6, a12
420390c7:	990b      	addi.n	a9, a9, -1
420390c9:	aa0b      	addi.n	a10, a10, -1
420390cb:	434480        	min	a4, a4, a8
420390ce:	7188      	l32i.n	a8, a1, 28
420390d0:	609090        	neg	a9, a9
420390d3:	60a0a0        	neg	a10, a10
420390d6:	4355c0        	min	a5, a5, a12
420390d9:	c0cbf0        	sub	a12, a11, a15
420390dc:	749090        	extui	a9, a9, 0, 8
420390df:	74a0a0        	extui	a10, a10, 0, 8
420390e2:	43ccf0        	min	a12, a12, a15
420390e5:	732867        	blt	a8, a6, 4203915c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x104>
420390e8:	682eb7        	blt	a14, a11, 42039154 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xfc>
420390eb:	0a0782        	l8ui	a8, a7, 10
420390ee:	7319a7        	beq	a9, a10, 42039165 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10d>
420390f1:	7f2db7        	blt	a13, a11, 42039174 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11c>
420390f4:	0a0762        	l8ui	a6, a7, 10
420390f7:	23d880        	sext	a13, a8, 15
420390fa:	11ddc0        	slli	a13, a13, 4
420390fd:	02acd7        	bge	a12, a13, 42039103 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
42039100:	002206        	j	4203918c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
42039103:	23f680        	sext	a15, a6, 15
42039106:	11ffc0        	slli	a15, a15, 4
42039109:	7f24f7        	blt	a4, a15, 4203918c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
4203910c:	43fc40        	min	a15, a12, a4
4203910f:	02af57        	bge	a15, a5, 42039115 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xbd>
42039112:	006f86        	j	420392d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27c>
42039115:	a198      	l32i.n	a9, a1, 40
42039117:	3188      	l32i.n	a8, a1, 12
42039119:	21a8      	l32i.n	a10, a1, 8
4203911b:	c08890        	sub	a8, a8, a9
4203911e:	7198      	l32i.n	a9, a1, 28
42039120:	0829a7        	blt	a9, a10, 4203912c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd4>
42039123:	4188      	l32i.n	a8, a1, 16
42039125:	8198      	l32i.n	a9, a1, 32
42039127:	c08890        	sub	a8, a8, a9
4203912a:	881b      	addi.n	a8, a8, 1
4203912c:	096c      	movi.n	a9, -32
4203912e:	02a897        	bge	a8, a9, 42039134 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdc>
42039131:	0064c6        	j	420392c8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
42039134:	092c      	movi.n	a9, 32
42039136:	02a987        	bge	a9, a8, 4203913c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
42039139:	006446        	j	420392ce <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
4203913c:	220b      	addi.n	a2, a2, -1
4203913e:	902220        	addx2	a2, a2, a2
42039141:	1192a0        	slli	a9, a2, 6
42039144:	229a      	add.n	a2, a2, a9
42039146:	1deb91        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
42039149:	898a      	add.n	a8, a9, a8
4203914b:	882a      	add.n	a8, a8, a2
4203914d:	200822        	l8ui	a2, a8, 32
42039150:	005746        	j	420392b1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
42039153:	078200        	ee.stf.64.xp	f8, f0, a0, a2
42039156:	c609      	s32i.n	a0, a6, 48
42039158:	e4          	.byte	0xe4
42039159:	b70000ff 	ee.vmulas.s16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
4203915d:	07820c2e 	ee.vmulas.s16.accx.ld.ip.qup	q6, a2, 0x1c0, q0, q1, q0, q2
42039161:	a70b      	addi.n	a10, a7, -1
42039163:	1699      	s32i.n	a9, a6, 4
42039165:	086d      	mov.n	a6, a8
42039167:	ffe306        	j	420390f7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
4203916a:	820000        	mull	a0, a0, a0
4203916d:	c60807        	bnone	a8, a0, 42039137 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdf>
42039170:	fffb      	addi.n	a15, a15, 15
42039172:	620000        	saltu	a0, a0, a0
42039175:	060907        	bnone	a9, a0, 4203917f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x127>
42039178:	0000ffdf 	ee.vmulas.s16.accx.ld.ip.qup	q0, a13, 240, q7, q6, q0, q0
4203917c:	052db7        	blt	a13, a11, 42039185 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12d>
4203917f:	0b0762        	l8ui	a6, a7, 11
42039182:	ffdc46        	j	420390f7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
42039185:	080762        	l8ui	a6, a7, 8
42039188:	ffdac6        	j	420390f7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
4203918b:	00          	.byte	00
4203918c:	5199a7        	bne	a9, a10, 420391e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
4203918f:	538680        	max	a8, a6, a8
42039192:	238880        	sext	a8, a8, 15
42039195:	1188c0        	slli	a8, a8, 4
42039198:	452587        	blt	a5, a8, 420391e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
4203919b:	538c40        	max	a8, a12, a4
4203919e:	3f2587        	blt	a5, a8, 420391e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
420391a1:	9198      	l32i.n	a9, a1, 36
420391a3:	6188      	l32i.n	a8, a1, 24
420391a5:	c08890        	sub	a8, a8, a9
420391a8:	881b      	addi.n	a8, a8, 1
420391aa:	290c      	movi.n	a9, 2
420391ac:	08aeb7        	bge	a14, a11, 420391b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x160>
420391af:	b198      	l32i.n	a9, a1, 44
420391b1:	5188      	l32i.n	a8, a1, 20
420391b3:	c08890        	sub	a8, a8, a9
420391b6:	190c      	movi.n	a9, 1
420391b8:	0a6c      	movi.n	a10, -32
420391ba:	02a8a7        	bge	a8, a10, 420391c0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x168>
420391bd:	0041c6        	j	420392c8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
420391c0:	0a2c      	movi.n	a10, 32
420391c2:	02aa87        	bge	a10, a8, 420391c8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x170>
420391c5:	004146        	j	420392ce <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
420391c8:	220b      	addi.n	a2, a2, -1
420391ca:	902220        	addx2	a2, a2, a2
420391cd:	229a      	add.n	a2, a2, a9
420391cf:	1dc991        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420391d2:	11a2a0        	slli	a10, a2, 6
420391d5:	aa2a      	add.n	a10, a10, a2
420391d7:	898a      	add.n	a8, a9, a8
420391d9:	88aa      	add.n	a8, a8, a10
420391db:	ffdb86        	j	4203914d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf5>
420391de:	000000        	ill
420391e1:	091782        	l16ui	a8, a7, 18
420391e4:	4395c0        	min	a9, a5, a12
420391e7:	608080        	neg	a8, a8
420391ea:	00a022        	movi	a2, 0
420391ed:	4e2987        	blt	a9, a8, 4203923f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1e7>
420391f0:	c0ad50        	sub	a10, a13, a5
420391f3:	090c      	movi.n	a9, 0
420391f5:	c08dc0        	sub	a8, a13, a12
420391f8:	0229a7        	blt	a9, a10, 420391fe <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a6>
420391fb:	002d46        	j	420392b4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
420391fe:	022987        	blt	a9, a8, 42039204 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ac>
42039201:	002bc6        	j	420392b4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
42039204:	828880        	mull	a8, a8, a8
42039207:	82aaa0        	mull	a10, a10, a10
4203920a:	21d9      	s32i.n	a13, a1, 8
4203920c:	80aa80        	add	a10, a10, a8
4203920f:	4d33e5        	call8	4208654c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039212:	0221d2        	l32i	a13, a1, 8
42039215:	0e0792        	l8ui	a9, a7, 14
42039218:	c08ad0        	sub	a8, a10, a13
4203921b:	c08890        	sub	a8, a8, a9
4203921e:	081792        	l16ui	a9, a7, 16
42039221:	828890        	mull	a8, a8, a9
42039224:	1db691        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
42039227:	889a      	add.n	a8, a8, a9
42039229:	218c80        	srai	a8, a8, 12
4203922c:	ffa092        	movi	a9, 255
4203922f:	438890        	min	a8, a8, a9
42039232:	090c      	movi.n	a9, 0
42039234:	538890        	max	a8, a8, a9
42039237:	1db091        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
4203923a:	898a      	add.n	a8, a9, a8
4203923c:	000822        	l8ui	a2, a8, 0
4203923f:	091392        	l16ui	a9, a3, 18
42039242:	43a540        	min	a10, a5, a4
42039245:	609090        	neg	a9, a9
42039248:	00a082        	movi	a8, 0
4203924b:	492a97        	blt	a10, a9, 42039298 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
4203924e:	236680        	sext	a6, a6, 15
42039251:	1166c0        	slli	a6, a6, 4
42039254:	c09640        	sub	a9, a6, a4
42039257:	c0a650        	sub	a10, a6, a5
4203925a:	62a897        	bge	a8, a9, 420392c0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
4203925d:	5fa8a7        	bge	a8, a10, 420392c0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
42039260:	829990        	mull	a9, a9, a9
42039263:	82aaa0        	mull	a10, a10, a10
42039266:	80a9a0        	add	a10, a9, a10
42039269:	4d2e25        	call8	4208654c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
4203926c:	0e0392        	l8ui	a9, a3, 14
4203926f:	c08a60        	sub	a8, a10, a6
42039272:	c08890        	sub	a8, a8, a9
42039275:	081392        	l16ui	a9, a3, 16
42039278:	828890        	mull	a8, a8, a9
4203927b:	1da091        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
4203927e:	808890        	add	a8, a8, a9
42039281:	218c80        	srai	a8, a8, 12
42039284:	ffa092        	movi	a9, 255
42039287:	438890        	min	a8, a8, a9
4203928a:	090c      	movi.n	a9, 0
4203928c:	538890        	max	a8, a8, a9
4203928f:	1d9c91        	l32r	a9, 42000900 <_stext+0x8e0> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
42039292:	808980        	add	a8, a9, a8
42039295:	000882        	l8ui	a8, a8, 0
42039298:	d19280        	mul16s	a9, a2, a8
4203929b:	1d91a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
4203929e:	7fc992        	addi	a9, a9, 127
420392a1:	b2a9a0        	mulsh	a10, a9, a10
420392a4:	282a      	add.n	a2, a8, a2
420392a6:	99aa      	add.n	a9, a9, a10
420392a8:	219790        	srai	a9, a9, 7
420392ab:	c02290        	sub	a2, a2, a9
420392ae:	742020        	extui	a2, a2, 0, 8
420392b1:	f01d      	retw.n
420392b3:	00          	.byte	00
420392b4:	022c57        	blt	a12, a5, 420392ba <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x262>
420392b7:	ffd686        	j	42039215 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
420392ba:	08ad      	mov.n	a10, a8
420392bc:	ffd546        	j	42039215 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
420392bf:	a45700        	extui	a5, a0, 7, 11
420392c2:	ada8      	l32i.n	a10, a13, 40
420392c4:	c609      	s32i.n	a0, a6, 48
420392c6:	ffe8      	l32i.n	a14, a15, 60
420392c8:	68a022        	movi	a2, 104
420392cb:	fff886        	j	420392b1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
420392ce:	020c      	movi.n	a2, 0
420392d0:	fff746        	j	420392b1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
420392d3:	00          	.byte	00
420392d4:	0219a7        	beq	a9, a10, 420392da <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x282>
420392d7:	ffc546        	j	420391f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x198>
420392da:	ffac46        	j	4203918f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x137>
