
/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


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


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420865e4 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
420865e4:	004136        	entry	a1, 32
420865e7:	029d      	mov.n	a9, a2
420865e9:	120c      	movi.n	a2, 1
420865eb:	832990        	moveqz	a2, a9, a9
420865ee:	4149b6        	bltui	a9, 4, 42086633 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
420865f1:	40f980        	nsau	a8, a9
420865f4:	dfc882        	addi	a8, a8, -33
420865f7:	608080        	neg	a8, a8
420865fa:	218180        	srai	a8, a8, 1
420865fd:	fec882        	addi	a8, a8, -2
42086600:	3a0c      	movi.n	a10, 3
42086602:	401800        	ssl	a8
42086605:	a1aa00        	sll	a10, a10
42086608:	c289a0        	quou	a8, a9, a10
4208660b:	88aa      	add.n	a8, a8, a10
4208660d:	418180        	srli	a8, a8, 1
42086610:	c2a980        	quou	a10, a9, a8
42086613:	aa8a      	add.n	a10, a10, a8
42086615:	41a1a0        	srli	a10, a10, 1
42086618:	c289a0        	quou	a8, a9, a10
4208661b:	88aa      	add.n	a8, a8, a10
4208661d:	1a0c      	movi.n	a10, 1
4208661f:	418180        	srli	a8, a8, 1
42086622:	11aa00        	slli	a10, a10, 16
42086625:	0518a7        	beq	a8, a10, 4208662e <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
42086628:	82a880        	mull	a10, a8, a8
4208662b:	01b9a7        	bgeu	a9, a10, 42086630 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
4208662e:	880b      	addi.n	a8, a8, -1
42086630:	f42080        	extui	a2, a8, 0, 16
42086633:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c64 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c64:	006136        	entry	a1, 48
42038c67:	038d      	mov.n	a8, a3
42038c69:	06dd      	mov.n	a13, a6
42038c6b:	059d      	mov.n	a9, a5
42038c6d:	043d      	mov.n	a3, a4
42038c6f:	087d      	mov.n	a7, a8
42038c71:	1f49f6        	bgeui	a9, 4, 42038c94 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x30>
42038c74:	419250        	srli	a9, a5, 2
42038c77:	11a9c0        	slli	a10, a9, 4
42038c7a:	6daa      	add.n	a6, a13, a10
42038c7c:	b0a980        	addx8	a10, a9, a8
42038c7f:	1199e0        	slli	a9, a9, 2
42038c82:	c05590        	sub	a5, a5, a9
42038c85:	a05560        	addx4	a5, a5, a6
42038c88:	c04480        	sub	a4, a4, a8
42038c8b:	8a4a      	add.n	a8, a10, a4
42038c8d:	509657        	bne	a6, a5, 42038ce1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x7d>
42038c90:	f01d      	retw.n
42038c92:	c20000        	quou	a0, a0, a0
42038c95:	b20093        	lsi	f9, a0, 0x2c8
42038c98:	ad0097        	bnone	a0, a9, 42038c49 <roo_windows::Decoration::read(short, short) const+0x1>
42038c9b:	31d902        	addmi	a0, a9, 0x3100
42038c9e:	2189      	s32i.n	a8, a1, 8
42038ca0:	1199      	s32i.n	a9, a1, 4
42038ca2:	fffa65        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038ca5:	0193c2        	l16si	a12, a3, 2
42038ca8:	0197b2        	l16si	a11, a7, 2
42038cab:	06a9      	s32i.n	a10, a6, 0
42038cad:	02ad      	mov.n	a10, a2
42038caf:	fff9a5        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038cb2:	0293c2        	l16si	a12, a3, 4
42038cb5:	0297b2        	l16si	a11, a7, 4
42038cb8:	16a9      	s32i.n	a10, a6, 4
42038cba:	02ad      	mov.n	a10, a2
42038cbc:	fff8a5        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038cbf:	0393c2        	l16si	a12, a3, 6
42038cc2:	0397b2        	l16si	a11, a7, 6
42038cc5:	26a9      	s32i.n	a10, a6, 8
42038cc7:	02ad      	mov.n	a10, a2
42038cc9:	fff7e5        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038ccc:	1198      	l32i.n	a9, a1, 4
42038cce:	36a9      	s32i.n	a10, a6, 12
42038cd0:	31d8      	l32i.n	a13, a1, 12
42038cd2:	2188      	l32i.n	a8, a1, 8
42038cd4:	778b      	addi.n	a7, a7, 8
42038cd6:	338b      	addi.n	a3, a3, 8
42038cd8:	10c662        	addi	a6, a6, 16
42038cdb:	fcc992        	addi	a9, a9, -4
42038cde:	ffe3c6        	j	42038c71 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xd>
42038ce1:	009ab2        	l16si	a11, a10, 0
42038ce4:	0098c2        	l16si	a12, a8, 0
42038ce7:	02ca72        	addi	a7, a10, 2
42038cea:	20a220        	or	a10, a2, a2
42038ced:	fff5a5        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038cf0:	0066a2        	s32i	a10, a6, 0
42038cf3:	664b      	addi.n	a6, a6, 4
42038cf5:	07ad      	mov.n	a10, a7
42038cf7:	ffe406        	j	42038c8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x27>


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42086444 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
42086444:	004136        	entry	a1, 32
42086447:	208230        	or	a8, a2, a3
4208644a:	0228d6        	bgez	a8, 42086470 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
4208644d:	c02240        	sub	a2, a2, a4
42086450:	40f290        	nsau	a9, a2
42086453:	419590        	srli	a9, a9, 5
42086456:	b28c      	beqz.n	a2, 42086465 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
42086458:	c03340        	sub	a3, a3, a4
4208645b:	180c      	movi.n	a8, 1
4208645d:	938930        	movnez	a8, a9, a3
42086460:	ffa022        	movi	a2, 255
42086463:	688c      	beqz.n	a8, 4208646d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086465:	1125c0        	slli	a2, a5, 4
42086468:	252a      	add.n	a2, a5, a2
4208646a:	742020        	extui	a2, a2, 0, 8
4208646d:	f01d      	retw.n
4208646f:	222000        	orb	b2, b0, b0
42086472:	3330d1        	l32r	a13, 42053134 <spi_device_acquire_bus+0x5c> (3aa5e2a6 <UserFrameTotalSize+0x3aa5e1a6>)
42086475:	823ad1        	l32r	a13, 42066d60 <esp_littlefs_free_fd$isra$0+0x48> (880b3d12 <_rtc_reserved_end+0x27fb3d12>)
42086478:	118880        	slli	a8, a8, 8
4208647b:	ffa022        	movi	a2, 255
4208647e:	eb48a6        	blti	a8, 4, 4208646d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086481:	40f890        	nsau	a9, a8
42086484:	dfc992        	addi	a9, a9, -33
42086487:	609090        	neg	a9, a9
4208648a:	219190        	srai	a9, a9, 1
4208648d:	fec992        	addi	a9, a9, -2
42086490:	3a0c      	movi.n	a10, 3
42086492:	401900        	ssl	a9
42086495:	a1aa00        	sll	a10, a10
42086498:	c298a0        	quou	a9, a8, a10
4208649b:	99aa      	add.n	a9, a9, a10
4208649d:	419190        	srli	a9, a9, 1
420864a0:	c2a890        	quou	a10, a8, a9
420864a3:	aa9a      	add.n	a10, a10, a9
420864a5:	41a1a0        	srli	a10, a10, 1
420864a8:	c298a0        	quou	a9, a8, a10
420864ab:	99aa      	add.n	a9, a9, a10
420864ad:	1a0c      	movi.n	a10, 1
420864af:	419190        	srli	a9, a9, 1
420864b2:	11aa00        	slli	a10, a10, 16
420864b5:	0519a7        	beq	a9, a10, 420864be <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7a>
420864b8:	82a990        	mull	a10, a9, a9
420864bb:	01b8a7        	bgeu	a8, a10, 420864c0 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x7c>
420864be:	990b      	addi.n	a9, a9, -1
420864c0:	1144c0        	slli	a4, a4, 4
420864c3:	554a      	add.n	a5, a5, a4
420864c5:	c05590        	sub	a5, a5, a9
420864c8:	f48050        	extui	a8, a5, 0, 16
420864cb:	239580        	sext	a9, a5, 15
420864ce:	020c      	movi.n	a2, 0
420864d0:	99e5f7        	bbsi	a5, 15, 4208646d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420864d3:	fa0c      	movi.n	a10, 15
420864d5:	ffa022        	movi	a2, 255
420864d8:	912a97        	blt	a10, a9, 4208646d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420864db:	1198c0        	slli	a9, a8, 4
420864de:	889a      	add.n	a8, a8, a9
420864e0:	742080        	extui	a2, a8, 0, 8
420864e3:	ffe186        	j	4208646d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


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
42033f4f:	04b7e5        	call8	42038acc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
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
42033f71:	3ccfe5        	call8	42070c70 <_Unwind_Resume>
42033f74:	fffac6        	j	42033f63 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


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
42038789:	4dcba5        	call8	42086444 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
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


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038acc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038acc:	008136        	entry	a1, 64
42038acf:	2002a2        	l8ui	a10, a2, 32
42038ad2:	021292        	l16ui	a9, a2, 4
42038ad5:	0512d2        	l16ui	a13, a2, 10
42038ad8:	0412c2        	l16ui	a12, a2, 8
42038adb:	0312b2        	l16ui	a11, a2, 6
42038ade:	233380        	sext	a3, a3, 15
42038ae1:	234480        	sext	a4, a4, 15
42038ae4:	1149      	s32i.n	a4, a1, 4
42038ae6:	0139      	s32i.n	a3, a1, 0
42038ae8:	c0dda0        	sub	a13, a13, a10
42038aeb:	c0cca0        	sub	a12, a12, a10
42038aee:	baba      	add.n	a11, a10, a11
42038af0:	2102f2        	l8ui	a15, a2, 33
42038af3:	aa9a      	add.n	a10, a10, a9
42038af5:	72e8      	l32i.n	a14, a2, 28
42038af7:	23dd80        	sext	a13, a13, 15
42038afa:	23cc80        	sext	a12, a12, 15
42038afd:	23bb80        	sext	a11, a11, 15
42038b00:	23aa80        	sext	a10, a10, 15
42038b03:	7169      	s32i.n	a6, a1, 28
42038b05:	ffc1e5        	call8	42038724 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038b08:	ffa092        	movi	a9, 255
42038b0b:	027d      	mov.n	a7, a2
42038b0d:	0a6d      	mov.n	a6, a10
42038b0f:	079a97        	bne	a10, a9, 42038b1a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x4e>
42038b12:	142292        	l32i	a9, a2, 80
42038b15:	052d      	mov.n	a2, a5
42038b17:	0a1916        	beqz	a9, 42038bbc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038b1a:	04dd      	mov.n	a13, a4
42038b1c:	03cd      	mov.n	a12, a3
42038b1e:	3cc7b2        	addi	a11, a7, 60
42038b21:	28c7a2        	addi	a10, a7, 40
42038b24:	005ca5        	call8	420390f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b27:	200792        	l8ui	a9, a7, 32
42038b2a:	6169      	s32i.n	a6, a1, 24
42038b2c:	012a80        	slli	a2, a10, 24
42038b2f:	03c916        	beqz	a9, 42038b6f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038b32:	1149      	s32i.n	a4, a1, 4
42038b34:	0139      	s32i.n	a3, a1, 0
42038b36:	67e8      	l32i.n	a14, a7, 24
42038b38:	0597d2        	l16si	a13, a7, 10
42038b3b:	0497c2        	l16si	a12, a7, 8
42038b3e:	0397b2        	l16si	a11, a7, 6
42038b41:	0297a2        	l16si	a10, a7, 4
42038b44:	ff0c      	movi.n	a15, 15
42038b46:	ffbde5        	call8	42038724 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038b49:	61a9      	s32i.n	a10, a1, 24
42038b4b:	0aac      	beqz.n	a10, 42038b6f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038b4d:	7188      	l32i.n	a8, a1, 28
42038b4f:	38cc      	bnez.n	a8, 42038b56 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x8a>
42038b51:	24c782        	addi	a8, a7, 36
42038b54:	7189      	s32i.n	a8, a1, 28
42038b56:	7188      	l32i.n	a8, a1, 28
42038b58:	02ad      	mov.n	a10, a2
42038b5a:	0898      	l32i.n	a9, a8, 0
42038b5c:	6188      	l32i.n	a8, a1, 24
42038b5e:	119980        	slli	a9, a9, 8
42038b61:	419890        	srli	a9, a9, 8
42038b64:	01b880        	slli	a11, a8, 24
42038b67:	20b9b0        	or	a11, a9, a11
42038b6a:	cecea5        	call8	42007854 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038b6d:	0a2d      	mov.n	a2, a10
42038b6f:	ffa092        	movi	a9, 255
42038b72:	4a9697        	bne	a6, a9, 42038bc0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf4>
42038b75:	fb7c      	movi.n	a11, -1
42038b77:	01bb80        	slli	a11, a11, 24
42038b7a:	20b5b0        	or	a11, a5, a11
42038b7d:	02ad      	mov.n	a10, a2
42038b7f:	cecd65        	call8	42007854 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038b82:	0a2d      	mov.n	a2, a10
42038b84:	1427a2        	l32i	a10, a7, 80
42038b87:	031a16        	beqz	a10, 42038bbc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038b8a:	180a82        	l8ui	a8, a10, 24
42038b8d:	053816        	beqz	a8, 42038be4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x118>
42038b90:	20c440        	or	a12, a4, a4
42038b93:	03bd      	mov.n	a11, a3
42038b95:	ee2f65        	call8	42026e8c <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
42038b98:	6188      	l32i.n	a8, a1, 24
42038b9a:	75b8a0        	extui	a11, a10, 24, 8
42038b9d:	d1bb80        	mul16s	a11, a11, a8
42038ba0:	1f5081        	l32r	a8, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038ba3:	11aa80        	slli	a10, a10, 8
42038ba6:	a2bb80        	muluh	a11, a11, a8
42038ba9:	01bbf0        	slli	a11, a11, 17
42038bac:	75b8b0        	extui	a11, a11, 24, 8
42038baf:	404800        	ssai	8
42038bb2:	81bba0        	src	a11, a11, a10
42038bb5:	02ad      	mov.n	a10, a2
42038bb7:	cec9e5        	call8	42007854 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038bba:	0a2d      	mov.n	a2, a10
42038bbc:	f01d      	retw.n
42038bbe:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038bc1:	50fc06        	j	4204cfb5 <uart_module_disable+0x61>
42038bc4:	7598      	l32i.n	a9, a5, 28
42038bc6:	d19960        	mul16s	a9, a9, a6
42038bc9:	1f45a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038bcc:	7fc992        	addi	a9, a9, 127
42038bcf:	a299a0        	muluh	a9, a9, a10
42038bd2:	11b580        	slli	a11, a5, 8
42038bd5:	0199f0        	slli	a9, a9, 17
42038bd8:	759890        	extui	a9, a9, 24, 8
42038bdb:	404800        	ssai	8
42038bde:	81b9b0        	src	a11, a9, a11
42038be1:	ffe606        	j	42038b7d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb1>
42038be4:	021a82        	l16ui	a8, a10, 4
42038be7:	3ab8      	l32i.n	a11, a10, 12
42038be9:	c03380        	sub	a3, a3, a8
42038bec:	031a82        	l16ui	a8, a10, 6
42038bef:	c04480        	sub	a4, a4, a8
42038bf2:	740444        	mul.aa.ll	a4, a4
42038bf5:	780334        	mula.aa.ll	a3, a3
42038bf8:	031080        	rsr.acclo	a8
42038bfb:	c0bb80        	sub	a11, a11, a8
42038bfe:	061be6        	bgei	a11, 1, 42038c08 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x13c>
42038c01:	5aa8      	l32i.n	a10, a10, 20
42038c03:	ffe446        	j	42038b98 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038c06:	00          	.byte	00
42038c07:	9a9200        	trunc.s	a9, f2, 0
42038c0a:	04          	.byte	0x4
42038c0b:	3119a6        	blti	a9, 1, 42038c40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c0e:	899a      	add.n	a8, a9, a9
42038c10:	60a0c2        	movi	a12, 96
42038c13:	0f2b87        	blt	a11, a8, 42038c26 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038c16:	261926        	beqi	a9, 1, 42038c40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c19:	c90b      	addi.n	a12, a9, -1
42038c1b:	23cc80        	sext	a12, a12, 15
42038c1e:	11cce0        	slli	a12, a12, 2
42038c21:	0c2cb7        	blt	a12, a11, 42038c31 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x165>
42038c24:	0c4c      	movi.n	a12, 64
42038c26:	4ab8      	l32i.n	a11, a10, 16
42038c28:	5aa8      	l32i.n	a10, a10, 20
42038c2a:	4c2b65        	call8	42084ee0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42038c2d:	ffd9c6        	j	42038b98 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038c30:	00          	.byte	00
42038c31:	0b2926        	beqi	a9, 2, 42038c40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038c34:	889a      	add.n	a8, a8, a9
42038c36:	888a      	add.n	a8, a8, a8
42038c38:	f8c882        	addi	a8, a8, -8
42038c3b:	0c2c      	movi.n	a12, 32
42038c3d:	e52b87        	blt	a11, a8, 42038c26 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038c40:	4aa8      	l32i.n	a10, a10, 16
42038c42:	ffd486        	j	42038b98 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038cfc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038cfc:	00c136        	entry	a1, 96
42038cff:	238680        	sext	a8, a6, 15
42038d02:	1189      	s32i.n	a8, a1, 4
42038d04:	142282        	l32i	a8, a2, 80
42038d07:	07fd      	mov.n	a15, a7
42038d09:	027d      	mov.n	a7, a2
42038d0b:	233380        	sext	a3, a3, 15
42038d0e:	234480        	sext	a4, a4, 15
42038d11:	235580        	sext	a5, a5, 15
42038d14:	204856        	bnez	a8, 42038f1c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038d17:	0288      	l32i.n	a8, a2, 0
42038d19:	11e8      	l32i.n	a14, a1, 4
42038d1b:	b888      	l32i.n	a8, a8, 44
42038d1d:	02ad      	mov.n	a10, a2
42038d1f:	a1f9      	s32i.n	a15, a1, 40
42038d21:	05dd      	mov.n	a13, a5
42038d23:	04cd      	mov.n	a12, a4
42038d25:	03bd      	mov.n	a11, a3
42038d27:	0008e0        	callx8	a8
42038d2a:	0a2d      	mov.n	a2, a10
42038d2c:	098a56        	bnez	a10, 42038dc8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcc>
42038d2f:	1a9792        	l16si	a9, a7, 52
42038d32:	249782        	l16si	a8, a7, 72
42038d35:	ec0c      	movi.n	a12, 14
42038d37:	538890        	max	a8, a8, a9
42038d3a:	98fb      	addi.n	a9, a8, 15
42038d3c:	1ec882        	addi	a8, a8, 30
42038d3f:	b38990        	movgez	a8, a9, a9
42038d42:	21b480        	srai	a11, a8, 4
42038d45:	210782        	l8ui	a8, a7, 33
42038d48:	1907a2        	l8ui	a10, a7, 25
42038d4b:	62cc80        	saltu	a12, a12, a8
42038d4e:	180782        	l8ui	a8, a7, 24
42038d51:	200792        	l8ui	a9, a7, 32
42038d54:	7388a0        	maxu	a8, a8, a10
42038d57:	1a07a2        	l8ui	a10, a7, 26
42038d5a:	cc0b      	addi.n	a12, a12, -1
42038d5c:	7388a0        	maxu	a8, a8, a10
42038d5f:	1b07a2        	l8ui	a10, a7, 27
42038d62:	60c0c0        	neg	a12, a12
42038d65:	73a8a0        	maxu	a10, a8, a10
42038d68:	89ca      	add.n	a8, a9, a12
42038d6a:	99ba      	add.n	a9, a9, a11
42038d6c:	538a80        	max	a8, a10, a8
42038d6f:	029762        	l16si	a6, a7, 4
42038d72:	0497a2        	l16si	a10, a7, 8
42038d75:	99ca      	add.n	a9, a9, a12
42038d77:	539980        	max	a9, a9, a8
42038d7a:	669a      	add.n	a6, a6, a9
42038d7c:	c09a90        	sub	a9, a10, a9
42038d7f:	439950        	min	a9, a9, a5
42038d82:	3199      	s32i.n	a9, a1, 12
42038d84:	039792        	l16si	a9, a7, 6
42038d87:	536630        	max	a6, a6, a3
42038d8a:	998a      	add.n	a9, a9, a8
42038d8c:	99ba      	add.n	a9, a9, a11
42038d8e:	7199      	s32i.n	a9, a1, 28
42038d90:	059792        	l16si	a9, a7, 10
42038d93:	a1f8      	l32i.n	a15, a1, 40
42038d95:	c08980        	sub	a8, a9, a8
42038d98:	2189      	s32i.n	a8, a1, 8
42038d9a:	c08530        	sub	a8, a5, a3
42038d9d:	881b      	addi.n	a8, a8, 1
42038d9f:	4189      	s32i.n	a8, a1, 16
42038da1:	3188      	l32i.n	a8, a1, 12
42038da3:	881b      	addi.n	a8, a8, 1
42038da5:	5189      	s32i.n	a8, a1, 20
42038da7:	c08860        	sub	a8, a8, a6
42038daa:	1188e0        	slli	a8, a8, 2
42038dad:	8189      	s32i.n	a8, a1, 32
42038daf:	4188      	l32i.n	a8, a1, 16
42038db1:	1128e0        	slli	a2, a8, 2
42038db4:	c08630        	sub	a8, a6, a3
42038db7:	1188e0        	slli	a8, a8, 2
42038dba:	9189      	s32i.n	a8, a1, 36
42038dbc:	238680        	sext	a8, a6, 15
42038dbf:	6189      	s32i.n	a8, a1, 24
42038dc1:	1188      	l32i.n	a8, a1, 4
42038dc3:	03a847        	bge	a8, a4, 42038dca <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xce>
42038dc6:	020c      	movi.n	a2, 0
42038dc8:	f01d      	retw.n
42038dca:	31a8      	l32i.n	a10, a1, 12
42038dcc:	038d      	mov.n	a8, a3
42038dce:	0f9d      	mov.n	a9, a15
42038dd0:	2b2a67        	blt	a10, a6, 42038dff <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x103>
42038dd3:	0f8d      	mov.n	a8, a15
42038dd5:	03bd      	mov.n	a11, a3
42038dd7:	23c480        	sext	a12, a4, 15
42038dda:	02ab67        	bge	a11, a6, 42038de0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe4>
42038ddd:	0024c6        	j	42038e74 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x178>
42038de0:	9188      	l32i.n	a8, a1, 36
42038de2:	61b8      	l32i.n	a11, a1, 24
42038de4:	9f8a      	add.n	a9, a15, a8
42038de6:	07ad      	mov.n	a10, a7
42038de8:	b1f9      	s32i.n	a15, a1, 44
42038dea:	a199      	s32i.n	a9, a1, 40
42038dec:	ffe5a5        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038def:	8188      	l32i.n	a8, a1, 32
42038df1:	a198      	l32i.n	a9, a1, 40
42038df3:	b1f8      	l32i.n	a15, a1, 44
42038df5:	898a      	add.n	a8, a9, a8
42038df7:	021897        	beq	a8, a9, 42038dfd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x101>
42038dfa:	0025c6        	j	42038e95 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x199>
42038dfd:	5188      	l32i.n	a8, a1, 20
42038dff:	09dd      	mov.n	a13, a9
42038e01:	08bd      	mov.n	a11, a8
42038e03:	23c480        	sext	a12, a4, 15
42038e06:	0225b7        	blt	a5, a11, 42038e0c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x110>
42038e09:	0023c6        	j	42038e9c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a0>
42038e0c:	b51b      	addi.n	a11, a5, 1
42038e0e:	72ab80        	salt	a10, a11, a8
42038e11:	aa0b      	addi.n	a10, a10, -1
42038e13:	60a0a0        	neg	a10, a10
42038e16:	74a0a0        	extui	a10, a10, 0, 8
42038e19:	0c0c      	movi.n	a12, 0
42038e1b:	4a8c      	beqz.n	a10, 42038e23 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x127>
42038e1d:	c0bb80        	sub	a11, a11, a8
42038e20:	11cbe0        	slli	a12, a11, 2
42038e23:	71a8      	l32i.n	a10, a1, 28
42038e25:	99ca      	add.n	a9, a9, a12
42038e27:	72a4a0        	salt	a10, a4, a10
42038e2a:	aa0b      	addi.n	a10, a10, -1
42038e2c:	60a0a0        	neg	a10, a10
42038e2f:	74a0a0        	extui	a10, a10, 0, 8
42038e32:	098d      	mov.n	a8, a9
42038e34:	4abc      	beqz.n	a10, 42038e6c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038e36:	21a8      	l32i.n	a10, a1, 8
42038e38:	72aa40        	salt	a10, a10, a4
42038e3b:	aa0b      	addi.n	a10, a10, -1
42038e3d:	60a0a0        	neg	a10, a10
42038e40:	74a0a0        	extui	a10, a10, 0, 8
42038e43:	5aac      	beqz.n	a10, 42038e6c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038e45:	11a8      	l32i.n	a10, a1, 4
42038e47:	21b8      	l32i.n	a11, a1, 8
42038e49:	04ed      	mov.n	a14, a4
42038e4b:	43dab0        	min	a13, a10, a11
42038e4e:	742ed7        	blt	a14, a13, 42038ec6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038e51:	728d40        	salt	a8, a13, a4
42038e54:	880b      	addi.n	a8, a8, -1
42038e56:	608080        	neg	a8, a8
42038e59:	748080        	extui	a8, a8, 0, 8
42038e5c:	00a0a2        	movi	a10, 0
42038e5f:	002816        	beqz	a8, 42038e65 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x169>
42038e62:	c0ad40        	sub	a10, a13, a4
42038e65:	828a20        	mull	a8, a10, a2
42038e68:	4a4a      	add.n	a4, a10, a4
42038e6a:	889a      	add.n	a8, a8, a9
42038e6c:	441b      	addi.n	a4, a4, 1
42038e6e:	08fd      	mov.n	a15, a8
42038e70:	ffd346        	j	42038dc1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xc5>
42038e73:	a77000        	ee.stf.64.xp	f7, f10, a0, a0
42038e76:	61f220        	xsr.ccompare2	a2
42038e79:	0d          	.byte	0xd
42038e7a:	0c6182        	s32i	a8, a1, 48
42038e7d:	b1c9      	s32i.n	a12, a1, 44
42038e7f:	a1b9      	s32i.n	a11, a1, 40
42038e81:	ffdc65        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038e84:	c188      	l32i.n	a8, a1, 48
42038e86:	a1b8      	l32i.n	a11, a1, 40
42038e88:	08a9      	s32i.n	a10, a8, 0
42038e8a:	d1f8      	l32i.n	a15, a1, 52
42038e8c:	b1c8      	l32i.n	a12, a1, 44
42038e8e:	bb1b      	addi.n	a11, a11, 1
42038e90:	884b      	addi.n	a8, a8, 4
42038e92:	ffd106        	j	42038dda <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xde>
42038e95:	09a9      	s32i.n	a10, a9, 0
42038e97:	994b      	addi.n	a9, a9, 4
42038e99:	ffd686        	j	42038df7 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xfb>
42038e9c:	20a770        	or	a10, a7, a7
42038e9f:	0f61f2        	s32i	a15, a1, 60
42038ea2:	0e61d2        	s32i	a13, a1, 56
42038ea5:	d189      	s32i.n	a8, a1, 52
42038ea7:	c199      	s32i.n	a9, a1, 48
42038ea9:	b1c9      	s32i.n	a12, a1, 44
42038eab:	a1b9      	s32i.n	a11, a1, 40
42038ead:	ffd9a5        	call8	42038c48 <roo_windows::Decoration::read(short, short) const>
42038eb0:	e1d8      	l32i.n	a13, a1, 56
42038eb2:	a1b8      	l32i.n	a11, a1, 40
42038eb4:	0da9      	s32i.n	a10, a13, 0
42038eb6:	f1f8      	l32i.n	a15, a1, 60
42038eb8:	d188      	l32i.n	a8, a1, 52
42038eba:	c198      	l32i.n	a9, a1, 48
42038ebc:	b1c8      	l32i.n	a12, a1, 44
42038ebe:	bb1b      	addi.n	a11, a11, 1
42038ec0:	dd4b      	addi.n	a13, a13, 4
42038ec2:	ffd006        	j	42038e06 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10a>
42038ec5:	41a800        	srli	a10, a0, 8
42038ec8:	201aa6        	blti	a10, 1, 42038eec <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038ecb:	254226        	beqi	a2, 4, 42038ef4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f8>
42038ece:	0fbd      	mov.n	a11, a15
42038ed0:	08ad      	mov.n	a10, a8
42038ed2:	02cd      	mov.n	a12, a2
42038ed4:	d199      	s32i.n	a9, a1, 52
42038ed6:	c1e9      	s32i.n	a14, a1, 48
42038ed8:	b1d9      	s32i.n	a13, a1, 44
42038eda:	a1f9      	s32i.n	a15, a1, 40
42038edc:	1e8281        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038edf:	0008e0        	callx8	a8
42038ee2:	a1f8      	l32i.n	a15, a1, 40
42038ee4:	b1d8      	l32i.n	a13, a1, 44
42038ee6:	c1e8      	l32i.n	a14, a1, 48
42038ee8:	d198      	l32i.n	a9, a1, 52
42038eea:	0a8d      	mov.n	a8, a10
42038eec:	882a      	add.n	a8, a8, a2
42038eee:	ee1b      	addi.n	a14, a14, 1
42038ef0:	ffd686        	j	42038e4e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x152>
42038ef3:	00          	.byte	00
42038ef4:	0fa8      	l32i.n	a10, a15, 0
42038ef6:	08a9      	s32i.n	a10, a8, 0
42038ef8:	fffc06        	j	42038eec <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038efb:	600000        	neg	a0, a0
42038efe:	4020b6        	bltui	a0, 2, 42038f42 <roo_windows::internal::MakeLookupShadow(roo_display::Box, roo_windows::BorderStyle::CornerRadii, int, bool)+0x16>
42038f01:	c4          	.byte	0xc4
42038f02:	a77020        	ee.stf.64.xp	f7, f10, a2, a0
42038f05:	d42520        	extui	a2, a2, 5, 14
42038f08:	a9661bff 	ee.fft.cmul.s16.st.xp	q4, q6, q6, a15, a11, 2, 2, 0
42038f0c:	02          	.byte	0x2
42038f0d:	236680        	sext	a6, a6, 15
42038f10:	224b      	addi.n	a2, a2, 4
42038f12:	02fd      	mov.n	a15, a2
42038f14:	e5a567        	bge	a5, a6, 42038efd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x201>
42038f17:	441b      	addi.n	a4, a4, 1
42038f19:	234480        	sext	a4, a4, 15
42038f1c:	1188      	l32i.n	a8, a1, 4
42038f1e:	02a847        	bge	a8, a4, 42038f24 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038f21:	ffa846        	j	42038dc6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xca>
42038f24:	0f2d      	mov.n	a2, a15
42038f26:	036d      	mov.n	a6, a3
42038f28:	fff986        	j	42038f12 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/unroll4/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420390f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
420390f0:	00a136        	entry	a1, 80
420390f3:	238480        	sext	a8, a4, 15
420390f6:	027d      	mov.n	a7, a2
420390f8:	a189      	s32i.n	a8, a1, 40
420390fa:	0f0222        	l8ui	a2, a2, 15
420390fd:	238580        	sext	a8, a5, 15
42039100:	b189      	s32i.n	a8, a1, 44
42039102:	243216        	beqz	a2, 42039349 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
42039105:	009782        	l16si	a8, a7, 0
42039108:	0697f2        	l16si	a15, a7, 12
4203910b:	3189      	s32i.n	a8, a1, 12
4203910d:	3198      	l32i.n	a9, a1, 12
4203910f:	a188      	l32i.n	a8, a1, 40
42039111:	c08890        	sub	a8, a8, a9
42039114:	11c8c0        	slli	a12, a8, 4
42039117:	4189      	s32i.n	a8, a1, 16
42039119:	019782        	l16si	a8, a7, 2
4203911c:	cc8b      	addi.n	a12, a12, 8
4203911e:	5189      	s32i.n	a8, a1, 20
42039120:	5198      	l32i.n	a9, a1, 20
42039122:	b188      	l32i.n	a8, a1, 44
42039124:	c08890        	sub	a8, a8, a9
42039127:	6189      	s32i.n	a8, a1, 24
42039129:	069392        	l16si	a9, a3, 12
4203912c:	1188c0        	slli	a8, a8, 4
4203912f:	888b      	addi.n	a8, a8, 8
42039131:	c0f8f0        	sub	a15, a8, a15
42039134:	c08890        	sub	a8, a8, a9
42039137:	9cca      	add.n	a9, a12, a12
42039139:	7199      	s32i.n	a9, a1, 28
4203913b:	029792        	l16si	a9, a7, 4
4203913e:	d88a      	add.n	a13, a8, a8
42039140:	8199      	s32i.n	a9, a1, 32
42039142:	1199c0        	slli	a9, a9, 4
42039145:	2199      	s32i.n	a9, a1, 8
42039147:	039792        	l16si	a9, a7, 6
4203914a:	2168      	l32i.n	a6, a1, 8
4203914c:	11b9c0        	slli	a11, a9, 4
4203914f:	effa      	add.n	a14, a15, a15
42039151:	c04b80        	sub	a4, a11, a8
42039154:	9199      	s32i.n	a9, a1, 36
42039156:	72adb0        	salt	a10, a13, a11
42039159:	729eb0        	salt	a9, a14, a11
4203915c:	c056c0        	sub	a5, a6, a12
4203915f:	990b      	addi.n	a9, a9, -1
42039161:	aa0b      	addi.n	a10, a10, -1
42039163:	434480        	min	a4, a4, a8
42039166:	7188      	l32i.n	a8, a1, 28
42039168:	609090        	neg	a9, a9
4203916b:	60a0a0        	neg	a10, a10
4203916e:	4355c0        	min	a5, a5, a12
42039171:	c0cbf0        	sub	a12, a11, a15
42039174:	749090        	extui	a9, a9, 0, 8
42039177:	74a0a0        	extui	a10, a10, 0, 8
4203917a:	43ccf0        	min	a12, a12, a15
4203917d:	732867        	blt	a8, a6, 420391f4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x104>
42039180:	682eb7        	blt	a14, a11, 420391ec <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xfc>
42039183:	0a0782        	l8ui	a8, a7, 10
42039186:	7319a7        	beq	a9, a10, 420391fd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10d>
42039189:	7f2db7        	blt	a13, a11, 4203920c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11c>
4203918c:	0a0762        	l8ui	a6, a7, 10
4203918f:	23d880        	sext	a13, a8, 15
42039192:	11ddc0        	slli	a13, a13, 4
42039195:	02acd7        	bge	a12, a13, 4203919b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
42039198:	002206        	j	42039224 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
4203919b:	23f680        	sext	a15, a6, 15
4203919e:	11ffc0        	slli	a15, a15, 4
420391a1:	7f24f7        	blt	a4, a15, 42039224 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
420391a4:	43fc40        	min	a15, a12, a4
420391a7:	02af57        	bge	a15, a5, 420391ad <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xbd>
420391aa:	006f86        	j	4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27c>
420391ad:	a198      	l32i.n	a9, a1, 40
420391af:	3188      	l32i.n	a8, a1, 12
420391b1:	21a8      	l32i.n	a10, a1, 8
420391b3:	c08890        	sub	a8, a8, a9
420391b6:	7198      	l32i.n	a9, a1, 28
420391b8:	0829a7        	blt	a9, a10, 420391c4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd4>
420391bb:	4188      	l32i.n	a8, a1, 16
420391bd:	8198      	l32i.n	a9, a1, 32
420391bf:	c08890        	sub	a8, a8, a9
420391c2:	881b      	addi.n	a8, a8, 1
420391c4:	096c      	movi.n	a9, -32
420391c6:	02a897        	bge	a8, a9, 420391cc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdc>
420391c9:	0064c6        	j	42039360 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
420391cc:	092c      	movi.n	a9, 32
420391ce:	02a987        	bge	a9, a8, 420391d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
420391d1:	006446        	j	42039366 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
420391d4:	220b      	addi.n	a2, a2, -1
420391d6:	902220        	addx2	a2, a2, a2
420391d9:	1192a0        	slli	a9, a2, 6
420391dc:	229a      	add.n	a2, a2, a9
420391de:	1dc591        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420391e1:	898a      	add.n	a8, a9, a8
420391e3:	882a      	add.n	a8, a8, a2
420391e5:	200822        	l8ui	a2, a8, 32
420391e8:	005746        	j	42039349 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
420391eb:	078200        	ee.stf.64.xp	f8, f0, a0, a2
420391ee:	c609      	s32i.n	a0, a6, 48
420391f0:	e4          	.byte	0xe4
420391f1:	b70000ff 	ee.vmulas.s16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
420391f5:	07820c2e 	ee.vmulas.s16.accx.ld.ip.qup	q6, a2, 0x1c0, q0, q1, q0, q2
420391f9:	a70b      	addi.n	a10, a7, -1
420391fb:	1699      	s32i.n	a9, a6, 4
420391fd:	086d      	mov.n	a6, a8
420391ff:	ffe306        	j	4203918f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
42039202:	820000        	mull	a0, a0, a0
42039205:	c60807        	bnone	a8, a0, 420391cf <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdf>
42039208:	fffb      	addi.n	a15, a15, 15
4203920a:	620000        	saltu	a0, a0, a0
4203920d:	060907        	bnone	a9, a0, 42039217 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x127>
42039210:	0000ffdf 	ee.vmulas.s16.accx.ld.ip.qup	q0, a13, 240, q7, q6, q0, q0
42039214:	052db7        	blt	a13, a11, 4203921d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12d>
42039217:	0b0762        	l8ui	a6, a7, 11
4203921a:	ffdc46        	j	4203918f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
4203921d:	080762        	l8ui	a6, a7, 8
42039220:	ffdac6        	j	4203918f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
42039223:	00          	.byte	00
42039224:	5199a7        	bne	a9, a10, 42039279 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
42039227:	538680        	max	a8, a6, a8
4203922a:	238880        	sext	a8, a8, 15
4203922d:	1188c0        	slli	a8, a8, 4
42039230:	452587        	blt	a5, a8, 42039279 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
42039233:	538c40        	max	a8, a12, a4
42039236:	3f2587        	blt	a5, a8, 42039279 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
42039239:	9198      	l32i.n	a9, a1, 36
4203923b:	6188      	l32i.n	a8, a1, 24
4203923d:	c08890        	sub	a8, a8, a9
42039240:	881b      	addi.n	a8, a8, 1
42039242:	290c      	movi.n	a9, 2
42039244:	08aeb7        	bge	a14, a11, 42039250 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x160>
42039247:	b198      	l32i.n	a9, a1, 44
42039249:	5188      	l32i.n	a8, a1, 20
4203924b:	c08890        	sub	a8, a8, a9
4203924e:	190c      	movi.n	a9, 1
42039250:	0a6c      	movi.n	a10, -32
42039252:	02a8a7        	bge	a8, a10, 42039258 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x168>
42039255:	0041c6        	j	42039360 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
42039258:	0a2c      	movi.n	a10, 32
4203925a:	02aa87        	bge	a10, a8, 42039260 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x170>
4203925d:	004146        	j	42039366 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
42039260:	220b      	addi.n	a2, a2, -1
42039262:	902220        	addx2	a2, a2, a2
42039265:	229a      	add.n	a2, a2, a9
42039267:	1da391        	l32r	a9, 420008f4 <_stext+0x8d4> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
4203926a:	11a2a0        	slli	a10, a2, 6
4203926d:	aa2a      	add.n	a10, a10, a2
4203926f:	898a      	add.n	a8, a9, a8
42039271:	88aa      	add.n	a8, a8, a10
42039273:	ffdb86        	j	420391e5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf5>
42039276:	000000        	ill
42039279:	091782        	l16ui	a8, a7, 18
4203927c:	4395c0        	min	a9, a5, a12
4203927f:	608080        	neg	a8, a8
42039282:	00a022        	movi	a2, 0
42039285:	4e2987        	blt	a9, a8, 420392d7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1e7>
42039288:	c0ad50        	sub	a10, a13, a5
4203928b:	090c      	movi.n	a9, 0
4203928d:	c08dc0        	sub	a8, a13, a12
42039290:	0229a7        	blt	a9, a10, 42039296 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a6>
42039293:	002d46        	j	4203934c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
42039296:	022987        	blt	a9, a8, 4203929c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ac>
42039299:	002bc6        	j	4203934c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
4203929c:	828880        	mull	a8, a8, a8
4203929f:	82aaa0        	mull	a10, a10, a10
420392a2:	21d9      	s32i.n	a13, a1, 8
420392a4:	80aa80        	add	a10, a10, a8
420392a7:	4d33e5        	call8	420865e4 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
420392aa:	0221d2        	l32i	a13, a1, 8
420392ad:	0e0792        	l8ui	a9, a7, 14
420392b0:	c08ad0        	sub	a8, a10, a13
420392b3:	c08890        	sub	a8, a8, a9
420392b6:	081792        	l16ui	a9, a7, 16
420392b9:	828890        	mull	a8, a8, a9
420392bc:	1d9091        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
420392bf:	889a      	add.n	a8, a8, a9
420392c1:	218c80        	srai	a8, a8, 12
420392c4:	ffa092        	movi	a9, 255
420392c7:	438890        	min	a8, a8, a9
420392ca:	090c      	movi.n	a9, 0
420392cc:	538890        	max	a8, a8, a9
420392cf:	1d8a91        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
420392d2:	898a      	add.n	a8, a9, a8
420392d4:	000822        	l8ui	a2, a8, 0
420392d7:	091392        	l16ui	a9, a3, 18
420392da:	43a540        	min	a10, a5, a4
420392dd:	609090        	neg	a9, a9
420392e0:	00a082        	movi	a8, 0
420392e3:	492a97        	blt	a10, a9, 42039330 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
420392e6:	236680        	sext	a6, a6, 15
420392e9:	1166c0        	slli	a6, a6, 4
420392ec:	c09640        	sub	a9, a6, a4
420392ef:	c0a650        	sub	a10, a6, a5
420392f2:	62a897        	bge	a8, a9, 42039358 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
420392f5:	5fa8a7        	bge	a8, a10, 42039358 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
420392f8:	829990        	mull	a9, a9, a9
420392fb:	82aaa0        	mull	a10, a10, a10
420392fe:	80a9a0        	add	a10, a9, a10
42039301:	4d2e25        	call8	420865e4 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039304:	0e0392        	l8ui	a9, a3, 14
42039307:	c08a60        	sub	a8, a10, a6
4203930a:	c08890        	sub	a8, a8, a9
4203930d:	081392        	l16ui	a9, a3, 16
42039310:	828890        	mull	a8, a8, a9
42039313:	1d7a91        	l32r	a9, 420008fc <_stext+0x8dc> (80000 <UserFrameTotalSize+0x7ff00>)
42039316:	808890        	add	a8, a8, a9
42039319:	218c80        	srai	a8, a8, 12
4203931c:	ffa092        	movi	a9, 255
4203931f:	438890        	min	a8, a8, a9
42039322:	090c      	movi.n	a9, 0
42039324:	538890        	max	a8, a8, a9
42039327:	1d7691        	l32r	a9, 42000900 <_stext+0x8e0> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
4203932a:	808980        	add	a8, a9, a8
4203932d:	000882        	l8ui	a8, a8, 0
42039330:	d19280        	mul16s	a9, a2, a8
42039333:	1d6ba1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42039336:	7fc992        	addi	a9, a9, 127
42039339:	b2a9a0        	mulsh	a10, a9, a10
4203933c:	282a      	add.n	a2, a8, a2
4203933e:	99aa      	add.n	a9, a9, a10
42039340:	219790        	srai	a9, a9, 7
42039343:	c02290        	sub	a2, a2, a9
42039346:	742020        	extui	a2, a2, 0, 8
42039349:	f01d      	retw.n
4203934b:	00          	.byte	00
4203934c:	022c57        	blt	a12, a5, 42039352 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x262>
4203934f:	ffd686        	j	420392ad <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
42039352:	08ad      	mov.n	a10, a8
42039354:	ffd546        	j	420392ad <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
42039357:	a45700        	extui	a5, a0, 7, 11
4203935a:	ada8      	l32i.n	a10, a13, 40
4203935c:	c609      	s32i.n	a0, a6, 48
4203935e:	ffe8      	l32i.n	a14, a15, 60
42039360:	68a022        	movi	a2, 104
42039363:	fff886        	j	42039349 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
42039366:	020c      	movi.n	a2, 0
42039368:	fff746        	j	42039349 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
4203936b:	00          	.byte	00
4203936c:	0219a7        	beq	a9, a10, 42039372 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x282>
4203936f:	ffc546        	j	42039288 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x198>
42039372:	ffac46        	j	42039227 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x137>
