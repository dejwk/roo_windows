
/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


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


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42086568 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
42086568:	004136        	entry	a1, 32
4208656b:	029d      	mov.n	a9, a2
4208656d:	120c      	movi.n	a2, 1
4208656f:	832990        	moveqz	a2, a9, a9
42086572:	4149b6        	bltui	a9, 4, 420865b7 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
42086575:	40f980        	nsau	a8, a9
42086578:	dfc882        	addi	a8, a8, -33
4208657b:	608080        	neg	a8, a8
4208657e:	218180        	srai	a8, a8, 1
42086581:	fec882        	addi	a8, a8, -2
42086584:	3a0c      	movi.n	a10, 3
42086586:	401800        	ssl	a8
42086589:	a1aa00        	sll	a10, a10
4208658c:	c289a0        	quou	a8, a9, a10
4208658f:	88aa      	add.n	a8, a8, a10
42086591:	418180        	srli	a8, a8, 1
42086594:	c2a980        	quou	a10, a9, a8
42086597:	aa8a      	add.n	a10, a10, a8
42086599:	41a1a0        	srli	a10, a10, 1
4208659c:	c289a0        	quou	a8, a9, a10
4208659f:	88aa      	add.n	a8, a8, a10
420865a1:	1a0c      	movi.n	a10, 1
420865a3:	418180        	srli	a8, a8, 1
420865a6:	11aa00        	slli	a10, a10, 16
420865a9:	0518a7        	beq	a8, a10, 420865b2 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
420865ac:	82a880        	mull	a10, a8, a8
420865af:	01b9a7        	bgeu	a9, a10, 420865b4 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
420865b2:	880b      	addi.n	a8, a8, -1
420865b4:	f42080        	extui	a2, a8, 0, 16
420865b7:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c7c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c7c:	006136        	entry	a1, 48
42038c7f:	038d      	mov.n	a8, a3
42038c81:	059d      	mov.n	a9, a5
42038c83:	05fd      	mov.n	a15, a5
42038c85:	063d      	mov.n	a3, a6
42038c87:	045d      	mov.n	a5, a4
42038c89:	087d      	mov.n	a7, a8
42038c8b:	1d4ff6        	bgeui	a15, 4, 42038cac <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x30>
42038c8e:	41a290        	srli	a10, a9, 2
42038c91:	11bac0        	slli	a11, a10, 4
42038c94:	66ba      	add.n	a6, a6, a11
42038c96:	b0ba80        	addx8	a11, a10, a8
42038c99:	11aae0        	slli	a10, a10, 2
42038c9c:	c099a0        	sub	a9, a9, a10
42038c9f:	a07960        	addx4	a7, a9, a6
42038ca2:	c04480        	sub	a4, a4, a8
42038ca5:	8b4a      	add.n	a8, a11, a4
42038ca7:	5e9677        	bne	a6, a7, 42038d09 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8d>
42038caa:	f01d      	retw.n
42038cac:	0095c2        	l16si	a12, a5, 0
42038caf:	0097b2        	l16si	a11, a7, 0
42038cb2:	52d8      	l32i.n	a13, a2, 20
42038cb4:	0e0c      	movi.n	a14, 0
42038cb6:	02ad      	mov.n	a10, a2
42038cb8:	3199      	s32i.n	a9, a1, 12
42038cba:	2189      	s32i.n	a8, a1, 8
42038cbc:	11f9      	s32i.n	a15, a1, 4
42038cbe:	ffe7a5        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cc1:	03a9      	s32i.n	a10, a3, 0
42038cc3:	0195c2        	l16si	a12, a5, 2
42038cc6:	0197b2        	l16si	a11, a7, 2
42038cc9:	52d8      	l32i.n	a13, a2, 20
42038ccb:	0e0c      	movi.n	a14, 0
42038ccd:	02ad      	mov.n	a10, a2
42038ccf:	ffe6a5        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cd2:	13a9      	s32i.n	a10, a3, 4
42038cd4:	0295c2        	l16si	a12, a5, 4
42038cd7:	0297b2        	l16si	a11, a7, 4
42038cda:	52d8      	l32i.n	a13, a2, 20
42038cdc:	0e0c      	movi.n	a14, 0
42038cde:	02ad      	mov.n	a10, a2
42038ce0:	ffe565        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ce3:	23a9      	s32i.n	a10, a3, 8
42038ce5:	0395c2        	l16si	a12, a5, 6
42038ce8:	0397b2        	l16si	a11, a7, 6
42038ceb:	52d8      	l32i.n	a13, a2, 20
42038ced:	0e0c      	movi.n	a14, 0
42038cef:	02ad      	mov.n	a10, a2
42038cf1:	ffe465        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cf4:	11f8      	l32i.n	a15, a1, 4
42038cf6:	33a9      	s32i.n	a10, a3, 12
42038cf8:	3198      	l32i.n	a9, a1, 12
42038cfa:	2188      	l32i.n	a8, a1, 8
42038cfc:	778b      	addi.n	a7, a7, 8
42038cfe:	558b      	addi.n	a5, a5, 8
42038d00:	10c332        	addi	a3, a3, 16
42038d03:	fccff2        	addi	a15, a15, -4
42038d06:	ffe046        	j	42038c8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xf>
42038d09:	02cb52        	addi	a5, a11, 2
42038d0c:	0522d2        	l32i	a13, a2, 20
42038d0f:	009bb2        	l16si	a11, a11, 0
42038d12:	0098c2        	l16si	a12, a8, 0
42038d15:	00a0e2        	movi	a14, 0
42038d18:	02ad      	mov.n	a10, a2
42038d1a:	ffe1e5        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038d1d:	06a9      	s32i.n	a10, a6, 0
42038d1f:	05bd      	mov.n	a11, a5
42038d21:	664b      	addi.n	a6, a6, 4
42038d23:	ffdf86        	j	42038ca5 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x29>


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420863b4 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
420863b4:	004136        	entry	a1, 32
420863b7:	208230        	or	a8, a2, a3
420863ba:	0228d6        	bgez	a8, 420863e0 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
420863bd:	c02240        	sub	a2, a2, a4
420863c0:	40f290        	nsau	a9, a2
420863c3:	419590        	srli	a9, a9, 5
420863c6:	b28c      	beqz.n	a2, 420863d5 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
420863c8:	c03340        	sub	a3, a3, a4
420863cb:	180c      	movi.n	a8, 1
420863cd:	938930        	movnez	a8, a9, a3
420863d0:	ffa022        	movi	a2, 255
420863d3:	688c      	beqz.n	a8, 420863dd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863d5:	1125c0        	slli	a2, a5, 4
420863d8:	252a      	add.n	a2, a5, a2
420863da:	742020        	extui	a2, a2, 0, 8
420863dd:	f01d      	retw.n
420863df:	222000        	orb	b2, b0, b0
420863e2:	3330c1        	l32r	a12, 420530a4 <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
420863e5:	44c0c1        	l32r	a12, 420576e8 <sdmmc_host_init_slot+0x4cc> (8e091e4 <UserFrameTotalSize+0x8e090e4>)
420863e8:	545a11        	l32r	a1, 4205b550 <esp_core_dump_write+0xd8> (900d0d0c <_rtc_reserved_end+0x2ffd0d0c>)
420863eb:	823a      	add.n	a8, a2, a3
420863ed:	c19550        	mul16u	a9, a5, a5
420863f0:	118880        	slli	a8, a8, 8
420863f3:	020c      	movi.n	a2, 0
420863f5:	e4b897        	bgeu	a8, a9, 420863dd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420863f8:	f1c592        	addi	a9, a5, -15
420863fb:	c19990        	mul16u	a9, a9, a9
420863fe:	ffa022        	movi	a2, 255
42086401:	d83897        	bltu	a8, a9, 420863dd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086404:	40f890        	nsau	a9, a8
42086407:	dfc992        	addi	a9, a9, -33
4208640a:	609090        	neg	a9, a9
4208640d:	219190        	srai	a9, a9, 1
42086410:	fec992        	addi	a9, a9, -2
42086413:	3a0c      	movi.n	a10, 3
42086415:	401900        	ssl	a9
42086418:	a1aa00        	sll	a10, a10
4208641b:	c298a0        	quou	a9, a8, a10
4208641e:	99aa      	add.n	a9, a9, a10
42086420:	419190        	srli	a9, a9, 1
42086423:	c2a890        	quou	a10, a8, a9
42086426:	aa9a      	add.n	a10, a10, a9
42086428:	41a1a0        	srli	a10, a10, 1
4208642b:	c298a0        	quou	a9, a8, a10
4208642e:	8099a0        	add	a9, a9, a10
42086431:	01a0a2        	movi	a10, 1
42086434:	419190        	srli	a9, a9, 1
42086437:	11aa00        	slli	a10, a10, 16
4208643a:	f45050        	extui	a5, a5, 0, 16
4208643d:	0519a7        	beq	a9, a10, 42086446 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x92>
42086440:	82a990        	mull	a10, a9, a9
42086443:	01b8a7        	bgeu	a8, a10, 42086448 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x94>
42086446:	990b      	addi.n	a9, a9, -1
42086448:	c05590        	sub	a5, a5, a9
4208644b:	f48050        	extui	a8, a5, 0, 16
4208644e:	239580        	sext	a9, a5, 15
42086451:	020c      	movi.n	a2, 0
42086453:	86e5f7        	bbsi	a5, 15, 420863dd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086456:	fa0c      	movi.n	a10, 15
42086458:	ffa022        	movi	a2, 255
4208645b:	02aa97        	bge	a10, a9, 42086461 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0xad>
4208645e:	ffdec6        	j	420863dd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086461:	1198c0        	slli	a9, a8, 4
42086464:	889a      	add.n	a8, a8, a9
42086466:	742080        	extui	a2, a8, 0, 8
42086469:	ffdc06        	j	420863dd <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


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
42033f3f:	04bfa5        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
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
42033f61:	3cc7e5        	call8	42070be0 <_Unwind_Resume>
42033f64:	fffac6        	j	42033f53 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


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
42038779:	4dc3a5        	call8	420863b4 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
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


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038b38:	008136        	entry	a1, 64
42038b3b:	021292        	l16ui	a9, a2, 4
42038b3e:	7169      	s32i.n	a6, a1, 28
42038b40:	233380        	sext	a3, a3, 15
42038b43:	23a980        	sext	a10, a9, 15
42038b46:	027d      	mov.n	a7, a2
42038b48:	234480        	sext	a4, a4, 15
42038b4b:	1a23a7        	blt	a3, a10, 42038b69 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b4e:	0412c2        	l16ui	a12, a2, 8
42038b51:	23ac80        	sext	a10, a12, 15
42038b54:	112a37        	blt	a10, a3, 42038b69 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b57:	0312b2        	l16ui	a11, a2, 6
42038b5a:	23ab80        	sext	a10, a11, 15
42038b5d:	0824a7        	blt	a4, a10, 42038b69 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b60:	0512d2        	l16ui	a13, a2, 10
42038b63:	23ad80        	sext	a10, a13, 15
42038b66:	12aa47        	bge	a10, a4, 42038b7c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x44>
42038b69:	04dd      	mov.n	a13, a4
42038b6b:	03cd      	mov.n	a12, a3
42038b6d:	3cc7b2        	addi	a11, a7, 60
42038b70:	28c7a2        	addi	a10, a7, 40
42038b73:	005aa5        	call8	4203911c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b76:	012a80        	slli	a2, a10, 24
42038b79:	f01d      	retw.n
42038b7b:	02a200        	andb	b10, b2, b0
42038b7e:	20          	.byte	0x20
42038b7f:	016142        	s32i	a4, a1, 4
42038b82:	006132        	s32i	a3, a1, 0
42038b85:	c0dda0        	sub	a13, a13, a10
42038b88:	c0cca0        	sub	a12, a12, a10
42038b8b:	80bab0        	add	a11, a10, a11
42038b8e:	2102f2        	l8ui	a15, a2, 33
42038b91:	aa9a      	add.n	a10, a10, a9
42038b93:	72e8      	l32i.n	a14, a2, 28
42038b95:	23dd80        	sext	a13, a13, 15
42038b98:	23cc80        	sext	a12, a12, 15
42038b9b:	23bb80        	sext	a11, a11, 15
42038b9e:	23aa80        	sext	a10, a10, 15
42038ba1:	ffb725        	call8	42038714 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038ba4:	ffa092        	movi	a9, 255
42038ba7:	0a6d      	mov.n	a6, a10
42038ba9:	079a97        	bne	a10, a9, 42038bb4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x7c>
42038bac:	142292        	l32i	a9, a2, 80
42038baf:	052d      	mov.n	a2, a5
42038bb1:	fc4916        	beqz	a9, 42038b79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038bb4:	04dd      	mov.n	a13, a4
42038bb6:	03cd      	mov.n	a12, a3
42038bb8:	3cc7b2        	addi	a11, a7, 60
42038bbb:	28c7a2        	addi	a10, a7, 40
42038bbe:	0055e5        	call8	4203911c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038bc1:	200792        	l8ui	a9, a7, 32
42038bc4:	6169      	s32i.n	a6, a1, 24
42038bc6:	012a80        	slli	a2, a10, 24
42038bc9:	03e916        	beqz	a9, 42038c0b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038bcc:	1149      	s32i.n	a4, a1, 4
42038bce:	0139      	s32i.n	a3, a1, 0
42038bd0:	67e8      	l32i.n	a14, a7, 24
42038bd2:	0597d2        	l16si	a13, a7, 10
42038bd5:	0497c2        	l16si	a12, a7, 8
42038bd8:	0397b2        	l16si	a11, a7, 6
42038bdb:	0297a2        	l16si	a10, a7, 4
42038bde:	ff0c      	movi.n	a15, 15
42038be0:	ffb325        	call8	42038714 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038be3:	61a9      	s32i.n	a10, a1, 24
42038be5:	2aac      	beqz.n	a10, 42038c0b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038be7:	7188      	l32i.n	a8, a1, 28
42038be9:	38cc      	bnez.n	a8, 42038bf0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb8>
42038beb:	24c782        	addi	a8, a7, 36
42038bee:	7189      	s32i.n	a8, a1, 28
42038bf0:	7188      	l32i.n	a8, a1, 28
42038bf2:	02ad      	mov.n	a10, a2
42038bf4:	002892        	l32i	a9, a8, 0
42038bf7:	062182        	l32i	a8, a1, 24
42038bfa:	119980        	slli	a9, a9, 8
42038bfd:	419890        	srli	a9, a9, 8
42038c00:	01b880        	slli	a11, a8, 24
42038c03:	20b9b0        	or	a11, a9, a11
42038c06:	cec425        	call8	42007848 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c09:	0a2d      	mov.n	a2, a10
42038c0b:	ffa092        	movi	a9, 255
42038c0e:	469697        	bne	a6, a9, 42038c58 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x120>
42038c11:	fb7c      	movi.n	a11, -1
42038c13:	01bb80        	slli	a11, a11, 24
42038c16:	20b5b0        	or	a11, a5, a11
42038c19:	02ad      	mov.n	a10, a2
42038c1b:	cec2e5        	call8	42007848 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c1e:	0a2d      	mov.n	a2, a10
42038c20:	1427a2        	l32i	a10, a7, 80
42038c23:	f52a16        	beqz	a10, 42038b79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c26:	20c440        	or	a12, a4, a4
42038c29:	20b330        	or	a11, a3, a3
42038c2c:	ffdd65        	call8	42038a04 <roo_windows::PressOverlay::get(short, short) const>
42038c2f:	6198      	l32i.n	a9, a1, 24
42038c31:	7588a0        	extui	a8, a10, 24, 8
42038c34:	d18890        	mul16s	a8, a8, a9
42038c37:	1f2a91        	l32r	a9, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c3a:	11aa80        	slli	a10, a10, 8
42038c3d:	a28890        	muluh	a8, a8, a9
42038c40:	0188f0        	slli	a8, a8, 17
42038c43:	758880        	extui	a8, a8, 24, 8
42038c46:	404800        	ssai	8
42038c49:	81b8a0        	src	a11, a8, a10
42038c4c:	02ad      	mov.n	a10, a2
42038c4e:	cebfa5        	call8	42007848 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c51:	0a2d      	mov.n	a2, a10
42038c53:	ffc886        	j	42038b79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c56:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038c59:	50fc46        	j	4204d04e <uart_set_stop_bits+0x5e>
42038c5c:	7598      	l32i.n	a9, a5, 28
42038c5e:	d19960        	mul16s	a9, a9, a6
42038c61:	1f1fa1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c64:	7fc992        	addi	a9, a9, 127
42038c67:	a299a0        	muluh	a9, a9, a10
42038c6a:	11b580        	slli	a11, a5, 8
42038c6d:	0199f0        	slli	a9, a9, 17
42038c70:	759890        	extui	a9, a9, 24, 8
42038c73:	404800        	ssai	8
42038c76:	81b9b0        	src	a11, a9, a11
42038c79:	ffe706        	j	42038c19 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xe1>


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203911c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
4203911c:	006136        	entry	a1, 48
4203911f:	027d      	mov.n	a7, a2
42039121:	0f0222        	l8ui	a2, a2, 15
42039124:	234480        	sext	a4, a4, 15
42039127:	235580        	sext	a5, a5, 15
4203912a:	167216        	beqz	a2, 42039295 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x179>
4203912d:	009782        	l16si	a8, a7, 0
42039130:	0693a2        	l16si	a10, a3, 12
42039133:	c04480        	sub	a4, a4, a8
42039136:	019782        	l16si	a8, a7, 2
42039139:	1194c0        	slli	a9, a4, 4
4203913c:	c08580        	sub	a8, a5, a8
4203913f:	069742        	l16si	a4, a7, 12
42039142:	1188c0        	slli	a8, a8, 4
42039145:	888b      	addi.n	a8, a8, 8
42039147:	c04840        	sub	a4, a8, a4
4203914a:	c088a0        	sub	a8, a8, a10
4203914d:	0397a2        	l16si	a10, a7, 6
42039150:	0297e2        	l16si	a14, a7, 4
42039153:	11aac0        	slli	a10, a10, 4
42039156:	644a      	add.n	a6, a4, a4
42039158:	f88a      	add.n	a15, a8, a8
4203915a:	998b      	addi.n	a9, a9, 8
4203915c:	11eec0        	slli	a14, a14, 4
4203915f:	72d6a0        	salt	a13, a6, a10
42039162:	72cfa0        	salt	a12, a15, a10
42039165:	c05e90        	sub	a5, a14, a9
42039168:	c0ba40        	sub	a11, a10, a4
4203916b:	dd0b      	addi.n	a13, a13, -1
4203916d:	cc0b      	addi.n	a12, a12, -1
4203916f:	60d0d0        	neg	a13, a13
42039172:	60c0c0        	neg	a12, a12
42039175:	435590        	min	a5, a5, a9
42039178:	43bb40        	min	a11, a11, a4
4203917b:	999a      	add.n	a9, a9, a9
4203917d:	c04a80        	sub	a4, a10, a8
42039180:	74d0d0        	extui	a13, a13, 0, 8
42039183:	74c0c0        	extui	a12, a12, 0, 8
42039186:	434480        	min	a4, a4, a8
42039189:	02a9e7        	bge	a9, a14, 4203918f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x73>
4203918c:	004406        	j	420392a0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x184>
4203918f:	02a6a7        	bge	a6, a10, 42039195 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x79>
42039192:	004086        	j	42039298 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x17c>
42039195:	0a0782        	l8ui	a8, a7, 10
42039198:	029dc7        	bne	a13, a12, 4203919e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x82>
4203919b:	004286        	j	420392a9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x18d>
4203919e:	02afa7        	bge	a15, a10, 420391a4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x88>
420391a1:	0044c6        	j	420392b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x19c>
420391a4:	0a0722        	l8ui	a2, a7, 10
420391a7:	0917c2        	l16ui	a12, a7, 18
420391aa:	4395b0        	min	a9, a5, a11
420391ad:	60a0c0        	neg	a10, a12
420391b0:	060c      	movi.n	a6, 0
420391b2:	5d29a7        	blt	a9, a10, 42039213 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf7>
420391b5:	238880        	sext	a8, a8, 15
420391b8:	1188c0        	slli	a8, a8, 4
420391bb:	c0a850        	sub	a10, a8, a5
420391be:	c098b0        	sub	a9, a8, a11
420391c1:	0226a7        	blt	a6, a10, 420391c7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
420391c4:	004206        	j	420392d0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1b4>
420391c7:	022697        	blt	a6, a9, 420391cd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb1>
420391ca:	004086        	j	420392d0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1b4>
420391cd:	cc8a      	add.n	a12, a12, a8
420391cf:	829990        	mull	a9, a9, a9
420391d2:	82aaa0        	mull	a10, a10, a10
420391d5:	cc1b      	addi.n	a12, a12, 1
420391d7:	82ccc0        	mull	a12, a12, a12
420391da:	80a9a0        	add	a10, a9, a10
420391dd:	036182        	s32i	a8, a1, 12
420391e0:	2fbac7        	bgeu	a10, a12, 42039213 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf7>
420391e3:	4d3865        	call8	42086568 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
420391e6:	032182        	l32i	a8, a1, 12
420391e9:	0e0792        	l8ui	a9, a7, 14
420391ec:	c08a80        	sub	a8, a10, a8
420391ef:	c08890        	sub	a8, a8, a9
420391f2:	081792        	l16ui	a9, a7, 16
420391f5:	828890        	mull	a8, a8, a9
420391f8:	1dc091        	l32r	a9, 420008f8 <_stext+0x8d8> (80000 <UserFrameTotalSize+0x7ff00>)
420391fb:	889a      	add.n	a8, a8, a9
420391fd:	218c80        	srai	a8, a8, 12
42039200:	ffa092        	movi	a9, 255
42039203:	438890        	min	a8, a8, a9
42039206:	090c      	movi.n	a9, 0
42039208:	538890        	max	a8, a8, a9
4203920b:	1dba91        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1160 <roo_windows::internal::(anonymous namespace)::kKey>)
4203920e:	898a      	add.n	a8, a9, a8
42039210:	000862        	l8ui	a6, a8, 0
42039213:	091392        	l16ui	a9, a3, 18
42039216:	43a540        	min	a10, a5, a4
42039219:	60b090        	neg	a11, a9
4203921c:	080c      	movi.n	a8, 0
4203921e:	5a2ab7        	blt	a10, a11, 4203927c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x160>
42039221:	232280        	sext	a2, a2, 15
42039224:	1122c0        	slli	a2, a2, 4
42039227:	c0a250        	sub	a10, a2, a5
4203922a:	c0b240        	sub	a11, a2, a4
4203922d:	0228a7        	blt	a8, a10, 42039233 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x117>
42039230:	002a06        	j	420392dc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c0>
42039233:	0228b7        	blt	a8, a11, 42039239 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11d>
42039236:	002886        	j	420392dc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c0>
42039239:	809920        	add	a9, a9, a2
4203923c:	82aaa0        	mull	a10, a10, a10
4203923f:	82bbb0        	mull	a11, a11, a11
42039242:	01c992        	addi	a9, a9, 1
42039245:	829990        	mull	a9, a9, a9
42039248:	80aab0        	add	a10, a10, a11
4203924b:	2dba97        	bgeu	a10, a9, 4203927c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x160>
4203924e:	4d31a5        	call8	42086568 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039251:	0e0392        	l8ui	a9, a3, 14
42039254:	c08a20        	sub	a8, a10, a2
42039257:	c08890        	sub	a8, a8, a9
4203925a:	081392        	l16ui	a9, a3, 16
4203925d:	828890        	mull	a8, a8, a9
42039260:	1da691        	l32r	a9, 420008f8 <_stext+0x8d8> (80000 <UserFrameTotalSize+0x7ff00>)
42039263:	889a      	add.n	a8, a8, a9
42039265:	218c80        	srai	a8, a8, 12
42039268:	ffa092        	movi	a9, 255
4203926b:	438890        	min	a8, a8, a9
4203926e:	090c      	movi.n	a9, 0
42039270:	538890        	max	a8, a8, a9
42039273:	1da291        	l32r	a9, 420008fc <_stext+0x8dc> (3c0b1060 <roo_windows::internal::(anonymous namespace)::kAmbient>)
42039276:	808980        	add	a8, a9, a8
42039279:	000882        	l8ui	a8, a8, 0
4203927c:	d19680        	mul16s	a9, a6, a8
4203927f:	1d98a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42039282:	7fc992        	addi	a9, a9, 127
42039285:	b2a9a0        	mulsh	a10, a9, a10
42039288:	286a      	add.n	a2, a8, a6
4203928a:	99aa      	add.n	a9, a9, a10
4203928c:	219790        	srai	a9, a9, 7
4203928f:	c02290        	sub	a2, a2, a9
42039292:	742020        	extui	a2, a2, 0, 8
42039295:	f01d      	retw.n
42039297:	078200        	ee.stf.64.xp	f8, f0, a0, a2
4203929a:	4609      	s32i.n	a0, a6, 16
4203929c:	0000ffbe 	ee.vmulas.s16.accx.ld.ip.qup	q0, a11, 240, q3, q6, q0, q0
420392a0:	0c26a7        	blt	a6, a10, 420392b0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x194>
420392a3:	0b0782        	l8ui	a8, a7, 11
420392a6:	169dc7        	bne	a13, a12, 420392c0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a4>
420392a9:	082d      	mov.n	a2, a8
420392ab:	ffbe06        	j	420391a7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x8b>
420392ae:	820000        	mull	a0, a0, a0
420392b1:	c60807        	bnone	a8, a0, 4203927b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x15f>
420392b4:	fffb      	addi.n	a15, a15, 15
420392b6:	220000        	orb	b0, b0, b0
420392b9:	060907        	bnone	a9, a0, 420392c3 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a7>
420392bc:	ffba      	add.n	a15, a15, a11
420392be:	a70000        	ee.stf.64.xp	f0, f10, a0, a0
420392c1:	0722052f 	ee.vmulas.s16.accx.ld.ip.qup	q6, a2, 0x150, q4, q0, q2, q2
420392c5:	460b      	addi.n	a4, a6, -1
420392c7:	22ffb7        	bbsi	a15, 27, 420392ed <roo_windows::internal::ShadowLookupTable(bool)+0x5>
420392ca:	c60807        	bnone	a8, a0, 42039294 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x178>
420392cd:	00ffb5        	call12	4203a2c8 <roo_windows::Keyboard::Keyboard(roo_windows::ApplicationContext&, roo_windows::KeyboardLayout)+0xa0>
420392d0:	022b57        	blt	a11, a5, 420392d6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ba>
420392d3:	ffc486        	j	420391e9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xcd>
420392d6:	09ad      	mov.n	a10, a9
420392d8:	ffc346        	j	420391e9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xcd>
420392db:	245700        	extui	a5, a0, 7, 3
420392de:	02          	.byte	0x2
420392df:	ffdb86        	j	42039251 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x135>
420392e2:	0bad      	mov.n	a10, a11
420392e4:	ffda46        	j	42039251 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x135>


