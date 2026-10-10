
/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033138 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42033138:	006136        	entry	a1, 48
4203313b:	002282        	l32i	a8, a2, 0
4203313e:	20f770        	or	a15, a7, a7
42033141:	0b2882        	l32i	a8, a8, 44
42033144:	233380        	sext	a3, a3, 15
42033147:	234480        	sext	a4, a4, 15
4203314a:	235580        	sext	a5, a5, 15
4203314d:	236680        	sext	a6, a6, 15
42033150:	02ad      	mov.n	a10, a2
42033152:	31f9      	s32i.n	a15, a1, 12
42033154:	06ed      	mov.n	a14, a6
42033156:	05dd      	mov.n	a13, a5
42033158:	04cd      	mov.n	a12, a4
4203315a:	03bd      	mov.n	a11, a3
4203315c:	0008e0        	callx8	a8
4203315f:	027d      	mov.n	a7, a2
42033161:	0a2d      	mov.n	a2, a10
42033163:	facc      	bnez.n	a10, 42033176 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
42033165:	31f8      	l32i.n	a15, a1, 12
42033167:	06ed      	mov.n	a14, a6
42033169:	05dd      	mov.n	a13, a5
4203316b:	04cd      	mov.n	a12, a4
4203316d:	03bd      	mov.n	a11, a3
4203316f:	07ad      	mov.n	a10, a7
42033171:	dc5d65        	call8	4200f748 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
42033174:	0a2d      	mov.n	a2, a10
42033176:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208660c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
4208660c:	004136        	entry	a1, 32
4208660f:	029d      	mov.n	a9, a2
42086611:	120c      	movi.n	a2, 1
42086613:	832990        	moveqz	a2, a9, a9
42086616:	4149b6        	bltui	a9, 4, 4208665b <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
42086619:	40f980        	nsau	a8, a9
4208661c:	dfc882        	addi	a8, a8, -33
4208661f:	608080        	neg	a8, a8
42086622:	218180        	srai	a8, a8, 1
42086625:	fec882        	addi	a8, a8, -2
42086628:	3a0c      	movi.n	a10, 3
4208662a:	401800        	ssl	a8
4208662d:	a1aa00        	sll	a10, a10
42086630:	c289a0        	quou	a8, a9, a10
42086633:	88aa      	add.n	a8, a8, a10
42086635:	418180        	srli	a8, a8, 1
42086638:	c2a980        	quou	a10, a9, a8
4208663b:	aa8a      	add.n	a10, a10, a8
4208663d:	41a1a0        	srli	a10, a10, 1
42086640:	c289a0        	quou	a8, a9, a10
42086643:	88aa      	add.n	a8, a8, a10
42086645:	1a0c      	movi.n	a10, 1
42086647:	418180        	srli	a8, a8, 1
4208664a:	11aa00        	slli	a10, a10, 16
4208664d:	0518a7        	beq	a8, a10, 42086656 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
42086650:	82a880        	mull	a10, a8, a8
42086653:	01b9a7        	bgeu	a9, a10, 42086658 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
42086656:	880b      	addi.n	a8, a8, -1
42086658:	f42080        	extui	a2, a8, 0, 16
4208665b:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c8c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c8c:	006136        	entry	a1, 48
42038c8f:	038d      	mov.n	a8, a3
42038c91:	059d      	mov.n	a9, a5
42038c93:	05fd      	mov.n	a15, a5
42038c95:	063d      	mov.n	a3, a6
42038c97:	045d      	mov.n	a5, a4
42038c99:	087d      	mov.n	a7, a8
42038c9b:	1d4ff6        	bgeui	a15, 4, 42038cbc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x30>
42038c9e:	41a290        	srli	a10, a9, 2
42038ca1:	11bac0        	slli	a11, a10, 4
42038ca4:	66ba      	add.n	a6, a6, a11
42038ca6:	b0ba80        	addx8	a11, a10, a8
42038ca9:	11aae0        	slli	a10, a10, 2
42038cac:	c099a0        	sub	a9, a9, a10
42038caf:	a07960        	addx4	a7, a9, a6
42038cb2:	c04480        	sub	a4, a4, a8
42038cb5:	8b4a      	add.n	a8, a11, a4
42038cb7:	5e9677        	bne	a6, a7, 42038d19 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8d>
42038cba:	f01d      	retw.n
42038cbc:	0095c2        	l16si	a12, a5, 0
42038cbf:	0097b2        	l16si	a11, a7, 0
42038cc2:	52d8      	l32i.n	a13, a2, 20
42038cc4:	0e0c      	movi.n	a14, 0
42038cc6:	02ad      	mov.n	a10, a2
42038cc8:	3199      	s32i.n	a9, a1, 12
42038cca:	2189      	s32i.n	a8, a1, 8
42038ccc:	11f9      	s32i.n	a15, a1, 4
42038cce:	ffe7a5        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cd1:	03a9      	s32i.n	a10, a3, 0
42038cd3:	0195c2        	l16si	a12, a5, 2
42038cd6:	0197b2        	l16si	a11, a7, 2
42038cd9:	52d8      	l32i.n	a13, a2, 20
42038cdb:	0e0c      	movi.n	a14, 0
42038cdd:	02ad      	mov.n	a10, a2
42038cdf:	ffe6a5        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ce2:	13a9      	s32i.n	a10, a3, 4
42038ce4:	0295c2        	l16si	a12, a5, 4
42038ce7:	0297b2        	l16si	a11, a7, 4
42038cea:	52d8      	l32i.n	a13, a2, 20
42038cec:	0e0c      	movi.n	a14, 0
42038cee:	02ad      	mov.n	a10, a2
42038cf0:	ffe565        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cf3:	23a9      	s32i.n	a10, a3, 8
42038cf5:	0395c2        	l16si	a12, a5, 6
42038cf8:	0397b2        	l16si	a11, a7, 6
42038cfb:	52d8      	l32i.n	a13, a2, 20
42038cfd:	0e0c      	movi.n	a14, 0
42038cff:	02ad      	mov.n	a10, a2
42038d01:	ffe465        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038d04:	11f8      	l32i.n	a15, a1, 4
42038d06:	33a9      	s32i.n	a10, a3, 12
42038d08:	3198      	l32i.n	a9, a1, 12
42038d0a:	2188      	l32i.n	a8, a1, 8
42038d0c:	778b      	addi.n	a7, a7, 8
42038d0e:	558b      	addi.n	a5, a5, 8
42038d10:	10c332        	addi	a3, a3, 16
42038d13:	fccff2        	addi	a15, a15, -4
42038d16:	ffe046        	j	42038c9b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xf>
42038d19:	02cb52        	addi	a5, a11, 2
42038d1c:	0522d2        	l32i	a13, a2, 20
42038d1f:	009bb2        	l16si	a11, a11, 0
42038d22:	0098c2        	l16si	a12, a8, 0
42038d25:	00a0e2        	movi	a14, 0
42038d28:	02ad      	mov.n	a10, a2
42038d2a:	ffe1e5        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038d2d:	06a9      	s32i.n	a10, a6, 0
42038d2f:	05bd      	mov.n	a11, a5
42038d31:	664b      	addi.n	a6, a6, 4
42038d33:	ffdf86        	j	42038cb5 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x29>


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42086458 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
42086458:	004136        	entry	a1, 32
4208645b:	208230        	or	a8, a2, a3
4208645e:	0228d6        	bgez	a8, 42086484 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
42086461:	c02240        	sub	a2, a2, a4
42086464:	40f290        	nsau	a9, a2
42086467:	419590        	srli	a9, a9, 5
4208646a:	b28c      	beqz.n	a2, 42086479 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
4208646c:	c03340        	sub	a3, a3, a4
4208646f:	180c      	movi.n	a8, 1
42086471:	938930        	movnez	a8, a9, a3
42086474:	ffa022        	movi	a2, 255
42086477:	688c      	beqz.n	a8, 42086481 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086479:	1125c0        	slli	a2, a5, 4
4208647c:	252a      	add.n	a2, a5, a2
4208647e:	742020        	extui	a2, a2, 0, 8
42086481:	f01d      	retw.n
42086483:	222000        	orb	b2, b0, b0
42086486:	3330c1        	l32r	a12, 42053148 <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
42086489:	44c0c1        	l32r	a12, 4205778c <sdmmc_host_init_slot+0x4cc> (8e091b8 <UserFrameTotalSize+0x8e090b8>)
4208648c:	545a11        	l32r	a1, 4205b5f4 <esp_core_dump_write+0xd8> (900d0d0c <_rtc_reserved_end+0x2ffd0d0c>)
4208648f:	823a      	add.n	a8, a2, a3
42086491:	c19550        	mul16u	a9, a5, a5
42086494:	118880        	slli	a8, a8, 8
42086497:	020c      	movi.n	a2, 0
42086499:	e4b897        	bgeu	a8, a9, 42086481 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208649c:	f1c592        	addi	a9, a5, -15
4208649f:	c19990        	mul16u	a9, a9, a9
420864a2:	ffa022        	movi	a2, 255
420864a5:	d83897        	bltu	a8, a9, 42086481 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420864a8:	40f890        	nsau	a9, a8
420864ab:	dfc992        	addi	a9, a9, -33
420864ae:	609090        	neg	a9, a9
420864b1:	219190        	srai	a9, a9, 1
420864b4:	fec992        	addi	a9, a9, -2
420864b7:	3a0c      	movi.n	a10, 3
420864b9:	401900        	ssl	a9
420864bc:	a1aa00        	sll	a10, a10
420864bf:	c298a0        	quou	a9, a8, a10
420864c2:	99aa      	add.n	a9, a9, a10
420864c4:	419190        	srli	a9, a9, 1
420864c7:	c2a890        	quou	a10, a8, a9
420864ca:	aa9a      	add.n	a10, a10, a9
420864cc:	41a1a0        	srli	a10, a10, 1
420864cf:	c298a0        	quou	a9, a8, a10
420864d2:	8099a0        	add	a9, a9, a10
420864d5:	01a0a2        	movi	a10, 1
420864d8:	419190        	srli	a9, a9, 1
420864db:	11aa00        	slli	a10, a10, 16
420864de:	f45050        	extui	a5, a5, 0, 16
420864e1:	0519a7        	beq	a9, a10, 420864ea <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x92>
420864e4:	82a990        	mull	a10, a9, a9
420864e7:	01b8a7        	bgeu	a8, a10, 420864ec <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x94>
420864ea:	990b      	addi.n	a9, a9, -1
420864ec:	c05590        	sub	a5, a5, a9
420864ef:	f48050        	extui	a8, a5, 0, 16
420864f2:	239580        	sext	a9, a5, 15
420864f5:	020c      	movi.n	a2, 0
420864f7:	86e5f7        	bbsi	a5, 15, 42086481 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420864fa:	fa0c      	movi.n	a10, 15
420864fc:	ffa022        	movi	a2, 255
420864ff:	02aa97        	bge	a10, a9, 42086505 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0xad>
42086502:	ffdec6        	j	42086481 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086505:	1198c0        	slli	a9, a8, 4
42086508:	889a      	add.n	a8, a8, a9
4208650a:	742080        	extui	a2, a8, 0, 8
4208650d:	ffdc06        	j	42086481 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033eac <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033eac:	00a136        	entry	a1, 80
42033eaf:	3169      	s32i.n	a6, a1, 12
42033eb1:	2159      	s32i.n	a5, a1, 8
42033eb3:	060c      	movi.n	a6, 0
42033eb5:	1cc172        	addi	a7, a1, 28
42033eb8:	2188      	l32i.n	a8, a1, 8
42033eba:	029687        	bne	a6, a8, 42033ec0 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033ebd:	f01d      	retw.n
42033ebf:	938200        	movnez	a8, a2, a0
42033ec2:	22d200        	orb	b13, b2, b0
42033ec5:	018917        	bany	a9, a1, 42033eca <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033ec8:	009482        	l16si	a8, a4, 0
42033ecb:	01b8      	l32i.n	a11, a1, 0
42033ecd:	1622a2        	l32i	a10, a2, 88
42033ed0:	08cd      	mov.n	a12, a8
42033ed2:	0067d2        	s32i	a13, a7, 0
42033ed5:	20e770        	or	a14, a7, a7
42033ed8:	016182        	s32i	a8, a1, 4
42033edb:	201110        	or	a1, a1, a1
42033ede:	fffa25        	call8	42033e80 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033ee1:	182282        	l32i	a8, a2, 96
42033ee4:	1922b2        	l32i	a11, a2, 100
42033ee7:	8189      	s32i.n	a8, a1, 32
42033ee9:	20c152        	addi	a5, a1, 32
42033eec:	050b16        	beqz	a11, 42033f40 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033eef:	0bc8      	l32i.n	a12, a11, 0
42033ef1:	24c1a2        	addi	a10, a1, 36
42033ef4:	201110        	or	a1, a1, a1
42033ef7:	ff3665        	call8	4203325c <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033efa:	07d8      	l32i.n	a13, a7, 0
42033efc:	11c8      	l32i.n	a12, a1, 4
42033efe:	0021b2        	l32i	a11, a1, 0
42033f01:	24c1a2        	addi	a10, a1, 36
42033f04:	201110        	or	a1, a1, a1
42033f07:	fe1865        	call8	4203208c <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f0a:	07a9      	s32i.n	a10, a7, 0
42033f0c:	24c1a2        	addi	a10, a1, 36
42033f0f:	201110        	or	a1, a1, a1
42033f12:	f894a5        	call8	4202c85c <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f15:	1922b2        	l32i	a11, a2, 100
42033f18:	24c1a2        	addi	a10, a1, 36
42033f1b:	002bc2        	l32i	a12, a11, 0
42033f1e:	201110        	or	a1, a1, a1
42033f21:	ff33a5        	call8	4203325c <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f24:	05d8      	l32i.n	a13, a5, 0
42033f26:	0094c2        	l16si	a12, a4, 0
42033f29:	0093b2        	l16si	a11, a3, 0
42033f2c:	24c1a2        	addi	a10, a1, 36
42033f2f:	201110        	or	a1, a1, a1
42033f32:	fe15a5        	call8	4203208c <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f35:	05a9      	s32i.n	a10, a5, 0
42033f37:	24c1a2        	addi	a10, a1, 36
42033f3a:	201110        	or	a1, a1, a1
42033f3d:	f891e5        	call8	4202c85c <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f40:	0094c2        	l16si	a12, a4, 0
42033f43:	0093b2        	l16si	a11, a3, 0
42033f46:	07d8      	l32i.n	a13, a7, 0
42033f48:	05ed      	mov.n	a14, a5
42033f4a:	a24b      	addi.n	a10, a2, 4
42033f4c:	201110        	or	a1, a1, a1
42033f4f:	04bfa5        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f52:	3188      	l32i.n	a8, a1, 12
42033f54:	332b      	addi.n	a3, a3, 2
42033f56:	a08680        	addx4	a8, a6, a8
42033f59:	08a9      	s32i.n	a10, a8, 0
42033f5b:	661b      	addi.n	a6, a6, 1
42033f5d:	442b      	addi.n	a4, a4, 2
42033f5f:	ffd546        	j	42033eb8 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f62:	c1a200        	mul16u	a10, a2, a0
42033f65:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f68:	20          	.byte	0x20
42033f69:	f88f25        	call8	4202c85c <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f6c:	02ad      	mov.n	a10, a2
42033f6e:	201110        	or	a1, a1, a1
42033f71:	3cd125        	call8	42070c84 <_Unwind_Resume>
42033f74:	fffac6        	j	42033f63 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038724 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
42038724:	006136        	entry	a1, 48
42038727:	189192        	l16si	a9, a1, 48
4203872a:	3169      	s32i.n	a6, a1, 12
4203872c:	f1cb      	addi.n	a15, a1, 12
4203872e:	1a9182        	l16si	a8, a1, 52
42038731:	02ed      	mov.n	a14, a2
42038733:	74d070        	extui	a13, a7, 0, 8
42038736:	02a927        	bge	a9, a2, 4203873c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
42038739:	0030c6        	j	42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203873c:	02a497        	bge	a4, a9, 42038742 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
4203873f:	002f46        	j	42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038742:	02a837        	bge	a8, a3, 42038748 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
42038745:	002dc6        	j	42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038748:	02a587        	bge	a5, a8, 4203874e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a>
4203874b:	002c46        	j	42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203874e:	000fc2        	l8ui	a12, a15, 0
42038751:	3c9c      	beqz.n	a12, 42038768 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038753:	a2ca      	add.n	a10, a2, a12
42038755:	0f2a97        	blt	a10, a9, 42038768 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038758:	b3ca      	add.n	a11, a3, a12
4203875a:	0a2b87        	blt	a11, a8, 42038768 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
4203875d:	c0bb80        	sub	a11, a11, a8
42038760:	c0aa90        	sub	a10, a10, a9
42038763:	000706        	j	42038783 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x5f>
42038766:	c20000        	quou	a0, a0, a0
42038769:	1c16010f 	ee.vmulas.s16.qacc.ld.ip.qup	q0, a0, -240, q4, q0, q1, q6
4203876d:	a4c002        	addi	a0, a0, -92
42038770:	c0          	.byte	0xc0
42038771:	1b29a7        	blt	a9, a10, 42038790 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038774:	80b3c0        	add	a11, a3, a12
42038777:	152b87        	blt	a11, a8, 42038790 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
4203877a:	c0bb80        	sub	a11, a11, a8
4203877d:	c0ac40        	sub	a10, a12, a4
42038780:	80aa90        	add	a10, a10, a9
42038783:	23bb80        	sext	a11, a11, 15
42038786:	23aa80        	sext	a10, a10, 15
42038789:	4dcce5        	call8	42086458 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
4203878c:	0a2d      	mov.n	a2, a10
4203878e:	f01d      	retw.n
42038790:	020fc2        	l8ui	a12, a15, 2
42038793:	2c9c      	beqz.n	a12, 420387a9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038795:	c0a4c0        	sub	a10, a4, a12
42038798:	0d29a7        	blt	a9, a10, 420387a9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
4203879b:	c0a5c0        	sub	a10, a5, a12
4203879e:	0728a7        	blt	a8, a10, 420387a9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
420387a1:	c0bc50        	sub	a11, a12, a5
420387a4:	bb8a      	add.n	a11, a11, a8
420387a6:	fff4c6        	j	4203877d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x59>
420387a9:	030fc2        	l8ui	a12, a15, 3
420387ac:	1c9c      	beqz.n	a12, 420387c1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387ae:	aeca      	add.n	a10, a14, a12
420387b0:	0d2a97        	blt	a10, a9, 420387c1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387b3:	c0b5c0        	sub	a11, a5, a12
420387b6:	0728b7        	blt	a8, a11, 420387c1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387b9:	c0bc50        	sub	a11, a12, a5
420387bc:	bb8a      	add.n	a11, a11, a8
420387be:	ffe786        	j	42038760 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3c>
420387c1:	ea0c      	movi.n	a10, 14
420387c3:	ffa022        	movi	a2, 255
420387c6:	c43ad7        	bltu	a10, a13, 4203878e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387c9:	c0e9e0        	sub	a14, a9, a14
420387cc:	40fea0        	nsau	a10, a14
420387cf:	1b0c      	movi.n	a11, 1
420387d1:	41a5a0        	srli	a10, a10, 5
420387d4:	ce9c      	beqz.n	a14, 420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387d6:	c09940        	sub	a9, a9, a4
420387d9:	83ab90        	moveqz	a10, a11, a9
420387dc:	014a56        	bnez	a10, 420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387df:	c03830        	sub	a3, a8, a3
420387e2:	40f390        	nsau	a9, a3
420387e5:	419590        	srli	a9, a9, 5
420387e8:	008316        	beqz	a3, 420387f4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387eb:	c08850        	sub	a8, a8, a5
420387ee:	93b980        	movnez	a11, a9, a8
420387f1:	f99b16        	beqz	a11, 4203878e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387f4:	118dc0        	slli	a8, a13, 4
420387f7:	dd8a      	add.n	a13, a13, a8
420387f9:	7420d0        	extui	a2, a13, 0, 8
420387fc:	ffe386        	j	4203878e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387ff:	020c00        	andb	b0, b12, b0
42038802:	ffe206        	j	4203878e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038b48:	008136        	entry	a1, 64
42038b4b:	021292        	l16ui	a9, a2, 4
42038b4e:	7169      	s32i.n	a6, a1, 28
42038b50:	233380        	sext	a3, a3, 15
42038b53:	23a980        	sext	a10, a9, 15
42038b56:	027d      	mov.n	a7, a2
42038b58:	234480        	sext	a4, a4, 15
42038b5b:	1a23a7        	blt	a3, a10, 42038b79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b5e:	0412c2        	l16ui	a12, a2, 8
42038b61:	23ac80        	sext	a10, a12, 15
42038b64:	112a37        	blt	a10, a3, 42038b79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b67:	0312b2        	l16ui	a11, a2, 6
42038b6a:	23ab80        	sext	a10, a11, 15
42038b6d:	0824a7        	blt	a4, a10, 42038b79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b70:	0512d2        	l16ui	a13, a2, 10
42038b73:	23ad80        	sext	a10, a13, 15
42038b76:	12aa47        	bge	a10, a4, 42038b8c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x44>
42038b79:	04dd      	mov.n	a13, a4
42038b7b:	03cd      	mov.n	a12, a3
42038b7d:	3cc7b2        	addi	a11, a7, 60
42038b80:	28c7a2        	addi	a10, a7, 40
42038b83:	005aa5        	call8	4203912c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b86:	012a80        	slli	a2, a10, 24
42038b89:	f01d      	retw.n
42038b8b:	02a200        	andb	b10, b2, b0
42038b8e:	20          	.byte	0x20
42038b8f:	016142        	s32i	a4, a1, 4
42038b92:	006132        	s32i	a3, a1, 0
42038b95:	c0dda0        	sub	a13, a13, a10
42038b98:	c0cca0        	sub	a12, a12, a10
42038b9b:	80bab0        	add	a11, a10, a11
42038b9e:	2102f2        	l8ui	a15, a2, 33
42038ba1:	aa9a      	add.n	a10, a10, a9
42038ba3:	72e8      	l32i.n	a14, a2, 28
42038ba5:	23dd80        	sext	a13, a13, 15
42038ba8:	23cc80        	sext	a12, a12, 15
42038bab:	23bb80        	sext	a11, a11, 15
42038bae:	23aa80        	sext	a10, a10, 15
42038bb1:	ffb725        	call8	42038724 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bb4:	ffa092        	movi	a9, 255
42038bb7:	0a6d      	mov.n	a6, a10
42038bb9:	079a97        	bne	a10, a9, 42038bc4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x7c>
42038bbc:	142292        	l32i	a9, a2, 80
42038bbf:	052d      	mov.n	a2, a5
42038bc1:	fc4916        	beqz	a9, 42038b89 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038bc4:	04dd      	mov.n	a13, a4
42038bc6:	03cd      	mov.n	a12, a3
42038bc8:	3cc7b2        	addi	a11, a7, 60
42038bcb:	28c7a2        	addi	a10, a7, 40
42038bce:	0055e5        	call8	4203912c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038bd1:	200792        	l8ui	a9, a7, 32
42038bd4:	6169      	s32i.n	a6, a1, 24
42038bd6:	012a80        	slli	a2, a10, 24
42038bd9:	03e916        	beqz	a9, 42038c1b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038bdc:	1149      	s32i.n	a4, a1, 4
42038bde:	0139      	s32i.n	a3, a1, 0
42038be0:	67e8      	l32i.n	a14, a7, 24
42038be2:	0597d2        	l16si	a13, a7, 10
42038be5:	0497c2        	l16si	a12, a7, 8
42038be8:	0397b2        	l16si	a11, a7, 6
42038beb:	0297a2        	l16si	a10, a7, 4
42038bee:	ff0c      	movi.n	a15, 15
42038bf0:	ffb325        	call8	42038724 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bf3:	61a9      	s32i.n	a10, a1, 24
42038bf5:	2aac      	beqz.n	a10, 42038c1b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038bf7:	7188      	l32i.n	a8, a1, 28
42038bf9:	38cc      	bnez.n	a8, 42038c00 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb8>
42038bfb:	24c782        	addi	a8, a7, 36
42038bfe:	7189      	s32i.n	a8, a1, 28
42038c00:	7188      	l32i.n	a8, a1, 28
42038c02:	02ad      	mov.n	a10, a2
42038c04:	002892        	l32i	a9, a8, 0
42038c07:	062182        	l32i	a8, a1, 24
42038c0a:	119980        	slli	a9, a9, 8
42038c0d:	419890        	srli	a9, a9, 8
42038c10:	01b880        	slli	a11, a8, 24
42038c13:	20b9b0        	or	a11, a9, a11
42038c16:	cec3e5        	call8	42007854 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c19:	0a2d      	mov.n	a2, a10
42038c1b:	ffa092        	movi	a9, 255
42038c1e:	469697        	bne	a6, a9, 42038c68 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x120>
42038c21:	fb7c      	movi.n	a11, -1
42038c23:	01bb80        	slli	a11, a11, 24
42038c26:	20b5b0        	or	a11, a5, a11
42038c29:	02ad      	mov.n	a10, a2
42038c2b:	cec2a5        	call8	42007854 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c2e:	0a2d      	mov.n	a2, a10
42038c30:	1427a2        	l32i	a10, a7, 80
42038c33:	f52a16        	beqz	a10, 42038b89 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c36:	20c440        	or	a12, a4, a4
42038c39:	20b330        	or	a11, a3, a3
42038c3c:	ffdd65        	call8	42038a14 <roo_windows::PressOverlay::get(short, short) const>
42038c3f:	6198      	l32i.n	a9, a1, 24
42038c41:	7588a0        	extui	a8, a10, 24, 8
42038c44:	d18890        	mul16s	a8, a8, a9
42038c47:	1f2691        	l32r	a9, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c4a:	11aa80        	slli	a10, a10, 8
42038c4d:	a28890        	muluh	a8, a8, a9
42038c50:	0188f0        	slli	a8, a8, 17
42038c53:	758880        	extui	a8, a8, 24, 8
42038c56:	404800        	ssai	8
42038c59:	81b8a0        	src	a11, a8, a10
42038c5c:	02ad      	mov.n	a10, a2
42038c5e:	cebf65        	call8	42007854 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c61:	0a2d      	mov.n	a2, a10
42038c63:	ffc886        	j	42038b89 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c66:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038c69:	50fc46        	j	4204d05e <uart_get_word_length+0xa>
42038c6c:	7598      	l32i.n	a9, a5, 28
42038c6e:	d19960        	mul16s	a9, a9, a6
42038c71:	1f1ba1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c74:	7fc992        	addi	a9, a9, 127
42038c77:	a299a0        	muluh	a9, a9, a10
42038c7a:	11b580        	slli	a11, a5, 8
42038c7d:	0199f0        	slli	a9, a9, 17
42038c80:	759890        	extui	a9, a9, 24, 8
42038c83:	404800        	ssai	8
42038c86:	81b9b0        	src	a11, a9, a11
42038c89:	ffe706        	j	42038c29 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xe1>


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038d38 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038d38:	00c136        	entry	a1, 96
42038d3b:	078d      	mov.n	a8, a7
42038d3d:	069d      	mov.n	a9, a6
42038d3f:	086d      	mov.n	a6, a8
42038d41:	238980        	sext	a8, a9, 15
42038d44:	3189      	s32i.n	a8, a1, 12
42038d46:	142282        	l32i	a8, a2, 80
42038d49:	027d      	mov.n	a7, a2
42038d4b:	233380        	sext	a3, a3, 15
42038d4e:	234480        	sext	a4, a4, 15
42038d51:	235580        	sext	a5, a5, 15
42038d54:	200856        	bnez	a8, 42038f58 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038d57:	0288      	l32i.n	a8, a2, 0
42038d59:	31e8      	l32i.n	a14, a1, 12
42038d5b:	b888      	l32i.n	a8, a8, 44
42038d5d:	02ad      	mov.n	a10, a2
42038d5f:	06fd      	mov.n	a15, a6
42038d61:	05dd      	mov.n	a13, a5
42038d63:	04cd      	mov.n	a12, a4
42038d65:	03bd      	mov.n	a11, a3
42038d67:	0008e0        	callx8	a8
42038d6a:	0a2d      	mov.n	a2, a10
42038d6c:	09ca56        	bnez	a10, 42038e0c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd4>
42038d6f:	249792        	l16si	a9, a7, 72
42038d72:	1a9782        	l16si	a8, a7, 52
42038d75:	ec0c      	movi.n	a12, 14
42038d77:	538890        	max	a8, a8, a9
42038d7a:	98fb      	addi.n	a9, a8, 15
42038d7c:	1ec882        	addi	a8, a8, 30
42038d7f:	b38990        	movgez	a8, a9, a9
42038d82:	21b480        	srai	a11, a8, 4
42038d85:	210782        	l8ui	a8, a7, 33
42038d88:	1907a2        	l8ui	a10, a7, 25
42038d8b:	62cc80        	saltu	a12, a12, a8
42038d8e:	180782        	l8ui	a8, a7, 24
42038d91:	200792        	l8ui	a9, a7, 32
42038d94:	7388a0        	maxu	a8, a8, a10
42038d97:	1a07a2        	l8ui	a10, a7, 26
42038d9a:	cc0b      	addi.n	a12, a12, -1
42038d9c:	7388a0        	maxu	a8, a8, a10
42038d9f:	1b07a2        	l8ui	a10, a7, 27
42038da2:	60c0c0        	neg	a12, a12
42038da5:	73a8a0        	maxu	a10, a8, a10
42038da8:	89ca      	add.n	a8, a9, a12
42038daa:	99ba      	add.n	a9, a9, a11
42038dac:	538a80        	max	a8, a10, a8
42038daf:	99ca      	add.n	a9, a9, a12
42038db1:	0297a2        	l16si	a10, a7, 4
42038db4:	539980        	max	a9, a9, a8
42038db7:	aa9a      	add.n	a10, a10, a9
42038db9:	53aa30        	max	a10, a10, a3
42038dbc:	11a9      	s32i.n	a10, a1, 4
42038dbe:	0497a2        	l16si	a10, a7, 8
42038dc1:	c09a90        	sub	a9, a10, a9
42038dc4:	439950        	min	a9, a9, a5
42038dc7:	5199      	s32i.n	a9, a1, 20
42038dc9:	039792        	l16si	a9, a7, 6
42038dcc:	998a      	add.n	a9, a9, a8
42038dce:	99ba      	add.n	a9, a9, a11
42038dd0:	9199      	s32i.n	a9, a1, 36
42038dd2:	059792        	l16si	a9, a7, 10
42038dd5:	c08980        	sub	a8, a9, a8
42038dd8:	4189      	s32i.n	a8, a1, 16
42038dda:	c08530        	sub	a8, a5, a3
42038ddd:	881b      	addi.n	a8, a8, 1
42038ddf:	6189      	s32i.n	a8, a1, 24
42038de1:	5188      	l32i.n	a8, a1, 20
42038de3:	1198      	l32i.n	a9, a1, 4
42038de5:	881b      	addi.n	a8, a8, 1
42038de7:	7189      	s32i.n	a8, a1, 28
42038de9:	c08890        	sub	a8, a8, a9
42038dec:	1188e0        	slli	a8, a8, 2
42038def:	a189      	s32i.n	a8, a1, 40
42038df1:	6188      	l32i.n	a8, a1, 24
42038df3:	1188e0        	slli	a8, a8, 2
42038df6:	2189      	s32i.n	a8, a1, 8
42038df8:	c08930        	sub	a8, a9, a3
42038dfb:	1188e0        	slli	a8, a8, 2
42038dfe:	b189      	s32i.n	a8, a1, 44
42038e00:	029182        	l16si	a8, a1, 4
42038e03:	8189      	s32i.n	a8, a1, 32
42038e05:	3188      	l32i.n	a8, a1, 12
42038e07:	03a847        	bge	a8, a4, 42038e0e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd6>
42038e0a:	020c      	movi.n	a2, 0
42038e0c:	f01d      	retw.n
42038e0e:	5198      	l32i.n	a9, a1, 20
42038e10:	11a8      	l32i.n	a10, a1, 4
42038e12:	038d      	mov.n	a8, a3
42038e14:	062d      	mov.n	a2, a6
42038e16:	03bd      	mov.n	a11, a3
42038e18:	23c480        	sext	a12, a4, 15
42038e1b:	0229a7        	blt	a9, a10, 42038e21 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038e1e:	002406        	j	42038eb2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x17a>
42038e21:	029d      	mov.n	a9, a2
42038e23:	08bd      	mov.n	a11, a8
42038e25:	23c480        	sext	a12, a4, 15
42038e28:	0225b7        	blt	a5, a11, 42038e2e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf6>
42038e2b:	002b46        	j	42038edc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a4>
42038e2e:	a51b      	addi.n	a10, a5, 1
42038e30:	729a80        	salt	a9, a10, a8
42038e33:	990b      	addi.n	a9, a9, -1
42038e35:	609090        	neg	a9, a9
42038e38:	749090        	extui	a9, a9, 0, 8
42038e3b:	0b0c      	movi.n	a11, 0
42038e3d:	498c      	beqz.n	a9, 42038e45 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10d>
42038e3f:	c0aa80        	sub	a10, a10, a8
42038e42:	11bae0        	slli	a11, a10, 2
42038e45:	9198      	l32i.n	a9, a1, 36
42038e47:	22ba      	add.n	a2, a2, a11
42038e49:	729490        	salt	a9, a4, a9
42038e4c:	990b      	addi.n	a9, a9, -1
42038e4e:	609090        	neg	a9, a9
42038e51:	749090        	extui	a9, a9, 0, 8
42038e54:	028d      	mov.n	a8, a2
42038e56:	79bc      	beqz.n	a9, 42038e91 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038e58:	4198      	l32i.n	a9, a1, 16
42038e5a:	729940        	salt	a9, a9, a4
42038e5d:	990b      	addi.n	a9, a9, -1
42038e5f:	609090        	neg	a9, a9
42038e62:	749090        	extui	a9, a9, 0, 8
42038e65:	89ac      	beqz.n	a9, 42038e91 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038e67:	3198      	l32i.n	a9, a1, 12
42038e69:	41a8      	l32i.n	a10, a1, 16
42038e6b:	04dd      	mov.n	a13, a4
42038e6d:	4399a0        	min	a9, a9, a10
42038e70:	02ad97        	bge	a13, a9, 42038e76 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x13e>
42038e73:	0022c6        	j	42038f02 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038e76:	728940        	salt	a8, a9, a4
42038e79:	880b      	addi.n	a8, a8, -1
42038e7b:	608080        	neg	a8, a8
42038e7e:	748080        	extui	a8, a8, 0, 8
42038e81:	0a0c      	movi.n	a10, 0
42038e83:	188c      	beqz.n	a8, 42038e88 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x150>
42038e85:	c0a940        	sub	a10, a9, a4
42038e88:	2188      	l32i.n	a8, a1, 8
42038e8a:	44aa      	add.n	a4, a4, a10
42038e8c:	828a80        	mull	a8, a10, a8
42038e8f:	882a      	add.n	a8, a8, a2
42038e91:	441b      	addi.n	a4, a4, 1
42038e93:	086d      	mov.n	a6, a8
42038e95:	ffdb06        	j	42038e05 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcd>
42038e98:	0527d2        	l32i	a13, a7, 20
42038e9b:	00a0e2        	movi	a14, 0
42038e9e:	20a770        	or	a10, a7, a7
42038ea1:	d1c9      	s32i.n	a12, a1, 52
42038ea3:	c1b9      	s32i.n	a11, a1, 48
42038ea5:	ffca25        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ea8:	c1b8      	l32i.n	a11, a1, 48
42038eaa:	d1c8      	l32i.n	a12, a1, 52
42038eac:	02a9      	s32i.n	a10, a2, 0
42038eae:	bb1b      	addi.n	a11, a11, 1
42038eb0:	224b      	addi.n	a2, a2, 4
42038eb2:	012182        	l32i	a8, a1, 4
42038eb5:	df2b87        	blt	a11, a8, 42038e98 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x160>
42038eb8:	0b2182        	l32i	a8, a1, 44
42038ebb:	0527d2        	l32i	a13, a7, 20
42038ebe:	81b8      	l32i.n	a11, a1, 32
42038ec0:	0e0c      	movi.n	a14, 0
42038ec2:	07ad      	mov.n	a10, a7
42038ec4:	268a      	add.n	a2, a6, a8
42038ec6:	ffc825        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ec9:	a188      	l32i.n	a8, a1, 40
42038ecb:	828a      	add.n	a8, a2, a8
42038ecd:	049827        	bne	a8, a2, 42038ed5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x19d>
42038ed0:	7188      	l32i.n	a8, a1, 28
42038ed2:	ffd2c6        	j	42038e21 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038ed5:	02a9      	s32i.n	a10, a2, 0
42038ed7:	224b      	addi.n	a2, a2, 4
42038ed9:	fffc06        	j	42038ecd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x195>
42038edc:	0527d2        	l32i	a13, a7, 20
42038edf:	00a0e2        	movi	a14, 0
42038ee2:	20a770        	or	a10, a7, a7
42038ee5:	f199      	s32i.n	a9, a1, 60
42038ee7:	e189      	s32i.n	a8, a1, 56
42038ee9:	d1c9      	s32i.n	a12, a1, 52
42038eeb:	c1b9      	s32i.n	a11, a1, 48
42038eed:	ffc5a5        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ef0:	f198      	l32i.n	a9, a1, 60
42038ef2:	c1b8      	l32i.n	a11, a1, 48
42038ef4:	09a9      	s32i.n	a10, a9, 0
42038ef6:	e188      	l32i.n	a8, a1, 56
42038ef8:	d1c8      	l32i.n	a12, a1, 52
42038efa:	bb1b      	addi.n	a11, a11, 1
42038efc:	994b      	addi.n	a9, a9, 4
42038efe:	ffc986        	j	42038e28 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf0>
42038f01:	00          	.byte	00
42038f02:	61a8      	l32i.n	a10, a1, 24
42038f04:	1a1aa6        	blti	a10, 1, 42038f22 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038f07:	21a8      	l32i.n	a10, a1, 8
42038f09:	1f4a26        	beqi	a10, 4, 42038f2c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f4>
42038f0c:	0acd      	mov.n	a12, a10
42038f0e:	06bd      	mov.n	a11, a6
42038f10:	08ad      	mov.n	a10, a8
42038f12:	d1d9      	s32i.n	a13, a1, 52
42038f14:	c199      	s32i.n	a9, a1, 48
42038f16:	1e7381        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038f19:	0008e0        	callx8	a8
42038f1c:	c198      	l32i.n	a9, a1, 48
42038f1e:	d1d8      	l32i.n	a13, a1, 52
42038f20:	0a8d      	mov.n	a8, a10
42038f22:	21a8      	l32i.n	a10, a1, 8
42038f24:	dd1b      	addi.n	a13, a13, 1
42038f26:	88aa      	add.n	a8, a8, a10
42038f28:	ffd106        	j	42038e70 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x138>
42038f2b:	06a800        	ee.ldf.64.xp	f10, f0, a0, a8
42038f2e:	08a9      	s32i.n	a10, a8, 0
42038f30:	fffb86        	j	42038f22 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038f33:	57d800        	ee.stf.64.xp	f13, f5, a0, a8
42038f36:	02bd      	mov.n	a11, a2
42038f38:	0e0c      	movi.n	a14, 0
42038f3a:	04cd      	mov.n	a12, a4
42038f3c:	07ad      	mov.n	a10, a7
42038f3e:	1189      	s32i.n	a8, a1, 4
42038f40:	ffc065        	call8	42038b48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038f43:	1188      	l32i.n	a8, a1, 4
42038f45:	221b      	addi.n	a2, a2, 1
42038f47:	08a9      	s32i.n	a10, a8, 0
42038f49:	232280        	sext	a2, a2, 15
42038f4c:	884b      	addi.n	a8, a8, 4
42038f4e:	086d      	mov.n	a6, a8
42038f50:	e0a527        	bge	a5, a2, 42038f34 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1fc>
42038f53:	441b      	addi.n	a4, a4, 1
42038f55:	234480        	sext	a4, a4, 15
42038f58:	3188      	l32i.n	a8, a1, 12
42038f5a:	02a847        	bge	a8, a4, 42038f60 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038f5d:	ffaa46        	j	42038e0a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd2>
42038f60:	068d      	mov.n	a8, a6
42038f62:	032d      	mov.n	a2, a3
42038f64:	fff986        	j	42038f4e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/retained_sides/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203912c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
4203912c:	006136        	entry	a1, 48
4203912f:	026d      	mov.n	a6, a2
42039131:	0f0222        	l8ui	a2, a2, 15
42039134:	234480        	sext	a4, a4, 15
42039137:	239580        	sext	a9, a5, 15
4203913a:	22b216        	beqz	a2, 42039369 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23d>
4203913d:	009682        	l16si	a8, a6, 0
42039140:	0696e2        	l16si	a14, a6, 12
42039143:	c04480        	sub	a4, a4, a8
42039146:	019682        	l16si	a8, a6, 2
42039149:	0396b2        	l16si	a11, a6, 6
4203914c:	c09980        	sub	a9, a9, a8
4203914f:	1199c0        	slli	a9, a9, 4
42039152:	998b      	addi.n	a9, a9, 8
42039154:	069382        	l16si	a8, a3, 12
42039157:	11a4c0        	slli	a10, a4, 4
4203915a:	c049e0        	sub	a4, a9, a14
4203915d:	11bbc0        	slli	a11, a11, 4
42039160:	d44a      	add.n	a13, a4, a4
42039162:	c09980        	sub	a9, a9, a8
42039165:	728db0        	salt	a8, a13, a11
42039168:	029652        	l16si	a5, a6, 4
4203916b:	880b      	addi.n	a8, a8, -1
4203916d:	608080        	neg	a8, a8
42039170:	f99a      	add.n	a15, a9, a9
42039172:	aa8b      	addi.n	a10, a10, 8
42039174:	748080        	extui	a8, a8, 0, 8
42039177:	1155c0        	slli	a5, a5, 4
4203917a:	72cfb0        	salt	a12, a15, a11
4203917d:	c075a0        	sub	a7, a5, a10
42039180:	3189      	s32i.n	a8, a1, 12
42039182:	cc0b      	addi.n	a12, a12, -1
42039184:	c08b40        	sub	a8, a11, a4
42039187:	60c0c0        	neg	a12, a12
4203918a:	4377a0        	min	a7, a7, a10
4203918d:	438840        	min	a8, a8, a4
42039190:	aaaa      	add.n	a10, a10, a10
42039192:	c04b90        	sub	a4, a11, a9
42039195:	74c0c0        	extui	a12, a12, 0, 8
42039198:	434490        	min	a4, a4, a9
4203919b:	622a57        	blt	a10, a5, 42039201 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd5>
4203919e:	562db7        	blt	a13, a11, 420391f8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xcc>
420391a1:	0a0692        	l8ui	a9, a6, 10
420391a4:	31a8      	l32i.n	a10, a1, 12
420391a6:	621ac7        	beq	a10, a12, 4203920c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe0>
420391a9:	6b2fb7        	blt	a15, a11, 42039218 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xec>
420391ac:	0a0652        	l8ui	a5, a6, 10
420391af:	23f980        	sext	a15, a9, 15
420391b2:	11ffc0        	slli	a15, a15, 4
420391b5:	7828f7        	blt	a8, a15, 42039231 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x105>
420391b8:	23a580        	sext	a10, a5, 15
420391bb:	11aac0        	slli	a10, a10, 4
420391be:	6f24a7        	blt	a4, a10, 42039231 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x105>
420391c1:	43a840        	min	a10, a8, a4
420391c4:	692a77        	blt	a10, a7, 42039231 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x105>
420391c7:	f8c772        	addi	a7, a7, -8
420391ca:	607070        	neg	a7, a7
420391cd:	217470        	srai	a7, a7, 4
420391d0:	086c      	movi.n	a8, -32
420391d2:	02a787        	bge	a7, a8, 420391d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xac>
420391d5:	0069c6        	j	42039380 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x254>
420391d8:	082c      	movi.n	a8, 32
420391da:	02a877        	bge	a8, a7, 420391e0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb4>
420391dd:	006946        	j	42039386 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25a>
420391e0:	220b      	addi.n	a2, a2, -1
420391e2:	902220        	addx2	a2, a2, a2
420391e5:	1182a0        	slli	a8, a2, 6
420391e8:	228a      	add.n	a2, a2, a8
420391ea:	1dc281        	l32r	a8, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420391ed:	887a      	add.n	a8, a8, a7
420391ef:	882a      	add.n	a8, a8, a2
420391f1:	200822        	l8ui	a2, a8, 32
420391f4:	005c46        	j	42039369 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23d>
420391f7:	069200        	ee.ldf.64.xp	f9, f0, a0, a2
420391fa:	4609      	s32i.n	a0, a6, 16
420391fc:	ffe9      	s32i.n	a14, a15, 60
420391fe:	000000        	ill
42039201:	0c2db7        	blt	a13, a11, 42039211 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe5>
42039204:	0b0692        	l8ui	a9, a6, 11
42039207:	31a8      	l32i.n	a10, a1, 12
42039209:	139ac7        	bne	a10, a12, 42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf4>
4203920c:	095d      	mov.n	a5, a9
4203920e:	ffe746        	j	420391af <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x83>
42039211:	080692        	l8ui	a9, a6, 8
42039214:	fffbc6        	j	42039207 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdb>
42039217:	065200        	ee.ldf.64.xp	f5, f0, a0, a2
4203921a:	0609      	s32i.n	a0, a6, 0
4203921c:	e4          	.byte	0xe4
4203921d:	b70000ff 	ee.vmulas.s16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
42039221:	0652052f 	ee.vmulas.s16.accx.ld.ip.qup	q4, a2, 0x150, q4, q0, q5, q2
42039225:	460b      	addi.n	a4, a6, -1
42039227:	52ffe1        	l32r	a14, 4200de24 <std::vector<roo_display::BlendingMode, std::allocator<roo_display::BlendingMode> >::_M_check_len(unsigned int, char const*) const+0x44> (3c090c27 <_flash_rodata_start+0xb07>)
4203922a:	c60806        	j	4202aa4e <roo_windows::DisplayWindow::refreshPaint()+0xba>
4203922d:	0000ffdf 	ee.vmulas.s16.accx.ld.ip.qup	q0, a13, 240, q7, q6, q0, q0
42039231:	31a8      	l32i.n	a10, a1, 12
42039233:	529ac7        	bne	a10, a12, 42039289 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x15d>
42039236:	539590        	max	a9, a5, a9
42039239:	239980        	sext	a9, a9, 15
4203923c:	1199c0        	slli	a9, a9, 4
4203923f:	462797        	blt	a7, a9, 42039289 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x15d>
42039242:	539840        	max	a9, a8, a4
42039245:	402797        	blt	a7, a9, 42039289 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x15d>
42039248:	f8c882        	addi	a8, a8, -8
4203924b:	608080        	neg	a8, a8
4203924e:	02adb7        	bge	a13, a11, 42039254 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x128>
42039251:	60e0e0        	neg	a14, a14
42039254:	88ea      	add.n	a8, a8, a14
42039256:	218480        	srai	a8, a8, 4
42039259:	190c      	movi.n	a9, 1
4203925b:	012db7        	blt	a13, a11, 42039260 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
4203925e:	290c      	movi.n	a9, 2
42039260:	0a6c      	movi.n	a10, -32
42039262:	02a8a7        	bge	a8, a10, 42039268 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x13c>
42039265:	0045c6        	j	42039380 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x254>
42039268:	0a2c      	movi.n	a10, 32
4203926a:	02aa87        	bge	a10, a8, 42039270 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x144>
4203926d:	004546        	j	42039386 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25a>
42039270:	220b      	addi.n	a2, a2, -1
42039272:	902220        	addx2	a2, a2, a2
42039275:	229a      	add.n	a2, a2, a9
42039277:	1d9f91        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
4203927a:	11a2a0        	slli	a10, a2, 6
4203927d:	aa2a      	add.n	a10, a10, a2
4203927f:	898a      	add.n	a8, a9, a8
42039281:	88aa      	add.n	a8, a8, a10
42039283:	ffda86        	j	420391f1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc5>
42039286:	000000        	ill
42039289:	0916b2        	l16ui	a11, a6, 18
4203928c:	439780        	min	a9, a7, a8
4203928f:	60a0b0        	neg	a10, a11
42039292:	020c      	movi.n	a2, 0
42039294:	5729a7        	blt	a9, a10, 420392ef <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c3>
42039297:	c0af70        	sub	a10, a15, a7
4203929a:	c09f80        	sub	a9, a15, a8
4203929d:	0222a7        	blt	a2, a10, 420392a3 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x177>
420392a0:	003206        	j	4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
420392a3:	022297        	blt	a2, a9, 420392a9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x17d>
420392a6:	003086        	j	4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
420392a9:	8fba      	add.n	a8, a15, a11
420392ab:	829990        	mull	a9, a9, a9
420392ae:	82aaa0        	mull	a10, a10, a10
420392b1:	881b      	addi.n	a8, a8, 1
420392b3:	828880        	mull	a8, a8, a8
420392b6:	80a9a0        	add	a10, a9, a10
420392b9:	0361f2        	s32i	a15, a1, 12
420392bc:	2fba87        	bgeu	a10, a8, 420392ef <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c3>
420392bf:	4d34e5        	call8	4208660c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
420392c2:	0321f2        	l32i	a15, a1, 12
420392c5:	0e0692        	l8ui	a9, a6, 14
420392c8:	c08af0        	sub	a8, a10, a15
420392cb:	c08890        	sub	a8, a8, a9
420392ce:	081692        	l16ui	a9, a6, 16
420392d1:	828890        	mull	a8, a8, a9
420392d4:	1d8a91        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
420392d7:	889a      	add.n	a8, a8, a9
420392d9:	218c80        	srai	a8, a8, 12
420392dc:	ffa092        	movi	a9, 255
420392df:	438890        	min	a8, a8, a9
420392e2:	090c      	movi.n	a9, 0
420392e4:	538890        	max	a8, a8, a9
420392e7:	1d8491        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
420392ea:	898a      	add.n	a8, a9, a8
420392ec:	000822        	l8ui	a2, a8, 0
420392ef:	091392        	l16ui	a9, a3, 18
420392f2:	43a740        	min	a10, a7, a4
420392f5:	60b090        	neg	a11, a9
420392f8:	080c      	movi.n	a8, 0
420392fa:	522ab7        	blt	a10, a11, 42039350 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x224>
420392fd:	235580        	sext	a5, a5, 15
42039300:	1155c0        	slli	a5, a5, 4
42039303:	c0a570        	sub	a10, a5, a7
42039306:	c0b540        	sub	a11, a5, a4
42039309:	6ba8a7        	bge	a8, a10, 42039378 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24c>
4203930c:	68a8b7        	bge	a8, a11, 42039378 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24c>
4203930f:	995a      	add.n	a9, a9, a5
42039311:	82aaa0        	mull	a10, a10, a10
42039314:	82bbb0        	mull	a11, a11, a11
42039317:	991b      	addi.n	a9, a9, 1
42039319:	829990        	mull	a9, a9, a9
4203931c:	80aab0        	add	a10, a10, a11
4203931f:	2dba97        	bgeu	a10, a9, 42039350 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x224>
42039322:	4d2ea5        	call8	4208660c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039325:	0e0392        	l8ui	a9, a3, 14
42039328:	c08a50        	sub	a8, a10, a5
4203932b:	c08890        	sub	a8, a8, a9
4203932e:	081392        	l16ui	a9, a3, 16
42039331:	828890        	mull	a8, a8, a9
42039334:	1d7291        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
42039337:	889a      	add.n	a8, a8, a9
42039339:	218c80        	srai	a8, a8, 12
4203933c:	ffa092        	movi	a9, 255
4203933f:	438890        	min	a8, a8, a9
42039342:	090c      	movi.n	a9, 0
42039344:	538890        	max	a8, a8, a9
42039347:	1d6e91        	l32r	a9, 42000900 <_stext+0x8e0> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
4203934a:	808980        	add	a8, a9, a8
4203934d:	000882        	l8ui	a8, a8, 0
42039350:	d19280        	mul16s	a9, a2, a8
42039353:	1d63a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42039356:	7fc992        	addi	a9, a9, 127
42039359:	b2a9a0        	mulsh	a10, a9, a10
4203935c:	282a      	add.n	a2, a8, a2
4203935e:	99aa      	add.n	a9, a9, a10
42039360:	219790        	srai	a9, a9, 7
42039363:	c02290        	sub	a2, a2, a9
42039366:	742020        	extui	a2, a2, 0, 8
42039369:	f01d      	retw.n
4203936b:	00          	.byte	00
4203936c:	022877        	blt	a8, a7, 42039372 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x246>
4203936f:	ffd486        	j	420392c5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x199>
42039372:	09ad      	mov.n	a10, a9
42039374:	ffd346        	j	420392c5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x199>
42039377:	a47700        	extui	a7, a0, 7, 11
4203937a:	ada9      	s32i.n	a10, a13, 40
4203937c:	060b      	addi.n	a0, a6, -1
4203937e:	ffe9      	s32i.n	a14, a15, 60
42039380:	68a022        	movi	a2, 104
42039383:	fff886        	j	42039369 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23d>
42039386:	020c      	movi.n	a2, 0
42039388:	fff746        	j	42039369 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23d>
