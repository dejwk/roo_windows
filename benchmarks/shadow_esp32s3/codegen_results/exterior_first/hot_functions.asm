
/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038ca0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038ca0:	004136        	entry	a1, 32
42038ca3:	15cc      	bnez.n	a5, 42038ca8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8>
42038ca5:	f01d      	retw.n
42038ca7:	94c200        	extui	a12, a0, 2, 10
42038caa:	93b200        	movnez	a11, a2, a0
42038cad:	a22000        	muluh	a2, a0, a0
42038cb0:	20          	.byte	0x20
42038cb1:	fffd25        	call8	42038c84 <roo_windows::Decoration::read(short, short) const>
42038cb4:	0066a2        	s32i	a10, a6, 0
42038cb7:	02c332        	addi	a3, a3, 2
42038cba:	442b      	addi.n	a4, a4, 2
42038cbc:	550b      	addi.n	a5, a5, -1
42038cbe:	664b      	addi.n	a6, a6, 4
42038cc0:	fff7c6        	j	42038ca3 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3>


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033130 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42033130:	006136        	entry	a1, 48
42033133:	002282        	l32i	a8, a2, 0
42033136:	20f770        	or	a15, a7, a7
42033139:	0b2882        	l32i	a8, a8, 44
4203313c:	233380        	sext	a3, a3, 15
4203313f:	234480        	sext	a4, a4, 15
42033142:	235580        	sext	a5, a5, 15
42033145:	236680        	sext	a6, a6, 15
42033148:	02ad      	mov.n	a10, a2
4203314a:	31f9      	s32i.n	a15, a1, 12
4203314c:	06ed      	mov.n	a14, a6
4203314e:	05dd      	mov.n	a13, a5
42033150:	04cd      	mov.n	a12, a4
42033152:	03bd      	mov.n	a11, a3
42033154:	0008e0        	callx8	a8
42033157:	027d      	mov.n	a7, a2
42033159:	0a2d      	mov.n	a2, a10
4203315b:	facc      	bnez.n	a10, 4203316e <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
4203315d:	31f8      	l32i.n	a15, a1, 12
4203315f:	06ed      	mov.n	a14, a6
42033161:	05dd      	mov.n	a13, a5
42033163:	04cd      	mov.n	a12, a4
42033165:	03bd      	mov.n	a11, a3
42033167:	07ad      	mov.n	a10, a7
42033169:	dc5d65        	call8	4200f740 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
4203316c:	0a2d      	mov.n	a2, a10
4203316e:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420865ac <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
420865ac:	004136        	entry	a1, 32
420865af:	029d      	mov.n	a9, a2
420865b1:	120c      	movi.n	a2, 1
420865b3:	832990        	moveqz	a2, a9, a9
420865b6:	4149b6        	bltui	a9, 4, 420865fb <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
420865b9:	40f980        	nsau	a8, a9
420865bc:	dfc882        	addi	a8, a8, -33
420865bf:	608080        	neg	a8, a8
420865c2:	218180        	srai	a8, a8, 1
420865c5:	fec882        	addi	a8, a8, -2
420865c8:	3a0c      	movi.n	a10, 3
420865ca:	401800        	ssl	a8
420865cd:	a1aa00        	sll	a10, a10
420865d0:	c289a0        	quou	a8, a9, a10
420865d3:	88aa      	add.n	a8, a8, a10
420865d5:	418180        	srli	a8, a8, 1
420865d8:	c2a980        	quou	a10, a9, a8
420865db:	aa8a      	add.n	a10, a10, a8
420865dd:	41a1a0        	srli	a10, a10, 1
420865e0:	c289a0        	quou	a8, a9, a10
420865e3:	88aa      	add.n	a8, a8, a10
420865e5:	1a0c      	movi.n	a10, 1
420865e7:	418180        	srli	a8, a8, 1
420865ea:	11aa00        	slli	a10, a10, 16
420865ed:	0518a7        	beq	a8, a10, 420865f6 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
420865f0:	82a880        	mull	a10, a8, a8
420865f3:	01b9a7        	bgeu	a9, a10, 420865f8 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
420865f6:	880b      	addi.n	a8, a8, -1
420865f8:	f42080        	extui	a2, a8, 0, 16
420865fb:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208640c <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
4208640c:	004136        	entry	a1, 32
4208640f:	208230        	or	a8, a2, a3
42086412:	0228d6        	bgez	a8, 42086438 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
42086415:	c02240        	sub	a2, a2, a4
42086418:	40f290        	nsau	a9, a2
4208641b:	419590        	srli	a9, a9, 5
4208641e:	b28c      	beqz.n	a2, 4208642d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
42086420:	c03340        	sub	a3, a3, a4
42086423:	180c      	movi.n	a8, 1
42086425:	938930        	movnez	a8, a9, a3
42086428:	ffa022        	movi	a2, 255
4208642b:	688c      	beqz.n	a8, 42086435 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208642d:	1125c0        	slli	a2, a5, 4
42086430:	252a      	add.n	a2, a5, a2
42086432:	742020        	extui	a2, a2, 0, 8
42086435:	f01d      	retw.n
42086437:	222000        	orb	b2, b0, b0
4208643a:	3330d1        	l32r	a13, 420530fc <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
4208643d:	823ad1        	l32r	a13, 42066d28 <esp_littlefs_free_fd$isra$0+0x48> (880b3d12 <_rtc_reserved_end+0x27fb3d12>)
42086440:	118880        	slli	a8, a8, 8
42086443:	ffa022        	movi	a2, 255
42086446:	eb48a6        	blti	a8, 4, 42086435 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086449:	40f890        	nsau	a9, a8
4208644c:	dfc992        	addi	a9, a9, -33
4208644f:	609090        	neg	a9, a9
42086452:	219190        	srai	a9, a9, 1
42086455:	fec992        	addi	a9, a9, -2
42086458:	3a0c      	movi.n	a10, 3
4208645a:	401900        	ssl	a9
4208645d:	a1aa00        	sll	a10, a10
42086460:	c298a0        	quou	a9, a8, a10
42086463:	99aa      	add.n	a9, a9, a10
42086465:	419190        	srli	a9, a9, 1
42086468:	c2a890        	quou	a10, a8, a9
4208646b:	aa9a      	add.n	a10, a10, a9
4208646d:	41a1a0        	srli	a10, a10, 1
42086470:	c298a0        	quou	a9, a8, a10
42086473:	99aa      	add.n	a9, a9, a10
42086475:	1a0c      	movi.n	a10, 1
42086477:	419190        	srli	a9, a9, 1
4208647a:	11aa00        	slli	a10, a10, 16
4208647d:	0519a7        	beq	a9, a10, 42086486 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7a>
42086480:	82a990        	mull	a10, a9, a9
42086483:	01b8a7        	bgeu	a8, a10, 42086488 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7c>
42086486:	990b      	addi.n	a9, a9, -1
42086488:	1144c0        	slli	a4, a4, 4
4208648b:	554a      	add.n	a5, a5, a4
4208648d:	c05590        	sub	a5, a5, a9
42086490:	f48050        	extui	a8, a5, 0, 16
42086493:	239580        	sext	a9, a5, 15
42086496:	020c      	movi.n	a2, 0
42086498:	99e5f7        	bbsi	a5, 15, 42086435 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208649b:	fa0c      	movi.n	a10, 15
4208649d:	ffa022        	movi	a2, 255
420864a0:	912a97        	blt	a10, a9, 42086435 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420864a3:	1198c0        	slli	a9, a8, 4
420864a6:	889a      	add.n	a8, a8, a9
420864a8:	742080        	extui	a2, a8, 0, 8
420864ab:	ffe186        	j	42086435 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033ea4 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033ea4:	00a136        	entry	a1, 80
42033ea7:	3169      	s32i.n	a6, a1, 12
42033ea9:	2159      	s32i.n	a5, a1, 8
42033eab:	060c      	movi.n	a6, 0
42033ead:	1cc172        	addi	a7, a1, 28
42033eb0:	2188      	l32i.n	a8, a1, 8
42033eb2:	029687        	bne	a6, a8, 42033eb8 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033eb5:	f01d      	retw.n
42033eb7:	938200        	movnez	a8, a2, a0
42033eba:	22d200        	orb	b13, b2, b0
42033ebd:	018917        	bany	a9, a1, 42033ec2 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033ec0:	009482        	l16si	a8, a4, 0
42033ec3:	01b8      	l32i.n	a11, a1, 0
42033ec5:	1622a2        	l32i	a10, a2, 88
42033ec8:	08cd      	mov.n	a12, a8
42033eca:	0067d2        	s32i	a13, a7, 0
42033ecd:	20e770        	or	a14, a7, a7
42033ed0:	016182        	s32i	a8, a1, 4
42033ed3:	201110        	or	a1, a1, a1
42033ed6:	fffa25        	call8	42033e78 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033ed9:	182282        	l32i	a8, a2, 96
42033edc:	1922b2        	l32i	a11, a2, 100
42033edf:	8189      	s32i.n	a8, a1, 32
42033ee1:	20c152        	addi	a5, a1, 32
42033ee4:	050b16        	beqz	a11, 42033f38 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033ee7:	0bc8      	l32i.n	a12, a11, 0
42033ee9:	24c1a2        	addi	a10, a1, 36
42033eec:	201110        	or	a1, a1, a1
42033eef:	ff3665        	call8	42033254 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033ef2:	07d8      	l32i.n	a13, a7, 0
42033ef4:	11c8      	l32i.n	a12, a1, 4
42033ef6:	0021b2        	l32i	a11, a1, 0
42033ef9:	24c1a2        	addi	a10, a1, 36
42033efc:	201110        	or	a1, a1, a1
42033eff:	fe1865        	call8	42032084 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f02:	07a9      	s32i.n	a10, a7, 0
42033f04:	24c1a2        	addi	a10, a1, 36
42033f07:	201110        	or	a1, a1, a1
42033f0a:	f894a5        	call8	4202c854 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f0d:	1922b2        	l32i	a11, a2, 100
42033f10:	24c1a2        	addi	a10, a1, 36
42033f13:	002bc2        	l32i	a12, a11, 0
42033f16:	201110        	or	a1, a1, a1
42033f19:	ff33a5        	call8	42033254 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f1c:	05d8      	l32i.n	a13, a5, 0
42033f1e:	0094c2        	l16si	a12, a4, 0
42033f21:	0093b2        	l16si	a11, a3, 0
42033f24:	24c1a2        	addi	a10, a1, 36
42033f27:	201110        	or	a1, a1, a1
42033f2a:	fe15a5        	call8	42032084 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f2d:	05a9      	s32i.n	a10, a5, 0
42033f2f:	24c1a2        	addi	a10, a1, 36
42033f32:	201110        	or	a1, a1, a1
42033f35:	f891e5        	call8	4202c854 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f38:	0094c2        	l16si	a12, a4, 0
42033f3b:	0093b2        	l16si	a11, a3, 0
42033f3e:	07d8      	l32i.n	a13, a7, 0
42033f40:	05ed      	mov.n	a14, a5
42033f42:	a24b      	addi.n	a10, a2, 4
42033f44:	201110        	or	a1, a1, a1
42033f47:	04bfa5        	call8	42038b40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f4a:	3188      	l32i.n	a8, a1, 12
42033f4c:	332b      	addi.n	a3, a3, 2
42033f4e:	a08680        	addx4	a8, a6, a8
42033f51:	08a9      	s32i.n	a10, a8, 0
42033f53:	661b      	addi.n	a6, a6, 1
42033f55:	442b      	addi.n	a4, a4, 2
42033f57:	ffd546        	j	42033eb0 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f5a:	c1a200        	mul16u	a10, a2, a0
42033f5d:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f60:	20          	.byte	0x20
42033f61:	f88f25        	call8	4202c854 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f64:	02ad      	mov.n	a10, a2
42033f66:	201110        	or	a1, a1, a1
42033f69:	3ccce5        	call8	42070c38 <_Unwind_Resume>
42033f6c:	fffac6        	j	42033f5b <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203871c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
4203871c:	006136        	entry	a1, 48
4203871f:	189192        	l16si	a9, a1, 48
42038722:	3169      	s32i.n	a6, a1, 12
42038724:	f1cb      	addi.n	a15, a1, 12
42038726:	1a9182        	l16si	a8, a1, 52
42038729:	02ed      	mov.n	a14, a2
4203872b:	74d070        	extui	a13, a7, 0, 8
4203872e:	02a927        	bge	a9, a2, 42038734 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
42038731:	0030c6        	j	420387f8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038734:	02a497        	bge	a4, a9, 4203873a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
42038737:	002f46        	j	420387f8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203873a:	02a837        	bge	a8, a3, 42038740 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
4203873d:	002dc6        	j	420387f8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038740:	02a587        	bge	a5, a8, 42038746 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a>
42038743:	002c46        	j	420387f8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038746:	000fc2        	l8ui	a12, a15, 0
42038749:	3c9c      	beqz.n	a12, 42038760 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
4203874b:	a2ca      	add.n	a10, a2, a12
4203874d:	0f2a97        	blt	a10, a9, 42038760 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038750:	b3ca      	add.n	a11, a3, a12
42038752:	0a2b87        	blt	a11, a8, 42038760 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038755:	c0bb80        	sub	a11, a11, a8
42038758:	c0aa90        	sub	a10, a10, a9
4203875b:	000706        	j	4203877b <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x5f>
4203875e:	c20000        	quou	a0, a0, a0
42038761:	1c16010f 	ee.vmulas.s16.qacc.ld.ip.qup	q0, a0, -240, q4, q0, q1, q6
42038765:	a4c002        	addi	a0, a0, -92
42038768:	c0          	.byte	0xc0
42038769:	1b29a7        	blt	a9, a10, 42038788 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
4203876c:	80b3c0        	add	a11, a3, a12
4203876f:	152b87        	blt	a11, a8, 42038788 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038772:	c0bb80        	sub	a11, a11, a8
42038775:	c0ac40        	sub	a10, a12, a4
42038778:	80aa90        	add	a10, a10, a9
4203877b:	23bb80        	sext	a11, a11, 15
4203877e:	23aa80        	sext	a10, a10, 15
42038781:	4dc8a5        	call8	4208640c <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
42038784:	0a2d      	mov.n	a2, a10
42038786:	f01d      	retw.n
42038788:	020fc2        	l8ui	a12, a15, 2
4203878b:	2c9c      	beqz.n	a12, 420387a1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
4203878d:	c0a4c0        	sub	a10, a4, a12
42038790:	0d29a7        	blt	a9, a10, 420387a1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038793:	c0a5c0        	sub	a10, a5, a12
42038796:	0728a7        	blt	a8, a10, 420387a1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038799:	c0bc50        	sub	a11, a12, a5
4203879c:	bb8a      	add.n	a11, a11, a8
4203879e:	fff4c6        	j	42038775 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x59>
420387a1:	030fc2        	l8ui	a12, a15, 3
420387a4:	1c9c      	beqz.n	a12, 420387b9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387a6:	aeca      	add.n	a10, a14, a12
420387a8:	0d2a97        	blt	a10, a9, 420387b9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387ab:	c0b5c0        	sub	a11, a5, a12
420387ae:	0728b7        	blt	a8, a11, 420387b9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387b1:	c0bc50        	sub	a11, a12, a5
420387b4:	bb8a      	add.n	a11, a11, a8
420387b6:	ffe786        	j	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3c>
420387b9:	ea0c      	movi.n	a10, 14
420387bb:	ffa022        	movi	a2, 255
420387be:	c43ad7        	bltu	a10, a13, 42038786 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387c1:	c0e9e0        	sub	a14, a9, a14
420387c4:	40fea0        	nsau	a10, a14
420387c7:	1b0c      	movi.n	a11, 1
420387c9:	41a5a0        	srli	a10, a10, 5
420387cc:	ce9c      	beqz.n	a14, 420387ec <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387ce:	c09940        	sub	a9, a9, a4
420387d1:	83ab90        	moveqz	a10, a11, a9
420387d4:	014a56        	bnez	a10, 420387ec <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387d7:	c03830        	sub	a3, a8, a3
420387da:	40f390        	nsau	a9, a3
420387dd:	419590        	srli	a9, a9, 5
420387e0:	008316        	beqz	a3, 420387ec <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387e3:	c08850        	sub	a8, a8, a5
420387e6:	93b980        	movnez	a11, a9, a8
420387e9:	f99b16        	beqz	a11, 42038786 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387ec:	118dc0        	slli	a8, a13, 4
420387ef:	dd8a      	add.n	a13, a13, a8
420387f1:	7420d0        	extui	a2, a13, 0, 8
420387f4:	ffe386        	j	42038786 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387f7:	020c00        	andb	b0, b12, b0
420387fa:	ffe206        	j	42038786 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038b40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038b40:	008136        	entry	a1, 64
42038b43:	021292        	l16ui	a9, a2, 4
42038b46:	7169      	s32i.n	a6, a1, 28
42038b48:	233380        	sext	a3, a3, 15
42038b4b:	23a980        	sext	a10, a9, 15
42038b4e:	027d      	mov.n	a7, a2
42038b50:	234480        	sext	a4, a4, 15
42038b53:	1a23a7        	blt	a3, a10, 42038b71 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b56:	0412c2        	l16ui	a12, a2, 8
42038b59:	23ac80        	sext	a10, a12, 15
42038b5c:	112a37        	blt	a10, a3, 42038b71 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b5f:	0312b2        	l16ui	a11, a2, 6
42038b62:	23ab80        	sext	a10, a11, 15
42038b65:	0824a7        	blt	a4, a10, 42038b71 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b68:	0512d2        	l16ui	a13, a2, 10
42038b6b:	23ad80        	sext	a10, a13, 15
42038b6e:	12aa47        	bge	a10, a4, 42038b84 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x44>
42038b71:	04dd      	mov.n	a13, a4
42038b73:	03cd      	mov.n	a12, a3
42038b75:	3cc7b2        	addi	a11, a7, 60
42038b78:	28c7a2        	addi	a10, a7, 40
42038b7b:	0053e5        	call8	420390b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b7e:	012a80        	slli	a2, a10, 24
42038b81:	f01d      	retw.n
42038b83:	02a200        	andb	b10, b2, b0
42038b86:	20          	.byte	0x20
42038b87:	016142        	s32i	a4, a1, 4
42038b8a:	006132        	s32i	a3, a1, 0
42038b8d:	c0dda0        	sub	a13, a13, a10
42038b90:	c0cca0        	sub	a12, a12, a10
42038b93:	80bab0        	add	a11, a10, a11
42038b96:	2102f2        	l8ui	a15, a2, 33
42038b99:	aa9a      	add.n	a10, a10, a9
42038b9b:	72e8      	l32i.n	a14, a2, 28
42038b9d:	23dd80        	sext	a13, a13, 15
42038ba0:	23cc80        	sext	a12, a12, 15
42038ba3:	23bb80        	sext	a11, a11, 15
42038ba6:	23aa80        	sext	a10, a10, 15
42038ba9:	ffb725        	call8	4203871c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bac:	ffa092        	movi	a9, 255
42038baf:	0a6d      	mov.n	a6, a10
42038bb1:	079a97        	bne	a10, a9, 42038bbc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x7c>
42038bb4:	142292        	l32i	a9, a2, 80
42038bb7:	052d      	mov.n	a2, a5
42038bb9:	fc4916        	beqz	a9, 42038b81 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038bbc:	04dd      	mov.n	a13, a4
42038bbe:	03cd      	mov.n	a12, a3
42038bc0:	3cc7b2        	addi	a11, a7, 60
42038bc3:	28c7a2        	addi	a10, a7, 40
42038bc6:	004f25        	call8	420390b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038bc9:	200792        	l8ui	a9, a7, 32
42038bcc:	6169      	s32i.n	a6, a1, 24
42038bce:	012a80        	slli	a2, a10, 24
42038bd1:	03e916        	beqz	a9, 42038c13 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038bd4:	1149      	s32i.n	a4, a1, 4
42038bd6:	0139      	s32i.n	a3, a1, 0
42038bd8:	67e8      	l32i.n	a14, a7, 24
42038bda:	0597d2        	l16si	a13, a7, 10
42038bdd:	0497c2        	l16si	a12, a7, 8
42038be0:	0397b2        	l16si	a11, a7, 6
42038be3:	0297a2        	l16si	a10, a7, 4
42038be6:	ff0c      	movi.n	a15, 15
42038be8:	ffb325        	call8	4203871c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038beb:	61a9      	s32i.n	a10, a1, 24
42038bed:	2aac      	beqz.n	a10, 42038c13 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038bef:	7188      	l32i.n	a8, a1, 28
42038bf1:	38cc      	bnez.n	a8, 42038bf8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb8>
42038bf3:	24c782        	addi	a8, a7, 36
42038bf6:	7189      	s32i.n	a8, a1, 28
42038bf8:	7188      	l32i.n	a8, a1, 28
42038bfa:	02ad      	mov.n	a10, a2
42038bfc:	002892        	l32i	a9, a8, 0
42038bff:	062182        	l32i	a8, a1, 24
42038c02:	119980        	slli	a9, a9, 8
42038c05:	419890        	srli	a9, a9, 8
42038c08:	01b880        	slli	a11, a8, 24
42038c0b:	20b9b0        	or	a11, a9, a11
42038c0e:	cec3e5        	call8	4200784c <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c11:	0a2d      	mov.n	a2, a10
42038c13:	ffa092        	movi	a9, 255
42038c16:	469697        	bne	a6, a9, 42038c60 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x120>
42038c19:	fb7c      	movi.n	a11, -1
42038c1b:	01bb80        	slli	a11, a11, 24
42038c1e:	20b5b0        	or	a11, a5, a11
42038c21:	02ad      	mov.n	a10, a2
42038c23:	cec2a5        	call8	4200784c <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c26:	0a2d      	mov.n	a2, a10
42038c28:	1427a2        	l32i	a10, a7, 80
42038c2b:	f52a16        	beqz	a10, 42038b81 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c2e:	20c440        	or	a12, a4, a4
42038c31:	20b330        	or	a11, a3, a3
42038c34:	ffdd65        	call8	42038a0c <roo_windows::PressOverlay::get(short, short) const>
42038c37:	6198      	l32i.n	a9, a1, 24
42038c39:	7588a0        	extui	a8, a10, 24, 8
42038c3c:	d18890        	mul16s	a8, a8, a9
42038c3f:	1f2891        	l32r	a9, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c42:	11aa80        	slli	a10, a10, 8
42038c45:	a28890        	muluh	a8, a8, a9
42038c48:	0188f0        	slli	a8, a8, 17
42038c4b:	758880        	extui	a8, a8, 24, 8
42038c4e:	404800        	ssai	8
42038c51:	81b8a0        	src	a11, a8, a10
42038c54:	02ad      	mov.n	a10, a2
42038c56:	cebf65        	call8	4200784c <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c59:	0a2d      	mov.n	a2, a10
42038c5b:	ffc886        	j	42038b81 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c5e:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038c61:	50fc46        	j	4204d056 <uart_set_stop_bits+0xe>
42038c64:	7598      	l32i.n	a9, a5, 28
42038c66:	d19960        	mul16s	a9, a9, a6
42038c69:	1f1da1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c6c:	7fc992        	addi	a9, a9, 127
42038c6f:	a299a0        	muluh	a9, a9, a10
42038c72:	11b580        	slli	a11, a5, 8
42038c75:	0199f0        	slli	a9, a9, 17
42038c78:	759890        	extui	a9, a9, 24, 8
42038c7b:	404800        	ssai	8
42038c7e:	81b9b0        	src	a11, a9, a11
42038c81:	ffe706        	j	42038c21 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xe1>


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038cc4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038cc4:	00c136        	entry	a1, 96
42038cc7:	238680        	sext	a8, a6, 15
42038cca:	1189      	s32i.n	a8, a1, 4
42038ccc:	142282        	l32i	a8, a2, 80
42038ccf:	07fd      	mov.n	a15, a7
42038cd1:	027d      	mov.n	a7, a2
42038cd3:	233380        	sext	a3, a3, 15
42038cd6:	234480        	sext	a4, a4, 15
42038cd9:	235580        	sext	a5, a5, 15
42038cdc:	204856        	bnez	a8, 42038ee4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038cdf:	0288      	l32i.n	a8, a2, 0
42038ce1:	11e8      	l32i.n	a14, a1, 4
42038ce3:	b888      	l32i.n	a8, a8, 44
42038ce5:	02ad      	mov.n	a10, a2
42038ce7:	a1f9      	s32i.n	a15, a1, 40
42038ce9:	05dd      	mov.n	a13, a5
42038ceb:	04cd      	mov.n	a12, a4
42038ced:	03bd      	mov.n	a11, a3
42038cef:	0008e0        	callx8	a8
42038cf2:	0a2d      	mov.n	a2, a10
42038cf4:	098a56        	bnez	a10, 42038d90 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcc>
42038cf7:	1a9792        	l16si	a9, a7, 52
42038cfa:	249782        	l16si	a8, a7, 72
42038cfd:	ec0c      	movi.n	a12, 14
42038cff:	538890        	max	a8, a8, a9
42038d02:	98fb      	addi.n	a9, a8, 15
42038d04:	1ec882        	addi	a8, a8, 30
42038d07:	b38990        	movgez	a8, a9, a9
42038d0a:	21b480        	srai	a11, a8, 4
42038d0d:	210782        	l8ui	a8, a7, 33
42038d10:	1907a2        	l8ui	a10, a7, 25
42038d13:	62cc80        	saltu	a12, a12, a8
42038d16:	180782        	l8ui	a8, a7, 24
42038d19:	200792        	l8ui	a9, a7, 32
42038d1c:	7388a0        	maxu	a8, a8, a10
42038d1f:	1a07a2        	l8ui	a10, a7, 26
42038d22:	cc0b      	addi.n	a12, a12, -1
42038d24:	7388a0        	maxu	a8, a8, a10
42038d27:	1b07a2        	l8ui	a10, a7, 27
42038d2a:	60c0c0        	neg	a12, a12
42038d2d:	73a8a0        	maxu	a10, a8, a10
42038d30:	89ca      	add.n	a8, a9, a12
42038d32:	99ba      	add.n	a9, a9, a11
42038d34:	538a80        	max	a8, a10, a8
42038d37:	029762        	l16si	a6, a7, 4
42038d3a:	0497a2        	l16si	a10, a7, 8
42038d3d:	99ca      	add.n	a9, a9, a12
42038d3f:	539980        	max	a9, a9, a8
42038d42:	669a      	add.n	a6, a6, a9
42038d44:	c09a90        	sub	a9, a10, a9
42038d47:	439950        	min	a9, a9, a5
42038d4a:	3199      	s32i.n	a9, a1, 12
42038d4c:	039792        	l16si	a9, a7, 6
42038d4f:	536630        	max	a6, a6, a3
42038d52:	998a      	add.n	a9, a9, a8
42038d54:	99ba      	add.n	a9, a9, a11
42038d56:	7199      	s32i.n	a9, a1, 28
42038d58:	059792        	l16si	a9, a7, 10
42038d5b:	a1f8      	l32i.n	a15, a1, 40
42038d5d:	c08980        	sub	a8, a9, a8
42038d60:	2189      	s32i.n	a8, a1, 8
42038d62:	c08530        	sub	a8, a5, a3
42038d65:	881b      	addi.n	a8, a8, 1
42038d67:	4189      	s32i.n	a8, a1, 16
42038d69:	3188      	l32i.n	a8, a1, 12
42038d6b:	881b      	addi.n	a8, a8, 1
42038d6d:	5189      	s32i.n	a8, a1, 20
42038d6f:	c08860        	sub	a8, a8, a6
42038d72:	1188e0        	slli	a8, a8, 2
42038d75:	8189      	s32i.n	a8, a1, 32
42038d77:	4188      	l32i.n	a8, a1, 16
42038d79:	1128e0        	slli	a2, a8, 2
42038d7c:	c08630        	sub	a8, a6, a3
42038d7f:	1188e0        	slli	a8, a8, 2
42038d82:	9189      	s32i.n	a8, a1, 36
42038d84:	238680        	sext	a8, a6, 15
42038d87:	6189      	s32i.n	a8, a1, 24
42038d89:	1188      	l32i.n	a8, a1, 4
42038d8b:	03a847        	bge	a8, a4, 42038d92 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xce>
42038d8e:	020c      	movi.n	a2, 0
42038d90:	f01d      	retw.n
42038d92:	31a8      	l32i.n	a10, a1, 12
42038d94:	038d      	mov.n	a8, a3
42038d96:	0f9d      	mov.n	a9, a15
42038d98:	2b2a67        	blt	a10, a6, 42038dc7 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x103>
42038d9b:	0f8d      	mov.n	a8, a15
42038d9d:	03bd      	mov.n	a11, a3
42038d9f:	23c480        	sext	a12, a4, 15
42038da2:	02ab67        	bge	a11, a6, 42038da8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe4>
42038da5:	0024c6        	j	42038e3c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x178>
42038da8:	9188      	l32i.n	a8, a1, 36
42038daa:	61b8      	l32i.n	a11, a1, 24
42038dac:	9f8a      	add.n	a9, a15, a8
42038dae:	07ad      	mov.n	a10, a7
42038db0:	b1f9      	s32i.n	a15, a1, 44
42038db2:	a199      	s32i.n	a9, a1, 40
42038db4:	ffece5        	call8	42038c84 <roo_windows::Decoration::read(short, short) const>
42038db7:	8188      	l32i.n	a8, a1, 32
42038db9:	a198      	l32i.n	a9, a1, 40
42038dbb:	b1f8      	l32i.n	a15, a1, 44
42038dbd:	898a      	add.n	a8, a9, a8
42038dbf:	021897        	beq	a8, a9, 42038dc5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x101>
42038dc2:	0025c6        	j	42038e5d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x199>
42038dc5:	5188      	l32i.n	a8, a1, 20
42038dc7:	09dd      	mov.n	a13, a9
42038dc9:	08bd      	mov.n	a11, a8
42038dcb:	23c480        	sext	a12, a4, 15
42038dce:	0225b7        	blt	a5, a11, 42038dd4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x110>
42038dd1:	0023c6        	j	42038e64 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a0>
42038dd4:	b51b      	addi.n	a11, a5, 1
42038dd6:	72ab80        	salt	a10, a11, a8
42038dd9:	aa0b      	addi.n	a10, a10, -1
42038ddb:	60a0a0        	neg	a10, a10
42038dde:	74a0a0        	extui	a10, a10, 0, 8
42038de1:	0c0c      	movi.n	a12, 0
42038de3:	4a8c      	beqz.n	a10, 42038deb <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x127>
42038de5:	c0bb80        	sub	a11, a11, a8
42038de8:	11cbe0        	slli	a12, a11, 2
42038deb:	71a8      	l32i.n	a10, a1, 28
42038ded:	99ca      	add.n	a9, a9, a12
42038def:	72a4a0        	salt	a10, a4, a10
42038df2:	aa0b      	addi.n	a10, a10, -1
42038df4:	60a0a0        	neg	a10, a10
42038df7:	74a0a0        	extui	a10, a10, 0, 8
42038dfa:	098d      	mov.n	a8, a9
42038dfc:	4abc      	beqz.n	a10, 42038e34 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038dfe:	21a8      	l32i.n	a10, a1, 8
42038e00:	72aa40        	salt	a10, a10, a4
42038e03:	aa0b      	addi.n	a10, a10, -1
42038e05:	60a0a0        	neg	a10, a10
42038e08:	74a0a0        	extui	a10, a10, 0, 8
42038e0b:	5aac      	beqz.n	a10, 42038e34 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038e0d:	11a8      	l32i.n	a10, a1, 4
42038e0f:	21b8      	l32i.n	a11, a1, 8
42038e11:	04ed      	mov.n	a14, a4
42038e13:	43dab0        	min	a13, a10, a11
42038e16:	742ed7        	blt	a14, a13, 42038e8e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038e19:	728d40        	salt	a8, a13, a4
42038e1c:	880b      	addi.n	a8, a8, -1
42038e1e:	608080        	neg	a8, a8
42038e21:	748080        	extui	a8, a8, 0, 8
42038e24:	00a0a2        	movi	a10, 0
42038e27:	002816        	beqz	a8, 42038e2d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x169>
42038e2a:	c0ad40        	sub	a10, a13, a4
42038e2d:	828a20        	mull	a8, a10, a2
42038e30:	4a4a      	add.n	a4, a10, a4
42038e32:	889a      	add.n	a8, a8, a9
42038e34:	441b      	addi.n	a4, a4, 1
42038e36:	08fd      	mov.n	a15, a8
42038e38:	ffd346        	j	42038d89 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xc5>
42038e3b:	a77000        	ee.stf.64.xp	f7, f10, a0, a0
42038e3e:	61f220        	xsr.ccompare2	a2
42038e41:	0d          	.byte	0xd
42038e42:	0c6182        	s32i	a8, a1, 48
42038e45:	b1c9      	s32i.n	a12, a1, 44
42038e47:	a1b9      	s32i.n	a11, a1, 40
42038e49:	ffe3a5        	call8	42038c84 <roo_windows::Decoration::read(short, short) const>
42038e4c:	c188      	l32i.n	a8, a1, 48
42038e4e:	a1b8      	l32i.n	a11, a1, 40
42038e50:	08a9      	s32i.n	a10, a8, 0
42038e52:	d1f8      	l32i.n	a15, a1, 52
42038e54:	b1c8      	l32i.n	a12, a1, 44
42038e56:	bb1b      	addi.n	a11, a11, 1
42038e58:	884b      	addi.n	a8, a8, 4
42038e5a:	ffd106        	j	42038da2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xde>
42038e5d:	09a9      	s32i.n	a10, a9, 0
42038e5f:	994b      	addi.n	a9, a9, 4
42038e61:	ffd686        	j	42038dbf <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xfb>
42038e64:	20a770        	or	a10, a7, a7
42038e67:	0f61f2        	s32i	a15, a1, 60
42038e6a:	0e61d2        	s32i	a13, a1, 56
42038e6d:	d189      	s32i.n	a8, a1, 52
42038e6f:	c199      	s32i.n	a9, a1, 48
42038e71:	b1c9      	s32i.n	a12, a1, 44
42038e73:	a1b9      	s32i.n	a11, a1, 40
42038e75:	ffe0e5        	call8	42038c84 <roo_windows::Decoration::read(short, short) const>
42038e78:	e1d8      	l32i.n	a13, a1, 56
42038e7a:	a1b8      	l32i.n	a11, a1, 40
42038e7c:	0da9      	s32i.n	a10, a13, 0
42038e7e:	f1f8      	l32i.n	a15, a1, 60
42038e80:	d188      	l32i.n	a8, a1, 52
42038e82:	c198      	l32i.n	a9, a1, 48
42038e84:	b1c8      	l32i.n	a12, a1, 44
42038e86:	bb1b      	addi.n	a11, a11, 1
42038e88:	dd4b      	addi.n	a13, a13, 4
42038e8a:	ffd006        	j	42038dce <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10a>
42038e8d:	41a800        	srli	a10, a0, 8
42038e90:	201aa6        	blti	a10, 1, 42038eb4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038e93:	254226        	beqi	a2, 4, 42038ebc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f8>
42038e96:	0fbd      	mov.n	a11, a15
42038e98:	08ad      	mov.n	a10, a8
42038e9a:	02cd      	mov.n	a12, a2
42038e9c:	d199      	s32i.n	a9, a1, 52
42038e9e:	c1e9      	s32i.n	a14, a1, 48
42038ea0:	b1d9      	s32i.n	a13, a1, 44
42038ea2:	a1f9      	s32i.n	a15, a1, 40
42038ea4:	1e9081        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038ea7:	0008e0        	callx8	a8
42038eaa:	a1f8      	l32i.n	a15, a1, 40
42038eac:	b1d8      	l32i.n	a13, a1, 44
42038eae:	c1e8      	l32i.n	a14, a1, 48
42038eb0:	d198      	l32i.n	a9, a1, 52
42038eb2:	0a8d      	mov.n	a8, a10
42038eb4:	882a      	add.n	a8, a8, a2
42038eb6:	ee1b      	addi.n	a14, a14, 1
42038eb8:	ffd686        	j	42038e16 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x152>
42038ebb:	00          	.byte	00
42038ebc:	0fa8      	l32i.n	a10, a15, 0
42038ebe:	08a9      	s32i.n	a10, a8, 0
42038ec0:	fffc06        	j	42038eb4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038ec3:	600000        	neg	a0, a0
42038ec6:	4020b6        	bltui	a0, 2, 42038f0a <roo_windows::internal::MakeLookupShadow(roo_display::Box, roo_windows::BorderStyle::CornerRadii, int, bool)+0x16>
42038ec9:	c4          	.byte	0xc4
42038eca:	a77020        	ee.stf.64.xp	f7, f10, a2, a0
42038ecd:	db6520        	movt.s	f6, f5, b2
42038ed0:	a9661bff 	ee.fft.cmul.s16.st.xp	q4, q6, q6, a15, a11, 2, 2, 0
42038ed4:	02          	.byte	0x2
42038ed5:	236680        	sext	a6, a6, 15
42038ed8:	224b      	addi.n	a2, a2, 4
42038eda:	02fd      	mov.n	a15, a2
42038edc:	e5a567        	bge	a5, a6, 42038ec5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x201>
42038edf:	441b      	addi.n	a4, a4, 1
42038ee1:	234480        	sext	a4, a4, 15
42038ee4:	1188      	l32i.n	a8, a1, 4
42038ee6:	02a847        	bge	a8, a4, 42038eec <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038ee9:	ffa846        	j	42038d8e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xca>
42038eec:	0f2d      	mov.n	a2, a15
42038eee:	036d      	mov.n	a6, a3
42038ef0:	fff986        	j	42038eda <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/exterior_first/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420390b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
420390b8:	00a136        	entry	a1, 80
420390bb:	238480        	sext	a8, a4, 15
420390be:	027d      	mov.n	a7, a2
420390c0:	a189      	s32i.n	a8, a1, 40
420390c2:	0f0222        	l8ui	a2, a2, 15
420390c5:	238580        	sext	a8, a5, 15
420390c8:	b189      	s32i.n	a8, a1, 44
420390ca:	243216        	beqz	a2, 42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
420390cd:	009782        	l16si	a8, a7, 0
420390d0:	0697f2        	l16si	a15, a7, 12
420390d3:	3189      	s32i.n	a8, a1, 12
420390d5:	3198      	l32i.n	a9, a1, 12
420390d7:	a188      	l32i.n	a8, a1, 40
420390d9:	c08890        	sub	a8, a8, a9
420390dc:	11c8c0        	slli	a12, a8, 4
420390df:	4189      	s32i.n	a8, a1, 16
420390e1:	019782        	l16si	a8, a7, 2
420390e4:	cc8b      	addi.n	a12, a12, 8
420390e6:	5189      	s32i.n	a8, a1, 20
420390e8:	5198      	l32i.n	a9, a1, 20
420390ea:	b188      	l32i.n	a8, a1, 44
420390ec:	c08890        	sub	a8, a8, a9
420390ef:	6189      	s32i.n	a8, a1, 24
420390f1:	069392        	l16si	a9, a3, 12
420390f4:	1188c0        	slli	a8, a8, 4
420390f7:	888b      	addi.n	a8, a8, 8
420390f9:	c0f8f0        	sub	a15, a8, a15
420390fc:	c08890        	sub	a8, a8, a9
420390ff:	9cca      	add.n	a9, a12, a12
42039101:	7199      	s32i.n	a9, a1, 28
42039103:	029792        	l16si	a9, a7, 4
42039106:	d88a      	add.n	a13, a8, a8
42039108:	8199      	s32i.n	a9, a1, 32
4203910a:	1199c0        	slli	a9, a9, 4
4203910d:	2199      	s32i.n	a9, a1, 8
4203910f:	039792        	l16si	a9, a7, 6
42039112:	2168      	l32i.n	a6, a1, 8
42039114:	11b9c0        	slli	a11, a9, 4
42039117:	effa      	add.n	a14, a15, a15
42039119:	c04b80        	sub	a4, a11, a8
4203911c:	9199      	s32i.n	a9, a1, 36
4203911e:	72adb0        	salt	a10, a13, a11
42039121:	729eb0        	salt	a9, a14, a11
42039124:	c056c0        	sub	a5, a6, a12
42039127:	990b      	addi.n	a9, a9, -1
42039129:	aa0b      	addi.n	a10, a10, -1
4203912b:	434480        	min	a4, a4, a8
4203912e:	7188      	l32i.n	a8, a1, 28
42039130:	609090        	neg	a9, a9
42039133:	60a0a0        	neg	a10, a10
42039136:	4355c0        	min	a5, a5, a12
42039139:	c0cbf0        	sub	a12, a11, a15
4203913c:	749090        	extui	a9, a9, 0, 8
4203913f:	74a0a0        	extui	a10, a10, 0, 8
42039142:	43ccf0        	min	a12, a12, a15
42039145:	732867        	blt	a8, a6, 420391bc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x104>
42039148:	682eb7        	blt	a14, a11, 420391b4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xfc>
4203914b:	0a0782        	l8ui	a8, a7, 10
4203914e:	7319a7        	beq	a9, a10, 420391c5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10d>
42039151:	7f2db7        	blt	a13, a11, 420391d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11c>
42039154:	0a0762        	l8ui	a6, a7, 10
42039157:	23d880        	sext	a13, a8, 15
4203915a:	11ddc0        	slli	a13, a13, 4
4203915d:	02acd7        	bge	a12, a13, 42039163 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
42039160:	002206        	j	420391ec <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
42039163:	23f680        	sext	a15, a6, 15
42039166:	11ffc0        	slli	a15, a15, 4
42039169:	7f24f7        	blt	a4, a15, 420391ec <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
4203916c:	43fc40        	min	a15, a12, a4
4203916f:	02af57        	bge	a15, a5, 42039175 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xbd>
42039172:	006f86        	j	42039334 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27c>
42039175:	a198      	l32i.n	a9, a1, 40
42039177:	3188      	l32i.n	a8, a1, 12
42039179:	21a8      	l32i.n	a10, a1, 8
4203917b:	c08890        	sub	a8, a8, a9
4203917e:	7198      	l32i.n	a9, a1, 28
42039180:	0829a7        	blt	a9, a10, 4203918c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd4>
42039183:	4188      	l32i.n	a8, a1, 16
42039185:	8198      	l32i.n	a9, a1, 32
42039187:	c08890        	sub	a8, a8, a9
4203918a:	881b      	addi.n	a8, a8, 1
4203918c:	096c      	movi.n	a9, -32
4203918e:	02a897        	bge	a8, a9, 42039194 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdc>
42039191:	0064c6        	j	42039328 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
42039194:	092c      	movi.n	a9, 32
42039196:	02a987        	bge	a9, a8, 4203919c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
42039199:	006446        	j	4203932e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
4203919c:	220b      	addi.n	a2, a2, -1
4203919e:	902220        	addx2	a2, a2, a2
420391a1:	1192a0        	slli	a9, a2, 6
420391a4:	229a      	add.n	a2, a2, a9
420391a6:	1dd391        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420391a9:	898a      	add.n	a8, a9, a8
420391ab:	882a      	add.n	a8, a8, a2
420391ad:	200822        	l8ui	a2, a8, 32
420391b0:	005746        	j	42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
420391b3:	078200        	ee.stf.64.xp	f8, f0, a0, a2
420391b6:	c609      	s32i.n	a0, a6, 48
420391b8:	e4          	.byte	0xe4
420391b9:	b70000ff 	ee.vmulas.s16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
420391bd:	07820c2e 	ee.vmulas.s16.accx.ld.ip.qup	q6, a2, 0x1c0, q0, q1, q0, q2
420391c1:	a70b      	addi.n	a10, a7, -1
420391c3:	1699      	s32i.n	a9, a6, 4
420391c5:	086d      	mov.n	a6, a8
420391c7:	ffe306        	j	42039157 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
420391ca:	820000        	mull	a0, a0, a0
420391cd:	c60807        	bnone	a8, a0, 42039197 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdf>
420391d0:	fffb      	addi.n	a15, a15, 15
420391d2:	620000        	saltu	a0, a0, a0
420391d5:	060907        	bnone	a9, a0, 420391df <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x127>
420391d8:	0000ffdf 	ee.vmulas.s16.accx.ld.ip.qup	q0, a13, 240, q7, q6, q0, q0
420391dc:	052db7        	blt	a13, a11, 420391e5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12d>
420391df:	0b0762        	l8ui	a6, a7, 11
420391e2:	ffdc46        	j	42039157 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
420391e5:	080762        	l8ui	a6, a7, 8
420391e8:	ffdac6        	j	42039157 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
420391eb:	00          	.byte	00
420391ec:	5199a7        	bne	a9, a10, 42039241 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
420391ef:	538680        	max	a8, a6, a8
420391f2:	238880        	sext	a8, a8, 15
420391f5:	1188c0        	slli	a8, a8, 4
420391f8:	452587        	blt	a5, a8, 42039241 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
420391fb:	538c40        	max	a8, a12, a4
420391fe:	3f2587        	blt	a5, a8, 42039241 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
42039201:	9198      	l32i.n	a9, a1, 36
42039203:	6188      	l32i.n	a8, a1, 24
42039205:	c08890        	sub	a8, a8, a9
42039208:	881b      	addi.n	a8, a8, 1
4203920a:	290c      	movi.n	a9, 2
4203920c:	08aeb7        	bge	a14, a11, 42039218 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x160>
4203920f:	b198      	l32i.n	a9, a1, 44
42039211:	5188      	l32i.n	a8, a1, 20
42039213:	c08890        	sub	a8, a8, a9
42039216:	190c      	movi.n	a9, 1
42039218:	0a6c      	movi.n	a10, -32
4203921a:	02a8a7        	bge	a8, a10, 42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x168>
4203921d:	0041c6        	j	42039328 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
42039220:	0a2c      	movi.n	a10, 32
42039222:	02aa87        	bge	a10, a8, 42039228 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x170>
42039225:	004146        	j	4203932e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
42039228:	220b      	addi.n	a2, a2, -1
4203922a:	902220        	addx2	a2, a2, a2
4203922d:	229a      	add.n	a2, a2, a9
4203922f:	1db191        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
42039232:	11a2a0        	slli	a10, a2, 6
42039235:	aa2a      	add.n	a10, a10, a2
42039237:	898a      	add.n	a8, a9, a8
42039239:	88aa      	add.n	a8, a8, a10
4203923b:	ffdb86        	j	420391ad <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf5>
4203923e:	000000        	ill
42039241:	091782        	l16ui	a8, a7, 18
42039244:	4395c0        	min	a9, a5, a12
42039247:	608080        	neg	a8, a8
4203924a:	00a022        	movi	a2, 0
4203924d:	4e2987        	blt	a9, a8, 4203929f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1e7>
42039250:	c0ad50        	sub	a10, a13, a5
42039253:	090c      	movi.n	a9, 0
42039255:	c08dc0        	sub	a8, a13, a12
42039258:	0229a7        	blt	a9, a10, 4203925e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a6>
4203925b:	002d46        	j	42039314 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
4203925e:	022987        	blt	a9, a8, 42039264 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ac>
42039261:	002bc6        	j	42039314 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
42039264:	828880        	mull	a8, a8, a8
42039267:	82aaa0        	mull	a10, a10, a10
4203926a:	21d9      	s32i.n	a13, a1, 8
4203926c:	80aa80        	add	a10, a10, a8
4203926f:	4d33e5        	call8	420865ac <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039272:	0221d2        	l32i	a13, a1, 8
42039275:	0e0792        	l8ui	a9, a7, 14
42039278:	c08ad0        	sub	a8, a10, a13
4203927b:	c08890        	sub	a8, a8, a9
4203927e:	081792        	l16ui	a9, a7, 16
42039281:	828890        	mull	a8, a8, a9
42039284:	1d9e91        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
42039287:	889a      	add.n	a8, a8, a9
42039289:	218c80        	srai	a8, a8, 12
4203928c:	ffa092        	movi	a9, 255
4203928f:	438890        	min	a8, a8, a9
42039292:	090c      	movi.n	a9, 0
42039294:	538890        	max	a8, a8, a9
42039297:	1d9891        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
4203929a:	898a      	add.n	a8, a9, a8
4203929c:	000822        	l8ui	a2, a8, 0
4203929f:	091392        	l16ui	a9, a3, 18
420392a2:	43a540        	min	a10, a5, a4
420392a5:	609090        	neg	a9, a9
420392a8:	00a082        	movi	a8, 0
420392ab:	492a97        	blt	a10, a9, 420392f8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
420392ae:	236680        	sext	a6, a6, 15
420392b1:	1166c0        	slli	a6, a6, 4
420392b4:	c09640        	sub	a9, a6, a4
420392b7:	c0a650        	sub	a10, a6, a5
420392ba:	62a897        	bge	a8, a9, 42039320 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
420392bd:	5fa8a7        	bge	a8, a10, 42039320 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
420392c0:	829990        	mull	a9, a9, a9
420392c3:	82aaa0        	mull	a10, a10, a10
420392c6:	80a9a0        	add	a10, a9, a10
420392c9:	4d2e25        	call8	420865ac <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
420392cc:	0e0392        	l8ui	a9, a3, 14
420392cf:	c08a60        	sub	a8, a10, a6
420392d2:	c08890        	sub	a8, a8, a9
420392d5:	081392        	l16ui	a9, a3, 16
420392d8:	828890        	mull	a8, a8, a9
420392db:	1d8891        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
420392de:	808890        	add	a8, a8, a9
420392e1:	218c80        	srai	a8, a8, 12
420392e4:	ffa092        	movi	a9, 255
420392e7:	438890        	min	a8, a8, a9
420392ea:	090c      	movi.n	a9, 0
420392ec:	538890        	max	a8, a8, a9
420392ef:	1d8491        	l32r	a9, 42000900 <_stext+0x8e0> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
420392f2:	808980        	add	a8, a9, a8
420392f5:	000882        	l8ui	a8, a8, 0
420392f8:	d19280        	mul16s	a9, a2, a8
420392fb:	1d79a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
420392fe:	7fc992        	addi	a9, a9, 127
42039301:	b2a9a0        	mulsh	a10, a9, a10
42039304:	282a      	add.n	a2, a8, a2
42039306:	99aa      	add.n	a9, a9, a10
42039308:	219790        	srai	a9, a9, 7
4203930b:	c02290        	sub	a2, a2, a9
4203930e:	742020        	extui	a2, a2, 0, 8
42039311:	f01d      	retw.n
42039313:	00          	.byte	00
42039314:	022c57        	blt	a12, a5, 4203931a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x262>
42039317:	ffd686        	j	42039275 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
4203931a:	08ad      	mov.n	a10, a8
4203931c:	ffd546        	j	42039275 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
4203931f:	a45700        	extui	a5, a0, 7, 11
42039322:	ada8      	l32i.n	a10, a13, 40
42039324:	c609      	s32i.n	a0, a6, 48
42039326:	ffe8      	l32i.n	a14, a15, 60
42039328:	68a022        	movi	a2, 104
4203932b:	fff886        	j	42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
4203932e:	020c      	movi.n	a2, 0
42039330:	fff746        	j	42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
42039333:	00          	.byte	00
42039334:	0219a7        	beq	a9, a10, 4203933a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x282>
42039337:	ffc546        	j	42039250 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x198>
4203933a:	ffac46        	j	420391ef <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x137>
