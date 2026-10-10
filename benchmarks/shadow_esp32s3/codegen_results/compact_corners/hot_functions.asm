
/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033144 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42033144:	006136        	entry	a1, 48
42033147:	002282        	l32i	a8, a2, 0
4203314a:	20f770        	or	a15, a7, a7
4203314d:	0b2882        	l32i	a8, a8, 44
42033150:	233380        	sext	a3, a3, 15
42033153:	234480        	sext	a4, a4, 15
42033156:	235580        	sext	a5, a5, 15
42033159:	236680        	sext	a6, a6, 15
4203315c:	02ad      	mov.n	a10, a2
4203315e:	31f9      	s32i.n	a15, a1, 12
42033160:	06ed      	mov.n	a14, a6
42033162:	05dd      	mov.n	a13, a5
42033164:	04cd      	mov.n	a12, a4
42033166:	03bd      	mov.n	a11, a3
42033168:	0008e0        	callx8	a8
4203316b:	027d      	mov.n	a7, a2
4203316d:	0a2d      	mov.n	a2, a10
4203316f:	facc      	bnez.n	a10, 42033182 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
42033171:	31f8      	l32i.n	a15, a1, 12
42033173:	06ed      	mov.n	a14, a6
42033175:	05dd      	mov.n	a13, a5
42033177:	04cd      	mov.n	a12, a4
42033179:	03bd      	mov.n	a11, a3
4203317b:	07ad      	mov.n	a10, a7
4203317d:	dc5d65        	call8	4200f754 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
42033180:	0a2d      	mov.n	a2, a10
42033182:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42086710 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
42086710:	004136        	entry	a1, 32
42086713:	029d      	mov.n	a9, a2
42086715:	120c      	movi.n	a2, 1
42086717:	832990        	moveqz	a2, a9, a9
4208671a:	4149b6        	bltui	a9, 4, 4208675f <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4f>
4208671d:	40f980        	nsau	a8, a9
42086720:	dfc882        	addi	a8, a8, -33
42086723:	608080        	neg	a8, a8
42086726:	218180        	srai	a8, a8, 1
42086729:	fec882        	addi	a8, a8, -2
4208672c:	3a0c      	movi.n	a10, 3
4208672e:	401800        	ssl	a8
42086731:	a1aa00        	sll	a10, a10
42086734:	c289a0        	quou	a8, a9, a10
42086737:	88aa      	add.n	a8, a8, a10
42086739:	418180        	srli	a8, a8, 1
4208673c:	c2a980        	quou	a10, a9, a8
4208673f:	aa8a      	add.n	a10, a10, a8
42086741:	41a1a0        	srli	a10, a10, 1
42086744:	c289a0        	quou	a8, a9, a10
42086747:	88aa      	add.n	a8, a8, a10
42086749:	1a0c      	movi.n	a10, 1
4208674b:	418180        	srli	a8, a8, 1
4208674e:	11aa00        	slli	a10, a10, 16
42086751:	0518a7        	beq	a8, a10, 4208675a <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4a>
42086754:	82a880        	mull	a10, a8, a8
42086757:	01b9a7        	bgeu	a9, a10, 4208675c <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x4c>
4208675a:	880b      	addi.n	a8, a8, -1
4208675c:	f42080        	extui	a2, a8, 0, 16
4208675f:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038c98 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038c98:	006136        	entry	a1, 48
42038c9b:	038d      	mov.n	a8, a3
42038c9d:	059d      	mov.n	a9, a5
42038c9f:	05fd      	mov.n	a15, a5
42038ca1:	063d      	mov.n	a3, a6
42038ca3:	045d      	mov.n	a5, a4
42038ca5:	087d      	mov.n	a7, a8
42038ca7:	1d4ff6        	bgeui	a15, 4, 42038cc8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x30>
42038caa:	41a290        	srli	a10, a9, 2
42038cad:	11bac0        	slli	a11, a10, 4
42038cb0:	66ba      	add.n	a6, a6, a11
42038cb2:	b0ba80        	addx8	a11, a10, a8
42038cb5:	11aae0        	slli	a10, a10, 2
42038cb8:	c099a0        	sub	a9, a9, a10
42038cbb:	a07960        	addx4	a7, a9, a6
42038cbe:	c04480        	sub	a4, a4, a8
42038cc1:	8b4a      	add.n	a8, a11, a4
42038cc3:	5e9677        	bne	a6, a7, 42038d25 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8d>
42038cc6:	f01d      	retw.n
42038cc8:	0095c2        	l16si	a12, a5, 0
42038ccb:	0097b2        	l16si	a11, a7, 0
42038cce:	52d8      	l32i.n	a13, a2, 20
42038cd0:	0e0c      	movi.n	a14, 0
42038cd2:	02ad      	mov.n	a10, a2
42038cd4:	3199      	s32i.n	a9, a1, 12
42038cd6:	2189      	s32i.n	a8, a1, 8
42038cd8:	11f9      	s32i.n	a15, a1, 4
42038cda:	ffe7a5        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cdd:	03a9      	s32i.n	a10, a3, 0
42038cdf:	0195c2        	l16si	a12, a5, 2
42038ce2:	0197b2        	l16si	a11, a7, 2
42038ce5:	52d8      	l32i.n	a13, a2, 20
42038ce7:	0e0c      	movi.n	a14, 0
42038ce9:	02ad      	mov.n	a10, a2
42038ceb:	ffe6a5        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cee:	13a9      	s32i.n	a10, a3, 4
42038cf0:	0295c2        	l16si	a12, a5, 4
42038cf3:	0297b2        	l16si	a11, a7, 4
42038cf6:	52d8      	l32i.n	a13, a2, 20
42038cf8:	0e0c      	movi.n	a14, 0
42038cfa:	02ad      	mov.n	a10, a2
42038cfc:	ffe565        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038cff:	23a9      	s32i.n	a10, a3, 8
42038d01:	0395c2        	l16si	a12, a5, 6
42038d04:	0397b2        	l16si	a11, a7, 6
42038d07:	52d8      	l32i.n	a13, a2, 20
42038d09:	0e0c      	movi.n	a14, 0
42038d0b:	02ad      	mov.n	a10, a2
42038d0d:	ffe465        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038d10:	11f8      	l32i.n	a15, a1, 4
42038d12:	33a9      	s32i.n	a10, a3, 12
42038d14:	3198      	l32i.n	a9, a1, 12
42038d16:	2188      	l32i.n	a8, a1, 8
42038d18:	778b      	addi.n	a7, a7, 8
42038d1a:	558b      	addi.n	a5, a5, 8
42038d1c:	10c332        	addi	a3, a3, 16
42038d1f:	fccff2        	addi	a15, a15, -4
42038d22:	ffe046        	j	42038ca7 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xf>
42038d25:	02cb52        	addi	a5, a11, 2
42038d28:	0522d2        	l32i	a13, a2, 20
42038d2b:	009bb2        	l16si	a11, a11, 0
42038d2e:	0098c2        	l16si	a12, a8, 0
42038d31:	00a0e2        	movi	a14, 0
42038d34:	02ad      	mov.n	a10, a2
42038d36:	ffe1e5        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038d39:	06a9      	s32i.n	a10, a6, 0
42038d3b:	05bd      	mov.n	a11, a5
42038d3d:	664b      	addi.n	a6, a6, 4
42038d3f:	ffdf86        	j	42038cc1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x29>


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4208655c <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
4208655c:	004136        	entry	a1, 32
4208655f:	208230        	or	a8, a2, a3
42086562:	0228d6        	bgez	a8, 42086588 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
42086565:	c02240        	sub	a2, a2, a4
42086568:	40f290        	nsau	a9, a2
4208656b:	419590        	srli	a9, a9, 5
4208656e:	b28c      	beqz.n	a2, 4208657d <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
42086570:	c03340        	sub	a3, a3, a4
42086573:	180c      	movi.n	a8, 1
42086575:	938930        	movnez	a8, a9, a3
42086578:	ffa022        	movi	a2, 255
4208657b:	688c      	beqz.n	a8, 42086585 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
4208657d:	1125c0        	slli	a2, a5, 4
42086580:	252a      	add.n	a2, a5, a2
42086582:	742020        	extui	a2, a2, 0, 8
42086585:	f01d      	retw.n
42086587:	222000        	orb	b2, b0, b0
4208658a:	3330c1        	l32r	a12, 4205324c <spi_device_acquire_bus+0x5c> (3aa5e2a5 <UserFrameTotalSize+0x3aa5e1a5>)
4208658d:	44c0c1        	l32r	a12, 42057890 <sdmmc_host_init_slot+0x4cc> (8e090c4 <UserFrameTotalSize+0x8e08fc4>)
42086590:	545a11        	l32r	a1, 4205b6f8 <esp_core_dump_write+0xd8> (900d0d0c <_rtc_reserved_end+0x2ffd0d0c>)
42086593:	823a      	add.n	a8, a2, a3
42086595:	c19550        	mul16u	a9, a5, a5
42086598:	118880        	slli	a8, a8, 8
4208659b:	020c      	movi.n	a2, 0
4208659d:	e4b897        	bgeu	a8, a9, 42086585 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420865a0:	f1c592        	addi	a9, a5, -15
420865a3:	c19990        	mul16u	a9, a9, a9
420865a6:	ffa022        	movi	a2, 255
420865a9:	d83897        	bltu	a8, a9, 42086585 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420865ac:	40f890        	nsau	a9, a8
420865af:	dfc992        	addi	a9, a9, -33
420865b2:	609090        	neg	a9, a9
420865b5:	219190        	srai	a9, a9, 1
420865b8:	fec992        	addi	a9, a9, -2
420865bb:	3a0c      	movi.n	a10, 3
420865bd:	401900        	ssl	a9
420865c0:	a1aa00        	sll	a10, a10
420865c3:	c298a0        	quou	a9, a8, a10
420865c6:	99aa      	add.n	a9, a9, a10
420865c8:	419190        	srli	a9, a9, 1
420865cb:	c2a890        	quou	a10, a8, a9
420865ce:	aa9a      	add.n	a10, a10, a9
420865d0:	41a1a0        	srli	a10, a10, 1
420865d3:	c298a0        	quou	a9, a8, a10
420865d6:	8099a0        	add	a9, a9, a10
420865d9:	01a0a2        	movi	a10, 1
420865dc:	419190        	srli	a9, a9, 1
420865df:	11aa00        	slli	a10, a10, 16
420865e2:	f45050        	extui	a5, a5, 0, 16
420865e5:	0519a7        	beq	a9, a10, 420865ee <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x92>
420865e8:	82a990        	mull	a10, a9, a9
420865eb:	01b8a7        	bgeu	a8, a10, 420865f0 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x94>
420865ee:	990b      	addi.n	a9, a9, -1
420865f0:	c05590        	sub	a5, a5, a9
420865f3:	f48050        	extui	a8, a5, 0, 16
420865f6:	239580        	sext	a9, a5, 15
420865f9:	020c      	movi.n	a2, 0
420865fb:	86e5f7        	bbsi	a5, 15, 42086585 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420865fe:	fa0c      	movi.n	a10, 15
42086600:	ffa022        	movi	a2, 255
42086603:	02aa97        	bge	a10, a9, 42086609 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0xad>
42086606:	ffdec6        	j	42086585 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42086609:	1198c0        	slli	a9, a8, 4
4208660c:	889a      	add.n	a8, a8, a9
4208660e:	742080        	extui	a2, a8, 0, 8
42086611:	ffdc06        	j	42086585 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033eb8 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033eb8:	00a136        	entry	a1, 80
42033ebb:	3169      	s32i.n	a6, a1, 12
42033ebd:	2159      	s32i.n	a5, a1, 8
42033ebf:	060c      	movi.n	a6, 0
42033ec1:	1cc172        	addi	a7, a1, 28
42033ec4:	2188      	l32i.n	a8, a1, 8
42033ec6:	029687        	bne	a6, a8, 42033ecc <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033ec9:	f01d      	retw.n
42033ecb:	938200        	movnez	a8, a2, a0
42033ece:	22d200        	orb	b13, b2, b0
42033ed1:	018917        	bany	a9, a1, 42033ed6 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033ed4:	009482        	l16si	a8, a4, 0
42033ed7:	01b8      	l32i.n	a11, a1, 0
42033ed9:	1622a2        	l32i	a10, a2, 88
42033edc:	08cd      	mov.n	a12, a8
42033ede:	0067d2        	s32i	a13, a7, 0
42033ee1:	20e770        	or	a14, a7, a7
42033ee4:	016182        	s32i	a8, a1, 4
42033ee7:	201110        	or	a1, a1, a1
42033eea:	fffa25        	call8	42033e8c <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033eed:	182282        	l32i	a8, a2, 96
42033ef0:	1922b2        	l32i	a11, a2, 100
42033ef3:	8189      	s32i.n	a8, a1, 32
42033ef5:	20c152        	addi	a5, a1, 32
42033ef8:	050b16        	beqz	a11, 42033f4c <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033efb:	0bc8      	l32i.n	a12, a11, 0
42033efd:	24c1a2        	addi	a10, a1, 36
42033f00:	201110        	or	a1, a1, a1
42033f03:	ff3665        	call8	42033268 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f06:	07d8      	l32i.n	a13, a7, 0
42033f08:	11c8      	l32i.n	a12, a1, 4
42033f0a:	0021b2        	l32i	a11, a1, 0
42033f0d:	24c1a2        	addi	a10, a1, 36
42033f10:	201110        	or	a1, a1, a1
42033f13:	fe1865        	call8	42032098 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f16:	07a9      	s32i.n	a10, a7, 0
42033f18:	24c1a2        	addi	a10, a1, 36
42033f1b:	201110        	or	a1, a1, a1
42033f1e:	f894a5        	call8	4202c868 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f21:	1922b2        	l32i	a11, a2, 100
42033f24:	24c1a2        	addi	a10, a1, 36
42033f27:	002bc2        	l32i	a12, a11, 0
42033f2a:	201110        	or	a1, a1, a1
42033f2d:	ff33a5        	call8	42033268 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f30:	05d8      	l32i.n	a13, a5, 0
42033f32:	0094c2        	l16si	a12, a4, 0
42033f35:	0093b2        	l16si	a11, a3, 0
42033f38:	24c1a2        	addi	a10, a1, 36
42033f3b:	201110        	or	a1, a1, a1
42033f3e:	fe15a5        	call8	42032098 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f41:	05a9      	s32i.n	a10, a5, 0
42033f43:	24c1a2        	addi	a10, a1, 36
42033f46:	201110        	or	a1, a1, a1
42033f49:	f891e5        	call8	4202c868 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f4c:	0094c2        	l16si	a12, a4, 0
42033f4f:	0093b2        	l16si	a11, a3, 0
42033f52:	07d8      	l32i.n	a13, a7, 0
42033f54:	05ed      	mov.n	a14, a5
42033f56:	a24b      	addi.n	a10, a2, 4
42033f58:	201110        	or	a1, a1, a1
42033f5b:	04bfa5        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f5e:	3188      	l32i.n	a8, a1, 12
42033f60:	332b      	addi.n	a3, a3, 2
42033f62:	a08680        	addx4	a8, a6, a8
42033f65:	08a9      	s32i.n	a10, a8, 0
42033f67:	661b      	addi.n	a6, a6, 1
42033f69:	442b      	addi.n	a4, a4, 2
42033f6b:	ffd546        	j	42033ec4 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f6e:	c1a200        	mul16u	a10, a2, a0
42033f71:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f74:	20          	.byte	0x20
42033f75:	f88f25        	call8	4202c868 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f78:	02ad      	mov.n	a10, a2
42033f7a:	201110        	or	a1, a1, a1
42033f7d:	3ce0a5        	call8	42070d88 <_Unwind_Resume>
42033f80:	fffac6        	j	42033f6f <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038730 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
42038730:	006136        	entry	a1, 48
42038733:	189192        	l16si	a9, a1, 48
42038736:	3169      	s32i.n	a6, a1, 12
42038738:	f1cb      	addi.n	a15, a1, 12
4203873a:	1a9182        	l16si	a8, a1, 52
4203873d:	02ed      	mov.n	a14, a2
4203873f:	74d070        	extui	a13, a7, 0, 8
42038742:	02a927        	bge	a9, a2, 42038748 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
42038745:	0030c6        	j	4203880c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038748:	02a497        	bge	a4, a9, 4203874e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
4203874b:	002f46        	j	4203880c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203874e:	02a837        	bge	a8, a3, 42038754 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
42038751:	002dc6        	j	4203880c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
42038754:	02a587        	bge	a5, a8, 4203875a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a>
42038757:	002c46        	j	4203880c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
4203875a:	000fc2        	l8ui	a12, a15, 0
4203875d:	3c9c      	beqz.n	a12, 42038774 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
4203875f:	a2ca      	add.n	a10, a2, a12
42038761:	0f2a97        	blt	a10, a9, 42038774 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038764:	b3ca      	add.n	a11, a3, a12
42038766:	0a2b87        	blt	a11, a8, 42038774 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038769:	c0bb80        	sub	a11, a11, a8
4203876c:	c0aa90        	sub	a10, a10, a9
4203876f:	000706        	j	4203878f <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x5f>
42038772:	c20000        	quou	a0, a0, a0
42038775:	1c16010f 	ee.vmulas.s16.qacc.ld.ip.qup	q0, a0, -240, q4, q0, q1, q6
42038779:	a4c002        	addi	a0, a0, -92
4203877c:	c0          	.byte	0xc0
4203877d:	1b29a7        	blt	a9, a10, 4203879c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038780:	80b3c0        	add	a11, a3, a12
42038783:	152b87        	blt	a11, a8, 4203879c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038786:	c0bb80        	sub	a11, a11, a8
42038789:	c0ac40        	sub	a10, a12, a4
4203878c:	80aa90        	add	a10, a10, a9
4203878f:	23bb80        	sext	a11, a11, 15
42038792:	23aa80        	sext	a10, a10, 15
42038795:	4ddc65        	call8	4208655c <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
42038798:	0a2d      	mov.n	a2, a10
4203879a:	f01d      	retw.n
4203879c:	020fc2        	l8ui	a12, a15, 2
4203879f:	2c9c      	beqz.n	a12, 420387b5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
420387a1:	c0a4c0        	sub	a10, a4, a12
420387a4:	0d29a7        	blt	a9, a10, 420387b5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
420387a7:	c0a5c0        	sub	a10, a5, a12
420387aa:	0728a7        	blt	a8, a10, 420387b5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
420387ad:	c0bc50        	sub	a11, a12, a5
420387b0:	bb8a      	add.n	a11, a11, a8
420387b2:	fff4c6        	j	42038789 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x59>
420387b5:	030fc2        	l8ui	a12, a15, 3
420387b8:	1c9c      	beqz.n	a12, 420387cd <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387ba:	aeca      	add.n	a10, a14, a12
420387bc:	0d2a97        	blt	a10, a9, 420387cd <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387bf:	c0b5c0        	sub	a11, a5, a12
420387c2:	0728b7        	blt	a8, a11, 420387cd <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
420387c5:	c0bc50        	sub	a11, a12, a5
420387c8:	bb8a      	add.n	a11, a11, a8
420387ca:	ffe786        	j	4203876c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3c>
420387cd:	ea0c      	movi.n	a10, 14
420387cf:	ffa022        	movi	a2, 255
420387d2:	c43ad7        	bltu	a10, a13, 4203879a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420387d5:	c0e9e0        	sub	a14, a9, a14
420387d8:	40fea0        	nsau	a10, a14
420387db:	1b0c      	movi.n	a11, 1
420387dd:	41a5a0        	srli	a10, a10, 5
420387e0:	ce9c      	beqz.n	a14, 42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387e2:	c09940        	sub	a9, a9, a4
420387e5:	83ab90        	moveqz	a10, a11, a9
420387e8:	014a56        	bnez	a10, 42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387eb:	c03830        	sub	a3, a8, a3
420387ee:	40f390        	nsau	a9, a3
420387f1:	419590        	srli	a9, a9, 5
420387f4:	008316        	beqz	a3, 42038800 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
420387f7:	c08850        	sub	a8, a8, a5
420387fa:	93b980        	movnez	a11, a9, a8
420387fd:	f99b16        	beqz	a11, 4203879a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
42038800:	118dc0        	slli	a8, a13, 4
42038803:	dd8a      	add.n	a13, a13, a8
42038805:	7420d0        	extui	a2, a13, 0, 8
42038808:	ffe386        	j	4203879a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
4203880b:	020c00        	andb	b0, b12, b0
4203880e:	ffe206        	j	4203879a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038b54:	008136        	entry	a1, 64
42038b57:	021292        	l16ui	a9, a2, 4
42038b5a:	7169      	s32i.n	a6, a1, 28
42038b5c:	233380        	sext	a3, a3, 15
42038b5f:	23a980        	sext	a10, a9, 15
42038b62:	027d      	mov.n	a7, a2
42038b64:	234480        	sext	a4, a4, 15
42038b67:	1a23a7        	blt	a3, a10, 42038b85 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b6a:	0412c2        	l16ui	a12, a2, 8
42038b6d:	23ac80        	sext	a10, a12, 15
42038b70:	112a37        	blt	a10, a3, 42038b85 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b73:	0312b2        	l16ui	a11, a2, 6
42038b76:	23ab80        	sext	a10, a11, 15
42038b79:	0824a7        	blt	a4, a10, 42038b85 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x31>
42038b7c:	0512d2        	l16ui	a13, a2, 10
42038b7f:	23ad80        	sext	a10, a13, 15
42038b82:	12aa47        	bge	a10, a4, 42038b98 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x44>
42038b85:	04dd      	mov.n	a13, a4
42038b87:	03cd      	mov.n	a12, a3
42038b89:	3cc7b2        	addi	a11, a7, 60
42038b8c:	28c7a2        	addi	a10, a7, 40
42038b8f:	005aa5        	call8	42039138 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038b92:	012a80        	slli	a2, a10, 24
42038b95:	f01d      	retw.n
42038b97:	02a200        	andb	b10, b2, b0
42038b9a:	20          	.byte	0x20
42038b9b:	016142        	s32i	a4, a1, 4
42038b9e:	006132        	s32i	a3, a1, 0
42038ba1:	c0dda0        	sub	a13, a13, a10
42038ba4:	c0cca0        	sub	a12, a12, a10
42038ba7:	80bab0        	add	a11, a10, a11
42038baa:	2102f2        	l8ui	a15, a2, 33
42038bad:	aa9a      	add.n	a10, a10, a9
42038baf:	72e8      	l32i.n	a14, a2, 28
42038bb1:	23dd80        	sext	a13, a13, 15
42038bb4:	23cc80        	sext	a12, a12, 15
42038bb7:	23bb80        	sext	a11, a11, 15
42038bba:	23aa80        	sext	a10, a10, 15
42038bbd:	ffb725        	call8	42038730 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bc0:	ffa092        	movi	a9, 255
42038bc3:	0a6d      	mov.n	a6, a10
42038bc5:	079a97        	bne	a10, a9, 42038bd0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x7c>
42038bc8:	142292        	l32i	a9, a2, 80
42038bcb:	052d      	mov.n	a2, a5
42038bcd:	fc4916        	beqz	a9, 42038b95 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038bd0:	04dd      	mov.n	a13, a4
42038bd2:	03cd      	mov.n	a12, a3
42038bd4:	3cc7b2        	addi	a11, a7, 60
42038bd7:	28c7a2        	addi	a10, a7, 40
42038bda:	0055e5        	call8	42039138 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038bdd:	200792        	l8ui	a9, a7, 32
42038be0:	6169      	s32i.n	a6, a1, 24
42038be2:	012a80        	slli	a2, a10, 24
42038be5:	03e916        	beqz	a9, 42038c27 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038be8:	1149      	s32i.n	a4, a1, 4
42038bea:	0139      	s32i.n	a3, a1, 0
42038bec:	67e8      	l32i.n	a14, a7, 24
42038bee:	0597d2        	l16si	a13, a7, 10
42038bf1:	0497c2        	l16si	a12, a7, 8
42038bf4:	0397b2        	l16si	a11, a7, 6
42038bf7:	0297a2        	l16si	a10, a7, 4
42038bfa:	ff0c      	movi.n	a15, 15
42038bfc:	ffb325        	call8	42038730 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bff:	61a9      	s32i.n	a10, a1, 24
42038c01:	2aac      	beqz.n	a10, 42038c27 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd3>
42038c03:	7188      	l32i.n	a8, a1, 28
42038c05:	38cc      	bnez.n	a8, 42038c0c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb8>
42038c07:	24c782        	addi	a8, a7, 36
42038c0a:	7189      	s32i.n	a8, a1, 28
42038c0c:	7188      	l32i.n	a8, a1, 28
42038c0e:	02ad      	mov.n	a10, a2
42038c10:	002892        	l32i	a9, a8, 0
42038c13:	062182        	l32i	a8, a1, 24
42038c16:	119980        	slli	a9, a9, 8
42038c19:	419890        	srli	a9, a9, 8
42038c1c:	01b880        	slli	a11, a8, 24
42038c1f:	20b9b0        	or	a11, a9, a11
42038c22:	cec3e5        	call8	42007860 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c25:	0a2d      	mov.n	a2, a10
42038c27:	ffa092        	movi	a9, 255
42038c2a:	469697        	bne	a6, a9, 42038c74 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x120>
42038c2d:	fb7c      	movi.n	a11, -1
42038c2f:	01bb80        	slli	a11, a11, 24
42038c32:	20b5b0        	or	a11, a5, a11
42038c35:	02ad      	mov.n	a10, a2
42038c37:	cec2a5        	call8	42007860 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c3a:	0a2d      	mov.n	a2, a10
42038c3c:	1427a2        	l32i	a10, a7, 80
42038c3f:	f52a16        	beqz	a10, 42038b95 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c42:	20c440        	or	a12, a4, a4
42038c45:	20b330        	or	a11, a3, a3
42038c48:	ffdd65        	call8	42038a20 <roo_windows::PressOverlay::get(short, short) const>
42038c4b:	6198      	l32i.n	a9, a1, 24
42038c4d:	7588a0        	extui	a8, a10, 24, 8
42038c50:	d18890        	mul16s	a8, a8, a9
42038c53:	1f2391        	l32r	a9, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c56:	11aa80        	slli	a10, a10, 8
42038c59:	a28890        	muluh	a8, a8, a9
42038c5c:	0188f0        	slli	a8, a8, 17
42038c5f:	758880        	extui	a8, a8, 24, 8
42038c62:	404800        	ssai	8
42038c65:	81b8a0        	src	a11, a8, a10
42038c68:	02ad      	mov.n	a10, a2
42038c6a:	cebf65        	call8	42007860 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c6d:	0a2d      	mov.n	a2, a10
42038c6f:	ffc886        	j	42038b95 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x41>
42038c72:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038c75:	50fc46        	j	4204d06a <uart_pattern_queue_update$isra$0+0x32>
42038c78:	7598      	l32i.n	a9, a5, 28
42038c7a:	d19960        	mul16s	a9, a9, a6
42038c7d:	1f18a1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
42038c80:	7fc992        	addi	a9, a9, 127
42038c83:	a299a0        	muluh	a9, a9, a10
42038c86:	11b580        	slli	a11, a5, 8
42038c89:	0199f0        	slli	a9, a9, 17
42038c8c:	759890        	extui	a9, a9, 24, 8
42038c8f:	404800        	ssai	8
42038c92:	81b9b0        	src	a11, a9, a11
42038c95:	ffe706        	j	42038c35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xe1>


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038d44 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038d44:	00c136        	entry	a1, 96
42038d47:	078d      	mov.n	a8, a7
42038d49:	069d      	mov.n	a9, a6
42038d4b:	086d      	mov.n	a6, a8
42038d4d:	238980        	sext	a8, a9, 15
42038d50:	3189      	s32i.n	a8, a1, 12
42038d52:	142282        	l32i	a8, a2, 80
42038d55:	027d      	mov.n	a7, a2
42038d57:	233380        	sext	a3, a3, 15
42038d5a:	234480        	sext	a4, a4, 15
42038d5d:	235580        	sext	a5, a5, 15
42038d60:	200856        	bnez	a8, 42038f64 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038d63:	0288      	l32i.n	a8, a2, 0
42038d65:	31e8      	l32i.n	a14, a1, 12
42038d67:	b888      	l32i.n	a8, a8, 44
42038d69:	02ad      	mov.n	a10, a2
42038d6b:	06fd      	mov.n	a15, a6
42038d6d:	05dd      	mov.n	a13, a5
42038d6f:	04cd      	mov.n	a12, a4
42038d71:	03bd      	mov.n	a11, a3
42038d73:	0008e0        	callx8	a8
42038d76:	0a2d      	mov.n	a2, a10
42038d78:	09ca56        	bnez	a10, 42038e18 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd4>
42038d7b:	249792        	l16si	a9, a7, 72
42038d7e:	1a9782        	l16si	a8, a7, 52
42038d81:	ec0c      	movi.n	a12, 14
42038d83:	538890        	max	a8, a8, a9
42038d86:	98fb      	addi.n	a9, a8, 15
42038d88:	1ec882        	addi	a8, a8, 30
42038d8b:	b38990        	movgez	a8, a9, a9
42038d8e:	21b480        	srai	a11, a8, 4
42038d91:	210782        	l8ui	a8, a7, 33
42038d94:	1907a2        	l8ui	a10, a7, 25
42038d97:	62cc80        	saltu	a12, a12, a8
42038d9a:	180782        	l8ui	a8, a7, 24
42038d9d:	200792        	l8ui	a9, a7, 32
42038da0:	7388a0        	maxu	a8, a8, a10
42038da3:	1a07a2        	l8ui	a10, a7, 26
42038da6:	cc0b      	addi.n	a12, a12, -1
42038da8:	7388a0        	maxu	a8, a8, a10
42038dab:	1b07a2        	l8ui	a10, a7, 27
42038dae:	60c0c0        	neg	a12, a12
42038db1:	73a8a0        	maxu	a10, a8, a10
42038db4:	89ca      	add.n	a8, a9, a12
42038db6:	99ba      	add.n	a9, a9, a11
42038db8:	538a80        	max	a8, a10, a8
42038dbb:	99ca      	add.n	a9, a9, a12
42038dbd:	0297a2        	l16si	a10, a7, 4
42038dc0:	539980        	max	a9, a9, a8
42038dc3:	aa9a      	add.n	a10, a10, a9
42038dc5:	53aa30        	max	a10, a10, a3
42038dc8:	11a9      	s32i.n	a10, a1, 4
42038dca:	0497a2        	l16si	a10, a7, 8
42038dcd:	c09a90        	sub	a9, a10, a9
42038dd0:	439950        	min	a9, a9, a5
42038dd3:	5199      	s32i.n	a9, a1, 20
42038dd5:	039792        	l16si	a9, a7, 6
42038dd8:	998a      	add.n	a9, a9, a8
42038dda:	99ba      	add.n	a9, a9, a11
42038ddc:	9199      	s32i.n	a9, a1, 36
42038dde:	059792        	l16si	a9, a7, 10
42038de1:	c08980        	sub	a8, a9, a8
42038de4:	4189      	s32i.n	a8, a1, 16
42038de6:	c08530        	sub	a8, a5, a3
42038de9:	881b      	addi.n	a8, a8, 1
42038deb:	6189      	s32i.n	a8, a1, 24
42038ded:	5188      	l32i.n	a8, a1, 20
42038def:	1198      	l32i.n	a9, a1, 4
42038df1:	881b      	addi.n	a8, a8, 1
42038df3:	7189      	s32i.n	a8, a1, 28
42038df5:	c08890        	sub	a8, a8, a9
42038df8:	1188e0        	slli	a8, a8, 2
42038dfb:	a189      	s32i.n	a8, a1, 40
42038dfd:	6188      	l32i.n	a8, a1, 24
42038dff:	1188e0        	slli	a8, a8, 2
42038e02:	2189      	s32i.n	a8, a1, 8
42038e04:	c08930        	sub	a8, a9, a3
42038e07:	1188e0        	slli	a8, a8, 2
42038e0a:	b189      	s32i.n	a8, a1, 44
42038e0c:	029182        	l16si	a8, a1, 4
42038e0f:	8189      	s32i.n	a8, a1, 32
42038e11:	3188      	l32i.n	a8, a1, 12
42038e13:	03a847        	bge	a8, a4, 42038e1a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd6>
42038e16:	020c      	movi.n	a2, 0
42038e18:	f01d      	retw.n
42038e1a:	5198      	l32i.n	a9, a1, 20
42038e1c:	11a8      	l32i.n	a10, a1, 4
42038e1e:	038d      	mov.n	a8, a3
42038e20:	062d      	mov.n	a2, a6
42038e22:	03bd      	mov.n	a11, a3
42038e24:	23c480        	sext	a12, a4, 15
42038e27:	0229a7        	blt	a9, a10, 42038e2d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038e2a:	002406        	j	42038ebe <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x17a>
42038e2d:	029d      	mov.n	a9, a2
42038e2f:	08bd      	mov.n	a11, a8
42038e31:	23c480        	sext	a12, a4, 15
42038e34:	0225b7        	blt	a5, a11, 42038e3a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf6>
42038e37:	002b46        	j	42038ee8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a4>
42038e3a:	a51b      	addi.n	a10, a5, 1
42038e3c:	729a80        	salt	a9, a10, a8
42038e3f:	990b      	addi.n	a9, a9, -1
42038e41:	609090        	neg	a9, a9
42038e44:	749090        	extui	a9, a9, 0, 8
42038e47:	0b0c      	movi.n	a11, 0
42038e49:	498c      	beqz.n	a9, 42038e51 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10d>
42038e4b:	c0aa80        	sub	a10, a10, a8
42038e4e:	11bae0        	slli	a11, a10, 2
42038e51:	9198      	l32i.n	a9, a1, 36
42038e53:	22ba      	add.n	a2, a2, a11
42038e55:	729490        	salt	a9, a4, a9
42038e58:	990b      	addi.n	a9, a9, -1
42038e5a:	609090        	neg	a9, a9
42038e5d:	749090        	extui	a9, a9, 0, 8
42038e60:	028d      	mov.n	a8, a2
42038e62:	79bc      	beqz.n	a9, 42038e9d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038e64:	4198      	l32i.n	a9, a1, 16
42038e66:	729940        	salt	a9, a9, a4
42038e69:	990b      	addi.n	a9, a9, -1
42038e6b:	609090        	neg	a9, a9
42038e6e:	749090        	extui	a9, a9, 0, 8
42038e71:	89ac      	beqz.n	a9, 42038e9d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x159>
42038e73:	3198      	l32i.n	a9, a1, 12
42038e75:	41a8      	l32i.n	a10, a1, 16
42038e77:	04dd      	mov.n	a13, a4
42038e79:	4399a0        	min	a9, a9, a10
42038e7c:	02ad97        	bge	a13, a9, 42038e82 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x13e>
42038e7f:	0022c6        	j	42038f0e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038e82:	728940        	salt	a8, a9, a4
42038e85:	880b      	addi.n	a8, a8, -1
42038e87:	608080        	neg	a8, a8
42038e8a:	748080        	extui	a8, a8, 0, 8
42038e8d:	0a0c      	movi.n	a10, 0
42038e8f:	188c      	beqz.n	a8, 42038e94 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x150>
42038e91:	c0a940        	sub	a10, a9, a4
42038e94:	2188      	l32i.n	a8, a1, 8
42038e96:	44aa      	add.n	a4, a4, a10
42038e98:	828a80        	mull	a8, a10, a8
42038e9b:	882a      	add.n	a8, a8, a2
42038e9d:	441b      	addi.n	a4, a4, 1
42038e9f:	086d      	mov.n	a6, a8
42038ea1:	ffdb06        	j	42038e11 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcd>
42038ea4:	0527d2        	l32i	a13, a7, 20
42038ea7:	00a0e2        	movi	a14, 0
42038eaa:	20a770        	or	a10, a7, a7
42038ead:	d1c9      	s32i.n	a12, a1, 52
42038eaf:	c1b9      	s32i.n	a11, a1, 48
42038eb1:	ffca25        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038eb4:	c1b8      	l32i.n	a11, a1, 48
42038eb6:	d1c8      	l32i.n	a12, a1, 52
42038eb8:	02a9      	s32i.n	a10, a2, 0
42038eba:	bb1b      	addi.n	a11, a11, 1
42038ebc:	224b      	addi.n	a2, a2, 4
42038ebe:	012182        	l32i	a8, a1, 4
42038ec1:	df2b87        	blt	a11, a8, 42038ea4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x160>
42038ec4:	0b2182        	l32i	a8, a1, 44
42038ec7:	0527d2        	l32i	a13, a7, 20
42038eca:	81b8      	l32i.n	a11, a1, 32
42038ecc:	0e0c      	movi.n	a14, 0
42038ece:	07ad      	mov.n	a10, a7
42038ed0:	268a      	add.n	a2, a6, a8
42038ed2:	ffc825        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038ed5:	a188      	l32i.n	a8, a1, 40
42038ed7:	828a      	add.n	a8, a2, a8
42038ed9:	049827        	bne	a8, a2, 42038ee1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x19d>
42038edc:	7188      	l32i.n	a8, a1, 28
42038ede:	ffd2c6        	j	42038e2d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe9>
42038ee1:	02a9      	s32i.n	a10, a2, 0
42038ee3:	224b      	addi.n	a2, a2, 4
42038ee5:	fffc06        	j	42038ed9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x195>
42038ee8:	0527d2        	l32i	a13, a7, 20
42038eeb:	00a0e2        	movi	a14, 0
42038eee:	20a770        	or	a10, a7, a7
42038ef1:	f199      	s32i.n	a9, a1, 60
42038ef3:	e189      	s32i.n	a8, a1, 56
42038ef5:	d1c9      	s32i.n	a12, a1, 52
42038ef7:	c1b9      	s32i.n	a11, a1, 48
42038ef9:	ffc5a5        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038efc:	f198      	l32i.n	a9, a1, 60
42038efe:	c1b8      	l32i.n	a11, a1, 48
42038f00:	09a9      	s32i.n	a10, a9, 0
42038f02:	e188      	l32i.n	a8, a1, 56
42038f04:	d1c8      	l32i.n	a12, a1, 52
42038f06:	bb1b      	addi.n	a11, a11, 1
42038f08:	994b      	addi.n	a9, a9, 4
42038f0a:	ffc986        	j	42038e34 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xf0>
42038f0d:	00          	.byte	00
42038f0e:	61a8      	l32i.n	a10, a1, 24
42038f10:	1a1aa6        	blti	a10, 1, 42038f2e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038f13:	21a8      	l32i.n	a10, a1, 8
42038f15:	1f4a26        	beqi	a10, 4, 42038f38 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f4>
42038f18:	0acd      	mov.n	a12, a10
42038f1a:	06bd      	mov.n	a11, a6
42038f1c:	08ad      	mov.n	a10, a8
42038f1e:	d1d9      	s32i.n	a13, a1, 52
42038f20:	c199      	s32i.n	a9, a1, 48
42038f22:	1e7081        	l32r	a8, 420008e4 <_stext+0x8c4> (40001200 <memmove>)
42038f25:	0008e0        	callx8	a8
42038f28:	c198      	l32i.n	a9, a1, 48
42038f2a:	d1d8      	l32i.n	a13, a1, 52
42038f2c:	0a8d      	mov.n	a8, a10
42038f2e:	21a8      	l32i.n	a10, a1, 8
42038f30:	dd1b      	addi.n	a13, a13, 1
42038f32:	88aa      	add.n	a8, a8, a10
42038f34:	ffd106        	j	42038e7c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x138>
42038f37:	06a800        	ee.ldf.64.xp	f10, f0, a0, a8
42038f3a:	08a9      	s32i.n	a10, a8, 0
42038f3c:	fffb86        	j	42038f2e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ea>
42038f3f:	57d800        	ee.stf.64.xp	f13, f5, a0, a8
42038f42:	02bd      	mov.n	a11, a2
42038f44:	0e0c      	movi.n	a14, 0
42038f46:	04cd      	mov.n	a12, a4
42038f48:	07ad      	mov.n	a10, a7
42038f4a:	1189      	s32i.n	a8, a1, 4
42038f4c:	ffc065        	call8	42038b54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42038f4f:	1188      	l32i.n	a8, a1, 4
42038f51:	221b      	addi.n	a2, a2, 1
42038f53:	08a9      	s32i.n	a10, a8, 0
42038f55:	232280        	sext	a2, a2, 15
42038f58:	884b      	addi.n	a8, a8, 4
42038f5a:	086d      	mov.n	a6, a8
42038f5c:	e0a527        	bge	a5, a2, 42038f40 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1fc>
42038f5f:	441b      	addi.n	a4, a4, 1
42038f61:	234480        	sext	a4, a4, 15
42038f64:	3188      	l32i.n	a8, a1, 12
42038f66:	02a847        	bge	a8, a4, 42038f6c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038f69:	ffaa46        	j	42038e16 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xd2>
42038f6c:	068d      	mov.n	a8, a6
42038f6e:	032d      	mov.n	a2, a3
42038f70:	fff986        	j	42038f5a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/compact_corners/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039138 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
42039138:	00a136        	entry	a1, 80
4203913b:	238480        	sext	a8, a4, 15
4203913e:	026d      	mov.n	a6, a2
42039140:	9189      	s32i.n	a8, a1, 36
42039142:	0f0222        	l8ui	a2, a2, 15
42039145:	238580        	sext	a8, a5, 15
42039148:	0139      	s32i.n	a3, a1, 0
4203914a:	a189      	s32i.n	a8, a1, 40
4203914c:	0d0216        	beqz	a2, 42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe8>
4203914f:	009682        	l16si	a8, a6, 0
42039152:	0696e2        	l16si	a14, a6, 12
42039155:	4189      	s32i.n	a8, a1, 16
42039157:	4198      	l32i.n	a9, a1, 16
42039159:	9188      	l32i.n	a8, a1, 36
4203915b:	c08890        	sub	a8, a8, a9
4203915e:	1198c0        	slli	a9, a8, 4
42039161:	1189      	s32i.n	a8, a1, 4
42039163:	019682        	l16si	a8, a6, 2
42039166:	998b      	addi.n	a9, a9, 8
42039168:	5189      	s32i.n	a8, a1, 20
4203916a:	51a8      	l32i.n	a10, a1, 20
4203916c:	a188      	l32i.n	a8, a1, 40
4203916e:	c088a0        	sub	a8, a8, a10
42039171:	2189      	s32i.n	a8, a1, 8
42039173:	0693a2        	l16si	a10, a3, 12
42039176:	1188c0        	slli	a8, a8, 4
42039179:	888b      	addi.n	a8, a8, 8
4203917b:	c058e0        	sub	a5, a8, a14
4203917e:	c088a0        	sub	a8, a8, a10
42039181:	a99a      	add.n	a10, a9, a9
42039183:	61a9      	s32i.n	a10, a1, 24
42039185:	0296a2        	l16si	a10, a6, 4
42039188:	d55a      	add.n	a13, a5, a5
4203918a:	71a9      	s32i.n	a10, a1, 28
4203918c:	11aac0        	slli	a10, a10, 4
4203918f:	31a9      	s32i.n	a10, a1, 12
42039191:	3178      	l32i.n	a7, a1, 12
42039193:	0396a2        	l16si	a10, a6, 6
42039196:	f88a      	add.n	a15, a8, a8
42039198:	c07790        	sub	a7, a7, a9
4203919b:	81a9      	s32i.n	a10, a1, 32
4203919d:	11aac0        	slli	a10, a10, 4
420391a0:	c04a80        	sub	a4, a10, a8
420391a3:	72bda0        	salt	a11, a13, a10
420391a6:	72cfa0        	salt	a12, a15, a10
420391a9:	437790        	min	a7, a7, a9
420391ac:	c09a50        	sub	a9, a10, a5
420391af:	bb0b      	addi.n	a11, a11, -1
420391b1:	cc0b      	addi.n	a12, a12, -1
420391b3:	439950        	min	a9, a9, a5
420391b6:	434480        	min	a4, a4, a8
420391b9:	3158      	l32i.n	a5, a1, 12
420391bb:	6188      	l32i.n	a8, a1, 24
420391bd:	60b0b0        	neg	a11, a11
420391c0:	60c0c0        	neg	a12, a12
420391c3:	74b0b0        	extui	a11, a11, 0, 8
420391c6:	74c0c0        	extui	a12, a12, 0, 8
420391c9:	5f2857        	blt	a8, a5, 4203922c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf4>
420391cc:	542da7        	blt	a13, a10, 42039224 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xec>
420391cf:	0a0682        	l8ui	a8, a6, 10
420391d2:	5f1bc7        	beq	a11, a12, 42039235 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xfd>
420391d5:	6b2fa7        	blt	a15, a10, 42039244 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10c>
420391d8:	0a0652        	l8ui	a5, a6, 10
420391db:	233880        	sext	a3, a8, 15
420391de:	1133c0        	slli	a3, a3, 4
420391e1:	772937        	blt	a9, a3, 4203925c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x124>
420391e4:	23f580        	sext	a15, a5, 15
420391e7:	11ffc0        	slli	a15, a15, 4
420391ea:	6e24f7        	blt	a4, a15, 4203925c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x124>
420391ed:	43f940        	min	a15, a9, a4
420391f0:	682f77        	blt	a15, a7, 4203925c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x124>
420391f3:	f8c772        	addi	a7, a7, -8
420391f6:	607070        	neg	a7, a7
420391f9:	217470        	srai	a7, a7, 4
420391fc:	086c      	movi.n	a8, -32
420391fe:	02a787        	bge	a7, a8, 42039204 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xcc>
42039201:	0097c6        	j	42039464 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x32c>
42039204:	082c      	movi.n	a8, 32
42039206:	02a877        	bge	a8, a7, 4203920c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd4>
42039209:	009746        	j	4203946a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x332>
4203920c:	220b      	addi.n	a2, a2, -1
4203920e:	902220        	addx2	a2, a2, a2
42039211:	1182a0        	slli	a8, a2, 6
42039214:	228a      	add.n	a2, a2, a8
42039216:	1db781        	l32r	a8, 420008f4 <_stext+0x8d4> (3c0bc77c <roo_windows::internal::(anonymous namespace)::kSideGradients>)
42039219:	887a      	add.n	a8, a8, a7
4203921b:	882a      	add.n	a8, a8, a2
4203921d:	200822        	l8ui	a2, a8, 32
42039220:	f01d      	retw.n
42039222:	820000        	mull	a0, a0, a0
42039225:	c60906        	j	4202aa4d <roo_windows::DisplayWindow::refreshPaint()+0xad>
42039228:	ffe9      	s32i.n	a14, a15, 60
4203922a:	a70000        	ee.stf.64.xp	f0, f10, a0, a0
4203922d:	0c2d      	mov.n	a2, a12
4203922f:	0b0682        	l8ui	a8, a6, 11
42039232:	169bc7        	bne	a11, a12, 4203924c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x114>
42039235:	085d      	mov.n	a5, a8
42039237:	ffe806        	j	420391db <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xa3>
4203923a:	820000        	mull	a0, a0, a0
4203923d:	c60806        	j	4202aa61 <roo_windows::DisplayWindow::refreshPaint()+0xc1>
42039240:	fffb      	addi.n	a15, a15, 15
42039242:	00          	.byte	00
42039243:	065200        	ee.ldf.64.xp	f5, f0, a0, a2
42039246:	0609      	s32i.n	a0, a6, 0
42039248:	e4          	.byte	0xe4
42039249:	a70000ff 	ee.fft.ams.s16.st.incp	q0, q6, a15, a0, q0, q4, q0, 1
4203924d:	0652052f 	ee.vmulas.s16.accx.ld.ip.qup	q4, a2, 0x150, q4, q0, q5, q2
42039251:	460b      	addi.n	a4, a6, -1
42039253:	52ffe1        	l32r	a14, 4200de50 <roo_display::(anonymous namespace)::PrepareComposition(std::vector<roo_display::StreamableStack::Input, std::allocator<roo_display::StreamableStack::Input> > const&, roo_display::Box const&, roo_display::(anonymous namespace)::Program*, std::vector<roo_display::internal::BufferingStream, std::allocator<roo_display::internal::BufferingStream> >*, std::vector<roo_display::BlendingMode, std::allocator<roo_display::BlendingMode> >*)+0xc> (132212 <UserFrameTotalSize+0x132112>)
42039256:	c60806        	j	4202aa7a <roo_windows::DisplayWindow::refreshPaint()+0xda>
42039259:	c700ffdf 	ee.vmulas.u16.qacc.ld.xp.qup	q6, a13, a15, q7, q6, q0, q0
4203925d:	021b      	addi.n	a0, a2, 1
4203925f:	004246        	j	4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
42039262:	53c580        	max	a12, a5, a8
42039265:	23cc80        	sext	a12, a12, 15
42039268:	11ccc0        	slli	a12, a12, 4
4203926b:	4b27c7        	blt	a7, a12, 420392ba <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x182>
4203926e:	53c940        	max	a12, a9, a4
42039271:	4527c7        	blt	a7, a12, 420392ba <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x182>
42039274:	f8c992        	addi	a9, a9, -8
42039277:	609090        	neg	a9, a9
4203927a:	02ada7        	bge	a13, a10, 42039280 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x148>
4203927d:	60e0e0        	neg	a14, a14
42039280:	99ea      	add.n	a9, a9, a14
42039282:	219490        	srai	a9, a9, 4
42039285:	180c      	movi.n	a8, 1
42039287:	012da7        	blt	a13, a10, 4203928c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x154>
4203928a:	280c      	movi.n	a8, 2
4203928c:	0a6c      	movi.n	a10, -32
4203928e:	02a9a7        	bge	a9, a10, 42039294 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x15c>
42039291:	0073c6        	j	42039464 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x32c>
42039294:	0a2c      	movi.n	a10, 32
42039296:	02aa97        	bge	a10, a9, 4203929c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x164>
42039299:	007346        	j	4203946a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x332>
4203929c:	220b      	addi.n	a2, a2, -1
4203929e:	232280        	sext	a2, a2, 15
420392a1:	902220        	addx2	a2, a2, a2
420392a4:	828a      	add.n	a8, a2, a8
420392a6:	238880        	sext	a8, a8, 15
420392a9:	11a8a0        	slli	a10, a8, 6
420392ac:	aa8a      	add.n	a10, a10, a8
420392ae:	1d9181        	l32r	a8, 420008f4 <_stext+0x8d4> (3c0bc77c <roo_windows::internal::(anonymous namespace)::kSideGradients>)
420392b1:	889a      	add.n	a8, a8, a9
420392b3:	88aa      	add.n	a8, a8, a10
420392b5:	ffd906        	j	4203921d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe5>
420392b8:	00          	.byte	00
420392b9:	00          	.byte	00
420392ba:	0c0c      	movi.n	a12, 0
420392bc:	1e0c      	movi.n	a14, 1
420392be:	83ec80        	moveqz	a14, a12, a8
420392c1:	b1e9      	s32i.n	a14, a1, 44
420392c3:	c0f580        	sub	a15, a5, a8
420392c6:	1e0c      	movi.n	a14, 1
420392c8:	83cef0        	moveqz	a12, a14, a15
420392cb:	b1e8      	l32i.n	a14, a1, 44
420392cd:	028ec7        	bany	a14, a12, 420392d3 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x19b>
420392d0:	002606        	j	4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
420392d3:	8c1c      	movi.n	a12, 24
420392d5:	72cc80        	salt	a12, a12, a8
420392d8:	cc0b      	addi.n	a12, a12, -1
420392da:	60c0c0        	neg	a12, a12
420392dd:	74c0c0        	extui	a12, a12, 0, 8
420392e0:	088c16        	beqz	a12, 4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
420392e3:	14c080        	extui	a12, a8, 0, 2
420392e6:	082c56        	bnez	a12, 4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
420392e9:	220b      	addi.n	a2, a2, -1
420392eb:	218280        	srai	a8, a8, 2
420392ee:	232280        	sext	a2, a2, 15
420392f1:	880b      	addi.n	a8, a8, -1
420392f3:	902220        	addx2	a2, a2, a2
420392f6:	902280        	addx2	a2, a2, a8
420392f9:	232280        	sext	a2, a2, 15
420392fc:	9022b0        	addx2	a2, a2, a11
420392ff:	6188      	l32i.n	a8, a1, 24
42039301:	31b8      	l32i.n	a11, a1, 12
42039303:	0e28b7        	blt	a8, a11, 42039315 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1dd>
42039306:	71b8      	l32i.n	a11, a1, 28
42039308:	4188      	l32i.n	a8, a1, 16
4203930a:	88ba      	add.n	a8, a8, a11
4203930c:	91b8      	l32i.n	a11, a1, 36
4203930e:	880b      	addi.n	a8, a8, -1
42039310:	c088b0        	sub	a8, a8, a11
42039313:	1189      	s32i.n	a8, a1, 4
42039315:	0f2da7        	blt	a13, a10, 42039328 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1f0>
42039318:	81a8      	l32i.n	a10, a1, 32
4203931a:	5188      	l32i.n	a8, a1, 20
4203931c:	88aa      	add.n	a8, a8, a10
4203931e:	a1a8      	l32i.n	a10, a1, 40
42039320:	880b      	addi.n	a8, a8, -1
42039322:	c088a0        	sub	a8, a8, a10
42039325:	026182        	s32i	a8, a1, 8
42039328:	1d74a1        	l32r	a10, 420008f8 <_stext+0x8d8> (3c0bc59c <roo_windows::internal::(anonymous namespace)::kCornerGradients>)
4203932b:	1188      	l32i.n	a8, a1, 4
4203932d:	b0a2a0        	addx8	a10, a2, a10
42039330:	040ab2        	l8ui	a11, a10, 4
42039333:	21c8      	l32i.n	a12, a1, 8
42039335:	23bb00        	sext	a11, a11, 7
42039338:	c0b8b0        	sub	a11, a8, a11
4203933b:	050a82        	l8ui	a8, a10, 5
4203933e:	020c      	movi.n	a2, 0
42039340:	238800        	sext	a8, a8, 7
42039343:	c08c80        	sub	a8, a12, a8
42039346:	20cb80        	or	a12, a11, a8
42039349:	ed3c96        	bltz	a12, 42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe8>
4203934c:	060ac2        	l8ui	a12, a10, 6
4203934f:	19abc7        	bge	a11, a12, 4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
42039352:	070ad2        	l8ui	a13, a10, 7
42039355:	13a8d7        	bge	a8, a13, 4203936c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x234>
42039358:	1d6991        	l32r	a9, 420008fc <_stext+0x8dc> (3c0b1058 <roo_windows::internal::(anonymous namespace)::kCornerPixels>)
4203935b:	8288c0        	mull	a8, a8, a12
4203935e:	0aa8      	l32i.n	a10, a10, 0
42039360:	99ba      	add.n	a9, a9, a11
42039362:	898a      	add.n	a8, a9, a8
42039364:	88aa      	add.n	a8, a8, a10
42039366:	000822        	l8ui	a2, a8, 0
42039369:	ffacc6        	j	42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe8>
4203936c:	091682        	l16ui	a8, a6, 18
4203936f:	43a790        	min	a10, a7, a9
42039372:	60b080        	neg	a11, a8
42039375:	00a022        	movi	a2, 0
42039378:	522ab7        	blt	a10, a11, 420393ce <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x296>
4203937b:	c0b390        	sub	a11, a3, a9
4203937e:	c0a370        	sub	a10, a3, a7
42039381:	0222b7        	blt	a2, a11, 42039387 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x24f>
42039384:	003206        	j	42039450 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x318>
42039387:	0222a7        	blt	a2, a10, 4203938d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x255>
4203938a:	003086        	j	42039450 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x318>
4203938d:	808380        	add	a8, a3, a8
42039390:	82aaa0        	mull	a10, a10, a10
42039393:	82bbb0        	mull	a11, a11, a11
42039396:	01c882        	addi	a8, a8, 1
42039399:	828880        	mull	a8, a8, a8
4203939c:	aaba      	add.n	a10, a10, a11
4203939e:	2cba87        	bgeu	a10, a8, 420393ce <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x296>
420393a1:	4d36e5        	call8	42086710 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
420393a4:	0e0692        	l8ui	a9, a6, 14
420393a7:	c08a30        	sub	a8, a10, a3
420393aa:	c08890        	sub	a8, a8, a9
420393ad:	081692        	l16ui	a9, a6, 16
420393b0:	828890        	mull	a8, a8, a9
420393b3:	1d5491        	l32r	a9, 42000904 <_stext+0x8e4> (80000 <UserFrameTotalSize+0x7ff00>)
420393b6:	889a      	add.n	a8, a8, a9
420393b8:	218c80        	srai	a8, a8, 12
420393bb:	ffa092        	movi	a9, 255
420393be:	438890        	min	a8, a8, a9
420393c1:	090c      	movi.n	a9, 0
420393c3:	538890        	max	a8, a8, a9
420393c6:	1d4e91        	l32r	a9, 42000900 <_stext+0x8e0> (3c0bcc4b <roo_windows::internal::(anonymous namespace)::kKey>)
420393c9:	898a      	add.n	a8, a9, a8
420393cb:	000822        	l8ui	a2, a8, 0
420393ce:	0188      	l32i.n	a8, a1, 0
420393d0:	439740        	min	a9, a7, a4
420393d3:	0918b2        	l16ui	a11, a8, 18
420393d6:	080c      	movi.n	a8, 0
420393d8:	60a0b0        	neg	a10, a11
420393db:	5529a7        	blt	a9, a10, 42039434 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2fc>
420393de:	235580        	sext	a5, a5, 15
420393e1:	1155c0        	slli	a5, a5, 4
420393e4:	c0a570        	sub	a10, a5, a7
420393e7:	c09540        	sub	a9, a5, a4
420393ea:	6ea8a7        	bge	a8, a10, 4203945c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x324>
420393ed:	6ba897        	bge	a8, a9, 4203945c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x324>
420393f0:	829990        	mull	a9, a9, a9
420393f3:	82aaa0        	mull	a10, a10, a10
420393f6:	a9aa      	add.n	a10, a9, a10
420393f8:	9b5a      	add.n	a9, a11, a5
420393fa:	01c992        	addi	a9, a9, 1
420393fd:	829990        	mull	a9, a9, a9
42039400:	30ba97        	bgeu	a10, a9, 42039434 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2fc>
42039403:	4d30e5        	call8	42086710 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039406:	0198      	l32i.n	a9, a1, 0
42039408:	c08a50        	sub	a8, a10, a5
4203940b:	0e0992        	l8ui	a9, a9, 14
4203940e:	c08890        	sub	a8, a8, a9
42039411:	0198      	l32i.n	a9, a1, 0
42039413:	081992        	l16ui	a9, a9, 16
42039416:	828890        	mull	a8, a8, a9
42039419:	1d3a91        	l32r	a9, 42000904 <_stext+0x8e4> (80000 <UserFrameTotalSize+0x7ff00>)
4203941c:	889a      	add.n	a8, a8, a9
4203941e:	218c80        	srai	a8, a8, 12
42039421:	ffa092        	movi	a9, 255
42039424:	438890        	min	a8, a8, a9
42039427:	090c      	movi.n	a9, 0
42039429:	538890        	max	a8, a8, a9
4203942c:	1d3791        	l32r	a9, 42000908 <_stext+0x8e8> (3c0bcb4b <roo_windows::internal::(anonymous namespace)::kAmbient>)
4203942f:	898a      	add.n	a8, a9, a8
42039431:	000882        	l8ui	a8, a8, 0
42039434:	d19280        	mul16s	a9, a2, a8
42039437:	1d2aa1        	l32r	a10, 420008e0 <_stext+0x8c0> (80808081 <_rtc_reserved_end+0x20708081>)
4203943a:	7fc992        	addi	a9, a9, 127
4203943d:	b2a9a0        	mulsh	a10, a9, a10
42039440:	282a      	add.n	a2, a8, a2
42039442:	99aa      	add.n	a9, a9, a10
42039444:	219790        	srai	a9, a9, 7
42039447:	c02290        	sub	a2, a2, a9
4203944a:	742020        	extui	a2, a2, 0, 8
4203944d:	ff73c6        	j	42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe8>
42039450:	022977        	blt	a9, a7, 42039456 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x31e>
42039453:	ffd346        	j	420393a4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x26c>
42039456:	0bad      	mov.n	a10, a11
42039458:	ffd206        	j	420393a4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x26c>
4203945b:	a47700        	extui	a7, a0, 7, 11
4203945e:	09ada6        	blti	a13, 12, 4203946b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x333>
42039461:	ffe846        	j	42039406 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2ce>
42039464:	68a022        	movi	a2, 104
42039467:	ff6d46        	j	42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe8>
4203946a:	020c      	movi.n	a2, 0
4203946c:	ff6c06        	j	42039220 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe8>
