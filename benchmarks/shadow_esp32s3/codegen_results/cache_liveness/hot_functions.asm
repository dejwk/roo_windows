
/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


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


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


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


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208653c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
4208653c:	004136        	entry	a1, 32
4208653f:	029d      	mov.n	a9, a2
42086541:	120c      	movi.n	a2, 1
42086543:	832990        	moveqz	a2, a9, a9
42086546:	4149b6        	bltui	a9, 4, 4208658b <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
42086549:	40f980        	nsau	a8, a9
4208654c:	dfc882        	addi	a8, a8, -33
4208654f:	608080        	neg	a8, a8
42086552:	218180        	srai	a8, a8, 1
42086555:	fec882        	addi	a8, a8, -2
42086558:	3a0c      	movi.n	a10, 3
4208655a:	401800        	ssl	a8
4208655d:	a1aa00        	sll	a10, a10
42086560:	c289a0        	quou	a8, a9, a10
42086563:	88aa      	add.n	a8, a8, a10
42086565:	418180        	srli	a8, a8, 1
42086568:	c2a980        	quou	a10, a9, a8
4208656b:	aa8a      	add.n	a10, a10, a8
4208656d:	41a1a0        	srli	a10, a10, 1
42086570:	c289a0        	quou	a8, a9, a10
42086573:	88aa      	add.n	a8, a8, a10
42086575:	1a0c      	movi.n	a10, 1
42086577:	418180        	srli	a8, a8, 1
4208657a:	11aa00        	slli	a10, a10, 16
4208657d:	0518a7        	beq	a8, a10, 42086586 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
42086580:	82a880        	mull	a10, a8, a8
42086583:	01b9a7        	bgeu	a9, a10, 42086588 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
42086586:	880b      	addi.n	a8, a8, -1
42086588:	f42080        	extui	a2, a8, 0, 16
4208658b:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208639c <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
4208639c:	004136        	entry	a1, 32
4208639f:	208230        	or	a8, a2, a3
420863a2:	0228d6        	bgez	a8, 420863c8 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
420863a5:	c02240        	sub	a2, a2, a4
420863a8:	40f290        	nsau	a9, a2
420863ab:	419590        	srli	a9, a9, 5
420863ae:	b28c      	beqz.n	a2, 420863bd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
420863b0:	c03340        	sub	a3, a3, a4
420863b3:	180c      	movi.n	a8, 1
420863b5:	938930        	movnez	a8, a9, a3
420863b8:	ffa022        	movi	a2, 255
420863bb:	688c      	beqz.n	a8, 420863c5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863bd:	1125c0        	slli	a2, a5, 4
420863c0:	252a      	add.n	a2, a5, a2
420863c2:	742020        	extui	a2, a2, 0, 8
420863c5:	f01d      	retw.n
420863c7:	222000        	orb	b2, b0, b0
420863ca:	3330d1        	l32r	a13, 4205308c <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
420863cd:	823ad1        	l32r	a13, 42066cb8 <esp_littlefs_free_fd$isra$0+0x48> (880b3d12 <_rtc_reserved_end+0x27fb3d12>)
420863d0:	118880        	slli	a8, a8, 8
420863d3:	ffa022        	movi	a2, 255
420863d6:	eb48a6        	blti	a8, 4, 420863c5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863d9:	40f890        	nsau	a9, a8
420863dc:	dfc992        	addi	a9, a9, -33
420863df:	609090        	neg	a9, a9
420863e2:	219190        	srai	a9, a9, 1
420863e5:	fec992        	addi	a9, a9, -2
420863e8:	3a0c      	movi.n	a10, 3
420863ea:	401900        	ssl	a9
420863ed:	a1aa00        	sll	a10, a10
420863f0:	c298a0        	quou	a9, a8, a10
420863f3:	99aa      	add.n	a9, a9, a10
420863f5:	419190        	srli	a9, a9, 1
420863f8:	c2a890        	quou	a10, a8, a9
420863fb:	aa9a      	add.n	a10, a10, a9
420863fd:	41a1a0        	srli	a10, a10, 1
42086400:	c298a0        	quou	a9, a8, a10
42086403:	99aa      	add.n	a9, a9, a10
42086405:	1a0c      	movi.n	a10, 1
42086407:	419190        	srli	a9, a9, 1
4208640a:	11aa00        	slli	a10, a10, 16
4208640d:	0519a7        	beq	a9, a10, 42086416 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7a>
42086410:	82a990        	mull	a10, a9, a9
42086413:	01b8a7        	bgeu	a8, a10, 42086418 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7c>
42086416:	990b      	addi.n	a9, a9, -1
42086418:	1144c0        	slli	a4, a4, 4
4208641b:	554a      	add.n	a5, a5, a4
4208641d:	c05590        	sub	a5, a5, a9
42086420:	f48050        	extui	a8, a5, 0, 16
42086423:	239580        	sext	a9, a5, 15
42086426:	020c      	movi.n	a2, 0
42086428:	99e5f7        	bbsi	a5, 15, 420863c5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208642b:	fa0c      	movi.n	a10, 15
4208642d:	ffa022        	movi	a2, 255
42086430:	912a97        	blt	a10, a9, 420863c5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086433:	1198c0        	slli	a9, a8, 4
42086436:	889a      	add.n	a8, a8, a9
42086438:	742080        	extui	a2, a8, 0, 8
4208643b:	ffe186        	j	420863c5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


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
42033f65:	3cc625        	call8	42070bc8 <_Unwind_Resume>
42033f68:	fffac6        	j	42033f57 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


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
4203877d:	4dc1e5        	call8	4208639c <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
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


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


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
42038bb5:	50fc06        	j	4204cfa9 <uart_get_word_length+0x11>
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
42038c1e:	4c21a5        	call8	42084e38 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
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


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


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