/tmp/roo-shadow-review/codegen/compact_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038d28 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038d28:	00c136        	entry	a1, 96
42038d2b:	078d      	mov.n	a8, a7
42038d2d:	069d      	mov.n	a9, a6
42038d2f:	086d      	mov.n	a6, a8
42038d31:	238980        	sext	a8, a9, 15
42038d34:	3189      	s32i.n	a8, a1, 12
42038d36:	142282        	l32i	a8, a2, 80
42038d39:	027d      	mov.n	a7, a2
42038d3b:	233380        	sext	a3, a3, 15
42038d3e:	234480        	sext	a4, a4, 15
42038d41:	235580        	sext	a5, a5, 15
42038d44:	200856        	bnez	a8, 42038f48 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038d47:	0288      	l32i.n	a8, a2, 0
42038d49:	31e8      	l32i.n	a14, a1, 12
42038d4b:	b888      	l32i.n	a8, a8, 44
42038d4d:	02ad      	mov.n	a10, a2
42038d4f:	06fd      	mov.n	a15, a6
42038d51:	05dd      	mov.n	a13, a5
42038d53:	04cd      	mov.n	a12, a4
42038d55:	03bd      	mov.n	a11, a3
42038d57:	0008e0        	callx8	a8
42038d5a:	0a2d      	mov.n	a2, a10
42038d5c:	09ca56        	bnez	a10, 42038dfc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd4>
42038d5f:	249792        	l16si	a9, a7, 72
42038d62:	1a9782        	l16si	a8, a7, 52
42038d65:	ec0c      	movi.n	a12, 14
42038d67:	538890        	max	a8, a8, a9
42038d6a:	98fb      	addi.n	a9, a8, 15
42038d6c:	1ec882        	addi	a8, a8, 30
42038d6f:	b38990        	movgez	a8, a9, a9
42038d72:	21b480        	srai	a11, a8, 4
42038d75:	210782        	l8ui	a8, a7, 33
42038d78:	1907a2        	l8ui	a10, a7, 25
42038d7b:	62cc80        	saltu	a12, a12, a8
42038d7e:	180782        	l8ui	a8, a7, 24
42038d81:	200792        	l8ui	a9, a7, 32
42038d84:	7388a0        	maxu	a8, a8, a10
42038d87:	1a07a2        	l8ui	a10, a7, 26
42038d8a:	cc0b      	addi.n	a12, a12, -1
42038d8c:	7388a0        	maxu	a8, a8, a10
42038d8f:	1b07a2        	l8ui	a10, a7, 27
42038d92:	60c0c0        	neg	a12, a12
42038d95:	73a8a0        	maxu	a10, a8, a10
42038d98:	89ca      	add.n	a8, a9, a12
42038d9a:	99ba      	add.n	a9, a9, a11
42038d9c:	538a80        	max	a8, a10, a8
42038d9f:	99ca      	add.n	a9, a9, a12
42038da1:	0297a2        	l16si	a10, a7, 4
42038da4:	539980        	max	a9, a9, a8
42038da7:	aa9a      	add.n	a10, a10, a9
42038da9:	53aa30        	max	a10, a10, a3
42038dac:	11a9      	s32i.n	a10, a1, 4
42038dae:	0497a2        	l16si	a10, a7, 8
42038db1:	c09a90        	sub	a9, a10, a9
42038db4:	439950        	min	a9, a9, a5
42038db7:	5199      	s32i.n	a9, a1, 20
42038db9:	039792        	l16si	a9, a7, 6
42038dbc:	998a      	add.n	a9, a9, a8
42038dbe:	99ba      	add.n	a9, a9, a11
42038dc0:	9199      	s32i.n	a9, a1, 36
42038dc2:	059792        	l16si	a9, a7, 10
42038dc5:	c08980        	sub	a8, a9, a8
42038dc8:	4189      	s32i.n	a8, a1, 16
42038dca:	c08530        	sub	a8, a5, a3
42038dcd:	881b      	addi.n	a8, a8, 1
42038dcf:	6189      	s32i.n	a8, a1, 24
42038dd1:	5188      	l32i.n	a8, a1, 20
42038dd3:	1198      	l32i.n	a9, a1, 4
42038dd5:	881b      	addi.n	a8, a8, 1
42038dd7:	7189      	s32i.n	a8, a1, 28
42038dd9:	c08890        	sub	a8, a8, a9
42038ddc:	1188e0        	slli	a8, a8, 2
42038ddf:	a189      	s32i.n	a8, a1, 40
42038de1:	6188      	l32i.n	a8, a1, 24
42038de3:	1188e0        	slli	a8, a8, 2
42038de6:	2189      	s32i.n	a8, a1, 8
42038de8:	c08930        	sub	a8, a9, a3
42038deb:	1188e0        	slli	a8, a8, 2
42038dee:	b189      	s32i.n	a8, a1, 44
42038df0:	029182        	l16si	a8, a1, 4
42038df3:	8189      	s32i.n	a8, a1, 32
42038df5:	3188      	l32i.n	a8, a1, 12
42038df7:	03a847        	bge	a8, a4, 42038dfe <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd6>
42038dfa:	020c      	movi.n	a2, 0
42038dfc:	f01d      	retw.n
42038dfe:	5198      	l32i.n	a9, a1, 20
42038e00:	11a8      	l32i.n	a10, a1, 4
42038e02:	038d      	mov.n	a8, a3
42038e04:	062d      	mov.n	a2, a6
42038e06:	03bd      	mov.n	a11, a3
42038e08:	23c480        	sext	a12, a4, 15
42038e0b:	0229a7        	blt	a9, a10, 42038e11 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038e0e:	002406        	j	42038ea2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x17a>
42038e11:	029d      	mov.n	a9, a2
42038e13:	08bd      	mov.n	a11, a8
42038e15:	23c480        	sext	a12, a4, 15
42038e18:	0225b7        	blt	a5, a11, 42038e1e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf6>
42038e1b:	002b46        	j	42038ecc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a4>
42038e1e:	a51b      	addi.n	a10, a5, 1
42038e20:	729a80        	salt	a9, a10, a8
42038e23:	990b      	addi.n	a9, a9, -1
42038e25:	609090        	neg	a9, a9
42038e28:	749090        	extui	a9, a9, 0, 8
42038e2b:	0b0c      	movi.n	a11, 0
42038e2d:	498c      	beqz.n	a9, 42038e35 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10d>
42038e2f:	c0aa80        	sub	a10, a10, a8
42038e32:	11bae0        	slli	a11, a10, 2
42038e35:	9198      	l32i.n	a9, a1, 36
42038e37:	22ba      	add.n	a2, a2, a11
42038e39:	729490        	salt	a9, a4, a9
42038e3c:	990b      	addi.n	a9, a9, -1
42038e3e:	609090        	neg	a9, a9
42038e41:	749090        	extui	a9, a9, 0, 8
42038e44:	028d      	mov.n	a8, a2
42038e46:	79bc      	beqz.n	a9, 42038e81 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038e48:	4198      	l32i.n	a9, a1, 16
42038e4a:	729940        	salt	a9, a9, a4
42038e4d:	990b      	addi.n	a9, a9, -1
42038e4f:	609090        	neg	a9, a9
42038e52:	749090        	extui	a9, a9, 0, 8
42038e55:	89ac      	beqz.n	a9, 42038e81 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038e57:	3198      	l32i.n	a9, a1, 12
42038e59:	41a8      	l32i.n	a10, a1, 16
42038e5b:	04dd      	mov.n	a13, a4
42038e5d:	4399a0        	min	a9, a9, a10
42038e60:	02ad97        	bge	a13, a9, 42038e66 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x13e>
42038e63:	0022c6        	j	42038ef2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038e66:	728940        	salt	a8, a9, a4
42038e69:	880b      	addi.n	a8, a8, -1
42038e6b:	608080        	neg	a8, a8
42038e6e:	748080        	extui	a8, a8, 0, 8
42038e71:	0a0c      	movi.n	a10, 0
42038e73:	188c      	beqz.n	a8, 42038e78 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x150>
42038e75:	c0a940        	sub	a10, a9, a4
42038e78:	2188      	l32i.n	a8, a1, 8
42038e7a:	44aa      	add.n	a4, a4, a10
42038e7c:	828a80        	mull	a8, a10, a8
42038e7f:	882a      	add.n	a8, a8, a2
42038e81:	441b      	addi.n	a4, a4, 1
42038e83:	086d      	mov.n	a6, a8
42038e85:	ffdb06        	j	42038df5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcd>
42038e88:	0527d2        	l32i	a13, a7, 20
42038e8b:	00a0e2        	movi	a14, 0
42038e8e:	20a770        	or	a10, a7, a7
42038e91:	d1c9      	s32i.n	a12, a1, 52
42038e93:	c1b9      	s32i.n	a11, a1, 48
42038e95:	ffca25        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038e98:	c1b8      	l32i.n	a11, a1, 48
42038e9a:	d1c8      	l32i.n	a12, a1, 52
42038e9c:	02a9      	s32i.n	a10, a2, 0
42038e9e:	bb1b      	addi.n	a11, a11, 1
42038ea0:	224b      	addi.n	a2, a2, 4
42038ea2:	012182        	l32i	a8, a1, 4
42038ea5:	df2b87        	blt	a11, a8, 42038e88 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x160>
42038ea8:	0b2182        	l32i	a8, a1, 44
42038eab:	0527d2        	l32i	a13, a7, 20
42038eae:	81b8      	l32i.n	a11, a1, 32
42038eb0:	0e0c      	movi.n	a14, 0
42038eb2:	07ad      	mov.n	a10, a7
42038eb4:	268a      	add.n	a2, a6, a8
42038eb6:	ffc825        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038eb9:	a188      	l32i.n	a8, a1, 40
42038ebb:	828a      	add.n	a8, a2, a8
42038ebd:	049827        	bne	a8, a2, 42038ec5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x19d>
42038ec0:	7188      	l32i.n	a8, a1, 28
42038ec2:	ffd2c6        	j	42038e11 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038ec5:	02a9      	s32i.n	a10, a2, 0
42038ec7:	224b      	addi.n	a2, a2, 4
42038ec9:	fffc06        	j	42038ebd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x195>
42038ecc:	0527d2        	l32i	a13, a7, 20
42038ecf:	00a0e2        	movi	a14, 0
42038ed2:	20a770        	or	a10, a7, a7
42038ed5:	f199      	s32i.n	a9, a1, 60
42038ed7:	e189      	s32i.n	a8, a1, 56
42038ed9:	d1c9      	s32i.n	a12, a1, 52
42038edb:	c1b9      	s32i.n	a11, a1, 48
42038edd:	ffc5a5        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ee0:	f198      	l32i.n	a9, a1, 60
42038ee2:	c1b8      	l32i.n	a11, a1, 48
42038ee4:	09a9      	s32i.n	a10, a9, 0
42038ee6:	e188      	l32i.n	a8, a1, 56
42038ee8:	d1c8      	l32i.n	a12, a1, 52
42038eea:	bb1b      	addi.n	a11, a11, 1
42038eec:	994b      	addi.n	a9, a9, 4
42038eee:	ffc986        	j	42038e18 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf0>
42038ef1:	00          	.byte	00
42038ef2:	61a8      	l32i.n	a10, a1, 24
42038ef4:	1a1aa6        	blti	a10, 1, 42038f12 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038ef7:	21a8      	l32i.n	a10, a1, 8
42038ef9:	1f4a26        	beqi	a10, 4, 42038f1c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f4>
42038efc:	0acd      	mov.n	a12, a10
42038efe:	06bd      	mov.n	a11, a6
42038f00:	08ad      	mov.n	a10, a8
42038f02:	d1d9      	s32i.n	a13, a1, 52
42038f04:	c199      	s32i.n	a9, a1, 48
42038f06:	1e7781        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038f09:	0008e0        	callx8	a8
42038f0c:	c198      	l32i.n	a9, a1, 48
42038f0e:	d1d8      	l32i.n	a13, a1, 52
42038f10:	0a8d      	mov.n	a8, a10
42038f12:	21a8      	l32i.n	a10, a1, 8
42038f14:	dd1b      	addi.n	a13, a13, 1
42038f16:	88aa      	add.n	a8, a8, a10
42038f18:	ffd106        	j	42038e60 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x138>
42038f1b:	06a800        	ee.ldf.64.xp	f10, f0, a0, a8
42038f1e:	08a9      	s32i.n	a10, a8, 0
42038f20:	fffb86        	j	42038f12 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038f23:	57d800        	ee.stf.64.xp	f13, f5, a0, a8
42038f26:	02bd      	mov.n	a11, a2
42038f28:	0e0c      	movi.n	a14, 0
42038f2a:	04cd      	mov.n	a12, a4
42038f2c:	07ad      	mov.n	a10, a7
42038f2e:	1189      	s32i.n	a8, a1, 4
42038f30:	ffc065        	call8	42038b38 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038f33:	1188      	l32i.n	a8, a1, 4
42038f35:	221b      	addi.n	a2, a2, 1
42038f37:	08a9      	s32i.n	a10, a8, 0
42038f39:	232280        	sext	a2, a2, 15
42038f3c:	884b      	addi.n	a8, a8, 4
42038f3e:	086d      	mov.n	a6, a8
42038f40:	e0a527        	bge	a5, a2, 42038f24 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1fc>
42038f43:	441b      	addi.n	a4, a4, 1
42038f45:	234480        	sext	a4, a4, 15
42038f48:	3188      	l32i.n	a8, a1, 12
42038f4a:	02a847        	bge	a8, a4, 42038f50 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038f4d:	ffaa46        	j	42038dfa <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd2>
42038f50:	068d      	mov.n	a8, a6
42038f52:	032d      	mov.n	a2, a3
42038f54:	fff986        	j	42038f3e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>