/tmp/roo-shadow-review/codegen/cache_liveness/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039070 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
42039070:	006136        	entry	a1, 48
42039073:	026d      	mov.n	a6, a2
42039075:	0f0222        	l8ui	a2, a2, 15
42039078:	234480        	sext	a4, a4, 15
4203907b:	239580        	sext	a9, a5, 15
4203907e:	21f216        	beqz	a2, 420392a1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x231>
42039081:	009682        	l16si	a8, a6, 0
42039084:	0396b2        	l16si	a11, a6, 6
42039087:	c04480        	sub	a4, a4, a8
4203908a:	019682        	l16si	a8, a6, 2
4203908d:	11a4c0        	slli	a10, a4, 4
42039090:	c09980        	sub	a9, a9, a8
42039093:	1199c0        	slli	a9, a9, 4
42039096:	069682        	l16si	a8, a6, 12
42039099:	998b      	addi.n	a9, a9, 8
4203909b:	c04980        	sub	a4, a9, a8
4203909e:	3189      	s32i.n	a8, a1, 12
420390a0:	069382        	l16si	a8, a3, 12
420390a3:	029652        	l16si	a5, a6, 4
420390a6:	c09980        	sub	a9, a9, a8
420390a9:	11bbc0        	slli	a11, a11, 4
420390ac:	e44a      	add.n	a14, a4, a4
420390ae:	f99a      	add.n	a15, a9, a9
420390b0:	aa8b      	addi.n	a10, a10, 8
420390b2:	1155c0        	slli	a5, a5, 4
420390b5:	72ceb0        	salt	a12, a14, a11
420390b8:	72dfb0        	salt	a13, a15, a11
420390bb:	c075a0        	sub	a7, a5, a10
420390be:	c08b40        	sub	a8, a11, a4
420390c1:	cc0b      	addi.n	a12, a12, -1
420390c3:	dd0b      	addi.n	a13, a13, -1
420390c5:	60c0c0        	neg	a12, a12
420390c8:	60d0d0        	neg	a13, a13
420390cb:	4377a0        	min	a7, a7, a10
420390ce:	438840        	min	a8, a8, a4
420390d1:	aaaa      	add.n	a10, a10, a10
420390d3:	c04b90        	sub	a4, a11, a9
420390d6:	74c0c0        	extui	a12, a12, 0, 8
420390d9:	74d0d0        	extui	a13, a13, 0, 8
420390dc:	434490        	min	a4, a4, a9
420390df:	612a57        	blt	a10, a5, 42039144 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd4>
420390e2:	562eb7        	blt	a14, a11, 4203913c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xcc>
420390e5:	0a0692        	l8ui	a9, a6, 10
420390e8:	611cd7        	beq	a12, a13, 4203914d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdd>
420390eb:	6d2fb7        	blt	a15, a11, 4203915c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xec>
420390ee:	0a0652        	l8ui	a5, a6, 10
420390f1:	23f980        	sext	a15, a9, 15
420390f4:	11ffc0        	slli	a15, a15, 4
420390f7:	7a28f7        	blt	a8, a15, 42039175 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x105>
420390fa:	23a580        	sext	a10, a5, 15
420390fd:	11aac0        	slli	a10, a10, 4
42039100:	7124a7        	blt	a4, a10, 42039175 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x105>
42039103:	43a840        	min	a10, a8, a4
42039106:	02aa77        	bge	a10, a7, 4203910c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9c>
42039109:	006dc6        	j	420392c4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x254>
4203910c:	f8c772        	addi	a7, a7, -8
4203910f:	607070        	neg	a7, a7
42039112:	217470        	srai	a7, a7, 4
42039115:	086c      	movi.n	a8, -32
42039117:	02a787        	bge	a7, a8, 4203911d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xad>
4203911a:	006686        	j	420392b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x248>
4203911d:	082c      	movi.n	a8, 32
4203911f:	02a877        	bge	a8, a7, 42039125 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb5>
42039122:	006606        	j	420392be <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24e>
42039125:	220b      	addi.n	a2, a2, -1
42039127:	902220        	addx2	a2, a2, a2
4203912a:	1182a0        	slli	a8, a2, 6
4203912d:	228a      	add.n	a2, a2, a8
4203912f:	1df181        	l32r	a8, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
42039132:	887a      	add.n	a8, a8, a7
42039134:	882a      	add.n	a8, a8, a2
42039136:	200822        	l8ui	a2, a8, 32
42039139:	005906        	j	420392a1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x231>
4203913c:	090692        	l8ui	a9, a6, 9
4203913f:	ffe946        	j	420390e8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x78>
42039142:	b70000        	ee.stf.64.xp	f0, f11, a0, a0
42039145:	06920c2e 	ee.vmulas.s16.accx.ld.ip.qup	q4, a2, 0x1c0, q0, q1, q1, q2
42039149:	d70b      	addi.n	a13, a7, -1
4203914b:	169c      	beqz.n	a6, 42039160 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf0>
4203914d:	095d      	mov.n	a5, a9
4203914f:	ffe786        	j	420390f1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x81>
42039152:	00          	.byte	00
42039153:	069200        	ee.ldf.64.xp	f9, f0, a0, a2
42039156:	c608      	l32i.n	a0, a6, 48
42039158:	fffb      	addi.n	a15, a15, 15
4203915a:	00          	.byte	00
4203915b:	065200        	ee.ldf.64.xp	f5, f0, a0, a2
4203915e:	8609      	s32i.n	a0, a6, 32
42039160:	e3          	.byte	0xe3
42039161:	b70000ff 	ee.vmulas.s16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
42039165:	0652052f 	ee.vmulas.s16.accx.ld.ip.qup	q4, a2, 0x150, q4, q0, q5, q2
42039169:	c60b      	addi.n	a12, a6, -1
4203916b:	e0          	.byte	0xe0
4203916c:	080652ff 	ee.vmulas.s16.accx.ld.ip.qup	q0, a15, 0xfffffe20, q5, q2, q0, q6
42039170:	ffdf46        	j	420390f1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x81>
42039173:	d70000        	ee.stf.64.xp	f0, f13, a0, a0
42039176:	589c      	beqz.n	a8, 4203918f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11f>
42039178:	539590        	max	a9, a5, a9
4203917b:	239980        	sext	a9, a9, 15
4203917e:	1199c0        	slli	a9, a9, 4
42039181:	4c2797        	blt	a7, a9, 420391d1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x161>
42039184:	539840        	max	a9, a8, a4
42039187:	462797        	blt	a7, a9, 420391d1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x161>
4203918a:	f8c882        	addi	a8, a8, -8
4203918d:	608080        	neg	a8, a8
42039190:	06aeb7        	bge	a14, a11, 4203919a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12a>
42039193:	3198      	l32i.n	a9, a1, 12
42039195:	609090        	neg	a9, a9
42039198:	3199      	s32i.n	a9, a1, 12
4203919a:	3198      	l32i.n	a9, a1, 12
4203919c:	889a      	add.n	a8, a8, a9
4203919e:	218480        	srai	a8, a8, 4
420391a1:	190c      	movi.n	a9, 1
420391a3:	012eb7        	blt	a14, a11, 420391a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x138>
420391a6:	290c      	movi.n	a9, 2
420391a8:	0a6c      	movi.n	a10, -32
420391aa:	02a8a7        	bge	a8, a10, 420391b0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x140>
420391ad:	0041c6        	j	420392b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x248>
420391b0:	0a2c      	movi.n	a10, 32
420391b2:	02aa87        	bge	a10, a8, 420391b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x148>
420391b5:	004146        	j	420392be <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24e>
420391b8:	220b      	addi.n	a2, a2, -1
420391ba:	902220        	addx2	a2, a2, a2
420391bd:	229a      	add.n	a2, a2, a9
420391bf:	1dcd91        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420391c2:	11a2a0        	slli	a10, a2, 6
420391c5:	aa2a      	add.n	a10, a10, a2
420391c7:	898a      	add.n	a8, a9, a8
420391c9:	88aa      	add.n	a8, a8, a10
420391cb:	ffd9c6        	j	42039136 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc6>
420391ce:	000000        	ill
420391d1:	091692        	l16ui	a9, a6, 18
420391d4:	43a780        	min	a10, a7, a8
420391d7:	609090        	neg	a9, a9
420391da:	00a022        	movi	a2, 0
420391dd:	4e2a97        	blt	a10, a9, 4203922f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bf>
420391e0:	c0af70        	sub	a10, a15, a7
420391e3:	0b0c      	movi.n	a11, 0
420391e5:	c09f80        	sub	a9, a15, a8
420391e8:	022ba7        	blt	a11, a10, 420391ee <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x17e>
420391eb:	002d46        	j	420392a4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
420391ee:	022b97        	blt	a11, a9, 420391f4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x184>
420391f1:	002bc6        	j	420392a4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
420391f4:	829990        	mull	a9, a9, a9
420391f7:	82aaa0        	mull	a10, a10, a10
420391fa:	31f9      	s32i.n	a15, a1, 12
420391fc:	80aa90        	add	a10, a10, a9
420391ff:	4d33e5        	call8	4208653c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039202:	0321f2        	l32i	a15, a1, 12
42039205:	0e0692        	l8ui	a9, a6, 14
42039208:	c08af0        	sub	a8, a10, a15
4203920b:	c08890        	sub	a8, a8, a9
4203920e:	081692        	l16ui	a9, a6, 16
42039211:	828890        	mull	a8, a8, a9
42039214:	1dba91        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
42039217:	889a      	add.n	a8, a8, a9
42039219:	218c80        	srai	a8, a8, 12
4203921c:	ffa092        	movi	a9, 255
4203921f:	438890        	min	a8, a8, a9
42039222:	090c      	movi.n	a9, 0
42039224:	538890        	max	a8, a8, a9
42039227:	1db491        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
4203922a:	898a      	add.n	a8, a9, a8
4203922c:	000822        	l8ui	a2, a8, 0
4203922f:	091392        	l16ui	a9, a3, 18
42039232:	43a740        	min	a10, a7, a4
42039235:	609090        	neg	a9, a9
42039238:	00a082        	movi	a8, 0
4203923b:	492a97        	blt	a10, a9, 42039288 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x218>
4203923e:	235580        	sext	a5, a5, 15
42039241:	1155c0        	slli	a5, a5, 4
42039244:	c09540        	sub	a9, a5, a4
42039247:	c0a570        	sub	a10, a5, a7
4203924a:	62a897        	bge	a8, a9, 420392b0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
4203924d:	5fa8a7        	bge	a8, a10, 420392b0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
42039250:	829990        	mull	a9, a9, a9
42039253:	82aaa0        	mull	a10, a10, a10
42039256:	80a9a0        	add	a10, a9, a10
42039259:	4d2e25        	call8	4208653c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
4203925c:	0e0392        	l8ui	a9, a3, 14
4203925f:	c08a50        	sub	a8, a10, a5
42039262:	c08890        	sub	a8, a8, a9
42039265:	081392        	l16ui	a9, a3, 16
42039268:	828890        	mull	a8, a8, a9
4203926b:	1da491        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
4203926e:	808890        	add	a8, a8, a9
42039271:	218c80        	srai	a8, a8, 12
42039274:	ffa092        	movi	a9, 255
42039277:	438890        	min	a8, a8, a9
4203927a:	090c      	movi.n	a9, 0
4203927c:	538890        	max	a8, a8, a9
4203927f:	1da091        	l32r	a9, 42000900 <_stext+0x8e0> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
42039282:	808980        	add	a8, a9, a8
42039285:	000882        	l8ui	a8, a8, 0
42039288:	d19280        	mul16s	a9, a2, a8
4203928b:	1d95a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
4203928e:	7fc992        	addi	a9, a9, 127
42039291:	b2a9a0        	mulsh	a10, a9, a10
42039294:	282a      	add.n	a2, a8, a2
42039296:	99aa      	add.n	a9, a9, a10
42039298:	219790        	srai	a9, a9, 7
4203929b:	c02290        	sub	a2, a2, a9
4203929e:	742020        	extui	a2, a2, 0, 8
420392a1:	f01d      	retw.n
420392a3:	00          	.byte	00
420392a4:	022877        	blt	a8, a7, 420392aa <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23a>
420392a7:	ffd686        	j	42039205 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x195>
420392aa:	09ad      	mov.n	a10, a9
420392ac:	ffd546        	j	42039205 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x195>
420392af:	a47700        	extui	a7, a0, 7, 11
420392b2:	ada8      	l32i.n	a10, a13, 40
420392b4:	c609      	s32i.n	a0, a6, 48
420392b6:	ffe8      	l32i.n	a14, a15, 60
420392b8:	68a022        	movi	a2, 104
420392bb:	fff886        	j	420392a1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x231>
420392be:	020c      	movi.n	a2, 0
420392c0:	fff746        	j	420392a1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x231>
420392c3:	00          	.byte	00
420392c4:	021cd7        	beq	a12, a13, 420392ca <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25a>
420392c7:	ffc546        	j	420391e0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x170>
420392ca:	ffaa86        	j	42039178 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x108>
