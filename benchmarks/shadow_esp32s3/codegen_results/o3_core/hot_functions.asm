
/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033190 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42033190:	006136        	entry	a1, 48
42033193:	002282        	l32i	a8, a2, 0
42033196:	20f770        	or	a15, a7, a7
42033199:	0b2882        	l32i	a8, a8, 44
4203319c:	233380        	sext	a3, a3, 15
4203319f:	234480        	sext	a4, a4, 15
420331a2:	235580        	sext	a5, a5, 15
420331a5:	236680        	sext	a6, a6, 15
420331a8:	02ad      	mov.n	a10, a2
420331aa:	31f9      	s32i.n	a15, a1, 12
420331ac:	06ed      	mov.n	a14, a6
420331ae:	05dd      	mov.n	a13, a5
420331b0:	04cd      	mov.n	a12, a4
420331b2:	03bd      	mov.n	a11, a3
420331b4:	0008e0        	callx8	a8
420331b7:	027d      	mov.n	a7, a2
420331b9:	0a2d      	mov.n	a2, a10
420331bb:	facc      	bnez.n	a10, 420331ce <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
420331bd:	31f8      	l32i.n	a15, a1, 12
420331bf:	06ed      	mov.n	a14, a6
420331c1:	05dd      	mov.n	a13, a5
420331c3:	04cd      	mov.n	a12, a4
420331c5:	03bd      	mov.n	a11, a3
420331c7:	07ad      	mov.n	a10, a7
420331c9:	dc5565        	call8	4200f720 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
420331cc:	0a2d      	mov.n	a2, a10
420331ce:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203b234 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
4203b234:	004136        	entry	a1, 32
4203b237:	02ad      	mov.n	a10, a2
4203b239:	0f0222        	l8ui	a2, a2, 15
4203b23c:	234480        	sext	a4, a4, 15
4203b23f:	235580        	sext	a5, a5, 15
4203b242:	084216        	beqz	a2, 4203b2ca <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x96>
4203b245:	019a92        	l16si	a9, a10, 2
4203b248:	009a82        	l16si	a8, a10, 0
4203b24b:	c05590        	sub	a5, a5, a9
4203b24e:	c04480        	sub	a4, a4, a8
4203b251:	0693c2        	l16si	a12, a3, 12
4203b254:	1185c0        	slli	a8, a5, 4
4203b257:	069ad2        	l16si	a13, a10, 12
4203b25a:	029ab2        	l16si	a11, a10, 4
4203b25d:	039ae2        	l16si	a14, a10, 6
4203b260:	888b      	addi.n	a8, a8, 8
4203b262:	1194c0        	slli	a9, a4, 4
4203b265:	c0d8d0        	sub	a13, a8, a13
4203b268:	998b      	addi.n	a9, a9, 8
4203b26a:	c088c0        	sub	a8, a8, a12
4203b26d:	11eec0        	slli	a14, a14, 4
4203b270:	11bbc0        	slli	a11, a11, 4
4203b273:	c04b90        	sub	a4, a11, a9
4203b276:	c0ced0        	sub	a12, a14, a13
4203b279:	c02e80        	sub	a2, a14, a8
4203b27c:	599a      	add.n	a5, a9, a9
4203b27e:	7dda      	add.n	a7, a13, a13
4203b280:	f88a      	add.n	a15, a8, a8
4203b282:	434490        	min	a4, a4, a9
4203b285:	43ccd0        	min	a12, a12, a13
4203b288:	432280        	min	a2, a2, a8
4203b28b:	3d25b7        	blt	a5, a11, 4203b2cc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x98>
4203b28e:	49a7e7        	bge	a7, a14, 4203b2db <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xa7>
4203b291:	090ad2        	l8ui	a13, a10, 9
4203b294:	5c2fe7        	blt	a15, a14, 4203b2f4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc0>
4203b297:	0a0a72        	l8ui	a7, a10, 10
4203b29a:	04bd      	mov.n	a11, a4
4203b29c:	ffcea5        	call8	4203af88 <unsigned char roo_windows::internal::(anonymous namespace)::ReadUncachedGradientLayer<false>(roo_windows::internal::LookupShadowSpec const&, int, int, int)>
4203b29f:	0a5d      	mov.n	a5, a10
4203b2a1:	02cd      	mov.n	a12, a2
4203b2a3:	07dd      	mov.n	a13, a7
4203b2a5:	04bd      	mov.n	a11, a4
4203b2a7:	03ad      	mov.n	a10, a3
4203b2a9:	ffc325        	call8	4203aedc <unsigned char roo_windows::internal::(anonymous namespace)::ReadUncachedGradientLayer<true>(roo_windows::internal::LookupShadowSpec const&, int, int, int)>
4203b2ac:	d185a0        	mul16s	a8, a5, a10
4203b2af:	159991        	l32r	a9, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203b2b2:	7fc882        	addi	a8, a8, 127
4203b2b5:	b29890        	mulsh	a9, a8, a9
4203b2b8:	312f80        	srai	a2, a8, 31
4203b2bb:	889a      	add.n	a8, a8, a9
4203b2bd:	218780        	srai	a8, a8, 7
4203b2c0:	c02280        	sub	a2, a2, a8
4203b2c3:	55aa      	add.n	a5, a5, a10
4203b2c5:	225a      	add.n	a2, a2, a5
4203b2c7:	742020        	extui	a2, a2, 0, 8
4203b2ca:	f01d      	retw.n
4203b2cc:	1827e7        	blt	a7, a14, 4203b2e8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb4>
4203b2cf:	0b0ad2        	l8ui	a13, a10, 11
4203b2d2:	1eafe7        	bge	a15, a14, 4203b2f4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc0>
4203b2d5:	080a72        	l8ui	a7, a10, 8
4203b2d8:	ffef86        	j	4203b29a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x66>
4203b2db:	0a0ad2        	l8ui	a13, a10, 10
4203b2de:	12afe7        	bge	a15, a14, 4203b2f4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc0>
4203b2e1:	090a72        	l8ui	a7, a10, 9
4203b2e4:	ffec86        	j	4203b29a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x66>
4203b2e7:	0ad200        	add.s	f13, f2, f0
4203b2ea:	e708      	l32i.n	a0, a7, 56
4203b2ec:	0a72052f 	ee.vmulas.s16.accx.ld.ip.qup	q4, a2, 0xfffffe50, q4, q0, q7, q2
4203b2f0:	460b      	addi.n	a4, a6, -1
4203b2f2:	ffe9      	s32i.n	a14, a15, 60
4203b2f4:	0d7d      	mov.n	a7, a13
4203b2f6:	ffe806        	j	4203b29a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x66>


/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033f04 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033f04:	00a136        	entry	a1, 80
42033f07:	3169      	s32i.n	a6, a1, 12
42033f09:	2159      	s32i.n	a5, a1, 8
42033f0b:	060c      	movi.n	a6, 0
42033f0d:	1cc172        	addi	a7, a1, 28
42033f10:	2188      	l32i.n	a8, a1, 8
42033f12:	029687        	bne	a6, a8, 42033f18 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033f15:	f01d      	retw.n
42033f17:	938200        	movnez	a8, a2, a0
42033f1a:	22d200        	orb	b13, b2, b0
42033f1d:	018917        	bany	a9, a1, 42033f22 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033f20:	009482        	l16si	a8, a4, 0
42033f23:	01b8      	l32i.n	a11, a1, 0
42033f25:	1622a2        	l32i	a10, a2, 88
42033f28:	08cd      	mov.n	a12, a8
42033f2a:	0067d2        	s32i	a13, a7, 0
42033f2d:	20e770        	or	a14, a7, a7
42033f30:	016182        	s32i	a8, a1, 4
42033f33:	201110        	or	a1, a1, a1
42033f36:	fffa25        	call8	42033ed8 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033f39:	182282        	l32i	a8, a2, 96
42033f3c:	1922b2        	l32i	a11, a2, 100
42033f3f:	8189      	s32i.n	a8, a1, 32
42033f41:	20c152        	addi	a5, a1, 32
42033f44:	050b16        	beqz	a11, 42033f98 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033f47:	0bc8      	l32i.n	a12, a11, 0
42033f49:	24c1a2        	addi	a10, a1, 36
42033f4c:	201110        	or	a1, a1, a1
42033f4f:	ff3665        	call8	420332b4 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f52:	07d8      	l32i.n	a13, a7, 0
42033f54:	11c8      	l32i.n	a12, a1, 4
42033f56:	0021b2        	l32i	a11, a1, 0
42033f59:	24c1a2        	addi	a10, a1, 36
42033f5c:	201110        	or	a1, a1, a1
42033f5f:	fe1865        	call8	420320e4 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f62:	07a9      	s32i.n	a10, a7, 0
42033f64:	24c1a2        	addi	a10, a1, 36
42033f67:	201110        	or	a1, a1, a1
42033f6a:	f894a5        	call8	4202c8b4 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f6d:	1922b2        	l32i	a11, a2, 100
42033f70:	24c1a2        	addi	a10, a1, 36
42033f73:	002bc2        	l32i	a12, a11, 0
42033f76:	201110        	or	a1, a1, a1
42033f79:	ff33a5        	call8	420332b4 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f7c:	05d8      	l32i.n	a13, a5, 0
42033f7e:	0094c2        	l16si	a12, a4, 0
42033f81:	0093b2        	l16si	a11, a3, 0
42033f84:	24c1a2        	addi	a10, a1, 36
42033f87:	201110        	or	a1, a1, a1
42033f8a:	fe15a5        	call8	420320e4 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f8d:	05a9      	s32i.n	a10, a5, 0
42033f8f:	24c1a2        	addi	a10, a1, 36
42033f92:	201110        	or	a1, a1, a1
42033f95:	f891e5        	call8	4202c8b4 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f98:	0094c2        	l16si	a12, a4, 0
42033f9b:	0093b2        	l16si	a11, a3, 0
42033f9e:	07d8      	l32i.n	a13, a7, 0
42033fa0:	05ed      	mov.n	a14, a5
42033fa2:	a24b      	addi.n	a10, a2, 4
42033fa4:	201110        	or	a1, a1, a1
42033fa7:	069165        	call8	4203a8bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033faa:	3188      	l32i.n	a8, a1, 12
42033fac:	332b      	addi.n	a3, a3, 2
42033fae:	a08680        	addx4	a8, a6, a8
42033fb1:	08a9      	s32i.n	a10, a8, 0
42033fb3:	661b      	addi.n	a6, a6, 1
42033fb5:	442b      	addi.n	a4, a4, 2
42033fb7:	ffd546        	j	42033f10 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033fba:	c1a200        	mul16u	a10, a2, a0
42033fbd:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033fc0:	20          	.byte	0x20
42033fc1:	f88f25        	call8	4202c8b4 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033fc4:	02ad      	mov.n	a10, a2
42033fc6:	201110        	or	a1, a1, a1
42033fc9:	3ecd25        	call8	42072c9c <_Unwind_Resume>
42033fcc:	fffac6        	j	42033fbb <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203877c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
4203877c:	006136        	entry	a1, 48
4203877f:	1891a2        	l16si	a10, a1, 48
42038782:	3169      	s32i.n	a6, a1, 12
42038784:	91cb      	addi.n	a9, a1, 12
42038786:	1a91b2        	l16si	a11, a1, 52
42038789:	028d      	mov.n	a8, a2
4203878b:	747070        	extui	a7, a7, 0, 8
4203878e:	02aa27        	bge	a10, a2, 42038794 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
42038791:	002c46        	j	42038846 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
42038794:	02a4a7        	bge	a4, a10, 4203879a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
42038797:	002ac6        	j	42038846 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
4203879a:	02ab37        	bge	a11, a3, 420387a0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
4203879d:	002946        	j	42038846 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
420387a0:	72e5b0        	salt	a14, a5, a11
420387a3:	02a5b7        	bge	a5, a11, 420387a9 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2d>
420387a6:	002706        	j	42038846 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
420387a9:	0009c2        	l8ui	a12, a9, 0
420387ac:	09ac16        	beqz	a12, 4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xce>
420387af:	d2ca      	add.n	a13, a2, a12
420387b1:	02ada7        	bge	a13, a10, 420387b7 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3b>
420387b4:	002486        	j	4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xce>
420387b7:	f3ca      	add.n	a15, a3, a12
420387b9:	02afb7        	bge	a15, a11, 420387bf <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x43>
420387bc:	002286        	j	4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xce>
420387bf:	c0dda0        	sub	a13, a13, a10
420387c2:	c0ffb0        	sub	a15, a15, a11
420387c5:	f4d0d0        	extui	a13, a13, 0, 16
420387c8:	f4f0f0        	extui	a15, a15, 0, 16
420387cb:	208df0        	or	a8, a13, a15
420387ce:	238880        	sext	a8, a8, 15
420387d1:	177896        	bltz	a8, 4203894c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1d0>
420387d4:	740ff4        	mul.aa.ll	a15, a15
420387d7:	780dd4        	mula.aa.ll	a13, a13
420387da:	031080        	rsr.acclo	a8
420387dd:	ffa022        	movi	a2, 255
420387e0:	11f880        	slli	a15, a8, 8
420387e3:	5d4fa6        	blti	a15, 4, 42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
420387e6:	40ff80        	nsau	a8, a15
420387e9:	dfc882        	addi	a8, a8, -33
420387ec:	608080        	neg	a8, a8
420387ef:	218180        	srai	a8, a8, 1
420387f2:	390c      	movi.n	a9, 3
420387f4:	fec882        	addi	a8, a8, -2
420387f7:	401800        	ssl	a8
420387fa:	a18900        	sll	a8, a9
420387fd:	c29f80        	quou	a9, a15, a8
42038800:	998a      	add.n	a9, a9, a8
42038802:	419190        	srli	a9, a9, 1
42038805:	c28f90        	quou	a8, a15, a9
42038808:	889a      	add.n	a8, a8, a9
4203880a:	418180        	srli	a8, a8, 1
4203880d:	c29f80        	quou	a9, a15, a8
42038810:	998a      	add.n	a9, a9, a8
42038812:	203d81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42038815:	419190        	srli	a9, a9, 1
42038818:	281987        	beq	a9, a8, 42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
4203881b:	82a990        	mull	a10, a9, a9
4203881e:	118cc0        	slli	a8, a12, 4
42038821:	62ffa0        	saltu	a15, a15, a10
42038824:	c099f0        	sub	a9, a9, a15
42038827:	878a      	add.n	a8, a7, a8
42038829:	c08890        	sub	a8, a8, a9
4203882c:	f49080        	extui	a9, a8, 0, 16
4203882f:	23a880        	sext	a10, a8, 15
42038832:	020c      	movi.n	a2, 0
42038834:	0ce8f7        	bbsi	a8, 15, 42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038837:	f80c      	movi.n	a8, 15
42038839:	4b28a7        	blt	a8, a10, 42038888 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x10c>
4203883c:	1189c0        	slli	a8, a9, 4
4203883f:	998a      	add.n	a9, a9, a8
42038841:	742090        	extui	a2, a9, 0, 8
42038844:	f01d      	retw.n
42038846:	020c      	movi.n	a2, 0
42038848:	f01d      	retw.n
4203884a:	0109c2        	l8ui	a12, a9, 1
4203884d:	ccbc      	beqz.n	a12, 4203888d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x111>
4203884f:	c0d4c0        	sub	a13, a4, a12
42038852:	372ad7        	blt	a10, a13, 4203888d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x111>
42038855:	d3ca      	add.n	a13, a3, a12
42038857:	322db7        	blt	a13, a11, 4203888d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x111>
4203885a:	c08c40        	sub	a8, a12, a4
4203885d:	88aa      	add.n	a8, a8, a10
4203885f:	c0ddb0        	sub	a13, a13, a11
42038862:	f48080        	extui	a8, a8, 0, 16
42038865:	f4d0d0        	extui	a13, a13, 0, 16
42038868:	2098d0        	or	a9, a8, a13
4203886b:	239980        	sext	a9, a9, 15
4203886e:	0f39d6        	bgez	a9, 42038965 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e9>
42038871:	c08c80        	sub	a8, a12, a8
42038874:	40f890        	nsau	a9, a8
42038877:	419590        	srli	a9, a9, 5
4203887a:	052816        	beqz	a8, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203887d:	c0dcd0        	sub	a13, a12, a13
42038880:	180c      	movi.n	a8, 1
42038882:	9389d0        	movnez	a8, a9, a13
42038885:	047856        	bnez	a8, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038888:	ffa022        	movi	a2, 255
4203888b:	f01d      	retw.n
4203888d:	0209c2        	l8ui	a12, a9, 2
42038890:	048c16        	beqz	a12, 420388dc <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038893:	c0d4c0        	sub	a13, a4, a12
42038896:	422ad7        	blt	a10, a13, 420388dc <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038899:	c0d5c0        	sub	a13, a5, a12
4203889c:	3c2bd7        	blt	a11, a13, 420388dc <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
4203889f:	c09c40        	sub	a9, a12, a4
420388a2:	c08c50        	sub	a8, a12, a5
420388a5:	99aa      	add.n	a9, a9, a10
420388a7:	88ba      	add.n	a8, a8, a11
420388a9:	f49090        	extui	a9, a9, 0, 16
420388ac:	f48080        	extui	a8, a8, 0, 16
420388af:	20a980        	or	a10, a9, a8
420388b2:	23aa80        	sext	a10, a10, 15
420388b5:	108ad6        	bgez	a10, 420389c1 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x245>
420388b8:	c09c90        	sub	a9, a12, a9
420388bb:	40f9a0        	nsau	a10, a9
420388be:	41a5a0        	srli	a10, a10, 5
420388c1:	c08c80        	sub	a8, a12, a8
420388c4:	008916        	beqz	a9, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
420388c7:	01a092        	movi	a9, 1
420388ca:	939a80        	movnez	a9, a10, a8
420388cd:	fb7916        	beqz	a9, 42038888 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x10c>
420388d0:	1127c0        	slli	a2, a7, 4
420388d3:	272a      	add.n	a2, a7, a2
420388d5:	742020        	extui	a2, a2, 0, 8
420388d8:	f01d      	retw.n
420388da:	00          	.byte	00
420388db:	099200        	l32e	a0, a2, -28
420388de:	03          	.byte	0x3
420388df:	29bc      	beqz.n	a9, 42038915 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x199>
420388e1:	c89a      	add.n	a12, a8, a9
420388e3:	2e2ca7        	blt	a12, a10, 42038915 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x199>
420388e6:	c0d590        	sub	a13, a5, a9
420388e9:	282bd7        	blt	a11, a13, 42038915 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x199>
420388ec:	c08950        	sub	a8, a9, a5
420388ef:	c0cca0        	sub	a12, a12, a10
420388f2:	88ba      	add.n	a8, a8, a11
420388f4:	f4c0c0        	extui	a12, a12, 0, 16
420388f7:	f48080        	extui	a8, a8, 0, 16
420388fa:	20ac80        	or	a10, a12, a8
420388fd:	23aa80        	sext	a10, a10, 15
42038900:	120ad6        	bgez	a10, 42038a24 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a8>
42038903:	c0c9c0        	sub	a12, a9, a12
42038906:	40fca0        	nsau	a10, a12
42038909:	41a5a0        	srli	a10, a10, 5
4203890c:	fc0c16        	beqz	a12, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203890f:	c08980        	sub	a8, a9, a8
42038912:	ffec46        	j	420388c7 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x14b>
42038915:	e90c      	movi.n	a9, 14
42038917:	ffa022        	movi	a2, 255
4203891a:	02b977        	bgeu	a9, a7, 42038920 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1a4>
4203891d:	ffc8c6        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038920:	c09a80        	sub	a9, a10, a8
42038923:	40f9c0        	nsau	a12, a9
42038926:	1d0c      	movi.n	a13, 1
42038928:	41c5c0        	srli	a12, a12, 5
4203892b:	fa1916        	beqz	a9, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203892e:	c08a40        	sub	a8, a10, a4
42038931:	83cd80        	moveqz	a12, a13, a8
42038934:	f98c56        	bnez	a12, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038937:	c03b30        	sub	a3, a11, a3
4203893a:	83cd30        	moveqz	a12, a13, a3
4203893d:	f8fc56        	bnez	a12, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038940:	c08b50        	sub	a8, a11, a5
42038943:	93dc80        	movnez	a13, a12, a8
42038946:	f86d56        	bnez	a13, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038949:	f01d      	retw.n
4203894b:	00          	.byte	00
4203894c:	c0dcd0        	sub	a13, a12, a13
4203894f:	180c      	movi.n	a8, 1
42038951:	83e8d0        	moveqz	a14, a8, a13
42038954:	f78e56        	bnez	a14, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038957:	c0fcf0        	sub	a15, a12, a15
4203895a:	938ef0        	movnez	a8, a14, a15
4203895d:	ffa022        	movi	a2, 255
42038960:	f6c856        	bnez	a8, 420388d0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038963:	f01d      	retw.n
42038965:	740dd4        	mul.aa.ll	a13, a13
42038968:	780884        	mula.aa.ll	a8, a8
4203896b:	031080        	rsr.acclo	a8
4203896e:	ffa022        	movi	a2, 255
42038971:	11d880        	slli	a13, a8, 8
42038974:	024de6        	bgei	a13, 4, 4203897a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1fe>
42038977:	ffb246        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
4203897a:	40fd80        	nsau	a8, a13
4203897d:	dfc882        	addi	a8, a8, -33
42038980:	608080        	neg	a8, a8
42038983:	218180        	srai	a8, a8, 1
42038986:	390c      	movi.n	a9, 3
42038988:	fec882        	addi	a8, a8, -2
4203898b:	401800        	ssl	a8
4203898e:	a18900        	sll	a8, a9
42038991:	c29d80        	quou	a9, a13, a8
42038994:	998a      	add.n	a9, a9, a8
42038996:	419190        	srli	a9, a9, 1
42038999:	c28d90        	quou	a8, a13, a9
4203899c:	889a      	add.n	a8, a8, a9
4203899e:	418180        	srli	a8, a8, 1
420389a1:	c29d80        	quou	a9, a13, a8
420389a4:	998a      	add.n	a9, a9, a8
420389a6:	1fd881        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
420389a9:	419190        	srli	a9, a9, 1
420389ac:	029987        	bne	a9, a8, 420389b2 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x236>
420389af:	ffa446        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
420389b2:	82a990        	mull	a10, a9, a9
420389b5:	118cc0        	slli	a8, a12, 4
420389b8:	62dda0        	saltu	a13, a13, a10
420389bb:	c099d0        	sub	a9, a9, a13
420389be:	ff9946        	j	42038827 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xab>
420389c1:	740884        	mul.aa.ll	a8, a8
420389c4:	780994        	mula.aa.ll	a9, a9
420389c7:	031080        	rsr.acclo	a8
420389ca:	ffa022        	movi	a2, 255
420389cd:	119880        	slli	a9, a8, 8
420389d0:	0249e6        	bgei	a9, 4, 420389d6 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x25a>
420389d3:	ff9b46        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
420389d6:	40f980        	nsau	a8, a9
420389d9:	dfc882        	addi	a8, a8, -33
420389dc:	608080        	neg	a8, a8
420389df:	218180        	srai	a8, a8, 1
420389e2:	3a0c      	movi.n	a10, 3
420389e4:	fec882        	addi	a8, a8, -2
420389e7:	401800        	ssl	a8
420389ea:	a18a00        	sll	a8, a10
420389ed:	c2a980        	quou	a10, a9, a8
420389f0:	aa8a      	add.n	a10, a10, a8
420389f2:	41a1a0        	srli	a10, a10, 1
420389f5:	c289a0        	quou	a8, a9, a10
420389f8:	88aa      	add.n	a8, a8, a10
420389fa:	418180        	srli	a8, a8, 1
420389fd:	c2a980        	quou	a10, a9, a8
42038a00:	aa8a      	add.n	a10, a10, a8
42038a02:	1fc181        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42038a05:	41a1a0        	srli	a10, a10, 1
42038a08:	029a87        	bne	a10, a8, 42038a0e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x292>
42038a0b:	ff8d46        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038a0e:	82baa0        	mull	a11, a10, a10
42038a11:	118cc0        	slli	a8, a12, 4
42038a14:	6299b0        	saltu	a9, a9, a11
42038a17:	c0aa90        	sub	a10, a10, a9
42038a1a:	878a      	add.n	a8, a7, a8
42038a1c:	c088a0        	sub	a8, a8, a10
42038a1f:	ff8246        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xb0>
42038a22:	840000        	extui	a0, a0, 0, 9
42038a25:	7408      	l32i.n	a0, a4, 28
42038a27:	780cc4        	mula.aa.ll	a12, a12
42038a2a:	031080        	rsr.acclo	a8
42038a2d:	ffa022        	movi	a2, 255
42038a30:	11a880        	slli	a10, a8, 8
42038a33:	024ae6        	bgei	a10, 4, 42038a39 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2bd>
42038a36:	ff8286        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038a39:	40fa80        	nsau	a8, a10
42038a3c:	dfc882        	addi	a8, a8, -33
42038a3f:	608080        	neg	a8, a8
42038a42:	218180        	srai	a8, a8, 1
42038a45:	3b0c      	movi.n	a11, 3
42038a47:	fec882        	addi	a8, a8, -2
42038a4a:	401800        	ssl	a8
42038a4d:	a18b00        	sll	a8, a11
42038a50:	c2ba80        	quou	a11, a10, a8
42038a53:	bb8a      	add.n	a11, a11, a8
42038a55:	41b1b0        	srli	a11, a11, 1
42038a58:	c28ab0        	quou	a8, a10, a11
42038a5b:	88ba      	add.n	a8, a8, a11
42038a5d:	418180        	srli	a8, a8, 1
42038a60:	c2ba80        	quou	a11, a10, a8
42038a63:	bb8a      	add.n	a11, a11, a8
42038a65:	1fa881        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42038a68:	41b1b0        	srli	a11, a11, 1
42038a6b:	029b87        	bne	a11, a8, 42038a71 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2f5>
42038a6e:	ff7486        	j	42038844 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038a71:	82cbb0        	mull	a12, a11, a11
42038a74:	1189c0        	slli	a8, a9, 4
42038a77:	62aac0        	saltu	a10, a10, a12
42038a7a:	c0bba0        	sub	a11, a11, a10
42038a7d:	878a      	add.n	a8, a7, a8
42038a7f:	c088b0        	sub	a8, a8, a11
42038a82:	ff6986        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xb0>


/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203a8bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
4203a8bc:	008136        	entry	a1, 64
4203a8bf:	200282        	l8ui	a8, a2, 32
4203a8c2:	0212a2        	l16ui	a10, a2, 4
4203a8c5:	0412c2        	l16ui	a12, a2, 8
4203a8c8:	a8aa      	add.n	a10, a8, a10
4203a8ca:	c09c80        	sub	a9, a12, a8
4203a8cd:	4169      	s32i.n	a6, a1, 16
4203a8cf:	233380        	sext	a3, a3, 15
4203a8d2:	23ba80        	sext	a11, a10, 15
4203a8d5:	027d      	mov.n	a7, a2
4203a8d7:	234480        	sext	a4, a4, 15
4203a8da:	f460a0        	extui	a6, a10, 0, 16
4203a8dd:	239980        	sext	a9, a9, 15
4203a8e0:	02a3b7        	bge	a3, a11, 4203a8e6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2a>
4203a8e3:	003946        	j	4203a9cc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a8e6:	02a937        	bge	a9, a3, 4203a8ec <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30>
4203a8e9:	0037c6        	j	4203a9cc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a8ec:	0312e2        	l16ui	a14, a2, 6
4203a8ef:	051222        	l16ui	a2, a2, 10
4203a8f2:	e8ea      	add.n	a14, a8, a14
4203a8f4:	23de80        	sext	a13, a14, 15
4203a8f7:	f4e0e0        	extui	a14, a14, 0, 16
4203a8fa:	c0a280        	sub	a10, a2, a8
4203a8fd:	61e9      	s32i.n	a14, a1, 24
4203a8ff:	23aa80        	sext	a10, a10, 15
4203a902:	02a4d7        	bge	a4, a13, 4203a908 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x4c>
4203a905:	0030c6        	j	4203a9cc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a908:	72fa40        	salt	a15, a10, a4
4203a90b:	71f9      	s32i.n	a15, a1, 28
4203a90d:	02aa47        	bge	a10, a4, 4203a913 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x57>
4203a910:	002e06        	j	4203a9cc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a913:	2107f2        	l8ui	a15, a7, 33
4203a916:	1c07e2        	l8ui	a14, a7, 28
4203a919:	51f9      	s32i.n	a15, a1, 20
4203a91b:	1dbe16        	beqz	a14, 4203aafa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23e>
4203a91e:	fbea      	add.n	a15, a11, a14
4203a920:	02af37        	bge	a15, a3, 4203a926 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x6a>
4203a923:	0074c6        	j	4203aafa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23e>
4203a926:	fdea      	add.n	a15, a13, a14
4203a928:	02af47        	bge	a15, a4, 4203a92e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x72>
4203a92b:	0072c6        	j	4203aafa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23e>
4203a92e:	6198      	l32i.n	a9, a1, 24
4203a930:	c0ae30        	sub	a10, a14, a3
4203a933:	c08e40        	sub	a8, a14, a4
4203a936:	898a      	add.n	a8, a9, a8
4203a938:	a6aa      	add.n	a10, a6, a10
4203a93a:	f4a0a0        	extui	a10, a10, 0, 16
4203a93d:	f48080        	extui	a8, a8, 0, 16
4203a940:	209a80        	or	a9, a10, a8
4203a943:	239980        	sext	a9, a9, 15
4203a946:	478996        	bltz	a9, 4203adc2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x506>
4203a949:	740884        	mul.aa.ll	a8, a8
4203a94c:	780aa4        	mula.aa.ll	a10, a10
4203a94f:	031080        	rsr.acclo	a8
4203a952:	119880        	slli	a9, a8, 8
4203a955:	6349a6        	blti	a9, 4, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203a958:	40f980        	nsau	a8, a9
4203a95b:	dfc882        	addi	a8, a8, -33
4203a95e:	608080        	neg	a8, a8
4203a961:	218180        	srai	a8, a8, 1
4203a964:	3a0c      	movi.n	a10, 3
4203a966:	fec882        	addi	a8, a8, -2
4203a969:	401800        	ssl	a8
4203a96c:	a18a00        	sll	a8, a10
4203a96f:	c2a980        	quou	a10, a9, a8
4203a972:	aa8a      	add.n	a10, a10, a8
4203a974:	41a1a0        	srli	a10, a10, 1
4203a977:	c289a0        	quou	a8, a9, a10
4203a97a:	88aa      	add.n	a8, a8, a10
4203a97c:	418180        	srli	a8, a8, 1
4203a97f:	c2a980        	quou	a10, a9, a8
4203a982:	aa8a      	add.n	a10, a10, a8
4203a984:	17e181        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a987:	41a1a0        	srli	a10, a10, 1
4203a98a:	2e1a87        	beq	a10, a8, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203a98d:	82baa0        	mull	a11, a10, a10
4203a990:	118ec0        	slli	a8, a14, 4
4203a993:	6299b0        	saltu	a9, a9, a11
4203a996:	c0aa90        	sub	a10, a10, a9
4203a999:	5198      	l32i.n	a9, a1, 20
4203a99b:	898a      	add.n	a8, a9, a8
4203a99d:	c088a0        	sub	a8, a8, a10
4203a9a0:	f49080        	extui	a9, a8, 0, 16
4203a9a3:	23a880        	sext	a10, a8, 15
4203a9a6:	22e8f7        	bbsi	a8, 15, 4203a9cc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a9a9:	f80c      	movi.n	a8, 15
4203a9ab:	0d28a7        	blt	a8, a10, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203a9ae:	1189c0        	slli	a8, a9, 4
4203a9b1:	898a      	add.n	a8, a9, a8
4203a9b3:	746080        	extui	a6, a8, 0, 8
4203a9b6:	ffa092        	movi	a9, 255
4203a9b9:	119697        	bne	a6, a9, 4203a9ce <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x112>
4203a9bc:	142782        	l32i	a8, a7, 80
4203a9bf:	052d      	mov.n	a2, a5
4203a9c1:	133816        	beqz	a8, 4203aaf8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203a9c4:	ffa062        	movi	a6, 255
4203a9c7:	0000c6        	j	4203a9ce <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x112>
4203a9ca:	00          	.byte	00
4203a9cb:	060c00        	ee.ldf.64.xp	f0, f0, a0, a12
4203a9ce:	04dd      	mov.n	a13, a4
4203a9d0:	3cc7b2        	addi	a11, a7, 60
4203a9d3:	03cd      	mov.n	a12, a3
4203a9d5:	28c7a2        	addi	a10, a7, 40
4203a9d8:	0085a5        	call8	4203b234 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
4203a9db:	2007b2        	l8ui	a11, a7, 32
4203a9de:	51a9      	s32i.n	a10, a1, 20
4203a9e0:	012a80        	slli	a2, a10, 24
4203a9e3:	06dd      	mov.n	a13, a6
4203a9e5:	04cb16        	beqz	a11, 4203aa35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203a9e8:	0597d2        	l16si	a13, a7, 10
4203a9eb:	67e8      	l32i.n	a14, a7, 24
4203a9ed:	0497c2        	l16si	a12, a7, 8
4203a9f0:	0397b2        	l16si	a11, a7, 6
4203a9f3:	0297a2        	l16si	a10, a7, 4
4203a9f6:	0fa0f2        	movi	a15, 15
4203a9f9:	1149      	s32i.n	a4, a1, 4
4203a9fb:	0139      	s32i.n	a3, a1, 0
4203a9fd:	fdd7e5        	call8	4203877c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
4203aa00:	0add      	mov.n	a13, a10
4203aa02:	faac      	beqz.n	a10, 4203aa35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203aa04:	4198      	l32i.n	a9, a1, 16
4203aa06:	24c7a2        	addi	a10, a7, 36
4203aa09:	098d      	mov.n	a8, a9
4203aa0b:	838a90        	moveqz	a8, a10, a9
4203aa0e:	08b8      	l32i.n	a11, a8, 0
4203aa10:	01cd80        	slli	a12, a13, 24
4203aa13:	11bb80        	slli	a11, a11, 8
4203aa16:	41b8b0        	srli	a11, a11, 8
4203aa19:	75a8c0        	extui	a10, a12, 24, 8
4203aa1c:	ffa0e2        	movi	a14, 255
4203aa1f:	20bbc0        	or	a11, a11, a12
4203aa22:	029ae7        	bne	a10, a14, 4203aa28 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x16c>
4203aa25:	007bc6        	j	4203ac18 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x35c>
4203aa28:	5198      	l32i.n	a9, a1, 20
4203aa2a:	0299e7        	bne	a9, a14, 4203aa30 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
4203aa2d:	0108c6        	j	4203ae54 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x598>
4203aa30:	0b2d      	mov.n	a2, a11
4203aa32:	3a6956        	bnez	a9, 4203addc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x520>
4203aa35:	ffa0a2        	movi	a10, 255
4203aa38:	0296a7        	bne	a6, a10, 4203aa3e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x182>
4203aa3b:	007246        	j	4203ac08 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x34c>
4203aa3e:	36bc      	beqz.n	a6, 4203aa75 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203aa40:	759850        	extui	a9, a5, 24, 8
4203aa43:	d18960        	mul16s	a8, a9, a6
4203aa46:	17b391        	l32r	a9, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203aa49:	7fc882        	addi	a8, a8, 127
4203aa4c:	a28890        	muluh	a8, a8, a9
4203aa4f:	115580        	slli	a5, a5, 8
4203aa52:	0188f0        	slli	a8, a8, 17
4203aa55:	759880        	extui	a9, a8, 24, 8
4203aa58:	098d      	mov.n	a8, a9
4203aa5a:	404800        	ssai	8
4203aa5d:	81b950        	src	a11, a9, a5
4203aa60:	0e19a7        	beq	a9, a10, 4203aa72 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b6>
4203aa63:	759820        	extui	a9, a2, 24, 8
4203aa66:	0299a7        	bne	a9, a10, 4203aa6c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b0>
4203aa69:	007646        	j	4203ac46 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x38a>
4203aa6c:	005816        	beqz	a8, 4203aa75 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203aa6f:	2b9956        	bnez	a9, 4203ad2c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x470>
4203aa72:	202bb0        	or	a2, a11, a11
4203aa75:	1427a2        	l32i	a10, a7, 80
4203aa78:	07ca16        	beqz	a10, 4203aaf8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203aa7b:	180a82        	l8ui	a8, a10, 24
4203aa7e:	19b856        	bnez	a8, 4203ac1d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x361>
4203aa81:	031a82        	l16ui	a8, a10, 6
4203aa84:	021a92        	l16ui	a9, a10, 4
4203aa87:	c04480        	sub	a4, a4, a8
4203aa8a:	c03390        	sub	a3, a3, a9
4203aa8d:	740444        	mul.aa.ll	a4, a4
4203aa90:	780334        	mula.aa.ll	a3, a3
4203aa93:	032a82        	l32i	a8, a10, 12
4203aa96:	031090        	rsr.acclo	a9
4203aa99:	c08890        	sub	a8, a8, a9
4203aa9c:	2218a6        	blti	a8, 1, 4203aac2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x206>
4203aa9f:	049a92        	l16si	a9, a10, 8
4203aaa2:	0219e6        	bgei	a9, 1, 4203aaa8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1ec>
4203aaa5:	0065c6        	j	4203ac40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x384>
4203aaa8:	80b990        	add	a11, a9, a9
4203aaab:	0228b7        	blt	a8, a11, 4203aab1 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1f5>
4203aaae:	0062c6        	j	4203ac3d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x381>
4203aab1:	4ab8      	l32i.n	a11, a10, 16
4203aab3:	5aa8      	l32i.n	a10, a10, 20
4203aab5:	60a0c2        	movi	a12, 96
4203aab8:	41d9      	s32i.n	a13, a1, 16
4203aaba:	4c4525        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203aabd:	41d8      	l32i.n	a13, a1, 16
4203aabf:	000046        	j	4203aac4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x208>
4203aac2:	5aa8      	l32i.n	a10, a10, 20
4203aac4:	7588a0        	extui	a8, a10, 24, 8
4203aac7:	179391        	l32r	a9, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203aaca:	d188d0        	mul16s	a8, a8, a13
4203aacd:	11aa80        	slli	a10, a10, 8
4203aad0:	a28890        	muluh	a8, a8, a9
4203aad3:	ffa0c2        	movi	a12, 255
4203aad6:	0188f0        	slli	a8, a8, 17
4203aad9:	759880        	extui	a9, a8, 24, 8
4203aadc:	098d      	mov.n	a8, a9
4203aade:	404800        	ssai	8
4203aae1:	81b9a0        	src	a11, a9, a10
4203aae4:	0e19c7        	beq	a9, a12, 4203aaf6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23a>
4203aae7:	759820        	extui	a9, a2, 24, 8
4203aaea:	0299c7        	bne	a9, a12, 4203aaf0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x234>
4203aaed:	004f46        	j	4203ac2e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x372>
4203aaf0:	004816        	beqz	a8, 4203aaf8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203aaf3:	18d956        	bnez	a9, 4203ac84 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x3c8>
4203aaf6:	0b2d      	mov.n	a2, a11
4203aaf8:	f01d      	retw.n
4203aafa:	1d07e2        	l8ui	a14, a7, 29
4203aafd:	043e16        	beqz	a14, 4203ab44 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x288>
4203ab00:	c0f9e0        	sub	a15, a9, a14
4203ab03:	3d23f7        	blt	a3, a15, 4203ab44 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x288>
4203ab06:	fdea      	add.n	a15, a13, a14
4203ab08:	382f47        	blt	a15, a4, 4203ab44 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x288>
4203ab0b:	61a8      	l32i.n	a10, a1, 24
4203ab0d:	98ea      	add.n	a9, a8, a14
4203ab0f:	993a      	add.n	a9, a9, a3
4203ab11:	c08e40        	sub	a8, a14, a4
4203ab14:	8a8a      	add.n	a8, a10, a8
4203ab16:	c099c0        	sub	a9, a9, a12
4203ab19:	f49090        	extui	a9, a9, 0, 16
4203ab1c:	f48080        	extui	a8, a8, 0, 16
4203ab1f:	20a980        	or	a10, a9, a8
4203ab22:	23aa80        	sext	a10, a10, 15
4203ab25:	33cad6        	bgez	a10, 4203ae65 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5a9>
4203ab28:	c09e90        	sub	a9, a14, a9
4203ab2b:	40f9a0        	nsau	a10, a9
4203ab2e:	41a5a0        	srli	a10, a10, 5
4203ab31:	c08e80        	sub	a8, a14, a8
4203ab34:	698c      	beqz.n	a9, 4203ab3e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203ab36:	190c      	movi.n	a9, 1
4203ab38:	939a80        	movnez	a9, a10, a8
4203ab3b:	e7d916        	beqz	a9, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203ab3e:	5198      	l32i.n	a9, a1, 20
4203ab40:	ff9a86        	j	4203a9ae <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf2>
4203ab43:	07e200        	ee.stf.64.xp	f14, f0, a0, a2
4203ab46:	042e161e 	ee.vmulas.s16.accx.ld.ip.qup	q1, a1, 0x160, q0, q2, q2, q6
4203ab4a:	c0f9e0        	sub	a15, a9, a14
4203ab4d:	3c23f7        	blt	a3, a15, 4203ab8d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d1>
4203ab50:	c0fae0        	sub	a15, a10, a14
4203ab53:	3624f7        	blt	a4, a15, 4203ab8d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d1>
4203ab56:	88ea      	add.n	a8, a8, a14
4203ab58:	c09820        	sub	a9, a8, a2
4203ab5b:	c0c8c0        	sub	a12, a8, a12
4203ab5e:	8c3a      	add.n	a8, a12, a3
4203ab60:	994a      	add.n	a9, a9, a4
4203ab62:	f48080        	extui	a8, a8, 0, 16
4203ab65:	f49090        	extui	a9, a9, 0, 16
4203ab68:	20a890        	or	a10, a8, a9
4203ab6b:	23aa80        	sext	a10, a10, 15
4203ab6e:	2fead6        	bgez	a10, 4203ae70 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5b4>
4203ab71:	c08e80        	sub	a8, a14, a8
4203ab74:	40f8a0        	nsau	a10, a8
4203ab77:	41a5a0        	srli	a10, a10, 5
4203ab7a:	fc0816        	beqz	a8, 4203ab3e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203ab7d:	c09e90        	sub	a9, a14, a9
4203ab80:	180c      	movi.n	a8, 1
4203ab82:	938a90        	movnez	a8, a10, a9
4203ab85:	e33816        	beqz	a8, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203ab88:	5198      	l32i.n	a9, a1, 20
4203ab8a:	ff8806        	j	4203a9ae <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf2>
4203ab8d:	1f07c2        	l8ui	a12, a7, 31
4203ab90:	6cbc      	beqz.n	a12, 4203abca <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30e>
4203ab92:	ebca      	add.n	a14, a11, a12
4203ab94:	322e37        	blt	a14, a3, 4203abca <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30e>
4203ab97:	c0eac0        	sub	a14, a10, a12
4203ab9a:	2c24e7        	blt	a4, a14, 4203abca <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30e>
4203ab9d:	88ca      	add.n	a8, a8, a12
4203ab9f:	c09c30        	sub	a9, a12, a3
4203aba2:	884a      	add.n	a8, a8, a4
4203aba4:	969a      	add.n	a9, a6, a9
4203aba6:	c08820        	sub	a8, a8, a2
4203aba9:	f49090        	extui	a9, a9, 0, 16
4203abac:	f48080        	extui	a8, a8, 0, 16
4203abaf:	20a980        	or	a10, a9, a8
4203abb2:	23aa80        	sext	a10, a10, 15
4203abb5:	2c0ad6        	bgez	a10, 4203ae79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5bd>
4203abb8:	c09c90        	sub	a9, a12, a9
4203abbb:	40f9a0        	nsau	a10, a9
4203abbe:	41a5a0        	srli	a10, a10, 5
4203abc1:	f79916        	beqz	a9, 4203ab3e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203abc4:	c08c80        	sub	a8, a12, a8
4203abc7:	ffdac6        	j	4203ab36 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x27a>
4203abca:	51c8      	l32i.n	a12, a1, 20
4203abcc:	e80c      	movi.n	a8, 14
4203abce:	02b8c7        	bgeu	a8, a12, 4203abd4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x318>
4203abd1:	ff79c6        	j	4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203abd4:	c0bb30        	sub	a11, a11, a3
4203abd7:	40fbc0        	nsau	a12, a11
4203abda:	180c      	movi.n	a8, 1
4203abdc:	41c5c0        	srli	a12, a12, 5
4203abdf:	7b9c      	beqz.n	a11, 4203abfa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x33e>
4203abe1:	c09930        	sub	a9, a9, a3
4203abe4:	83c890        	moveqz	a12, a8, a9
4203abe7:	fccc      	bnez.n	a12, 4203abfa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x33e>
4203abe9:	c0dd40        	sub	a13, a13, a4
4203abec:	83c8d0        	moveqz	a12, a8, a13
4203abef:	7ccc      	bnez.n	a12, 4203abfa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x33e>
4203abf1:	c0aa40        	sub	a10, a10, a4
4203abf4:	938ca0        	movnez	a8, a12, a10
4203abf7:	dc1816        	beqz	a8, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203abfa:	5198      	l32i.n	a9, a1, 20
4203abfc:	1189c0        	slli	a8, a9, 4
4203abff:	898a      	add.n	a8, a9, a8
4203ac01:	746080        	extui	a6, a8, 0, 8
4203ac04:	ff7186        	j	4203a9ce <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x112>
4203ac07:	422100        	xorb	b2, b1, b0
4203ac0a:	27a217        	bge	a2, a1, 4203ac35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x379>
4203ac0d:	14          	.byte	0x14
4203ac0e:	202520        	or	a2, a5, a2
4203ac11:	e66a56        	bnez	a10, 4203aa7b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1bf>
4203ac14:	ffb806        	j	4203aaf8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203ac17:	00          	.byte	00
4203ac18:	0b2d      	mov.n	a2, a11
4203ac1a:	ff85c6        	j	4203aa35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203ac1d:	20c440        	or	a12, a4, a4
4203ac20:	20b330        	or	a11, a3, a3
4203ac23:	0461d2        	s32i	a13, a1, 16
4203ac26:	ec2be5        	call8	42026ee4 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
4203ac29:	41d8      	l32i.n	a13, a1, 16
4203ac2b:	ffa546        	j	4203aac4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x208>
4203ac2e:	20a220        	or	a10, a2, a2
4203ac31:	4d8e65        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203ac34:	202aa0        	or	a2, a10, a10
4203ac37:	000090        	retw
4203ac3a:	000000        	ill
4203ac3d:	171966        	bnei	a9, 1, 4203ac58 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x39c>
4203ac40:	4aa8      	l32i.n	a10, a10, 16
4203ac42:	ff9f86        	j	4203aac4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x208>
4203ac45:	a22000        	muluh	a2, a0, a0
4203ac48:	61d220        	xsr.excsave2	a2
4203ac4b:	8ca504        	ee.vprelu.s8	q1, q5, q0, a0
4203ac4e:	4d          	.byte	0x4d
4203ac4f:	0421d2        	l32i	a13, a1, 16
4203ac52:	0a2d      	mov.n	a2, a10
4203ac54:	ff8746        	j	4203aa75 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203ac57:	00          	.byte	00
4203ac58:	ffc9c2        	addi	a12, a9, -1
4203ac5b:	23cc80        	sext	a12, a12, 15
4203ac5e:	11cce0        	slli	a12, a12, 2
4203ac61:	022c87        	blt	a12, a8, 4203ac67 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x3ab>
4203ac64:	002c86        	j	4203ad1a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x45e>
4203ac67:	d52926        	beqi	a9, 2, 4203ac40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x384>
4203ac6a:	80bb90        	add	a11, a11, a9
4203ac6d:	80bbb0        	add	a11, a11, a11
4203ac70:	f8cbb2        	addi	a11, a11, -8
4203ac73:	c9a8b7        	bge	a8, a11, 4203ac40 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x384>
4203ac76:	4ab8      	l32i.n	a11, a10, 16
4203ac78:	5aa8      	l32i.n	a10, a10, 20
4203ac7a:	0c2c      	movi.n	a12, 32
4203ac7c:	41d9      	s32i.n	a13, a1, 16
4203ac7e:	4c28e5        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203ac81:	ff8e06        	j	4203aabd <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x201>
4203ac84:	82a890        	mull	a10, a8, a9
4203ac87:	80a0d2        	movi	a13, 128
4203ac8a:	aada      	add.n	a10, a10, a13
4203ac8c:	f4a0a0        	extui	a10, a10, 0, 16
4203ac8f:	11ea80        	slli	a14, a10, 8
4203ac92:	aeaa      	add.n	a10, a14, a10
4203ac94:	989a      	add.n	a9, a8, a9
4203ac96:	31a0a0        	srai	a10, a10, 16
4203ac99:	c099a0        	sub	a9, a9, a10
4203ac9c:	11f880        	slli	a15, a8, 8
4203ac9f:	f49090        	extui	a9, a9, 0, 16
4203aca2:	c0ff80        	sub	a15, a15, a8
4203aca5:	d2ff90        	quos	a15, a15, a9
4203aca8:	f4f0f0        	extui	a15, a15, 0, 16
4203acab:	c08cf0        	sub	a8, a12, a15
4203acae:	75e0b0        	extui	a14, a11, 16, 8
4203acb1:	f48080        	extui	a8, a8, 0, 16
4203acb4:	c1eef0        	mul16u	a14, a14, a15
4203acb7:	757020        	extui	a7, a2, 16, 8
4203acba:	74c8b0        	extui	a12, a11, 8, 8
4203acbd:	c17780        	mul16u	a7, a7, a8
4203acc0:	c1ccf0        	mul16u	a12, a12, a15
4203acc3:	746820        	extui	a6, a2, 8, 8
4203acc6:	74a0b0        	extui	a10, a11, 0, 8
4203acc9:	c1aaf0        	mul16u	a10, a10, a15
4203accc:	eeda      	add.n	a14, a14, a13
4203acce:	c1f680        	mul16u	a15, a6, a8
4203acd1:	742020        	extui	a2, a2, 0, 8
4203acd4:	c12280        	mul16u	a2, a2, a8
4203acd7:	bcda      	add.n	a11, a12, a13
4203acd9:	ee7a      	add.n	a14, a14, a7
4203acdb:	f4c0e0        	extui	a12, a14, 0, 16
4203acde:	aada      	add.n	a10, a10, a13
4203ace0:	bbfa      	add.n	a11, a11, a15
4203ace2:	f4b0b0        	extui	a11, a11, 0, 16
4203ace5:	118c80        	slli	a8, a12, 8
4203ace8:	aa2a      	add.n	a10, a10, a2
4203acea:	f4a0a0        	extui	a10, a10, 0, 16
4203aced:	88ca      	add.n	a8, a8, a12
4203acef:	112b80        	slli	a2, a11, 8
4203acf2:	11ca80        	slli	a12, a10, 8
4203acf5:	22ba      	add.n	a2, a2, a11
4203acf7:	758080        	extui	a8, a8, 16, 8
4203acfa:	019980        	slli	a9, a9, 24
4203acfd:	acaa      	add.n	a10, a12, a10
4203acff:	118800        	slli	a8, a8, 16
4203ad02:	412820        	srli	a2, a2, 8
4203ad05:	208890        	or	a8, a8, a9
4203ad08:	742820        	extui	a2, a2, 8, 8
4203ad0b:	7590a0        	extui	a9, a10, 16, 8
4203ad0e:	208890        	or	a8, a8, a9
4203ad11:	112280        	slli	a2, a2, 8
4203ad14:	202820        	or	a2, a8, a2
4203ad17:	f01d      	retw.n
4203ad19:	2ab200        	mul.s	f11, f2, f0
4203ad1c:	04          	.byte	0x4
4203ad1d:	052aa2        	l32i	a10, a10, 20
4203ad20:	40a0c2        	movi	a12, 64
4203ad23:	41d9      	s32i.n	a13, a1, 16
4203ad25:	4c1e65        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203ad28:	ff6446        	j	4203aabd <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x201>
4203ad2b:	00          	.byte	00
4203ad2c:	82e890        	mull	a14, a8, a9
4203ad2f:	80a0c2        	movi	a12, 128
4203ad32:	eeca      	add.n	a14, a14, a12
4203ad34:	f4e0e0        	extui	a14, a14, 0, 16
4203ad37:	11fe80        	slli	a15, a14, 8
4203ad3a:	efea      	add.n	a14, a15, a14
4203ad3c:	989a      	add.n	a9, a8, a9
4203ad3e:	31e0e0        	srai	a14, a14, 16
4203ad41:	c099e0        	sub	a9, a9, a14
4203ad44:	11f880        	slli	a15, a8, 8
4203ad47:	f49090        	extui	a9, a9, 0, 16
4203ad4a:	c0ff80        	sub	a15, a15, a8
4203ad4d:	d2ff90        	quos	a15, a15, a9
4203ad50:	f4f0f0        	extui	a15, a15, 0, 16
4203ad53:	75e0b0        	extui	a14, a11, 16, 8
4203ad56:	c0aaf0        	sub	a10, a10, a15
4203ad59:	f4a0a0        	extui	a10, a10, 0, 16
4203ad5c:	755020        	extui	a5, a2, 16, 8
4203ad5f:	7488b0        	extui	a8, a11, 8, 8
4203ad62:	c1eef0        	mul16u	a14, a14, a15
4203ad65:	c155a0        	mul16u	a5, a5, a10
4203ad68:	746820        	extui	a6, a2, 8, 8
4203ad6b:	c188f0        	mul16u	a8, a8, a15
4203ad6e:	74b0b0        	extui	a11, a11, 0, 8
4203ad71:	c166a0        	mul16u	a6, a6, a10
4203ad74:	eeca      	add.n	a14, a14, a12
4203ad76:	c1bbf0        	mul16u	a11, a11, a15
4203ad79:	742020        	extui	a2, a2, 0, 8
4203ad7c:	c122a0        	mul16u	a2, a2, a10
4203ad7f:	88ca      	add.n	a8, a8, a12
4203ad81:	ee5a      	add.n	a14, a14, a5
4203ad83:	bbca      	add.n	a11, a11, a12
4203ad85:	f4e0e0        	extui	a14, a14, 0, 16
4203ad88:	c86a      	add.n	a12, a8, a6
4203ad8a:	ab2a      	add.n	a10, a11, a2
4203ad8c:	f4c0c0        	extui	a12, a12, 0, 16
4203ad8f:	118e80        	slli	a8, a14, 8
4203ad92:	f4a0a0        	extui	a10, a10, 0, 16
4203ad95:	88ea      	add.n	a8, a8, a14
4203ad97:	112c80        	slli	a2, a12, 8
4203ad9a:	758080        	extui	a8, a8, 16, 8
4203ad9d:	11ba80        	slli	a11, a10, 8
4203ada0:	22ca      	add.n	a2, a2, a12
4203ada2:	019980        	slli	a9, a9, 24
4203ada5:	118800        	slli	a8, a8, 16
4203ada8:	abaa      	add.n	a10, a11, a10
4203adaa:	412820        	srli	a2, a2, 8
4203adad:	208890        	or	a8, a8, a9
4203adb0:	742820        	extui	a2, a2, 8, 8
4203adb3:	7590a0        	extui	a9, a10, 16, 8
4203adb6:	208890        	or	a8, a8, a9
4203adb9:	112280        	slli	a2, a2, 8
4203adbc:	202820        	or	a2, a8, a2
4203adbf:	ff2c86        	j	4203aa75 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203adc2:	71f8      	l32i.n	a15, a1, 28
4203adc4:	c0aea0        	sub	a10, a14, a10
4203adc7:	190c      	movi.n	a9, 1
4203adc9:	83f9a0        	moveqz	a15, a9, a10
4203adcc:	d6ef56        	bnez	a15, 4203ab3e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203adcf:	c08e80        	sub	a8, a14, a8
4203add2:	939f80        	movnez	a9, a15, a8
4203add5:	be3916        	beqz	a9, 4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203add8:	ff5886        	j	4203ab3e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203addb:	ca9000        	float.s	f9, a0, 0
4203adde:	a0b282        	l32ai	a8, a2, 0x280
4203ade1:	80          	.byte	0x80
4203ade2:	ccba      	add.n	a12, a12, a11
4203ade4:	f4c0c0        	extui	a12, a12, 0, 16
4203ade7:	11ec80        	slli	a14, a12, 8
4203adea:	ceca      	add.n	a12, a14, a12
4203adec:	9a9a      	add.n	a9, a10, a9
4203adee:	31c0c0        	srai	a12, a12, 16
4203adf1:	c099c0        	sub	a9, a9, a12
4203adf4:	11ca80        	slli	a12, a10, 8
4203adf7:	f49090        	extui	a9, a9, 0, 16
4203adfa:	c0aca0        	sub	a10, a12, a10
4203adfd:	d2aa90        	quos	a10, a10, a9
4203ae00:	f4a0a0        	extui	a10, a10, 0, 16
4203ae03:	75c020        	extui	a12, a2, 16, 8
4203ae06:	c1cca0        	mul16u	a12, a12, a10
4203ae09:	74f820        	extui	a15, a2, 8, 8
4203ae0c:	c1ffa0        	mul16u	a15, a15, a10
4203ae0f:	74e020        	extui	a14, a2, 0, 8
4203ae12:	c1eea0        	mul16u	a14, a14, a10
4203ae15:	ccba      	add.n	a12, a12, a11
4203ae17:	f4c0c0        	extui	a12, a12, 0, 16
4203ae1a:	ffba      	add.n	a15, a15, a11
4203ae1c:	f4f0f0        	extui	a15, a15, 0, 16
4203ae1f:	11ac80        	slli	a10, a12, 8
4203ae22:	eeba      	add.n	a14, a14, a11
4203ae24:	f4e0e0        	extui	a14, a14, 0, 16
4203ae27:	aaca      	add.n	a10, a10, a12
4203ae29:	11bf80        	slli	a11, a15, 8
4203ae2c:	bbfa      	add.n	a11, a11, a15
4203ae2e:	75a0a0        	extui	a10, a10, 16, 8
4203ae31:	11ce80        	slli	a12, a14, 8
4203ae34:	01f980        	slli	a15, a9, 24
4203ae37:	11aa00        	slli	a10, a10, 16
4203ae3a:	4198b0        	srli	a9, a11, 8
4203ae3d:	ccea      	add.n	a12, a12, a14
4203ae3f:	20aaf0        	or	a10, a10, a15
4203ae42:	75b0c0        	extui	a11, a12, 16, 8
4203ae45:	749890        	extui	a9, a9, 8, 8
4203ae48:	20aab0        	or	a10, a10, a11
4203ae4b:	119980        	slli	a9, a9, 8
4203ae4e:	202a90        	or	a2, a10, a9
4203ae51:	fef806        	j	4203aa35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203ae54:	20a220        	or	a10, a2, a2
4203ae57:	0461d2        	s32i	a13, a1, 16
4203ae5a:	4d6be5        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203ae5d:	0421d2        	l32i	a13, a1, 16
4203ae60:	0a2d      	mov.n	a2, a10
4203ae62:	fef3c6        	j	4203aa35 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203ae65:	740884        	mul.aa.ll	a8, a8
4203ae68:	780994        	mula.aa.ll	a9, a9
4203ae6b:	feb806        	j	4203a94f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x93>
4203ae6e:	940000        	extui	a0, a0, 0, 10
4203ae71:	7409      	s32i.n	a0, a4, 28
4203ae73:	780884        	mula.aa.ll	a8, a8
4203ae76:	feb546        	j	4203a94f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x93>
4203ae79:	740884        	mul.aa.ll	a8, a8
4203ae7c:	780994        	mula.aa.ll	a9, a9
4203ae7f:	031080        	rsr.acclo	a8
4203ae82:	118880        	slli	a8, a8, 8
4203ae85:	0248e6        	bgei	a8, 4, 4203ae8b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5cf>
4203ae88:	fecc06        	j	4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203ae8b:	40f890        	nsau	a9, a8
4203ae8e:	dfc992        	addi	a9, a9, -33
4203ae91:	609090        	neg	a9, a9
4203ae94:	219190        	srai	a9, a9, 1
4203ae97:	fec992        	addi	a9, a9, -2
4203ae9a:	3a0c      	movi.n	a10, 3
4203ae9c:	401900        	ssl	a9
4203ae9f:	a1aa00        	sll	a10, a10
4203aea2:	c298a0        	quou	a9, a8, a10
4203aea5:	99aa      	add.n	a9, a9, a10
4203aea7:	419190        	srli	a9, a9, 1
4203aeaa:	c2a890        	quou	a10, a8, a9
4203aead:	aa9a      	add.n	a10, a10, a9
4203aeaf:	41a1a0        	srli	a10, a10, 1
4203aeb2:	c298a0        	quou	a9, a8, a10
4203aeb5:	99aa      	add.n	a9, a9, a10
4203aeb7:	1694a1        	l32r	a10, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203aeba:	419190        	srli	a9, a9, 1
4203aebd:	0299a7        	bne	a9, a10, 4203aec3 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x607>
4203aec0:	febe06        	j	4203a9bc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203aec3:	82a990        	mull	a10, a9, a9
4203aec6:	11ccc0        	slli	a12, a12, 4
4203aec9:	6288a0        	saltu	a8, a8, a10
4203aecc:	c09980        	sub	a9, a9, a8
4203aecf:	5188      	l32i.n	a8, a1, 20
4203aed1:	88ca      	add.n	a8, a8, a12
4203aed3:	c08890        	sub	a8, a8, a9
4203aed6:	feb186        	j	4203a9a0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xe4>


/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039de0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42039de0:	00c136        	entry	a1, 96
42039de3:	850b      	addi.n	a8, a5, -1
42039de5:	a189      	s32i.n	a8, a1, 40
42039de7:	3cc282        	addi	a8, a2, 60
42039dea:	6149      	s32i.n	a4, a1, 24
42039dec:	b169      	s32i.n	a6, a1, 44
42039dee:	7139      	s32i.n	a3, a1, 28
42039df0:	d189      	s32i.n	a8, a1, 52
42039df2:	252516        	beqz	a5, 4203a048 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x268>
42039df5:	61b8      	l32i.n	a11, a1, 24
42039df7:	71a8      	l32i.n	a10, a1, 28
42039df9:	200292        	l8ui	a9, a2, 32
42039dfc:	021282        	l16ui	a8, a2, 4
42039dff:	001bc2        	l16ui	a12, a11, 0
42039e02:	001aa2        	l16ui	a10, a10, 0
42039e05:	0412e2        	l16ui	a14, a2, 8
42039e08:	52f8      	l32i.n	a15, a2, 20
42039e0a:	898a      	add.n	a8, a9, a8
42039e0c:	c0be90        	sub	a11, a14, a9
42039e0f:	23d880        	sext	a13, a8, 15
42039e12:	1051a2        	s16i	a10, a1, 32
42039e15:	237a80        	sext	a7, a10, 15
42039e18:	c1f9      	s32i.n	a15, a1, 48
42039e1a:	1251c2        	s16i	a12, a1, 36
42039e1d:	233c80        	sext	a3, a12, 15
42039e20:	f48080        	extui	a8, a8, 0, 16
42039e23:	23bb80        	sext	a11, a11, 15
42039e26:	02a7d7        	bge	a7, a13, 42039e2c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4c>
42039e29:	008ac6        	j	4203a058 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e2c:	02ab77        	bge	a11, a7, 42039e32 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x52>
42039e2f:	008946        	j	4203a058 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e32:	0312a2        	l16ui	a10, a2, 6
42039e35:	051252        	l16ui	a5, a2, 10
42039e38:	a9aa      	add.n	a10, a9, a10
42039e3a:	c0c590        	sub	a12, a5, a9
42039e3d:	23fa80        	sext	a15, a10, 15
42039e40:	23cc80        	sext	a12, a12, 15
42039e43:	f4a0a0        	extui	a10, a10, 0, 16
42039e46:	02a3f7        	bge	a3, a15, 42039e4c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x6c>
42039e49:	0082c6        	j	4203a058 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e4c:	724c30        	salt	a4, a12, a3
42039e4f:	f149      	s32i.n	a4, a1, 60
42039e51:	02ac37        	bge	a12, a3, 42039e57 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x77>
42039e54:	008006        	j	4203a058 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e57:	210242        	l8ui	a4, a2, 33
42039e5a:	1c0262        	l8ui	a6, a2, 28
42039e5d:	e149      	s32i.n	a4, a1, 56
42039e5f:	00e616        	beqz	a6, 42039e71 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x91>
42039e62:	804d60        	add	a4, a13, a6
42039e65:	082477        	blt	a4, a7, 42039e71 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x91>
42039e68:	804f60        	add	a4, a15, a6
42039e6b:	022437        	blt	a4, a3, 42039e71 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x91>
42039e6e:	00b3c6        	j	4203a141 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x361>
42039e71:	1d0262        	l8ui	a6, a2, 29
42039e74:	d68c      	beqz.n	a6, 42039e85 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa5>
42039e76:	c04b60        	sub	a4, a11, a6
42039e79:	082747        	blt	a7, a4, 42039e85 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa5>
42039e7c:	804f60        	add	a4, a15, a6
42039e7f:	022437        	blt	a4, a3, 42039e85 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa5>
42039e82:	00eb06        	j	4203a232 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x452>
42039e85:	1e02a2        	l8ui	a10, a2, 30
42039e88:	da8c      	beqz.n	a10, 42039e99 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb9>
42039e8a:	c06ba0        	sub	a6, a11, a10
42039e8d:	082767        	blt	a7, a6, 42039e99 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb9>
42039e90:	c06ca0        	sub	a6, a12, a10
42039e93:	022367        	blt	a3, a6, 42039e99 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb9>
42039e96:	00f486        	j	4203a26c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x48c>
42039e99:	1f02a2        	l8ui	a10, a2, 31
42039e9c:	da8c      	beqz.n	a10, 42039ead <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xcd>
42039e9e:	80eda0        	add	a14, a13, a10
42039ea1:	082e77        	blt	a14, a7, 42039ead <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xcd>
42039ea4:	c0eca0        	sub	a14, a12, a10
42039ea7:	0223e7        	blt	a3, a14, 42039ead <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xcd>
42039eaa:	010f86        	j	4203a2ec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x50c>
42039ead:	e198      	l32i.n	a9, a1, 56
42039eaf:	e80c      	movi.n	a8, 14
42039eb1:	283897        	bltu	a8, a9, 42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
42039eb4:	c0d7d0        	sub	a13, a7, a13
42039eb7:	40fd80        	nsau	a8, a13
42039eba:	190c      	movi.n	a9, 1
42039ebc:	418580        	srli	a8, a8, 5
42039ebf:	187d16        	beqz	a13, 4203a04a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ec2:	c0b7b0        	sub	a11, a7, a11
42039ec5:	8389b0        	moveqz	a8, a9, a11
42039ec8:	17e856        	bnez	a8, 4203a04a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ecb:	c0f3f0        	sub	a15, a3, a15
42039ece:	8389f0        	moveqz	a8, a9, a15
42039ed1:	175856        	bnez	a8, 4203a04a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ed4:	c0c3c0        	sub	a12, a3, a12
42039ed7:	9398c0        	movnez	a9, a8, a12
42039eda:	16c956        	bnez	a9, 4203a04a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039edd:	142282        	l32i	a8, a2, 80
42039ee0:	c168      	l32i.n	a6, a1, 48
42039ee2:	142816        	beqz	a8, 4203a028 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
42039ee5:	ffa052        	movi	a5, 255
42039ee8:	d1b8      	l32i.n	a11, a1, 52
42039eea:	03dd      	mov.n	a13, a3
42039eec:	07cd      	mov.n	a12, a7
42039eee:	28c2a2        	addi	a10, a2, 40
42039ef1:	013425        	call8	4203b234 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42039ef4:	200282        	l8ui	a8, a2, 32
42039ef7:	0a4d      	mov.n	a4, a10
42039ef9:	016a80        	slli	a6, a10, 24
42039efc:	05ed      	mov.n	a14, a5
42039efe:	042816        	beqz	a8, 42039f44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
42039f01:	62e8      	l32i.n	a14, a2, 24
42039f03:	0592d2        	l16si	a13, a2, 10
42039f06:	0492c2        	l16si	a12, a2, 8
42039f09:	0392b2        	l16si	a11, a2, 6
42039f0c:	0292a2        	l16si	a10, a2, 4
42039f0f:	ff0c      	movi.n	a15, 15
42039f11:	1139      	s32i.n	a3, a1, 4
42039f13:	0179      	s32i.n	a7, a1, 0
42039f15:	fe8665        	call8	4203877c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42039f18:	20eaa0        	or	a14, a10, a10
42039f1b:	5aac      	beqz.n	a10, 42039f44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
42039f1d:	0922b2        	l32i	a11, a2, 36
42039f20:	019a80        	slli	a9, a10, 24
42039f23:	11bb80        	slli	a11, a11, 8
42039f26:	41b8b0        	srli	a11, a11, 8
42039f29:	758890        	extui	a8, a9, 24, 8
42039f2c:	ffa0a2        	movi	a10, 255
42039f2f:	20bb90        	or	a11, a11, a9
42039f32:	0298a7        	bne	a8, a10, 42039f38 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x158>
42039f35:	0054c6        	j	4203a08c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2ac>
42039f38:	0294a7        	bne	a4, a10, 42039f3e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x15e>
42039f3b:	010b46        	j	4203a36c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x58c>
42039f3e:	206bb0        	or	a6, a11, a11
42039f41:	4d0456        	bnez	a4, 4203a415 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x635>
42039f44:	ffa092        	movi	a9, 255
42039f47:	029597        	bne	a5, a9, 42039f4d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x16d>
42039f4a:	0043c6        	j	4203a05d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x27d>
42039f4d:	25bc      	beqz.n	a5, 42039f83 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
42039f4f:	c1b8      	l32i.n	a11, a1, 48
42039f51:	1a70a1        	l32r	a10, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
42039f54:	7588b0        	extui	a8, a11, 24, 8
42039f57:	d18850        	mul16s	a8, a8, a5
42039f5a:	11bb80        	slli	a11, a11, 8
42039f5d:	7fc882        	addi	a8, a8, 127
42039f60:	a288a0        	muluh	a8, a8, a10
42039f63:	0188f0        	slli	a8, a8, 17
42039f66:	75a880        	extui	a10, a8, 24, 8
42039f69:	0a8d      	mov.n	a8, a10
42039f6b:	404800        	ssai	8
42039f6e:	81bab0        	src	a11, a10, a11
42039f71:	029a97        	bne	a10, a9, 42039f77 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x197>
42039f74:	004006        	j	4203a078 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x298>
42039f77:	75a860        	extui	a10, a6, 24, 8
42039f7a:	029a97        	bne	a10, a9, 42039f80 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a0>
42039f7d:	004546        	j	4203a096 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b6>
42039f80:	124856        	bnez	a8, 4203a0a8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2c8>
42039f83:	1422a2        	l32i	a10, a2, 80
42039f86:	09ea16        	beqz	a10, 4203a028 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
42039f89:	180a82        	l8ui	a8, a10, 24
42039f8c:	0d8856        	bnez	a8, 4203a068 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x288>
42039f8f:	9198      	l32i.n	a9, a1, 36
42039f91:	031a82        	l16ui	a8, a10, 6
42039f94:	021ab2        	l16ui	a11, a10, 4
42039f97:	c08980        	sub	a8, a9, a8
42039f9a:	740884        	mul.aa.ll	a8, a8
42039f9d:	8188      	l32i.n	a8, a1, 32
42039f9f:	3a98      	l32i.n	a9, a10, 12
42039fa1:	c088b0        	sub	a8, a8, a11
42039fa4:	780884        	mula.aa.ll	a8, a8
42039fa7:	031080        	rsr.acclo	a8
42039faa:	c08980        	sub	a8, a9, a8
42039fad:	0218e6        	bgei	a8, 1, 42039fb3 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d3>
42039fb0:	003246        	j	4203a07d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x29d>
42039fb3:	049a92        	l16si	a9, a10, 8
42039fb6:	0219e6        	bgei	a9, 1, 42039fbc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1dc>
42039fb9:	003506        	j	4203a091 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fbc:	b99a      	add.n	a11, a9, a9
42039fbe:	02a8b7        	bge	a8, a11, 42039fc4 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e4>
42039fc1:	007146        	j	4203a18a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3aa>
42039fc4:	021966        	bnei	a9, 1, 42039fca <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1ea>
42039fc7:	003186        	j	4203a091 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fca:	c90b      	addi.n	a12, a9, -1
42039fcc:	23cc80        	sext	a12, a12, 15
42039fcf:	11cce0        	slli	a12, a12, 2
42039fd2:	022c87        	blt	a12, a8, 42039fd8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1f8>
42039fd5:	00e946        	j	4203a37e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x59e>
42039fd8:	022966        	bnei	a9, 2, 42039fde <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1fe>
42039fdb:	002c86        	j	4203a091 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fde:	bb9a      	add.n	a11, a11, a9
42039fe0:	bbba      	add.n	a11, a11, a11
42039fe2:	f8cbb2        	addi	a11, a11, -8
42039fe5:	0228b7        	blt	a8, a11, 42039feb <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x20b>
42039fe8:	002946        	j	4203a091 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039feb:	4ab8      	l32i.n	a11, a10, 16
42039fed:	5aa8      	l32i.n	a10, a10, 20
42039fef:	0c2c      	movi.n	a12, 32
42039ff1:	81e9      	s32i.n	a14, a1, 32
42039ff3:	4cf1a5        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039ff6:	81e8      	l32i.n	a14, a1, 32
42039ff8:	7598a0        	extui	a9, a10, 24, 8
42039ffb:	1a4681        	l32r	a8, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
42039ffe:	d199e0        	mul16s	a9, a9, a14
4203a001:	11aa80        	slli	a10, a10, 8
4203a004:	a29980        	muluh	a9, a9, a8
4203a007:	ffa082        	movi	a8, 255
4203a00a:	0199f0        	slli	a9, a9, 17
4203a00d:	75b890        	extui	a11, a9, 24, 8
4203a010:	0bcd      	mov.n	a12, a11
4203a012:	404800        	ssai	8
4203a015:	81bba0        	src	a11, a11, a10
4203a018:	0a1c87        	beq	a12, a8, 4203a026 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x246>
4203a01b:	759860        	extui	a9, a6, 24, 8
4203a01e:	601987        	beq	a9, a8, 4203a082 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2a2>
4203a021:	3c8c      	beqz.n	a12, 4203a028 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
4203a023:	175956        	bnez	a9, 4203a19c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3bc>
4203a026:	0b6d      	mov.n	a6, a11
4203a028:	b188      	l32i.n	a8, a1, 44
4203a02a:	a198      	l32i.n	a9, a1, 40
4203a02c:	0869      	s32i.n	a6, a8, 0
4203a02e:	884b      	addi.n	a8, a8, 4
4203a030:	61a8      	l32i.n	a10, a1, 24
4203a032:	b189      	s32i.n	a8, a1, 44
4203a034:	7188      	l32i.n	a8, a1, 28
4203a036:	990b      	addi.n	a9, a9, -1
4203a038:	aa2b      	addi.n	a10, a10, 2
4203a03a:	882b      	addi.n	a8, a8, 2
4203a03c:	a199      	s32i.n	a9, a1, 40
4203a03e:	61a9      	s32i.n	a10, a1, 24
4203a040:	7189      	s32i.n	a8, a1, 28
4203a042:	020926        	beqi	a9, -1, 4203a048 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x268>
4203a045:	ff6b06        	j	42039df5 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x15>
4203a048:	f01d      	retw.n
4203a04a:	e198      	l32i.n	a9, a1, 56
4203a04c:	1189c0        	slli	a8, a9, 4
4203a04f:	598a      	add.n	a5, a9, a8
4203a051:	745050        	extui	a5, a5, 0, 8
4203a054:	ffa406        	j	42039ee8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x108>
4203a057:	050c00        	extui	a0, a0, 28, 1
4203a05a:	ffa286        	j	42039ee8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x108>
4203a05d:	1a2c61        	l32r	a6, 42000910 <_stext+0x8f0> (ff000000 <_rtc_reserved_end+0x9ef00000>)
4203a060:	c188      	l32i.n	a8, a1, 48
4203a062:	206680        	or	a6, a6, a8
4203a065:	ffc686        	j	42039f83 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a068:	20c330        	or	a12, a3, a3
4203a06b:	20b770        	or	a11, a7, a7
4203a06e:	0861e2        	s32i	a14, a1, 32
4203a071:	ece725        	call8	42026ee4 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
4203a074:	ffdf86        	j	42039ff6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x216>
4203a077:	00          	.byte	00
4203a078:	0b6d      	mov.n	a6, a11
4203a07a:	ffc146        	j	42039f83 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a07d:	5aa8      	l32i.n	a10, a10, 20
4203a07f:	ffdd46        	j	42039ff8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x218>
4203a082:	06ad      	mov.n	a10, a6
4203a084:	4e4925        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a087:	0a6d      	mov.n	a6, a10
4203a089:	ffe6c6        	j	4203a028 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
4203a08c:	0b6d      	mov.n	a6, a11
4203a08e:	ffac86        	j	42039f44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
4203a091:	4aa8      	l32i.n	a10, a10, 16
4203a093:	ffd846        	j	42039ff8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x218>
4203a096:	20a660        	or	a10, a6, a6
4203a099:	0c61e2        	s32i	a14, a1, 48
4203a09c:	4e47a5        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a09f:	0c21e2        	l32i	a14, a1, 48
4203a0a2:	0a6d      	mov.n	a6, a10
4203a0a4:	ffb6c6        	j	42039f83 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a0a7:	ca1600        	float.s	f1, a6, 0
4203a0aa:	a0fc      	bnez.n	a0, 4203a0e8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x308>
4203a0ac:	82c8      	l32i.n	a12, a2, 32
4203a0ae:	80a0f2        	movi	a15, 128
4203a0b1:	ccfa      	add.n	a12, a12, a15
4203a0b3:	f4c0c0        	extui	a12, a12, 0, 16
4203a0b6:	11dc80        	slli	a13, a12, 8
4203a0b9:	cdca      	add.n	a12, a13, a12
4203a0bb:	a8aa      	add.n	a10, a8, a10
4203a0bd:	31c0c0        	srai	a12, a12, 16
4203a0c0:	c0cac0        	sub	a12, a10, a12
4203a0c3:	11a880        	slli	a10, a8, 8
4203a0c6:	f450c0        	extui	a5, a12, 0, 16
4203a0c9:	c08a80        	sub	a8, a10, a8
4203a0cc:	d28850        	quos	a8, a8, a5
4203a0cf:	f48080        	extui	a8, a8, 0, 16
4203a0d2:	75a0b0        	extui	a10, a11, 16, 8
4203a0d5:	c09980        	sub	a9, a9, a8
4203a0d8:	f49090        	extui	a9, a9, 0, 16
4203a0db:	c14a80        	mul16u	a4, a10, a8
4203a0de:	75d060        	extui	a13, a6, 16, 8
4203a0e1:	74c8b0        	extui	a12, a11, 8, 8
4203a0e4:	c1dd90        	mul16u	a13, a13, a9
4203a0e7:	c1cc80        	mul16u	a12, a12, a8
4203a0ea:	74a860        	extui	a10, a6, 8, 8
4203a0ed:	74b0b0        	extui	a11, a11, 0, 8
4203a0f0:	c1aa90        	mul16u	a10, a10, a9
4203a0f3:	c1bb80        	mul16u	a11, a11, a8
4203a0f6:	746060        	extui	a6, a6, 0, 8
4203a0f9:	84fa      	add.n	a8, a4, a15
4203a0fb:	c19690        	mul16u	a9, a6, a9
4203a0fe:	88da      	add.n	a8, a8, a13
4203a100:	ccfa      	add.n	a12, a12, a15
4203a102:	f48080        	extui	a8, a8, 0, 16
4203a105:	ccaa      	add.n	a12, a12, a10
4203a107:	bbfa      	add.n	a11, a11, a15
4203a109:	f4c0c0        	extui	a12, a12, 0, 16
4203a10c:	11a880        	slli	a10, a8, 8
4203a10f:	bb9a      	add.n	a11, a11, a9
4203a111:	f4b0b0        	extui	a11, a11, 0, 16
4203a114:	8a8a      	add.n	a8, a10, a8
4203a116:	119c80        	slli	a9, a12, 8
4203a119:	99ca      	add.n	a9, a9, a12
4203a11b:	758080        	extui	a8, a8, 16, 8
4203a11e:	11ab80        	slli	a10, a11, 8
4203a121:	118800        	slli	a8, a8, 16
4203a124:	01c580        	slli	a12, a5, 24
4203a127:	aaba      	add.n	a10, a10, a11
4203a129:	419890        	srli	a9, a9, 8
4203a12c:	2068c0        	or	a6, a8, a12
4203a12f:	75a0a0        	extui	a10, a10, 16, 8
4203a132:	748890        	extui	a8, a9, 8, 8
4203a135:	2066a0        	or	a6, a6, a10
4203a138:	118880        	slli	a8, a8, 8
4203a13b:	206680        	or	a6, a6, a8
4203a13e:	ff9046        	j	42039f83 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a141:	986a      	add.n	a9, a8, a6
4203a143:	8188      	l32i.n	a8, a1, 32
4203a145:	aa6a      	add.n	a10, a10, a6
4203a147:	c09980        	sub	a9, a9, a8
4203a14a:	9188      	l32i.n	a8, a1, 36
4203a14c:	f49090        	extui	a9, a9, 0, 16
4203a14f:	c08a80        	sub	a8, a10, a8
4203a152:	f48080        	extui	a8, a8, 0, 16
4203a155:	20a980        	or	a10, a9, a8
4203a158:	23aa80        	sext	a10, a10, 15
4203a15b:	231ad6        	bgez	a10, 4203a390 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5b0>
4203a15e:	f148      	l32i.n	a4, a1, 60
4203a160:	c09690        	sub	a9, a6, a9
4203a163:	1a0c      	movi.n	a10, 1
4203a165:	834a90        	moveqz	a4, a10, a9
4203a168:	008456        	bnez	a4, 4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a16b:	c08680        	sub	a8, a6, a8
4203a16e:	93a480        	movnez	a10, a4, a8
4203a171:	d68a16        	beqz	a10, 42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a174:	e198      	l32i.n	a9, a1, 56
4203a176:	1189c0        	slli	a8, a9, 4
4203a179:	898a      	add.n	a8, a9, a8
4203a17b:	745080        	extui	a5, a8, 0, 8
4203a17e:	ffa082        	movi	a8, 255
4203a181:	029587        	bne	a5, a8, 4203a187 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3a7>
4203a184:	ff5546        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a187:	ff5746        	j	42039ee8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x108>
4203a18a:	042ab2        	l32i	a11, a10, 16
4203a18d:	052aa2        	l32i	a10, a10, 20
4203a190:	60a0c2        	movi	a12, 96
4203a193:	0861e2        	s32i	a14, a1, 32
4203a196:	4cd765        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a199:	ff9646        	j	42039ff6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x216>
4203a19c:	82ac90        	mull	a10, a12, a9
4203a19f:	80a0e2        	movi	a14, 128
4203a1a2:	aaea      	add.n	a10, a10, a14
4203a1a4:	f4a0a0        	extui	a10, a10, 0, 16
4203a1a7:	11da80        	slli	a13, a10, 8
4203a1aa:	adaa      	add.n	a10, a13, a10
4203a1ac:	9c9a      	add.n	a9, a12, a9
4203a1ae:	31a0a0        	srai	a10, a10, 16
4203a1b1:	c099a0        	sub	a9, a9, a10
4203a1b4:	11dc80        	slli	a13, a12, 8
4203a1b7:	f49090        	extui	a9, a9, 0, 16
4203a1ba:	c0ddc0        	sub	a13, a13, a12
4203a1bd:	d2dd90        	quos	a13, a13, a9
4203a1c0:	f4d0d0        	extui	a13, a13, 0, 16
4203a1c3:	c088d0        	sub	a8, a8, a13
4203a1c6:	75c0b0        	extui	a12, a11, 16, 8
4203a1c9:	f48080        	extui	a8, a8, 0, 16
4203a1cc:	c1ccd0        	mul16u	a12, a12, a13
4203a1cf:	757060        	extui	a7, a6, 16, 8
4203a1d2:	74a8b0        	extui	a10, a11, 8, 8
4203a1d5:	c17780        	mul16u	a7, a7, a8
4203a1d8:	c1aad0        	mul16u	a10, a10, a13
4203a1db:	74f860        	extui	a15, a6, 8, 8
4203a1de:	74b0b0        	extui	a11, a11, 0, 8
4203a1e1:	c1bbd0        	mul16u	a11, a11, a13
4203a1e4:	c1ff80        	mul16u	a15, a15, a8
4203a1e7:	ccea      	add.n	a12, a12, a14
4203a1e9:	746060        	extui	a6, a6, 0, 8
4203a1ec:	c16680        	mul16u	a6, a6, a8
4203a1ef:	aaea      	add.n	a10, a10, a14
4203a1f1:	cc7a      	add.n	a12, a12, a7
4203a1f3:	bbea      	add.n	a11, a11, a14
4203a1f5:	f4c0c0        	extui	a12, a12, 0, 16
4203a1f8:	eafa      	add.n	a14, a10, a15
4203a1fa:	db6a      	add.n	a13, a11, a6
4203a1fc:	f4e0e0        	extui	a14, a14, 0, 16
4203a1ff:	118c80        	slli	a8, a12, 8
4203a202:	f4d0d0        	extui	a13, a13, 0, 16
4203a205:	88ca      	add.n	a8, a8, a12
4203a207:	116e80        	slli	a6, a14, 8
4203a20a:	758080        	extui	a8, a8, 16, 8
4203a20d:	11ad80        	slli	a10, a13, 8
4203a210:	66ea      	add.n	a6, a6, a14
4203a212:	01b980        	slli	a11, a9, 24
4203a215:	118800        	slli	a8, a8, 16
4203a218:	9ada      	add.n	a9, a10, a13
4203a21a:	416860        	srli	a6, a6, 8
4203a21d:	2088b0        	or	a8, a8, a11
4203a220:	759090        	extui	a9, a9, 16, 8
4203a223:	746860        	extui	a6, a6, 8, 8
4203a226:	208890        	or	a8, a8, a9
4203a229:	116680        	slli	a6, a6, 8
4203a22c:	206860        	or	a6, a8, a6
4203a22f:	ff7d46        	j	4203a028 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
4203a232:	896a      	add.n	a8, a9, a6
4203a234:	8198      	l32i.n	a9, a1, 32
4203a236:	c088e0        	sub	a8, a8, a14
4203a239:	889a      	add.n	a8, a8, a9
4203a23b:	9198      	l32i.n	a9, a1, 36
4203a23d:	aa6a      	add.n	a10, a10, a6
4203a23f:	c0aa90        	sub	a10, a10, a9
4203a242:	f48080        	extui	a8, a8, 0, 16
4203a245:	f4a0a0        	extui	a10, a10, 0, 16
4203a248:	2098a0        	or	a9, a8, a10
4203a24b:	239980        	sext	a9, a9, 15
4203a24e:	1ba9d6        	bgez	a9, 4203a40c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x62c>
4203a251:	c08680        	sub	a8, a6, a8
4203a254:	40f890        	nsau	a9, a8
4203a257:	419590        	srli	a9, a9, 5
4203a25a:	f16816        	beqz	a8, 4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a25d:	c0a6a0        	sub	a10, a6, a10
4203a260:	180c      	movi.n	a8, 1
4203a262:	9389a0        	movnez	a8, a9, a10
4203a265:	c74816        	beqz	a8, 42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a268:	ffc206        	j	4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a26b:	818800        	src	a8, a8, a0
4203a26e:	99aa      	add.n	a9, a9, a10
4203a270:	c0e9e0        	sub	a14, a9, a14
4203a273:	ee8a      	add.n	a14, a14, a8
4203a275:	9188      	l32i.n	a8, a1, 36
4203a277:	c09950        	sub	a9, a9, a5
4203a27a:	998a      	add.n	a9, a9, a8
4203a27c:	f4e0e0        	extui	a14, a14, 0, 16
4203a27f:	f49090        	extui	a9, a9, 0, 16
4203a282:	208e90        	or	a8, a14, a9
4203a285:	238880        	sext	a8, a8, 15
4203a288:	201896        	bltz	a8, 4203a48d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x6ad>
4203a28b:	740994        	mul.aa.ll	a9, a9
4203a28e:	780ee4        	mula.aa.ll	a14, a14
4203a291:	031080        	rsr.acclo	a8
4203a294:	119880        	slli	a9, a8, 8
4203a297:	0249e6        	bgei	a9, 4, 4203a29d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4bd>
4203a29a:	ff0fc6        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a29d:	40f980        	nsau	a8, a9
4203a2a0:	dfc882        	addi	a8, a8, -33
4203a2a3:	608080        	neg	a8, a8
4203a2a6:	218180        	srai	a8, a8, 1
4203a2a9:	3b0c      	movi.n	a11, 3
4203a2ab:	fec882        	addi	a8, a8, -2
4203a2ae:	401800        	ssl	a8
4203a2b1:	a18b00        	sll	a8, a11
4203a2b4:	c2b980        	quou	a11, a9, a8
4203a2b7:	bb8a      	add.n	a11, a11, a8
4203a2b9:	41b1b0        	srli	a11, a11, 1
4203a2bc:	c289b0        	quou	a8, a9, a11
4203a2bf:	88ba      	add.n	a8, a8, a11
4203a2c1:	418180        	srli	a8, a8, 1
4203a2c4:	c2b980        	quou	a11, a9, a8
4203a2c7:	bb8a      	add.n	a11, a11, a8
4203a2c9:	198f81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a2cc:	41b1b0        	srli	a11, a11, 1
4203a2cf:	029b87        	bne	a11, a8, 4203a2d5 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4f5>
4203a2d2:	ff01c6        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a2d5:	82cbb0        	mull	a12, a11, a11
4203a2d8:	118ac0        	slli	a8, a10, 4
4203a2db:	6299c0        	saltu	a9, a9, a12
4203a2de:	c0bb90        	sub	a11, a11, a9
4203a2e1:	e198      	l32i.n	a9, a1, 56
4203a2e3:	898a      	add.n	a8, a9, a8
4203a2e5:	c088b0        	sub	a8, a8, a11
4203a2e8:	004046        	j	4203a3ed <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x60d>
4203a2eb:	00          	.byte	00
4203a2ec:	99aa      	add.n	a9, a9, a10
4203a2ee:	b8aa      	add.n	a11, a8, a10
4203a2f0:	c08950        	sub	a8, a9, a5
4203a2f3:	8198      	l32i.n	a9, a1, 32
4203a2f5:	c09b90        	sub	a9, a11, a9
4203a2f8:	91b8      	l32i.n	a11, a1, 36
4203a2fa:	f49090        	extui	a9, a9, 0, 16
4203a2fd:	88ba      	add.n	a8, a8, a11
4203a2ff:	f48080        	extui	a8, a8, 0, 16
4203a302:	20b980        	or	a11, a9, a8
4203a305:	23bb80        	sext	a11, a11, 15
4203a308:	19cb96        	bltz	a11, 4203a4a8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x6c8>
4203a30b:	740884        	mul.aa.ll	a8, a8
4203a30e:	780994        	mula.aa.ll	a9, a9
4203a311:	031080        	rsr.acclo	a8
4203a314:	118880        	slli	a8, a8, 8
4203a317:	0248e6        	bgei	a8, 4, 4203a31d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x53d>
4203a31a:	feefc6        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a31d:	40f890        	nsau	a9, a8
4203a320:	dfc992        	addi	a9, a9, -33
4203a323:	609090        	neg	a9, a9
4203a326:	219190        	srai	a9, a9, 1
4203a329:	3b0c      	movi.n	a11, 3
4203a32b:	fec992        	addi	a9, a9, -2
4203a32e:	401900        	ssl	a9
4203a331:	a19b00        	sll	a9, a11
4203a334:	c2b890        	quou	a11, a8, a9
4203a337:	bb9a      	add.n	a11, a11, a9
4203a339:	41b1b0        	srli	a11, a11, 1
4203a33c:	c298b0        	quou	a9, a8, a11
4203a33f:	99ba      	add.n	a9, a9, a11
4203a341:	419190        	srli	a9, a9, 1
4203a344:	c2b890        	quou	a11, a8, a9
4203a347:	9b9a      	add.n	a9, a11, a9
4203a349:	196fb1        	l32r	a11, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a34c:	419190        	srli	a9, a9, 1
4203a34f:	0299b7        	bne	a9, a11, 4203a355 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x575>
4203a352:	fee1c6        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a355:	82b990        	mull	a11, a9, a9
4203a358:	11aac0        	slli	a10, a10, 4
4203a35b:	6288b0        	saltu	a8, a8, a11
4203a35e:	c09980        	sub	a9, a9, a8
4203a361:	e188      	l32i.n	a8, a1, 56
4203a363:	88aa      	add.n	a8, a8, a10
4203a365:	c08890        	sub	a8, a8, a9
4203a368:	002046        	j	4203a3ed <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x60d>
4203a36b:	a66000        	ee.ldf.64.xp	f6, f10, a0, a0
4203a36e:	20          	.byte	0x20
4203a36f:	0e61e2        	s32i	a14, a1, 56
4203a372:	4e1a65        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a375:	0e21e2        	l32i	a14, a1, 56
4203a378:	0a6d      	mov.n	a6, a10
4203a37a:	fef186        	j	42039f44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
4203a37d:	2ab200        	mul.s	f11, f2, f0
4203a380:	04          	.byte	0x4
4203a381:	052aa2        	l32i	a10, a10, 20
4203a384:	40a0c2        	movi	a12, 64
4203a387:	81e9      	s32i.n	a14, a1, 32
4203a389:	4cb825        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a38c:	ff1986        	j	42039ff6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x216>
4203a38f:	088400        	lsx	f8, a4, a0
4203a392:	74          	.byte	0x74
4203a393:	780994        	mula.aa.ll	a9, a9
4203a396:	031080        	rsr.acclo	a8
4203a399:	119880        	slli	a9, a8, 8
4203a39c:	0249e6        	bgei	a9, 4, 4203a3a2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5c2>
4203a39f:	fece86        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a3a2:	40f980        	nsau	a8, a9
4203a3a5:	dfc882        	addi	a8, a8, -33
4203a3a8:	608080        	neg	a8, a8
4203a3ab:	218180        	srai	a8, a8, 1
4203a3ae:	3a0c      	movi.n	a10, 3
4203a3b0:	fec882        	addi	a8, a8, -2
4203a3b3:	401800        	ssl	a8
4203a3b6:	a18a00        	sll	a8, a10
4203a3b9:	c2a980        	quou	a10, a9, a8
4203a3bc:	aa8a      	add.n	a10, a10, a8
4203a3be:	41a1a0        	srli	a10, a10, 1
4203a3c1:	c289a0        	quou	a8, a9, a10
4203a3c4:	88aa      	add.n	a8, a8, a10
4203a3c6:	418180        	srli	a8, a8, 1
4203a3c9:	c2a980        	quou	a10, a9, a8
4203a3cc:	aa8a      	add.n	a10, a10, a8
4203a3ce:	194e81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a3d1:	41a1a0        	srli	a10, a10, 1
4203a3d4:	029a87        	bne	a10, a8, 4203a3da <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5fa>
4203a3d7:	fec086        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a3da:	82baa0        	mull	a11, a10, a10
4203a3dd:	1186c0        	slli	a8, a6, 4
4203a3e0:	6299b0        	saltu	a9, a9, a11
4203a3e3:	c0aa90        	sub	a10, a10, a9
4203a3e6:	e198      	l32i.n	a9, a1, 56
4203a3e8:	898a      	add.n	a8, a9, a8
4203a3ea:	c088a0        	sub	a8, a8, a10
4203a3ed:	f49080        	extui	a9, a8, 0, 16
4203a3f0:	23a880        	sext	a10, a8, 15
4203a3f3:	0268f7        	bbci	a8, 15, 4203a3f9 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x619>
4203a3f6:	ff1786        	j	4203a058 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
4203a3f9:	f80c      	movi.n	a8, 15
4203a3fb:	02a8a7        	bge	a8, a10, 4203a401 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x621>
4203a3fe:	feb6c6        	j	42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a401:	1189c0        	slli	a8, a9, 4
4203a404:	998a      	add.n	a9, a9, a8
4203a406:	745090        	extui	a5, a9, 0, 8
4203a409:	ff5c46        	j	4203a17e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x39e>
4203a40c:	740aa4        	mul.aa.ll	a10, a10
4203a40f:	780884        	mula.aa.ll	a8, a8
4203a412:	ffe006        	j	4203a396 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5b6>
4203a415:	829840        	mull	a9, a8, a4
4203a418:	80a0c2        	movi	a12, 128
4203a41b:	99ca      	add.n	a9, a9, a12
4203a41d:	f49090        	extui	a9, a9, 0, 16
4203a420:	11a980        	slli	a10, a9, 8
4203a423:	9a9a      	add.n	a9, a10, a9
4203a425:	484a      	add.n	a4, a8, a4
4203a427:	319090        	srai	a9, a9, 16
4203a42a:	c04490        	sub	a4, a4, a9
4203a42d:	119880        	slli	a9, a8, 8
4203a430:	f4d040        	extui	a13, a4, 0, 16
4203a433:	c08980        	sub	a8, a9, a8
4203a436:	d288d0        	quos	a8, a8, a13
4203a439:	f48080        	extui	a8, a8, 0, 16
4203a43c:	7590b0        	extui	a9, a11, 16, 8
4203a43f:	c19980        	mul16u	a9, a9, a8
4203a442:	74b8b0        	extui	a11, a11, 8, 8
4203a445:	c1bb80        	mul16u	a11, a11, a8
4203a448:	74a060        	extui	a10, a6, 0, 8
4203a44b:	c1aa80        	mul16u	a10, a10, a8
4203a44e:	99ca      	add.n	a9, a9, a12
4203a450:	f49090        	extui	a9, a9, 0, 16
4203a453:	bbca      	add.n	a11, a11, a12
4203a455:	f4b0b0        	extui	a11, a11, 0, 16
4203a458:	118980        	slli	a8, a9, 8
4203a45b:	aaca      	add.n	a10, a10, a12
4203a45d:	f4a0a0        	extui	a10, a10, 0, 16
4203a460:	889a      	add.n	a8, a8, a9
4203a462:	119b80        	slli	a9, a11, 8
4203a465:	99ba      	add.n	a9, a9, a11
4203a467:	758080        	extui	a8, a8, 16, 8
4203a46a:	11ca80        	slli	a12, a10, 8
4203a46d:	118800        	slli	a8, a8, 16
4203a470:	01bd80        	slli	a11, a13, 24
4203a473:	ccaa      	add.n	a12, a12, a10
4203a475:	419890        	srli	a9, a9, 8
4203a478:	2088b0        	or	a8, a8, a11
4203a47b:	75c0c0        	extui	a12, a12, 16, 8
4203a47e:	749890        	extui	a9, a9, 8, 8
4203a481:	2088c0        	or	a8, a8, a12
4203a484:	119980        	slli	a9, a9, 8
4203a487:	206890        	or	a6, a8, a9
4203a48a:	fead86        	j	42039f44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
4203a48d:	c0eae0        	sub	a14, a10, a14
4203a490:	40fe80        	nsau	a8, a14
4203a493:	418580        	srli	a8, a8, 5
4203a496:	cdae16        	beqz	a14, 4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a499:	c09a90        	sub	a9, a10, a9
4203a49c:	1a0c      	movi.n	a10, 1
4203a49e:	838a90        	moveqz	a8, a10, a9
4203a4a1:	a38816        	beqz	a8, 42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a4a4:	ff3306        	j	4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a4a7:	9a9000        	trunc.s	a9, f0, 0
4203a4aa:	c0          	.byte	0xc0
4203a4ab:	40f9b0        	nsau	a11, a9
4203a4ae:	41b5b0        	srli	a11, a11, 5
4203a4b1:	cbf916        	beqz	a9, 4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a4b4:	c08a80        	sub	a8, a10, a8
4203a4b7:	190c      	movi.n	a9, 1
4203a4b9:	939b80        	movnez	a9, a11, a8
4203a4bc:	a1d916        	beqz	a9, 42039edd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a4bf:	ff2c46        	j	4203a174 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>


/tmp/roo-shadow-review/codegen/o3_core/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420394a4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
420394a4:	010136        	entry	a1, 128
420394a7:	078d      	mov.n	a8, a7
420394a9:	06ad      	mov.n	a10, a6
420394ab:	142292        	l32i	a9, a2, 80
420394ae:	086d      	mov.n	a6, a8
420394b0:	23b380        	sext	a11, a3, 15
420394b3:	23c580        	sext	a12, a5, 15
420394b6:	238a80        	sext	a8, a10, 15
420394b9:	d1b9      	s32i.n	a11, a1, 52
420394bb:	81c9      	s32i.n	a12, a1, 32
420394bd:	e189      	s32i.n	a8, a1, 56
420394bf:	027d      	mov.n	a7, a2
420394c1:	232480        	sext	a2, a4, 15
420394c4:	67d916        	beqz	a9, 42039b45 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6a1>
420394c7:	02a827        	bge	a8, a2, 420394cd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x29>
420394ca:	01f406        	j	42039c9e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
420394cd:	3cc782        	addi	a8, a7, 60
420394d0:	a189      	s32i.n	a8, a1, 40
420394d2:	02acb7        	bge	a12, a11, 420394d8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x34>
420394d5:	01f146        	j	42039c9e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
420394d8:	f48020        	extui	a8, a2, 0, 16
420394db:	7169      	s32i.n	a6, a1, 28
420394dd:	d168      	l32i.n	a6, a1, 52
420394df:	b189      	s32i.n	a8, a1, 44
420394e1:	200792        	l8ui	a9, a7, 32
420394e4:	57a8      	l32i.n	a10, a7, 20
420394e6:	021782        	l16ui	a8, a7, 4
420394e9:	0417e2        	l16ui	a14, a7, 8
420394ec:	898a      	add.n	a8, a9, a8
420394ee:	61a9      	s32i.n	a10, a1, 24
420394f0:	f4a060        	extui	a10, a6, 0, 16
420394f3:	c0be90        	sub	a11, a14, a9
420394f6:	23d880        	sext	a13, a8, 15
420394f9:	51a9      	s32i.n	a10, a1, 20
420394fb:	f48080        	extui	a8, a8, 0, 16
420394fe:	23bb80        	sext	a11, a11, 15
42039501:	02a6d7        	bge	a6, a13, 42039507 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x63>
42039504:	00ee86        	j	420398c2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039507:	02ab67        	bge	a11, a6, 4203950d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x69>
4203950a:	00ed06        	j	420398c2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
4203950d:	0317a2        	l16ui	a10, a7, 6
42039510:	051742        	l16ui	a4, a7, 10
42039513:	a9aa      	add.n	a10, a9, a10
42039515:	c0c490        	sub	a12, a4, a9
42039518:	23fa80        	sext	a15, a10, 15
4203951b:	23cc80        	sext	a12, a12, 15
4203951e:	f4a0a0        	extui	a10, a10, 0, 16
42039521:	02a2f7        	bge	a2, a15, 42039527 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x83>
42039524:	00e686        	j	420398c2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039527:	723c20        	salt	a3, a12, a2
4203952a:	c139      	s32i.n	a3, a1, 48
4203952c:	02ac27        	bge	a12, a2, 42039532 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8e>
4203952f:	00e3c6        	j	420398c2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039532:	210732        	l8ui	a3, a7, 33
42039535:	1c0752        	l8ui	a5, a7, 28
42039538:	9139      	s32i.n	a3, a1, 36
4203953a:	327516        	beqz	a5, 42039865 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3c1>
4203953d:	3d5a      	add.n	a3, a13, a5
4203953f:	02a367        	bge	a3, a6, 42039545 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xa1>
42039542:	00c7c6        	j	42039865 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3c1>
42039545:	3f5a      	add.n	a3, a15, a5
42039547:	02a327        	bge	a3, a2, 4203954d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xa9>
4203954a:	00c5c6        	j	42039865 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3c1>
4203954d:	985a      	add.n	a9, a8, a5
4203954f:	f48060        	extui	a8, a6, 0, 16
42039552:	c09980        	sub	a9, a9, a8
42039555:	b188      	l32i.n	a8, a1, 44
42039557:	aa5a      	add.n	a10, a10, a5
42039559:	c08a80        	sub	a8, a10, a8
4203955c:	f49090        	extui	a9, a9, 0, 16
4203955f:	f48080        	extui	a8, a8, 0, 16
42039562:	20a980        	or	a10, a9, a8
42039565:	23aa80        	sext	a10, a10, 15
42039568:	4f3ad6        	bgez	a10, 42039a5f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5bb>
4203956b:	c138      	l32i.n	a3, a1, 48
4203956d:	c09590        	sub	a9, a5, a9
42039570:	1a0c      	movi.n	a10, 1
42039572:	833a90        	moveqz	a3, a10, a9
42039575:	333356        	bnez	a3, 420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039578:	c08580        	sub	a8, a5, a8
4203957b:	93a380        	movnez	a10, a3, a8
4203957e:	32aa56        	bnez	a10, 420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039581:	142782        	l32i	a8, a7, 80
42039584:	6158      	l32i.n	a5, a1, 24
42039586:	2c0816        	beqz	a8, 4203984a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
42039589:	ffa042        	movi	a4, 255
4203958c:	a1b8      	l32i.n	a11, a1, 40
4203958e:	02dd      	mov.n	a13, a2
42039590:	06cd      	mov.n	a12, a6
42039592:	28c7a2        	addi	a10, a7, 40
42039595:	01c9e5        	call8	4203b234 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42039598:	200782        	l8ui	a8, a7, 32
4203959b:	0a3d      	mov.n	a3, a10
4203959d:	015a80        	slli	a5, a10, 24
420395a0:	04ed      	mov.n	a14, a4
420395a2:	0b5816        	beqz	a8, 4203965b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420395a5:	67e8      	l32i.n	a14, a7, 24
420395a7:	0597d2        	l16si	a13, a7, 10
420395aa:	0497c2        	l16si	a12, a7, 8
420395ad:	0397b2        	l16si	a11, a7, 6
420395b0:	0297a2        	l16si	a10, a7, 4
420395b3:	ff0c      	movi.n	a15, 15
420395b5:	1129      	s32i.n	a2, a1, 4
420395b7:	0169      	s32i.n	a6, a1, 0
420395b9:	ff1c25        	call8	4203877c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
420395bc:	0aed      	mov.n	a14, a10
420395be:	099a16        	beqz	a10, 4203965b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420395c1:	97b8      	l32i.n	a11, a7, 36
420395c3:	018a80        	slli	a8, a10, 24
420395c6:	11bb80        	slli	a11, a11, 8
420395c9:	41b8b0        	srli	a11, a11, 8
420395cc:	759880        	extui	a9, a8, 24, 8
420395cf:	ffa0a2        	movi	a10, 255
420395d2:	20bb80        	or	a11, a11, a8
420395d5:	0299a7        	bne	a9, a10, 420395db <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x137>
420395d8:	010606        	j	420399f4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x550>
420395db:	0293a7        	bne	a3, a10, 420395e1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x13d>
420395de:	011386        	j	42039a30 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x58c>
420395e1:	0b5d      	mov.n	a5, a11
420395e3:	074316        	beqz	a3, 4203965b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420395e6:	828930        	mull	a8, a9, a3
420395e9:	80a0c2        	movi	a12, 128
420395ec:	88ca      	add.n	a8, a8, a12
420395ee:	f48080        	extui	a8, a8, 0, 16
420395f1:	11a880        	slli	a10, a8, 8
420395f4:	aa8a      	add.n	a10, a10, a8
420395f6:	31a0a0        	srai	a10, a10, 16
420395f9:	893a      	add.n	a8, a9, a3
420395fb:	c088a0        	sub	a8, a8, a10
420395fe:	11a980        	slli	a10, a9, 8
42039601:	f48080        	extui	a8, a8, 0, 16
42039604:	c09a90        	sub	a9, a10, a9
42039607:	d29980        	quos	a9, a9, a8
4203960a:	f49090        	extui	a9, a9, 0, 16
4203960d:	75b0b0        	extui	a11, a11, 16, 8
42039610:	c1bb90        	mul16u	a11, a11, a9
42039613:	74a850        	extui	a10, a5, 8, 8
42039616:	c1aa90        	mul16u	a10, a10, a9
42039619:	745050        	extui	a5, a5, 0, 8
4203961c:	c15590        	mul16u	a5, a5, a9
4203961f:	bbca      	add.n	a11, a11, a12
42039621:	f4b0b0        	extui	a11, a11, 0, 16
42039624:	aaca      	add.n	a10, a10, a12
42039626:	f4a0a0        	extui	a10, a10, 0, 16
42039629:	119b80        	slli	a9, a11, 8
4203962c:	55ca      	add.n	a5, a5, a12
4203962e:	f45050        	extui	a5, a5, 0, 16
42039631:	99ba      	add.n	a9, a9, a11
42039633:	11ba80        	slli	a11, a10, 8
42039636:	11c580        	slli	a12, a5, 8
42039639:	bbaa      	add.n	a11, a11, a10
4203963b:	759090        	extui	a9, a9, 16, 8
4203963e:	01a880        	slli	a10, a8, 24
42039641:	cc5a      	add.n	a12, a12, a5
42039643:	119900        	slli	a9, a9, 16
42039646:	41b8b0        	srli	a11, a11, 8
42039649:	2099a0        	or	a9, a9, a10
4203964c:	75c0c0        	extui	a12, a12, 16, 8
4203964f:	7488b0        	extui	a8, a11, 8, 8
42039652:	2099c0        	or	a9, a9, a12
42039655:	118880        	slli	a8, a8, 8
42039658:	205980        	or	a5, a9, a8
4203965b:	ffa082        	movi	a8, 255
4203965e:	029487        	bne	a4, a8, 42039664 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
42039661:	00bec6        	j	42039960 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4bc>
42039664:	0cb416        	beqz	a4, 42039733 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
42039667:	61b8      	l32i.n	a11, a1, 24
42039669:	1caaa1        	l32r	a10, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203966c:	7598b0        	extui	a9, a11, 24, 8
4203966f:	d19940        	mul16s	a9, a9, a4
42039672:	11bb80        	slli	a11, a11, 8
42039675:	7fc992        	addi	a9, a9, 127
42039678:	a299a0        	muluh	a9, a9, a10
4203967b:	0199f0        	slli	a9, a9, 17
4203967e:	75a890        	extui	a10, a9, 24, 8
42039681:	0a9d      	mov.n	a9, a10
42039683:	404800        	ssai	8
42039686:	81bab0        	src	a11, a10, a11
42039689:	029a87        	bne	a10, a8, 4203968f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1eb>
4203968c:	00d406        	j	420399e0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x53c>
4203968f:	75a850        	extui	a10, a5, 24, 8
42039692:	029a87        	bne	a10, a8, 42039698 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f4>
42039695:	00d946        	j	420399fe <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x55a>
42039698:	097916        	beqz	a9, 42039733 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
4203969b:	341a16        	beqz	a10, 420399e0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x53c>
4203969e:	82c9a0        	mull	a12, a9, a10
420396a1:	80a0f2        	movi	a15, 128
420396a4:	ccfa      	add.n	a12, a12, a15
420396a6:	f4c0c0        	extui	a12, a12, 0, 16
420396a9:	11dc80        	slli	a13, a12, 8
420396ac:	cdca      	add.n	a12, a13, a12
420396ae:	a9aa      	add.n	a10, a9, a10
420396b0:	31c0c0        	srai	a12, a12, 16
420396b3:	c0cac0        	sub	a12, a10, a12
420396b6:	11a980        	slli	a10, a9, 8
420396b9:	f440c0        	extui	a4, a12, 0, 16
420396bc:	c0ca90        	sub	a12, a10, a9
420396bf:	d2cc40        	quos	a12, a12, a4
420396c2:	f4c0c0        	extui	a12, a12, 0, 16
420396c5:	c088c0        	sub	a8, a8, a12
420396c8:	75a0b0        	extui	a10, a11, 16, 8
420396cb:	f48080        	extui	a8, a8, 0, 16
420396ce:	c1aac0        	mul16u	a10, a10, a12
420396d1:	753050        	extui	a3, a5, 16, 8
420396d4:	7498b0        	extui	a9, a11, 8, 8
420396d7:	c13380        	mul16u	a3, a3, a8
420396da:	c199c0        	mul16u	a9, a9, a12
420396dd:	74d850        	extui	a13, a5, 8, 8
420396e0:	74b0b0        	extui	a11, a11, 0, 8
420396e3:	c1bbc0        	mul16u	a11, a11, a12
420396e6:	c1dd80        	mul16u	a13, a13, a8
420396e9:	745050        	extui	a5, a5, 0, 8
420396ec:	aafa      	add.n	a10, a10, a15
420396ee:	c15580        	mul16u	a5, a5, a8
420396f1:	aa3a      	add.n	a10, a10, a3
420396f3:	99fa      	add.n	a9, a9, a15
420396f5:	f4a0a0        	extui	a10, a10, 0, 16
420396f8:	bbfa      	add.n	a11, a11, a15
420396fa:	99da      	add.n	a9, a9, a13
420396fc:	f49090        	extui	a9, a9, 0, 16
420396ff:	118a80        	slli	a8, a10, 8
42039702:	bb5a      	add.n	a11, a11, a5
42039704:	f4b0b0        	extui	a11, a11, 0, 16
42039707:	88aa      	add.n	a8, a8, a10
42039709:	11a980        	slli	a10, a9, 8
4203970c:	80aa90        	add	a10, a10, a9
4203970f:	758080        	extui	a8, a8, 16, 8
42039712:	11cb80        	slli	a12, a11, 8
42039715:	118800        	slli	a8, a8, 16
42039718:	019480        	slli	a9, a4, 24
4203971b:	80ccb0        	add	a12, a12, a11
4203971e:	4158a0        	srli	a5, a10, 8
42039721:	208890        	or	a8, a8, a9
42039724:	75c0c0        	extui	a12, a12, 16, 8
42039727:	745850        	extui	a5, a5, 8, 8
4203972a:	2088c0        	or	a8, a8, a12
4203972d:	115580        	slli	a5, a5, 8
42039730:	205850        	or	a5, a8, a5
42039733:	1427a2        	l32i	a10, a7, 80
42039736:	110a16        	beqz	a10, 4203984a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
42039739:	180a82        	l8ui	a8, a10, 24
4203973c:	290856        	bnez	a8, 420399d0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x52c>
4203973f:	b1b8      	l32i.n	a11, a1, 44
42039741:	031a82        	l16ui	a8, a10, 6
42039744:	021a92        	l16ui	a9, a10, 4
42039747:	c08b80        	sub	a8, a11, a8
4203974a:	51b8      	l32i.n	a11, a1, 20
4203974c:	740884        	mul.aa.ll	a8, a8
4203974f:	c09b90        	sub	a9, a11, a9
42039752:	780994        	mula.aa.ll	a9, a9
42039755:	3a88      	l32i.n	a8, a10, 12
42039757:	031090        	rsr.acclo	a9
4203975a:	c08890        	sub	a8, a8, a9
4203975d:	0218e6        	bgei	a8, 1, 42039763 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2bf>
42039760:	00a046        	j	420399e5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x541>
42039763:	049a92        	l16si	a9, a10, 8
42039766:	0219e6        	bgei	a9, 1, 4203976c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c8>
42039769:	00a306        	j	420399f9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
4203976c:	b99a      	add.n	a11, a9, a9
4203976e:	0228b7        	blt	a8, a11, 42039774 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2d0>
42039771:	00a6c6        	j	42039a10 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x56c>
42039774:	4ab8      	l32i.n	a11, a10, 16
42039776:	5aa8      	l32i.n	a10, a10, 20
42039778:	60a0c2        	movi	a12, 96
4203977b:	61e9      	s32i.n	a14, a1, 24
4203977d:	4d78e5        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039780:	61e8      	l32i.n	a14, a1, 24
42039782:	75c8a0        	extui	a12, a10, 24, 8
42039785:	1c6381        	l32r	a8, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
42039788:	d1cce0        	mul16s	a12, a12, a14
4203978b:	11aa80        	slli	a10, a10, 8
4203978e:	a2cc80        	muluh	a12, a12, a8
42039791:	ffa082        	movi	a8, 255
42039794:	01ccf0        	slli	a12, a12, 17
42039797:	7598c0        	extui	a9, a12, 24, 8
4203979a:	09cd      	mov.n	a12, a9
4203979c:	404800        	ssai	8
4203979f:	81b9a0        	src	a11, a9, a10
420397a2:	029987        	bne	a9, a8, 420397a8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x304>
420397a5:	007f06        	j	420399a5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x501>
420397a8:	759850        	extui	a9, a5, 24, 8
420397ab:	029987        	bne	a9, a8, 420397b1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x30d>
420397ae:	008e06        	j	420399ea <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x546>
420397b1:	095c16        	beqz	a12, 4203984a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
420397b4:	1ed916        	beqz	a9, 420399a5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x501>
420397b7:	82ac90        	mull	a10, a12, a9
420397ba:	80a0e2        	movi	a14, 128
420397bd:	aaea      	add.n	a10, a10, a14
420397bf:	f4a0a0        	extui	a10, a10, 0, 16
420397c2:	11da80        	slli	a13, a10, 8
420397c5:	adaa      	add.n	a10, a13, a10
420397c7:	9c9a      	add.n	a9, a12, a9
420397c9:	31a0a0        	srai	a10, a10, 16
420397cc:	c099a0        	sub	a9, a9, a10
420397cf:	11dc80        	slli	a13, a12, 8
420397d2:	f49090        	extui	a9, a9, 0, 16
420397d5:	c0ddc0        	sub	a13, a13, a12
420397d8:	d2dd90        	quos	a13, a13, a9
420397db:	f4d0d0        	extui	a13, a13, 0, 16
420397de:	75c0b0        	extui	a12, a11, 16, 8
420397e1:	c088d0        	sub	a8, a8, a13
420397e4:	f48080        	extui	a8, a8, 0, 16
420397e7:	756050        	extui	a6, a5, 16, 8
420397ea:	74a8b0        	extui	a10, a11, 8, 8
420397ed:	c1ccd0        	mul16u	a12, a12, a13
420397f0:	c16680        	mul16u	a6, a6, a8
420397f3:	74f850        	extui	a15, a5, 8, 8
420397f6:	c1aad0        	mul16u	a10, a10, a13
420397f9:	74b0b0        	extui	a11, a11, 0, 8
420397fc:	c1ff80        	mul16u	a15, a15, a8
420397ff:	c1bbd0        	mul16u	a11, a11, a13
42039802:	745050        	extui	a5, a5, 0, 8
42039805:	ccea      	add.n	a12, a12, a14
42039807:	c15580        	mul16u	a5, a5, a8
4203980a:	cc6a      	add.n	a12, a12, a6
4203980c:	aaea      	add.n	a10, a10, a14
4203980e:	f4c0c0        	extui	a12, a12, 0, 16
42039811:	bbea      	add.n	a11, a11, a14
42039813:	aafa      	add.n	a10, a10, a15
42039815:	f4a0a0        	extui	a10, a10, 0, 16
42039818:	118c80        	slli	a8, a12, 8
4203981b:	bb5a      	add.n	a11, a11, a5
4203981d:	f4b0b0        	extui	a11, a11, 0, 16
42039820:	88ca      	add.n	a8, a8, a12
42039822:	115a80        	slli	a5, a10, 8
42039825:	55aa      	add.n	a5, a5, a10
42039827:	758080        	extui	a8, a8, 16, 8
4203982a:	11cb80        	slli	a12, a11, 8
4203982d:	01a980        	slli	a10, a9, 24
42039830:	118800        	slli	a8, a8, 16
42039833:	9cba      	add.n	a9, a12, a11
42039835:	415850        	srli	a5, a5, 8
42039838:	2088a0        	or	a8, a8, a10
4203983b:	759090        	extui	a9, a9, 16, 8
4203983e:	745850        	extui	a5, a5, 8, 8
42039841:	208890        	or	a8, a8, a9
42039844:	115580        	slli	a5, a5, 8
42039847:	205850        	or	a5, a8, a5
4203984a:	5188      	l32i.n	a8, a1, 20
4203984c:	7198      	l32i.n	a9, a1, 28
4203984e:	881b      	addi.n	a8, a8, 1
42039850:	0959      	s32i.n	a5, a9, 0
42039852:	236880        	sext	a6, a8, 15
42039855:	894b      	addi.n	a8, a9, 4
42039857:	082192        	l32i	a9, a1, 32
4203985a:	02a967        	bge	a9, a6, 42039860 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3bc>
4203985d:	0056c6        	j	420399bc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x518>
42039860:	7189      	s32i.n	a8, a1, 28
42039862:	ff1ec6        	j	420394e1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3d>
42039865:	1d0752        	l8ui	a5, a7, 29
42039868:	05c516        	beqz	a5, 420398c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x424>
4203986b:	c03b50        	sub	a3, a11, a5
4203986e:	562637        	blt	a6, a3, 420398c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x424>
42039871:	3f5a      	add.n	a3, a15, a5
42039873:	512327        	blt	a3, a2, 420398c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x424>
42039876:	895a      	add.n	a8, a9, a5
42039878:	c088e0        	sub	a8, a8, a14
4203987b:	f49060        	extui	a9, a6, 0, 16
4203987e:	898a      	add.n	a8, a9, a8
42039880:	b198      	l32i.n	a9, a1, 44
42039882:	aa5a      	add.n	a10, a10, a5
42039884:	c0aa90        	sub	a10, a10, a9
42039887:	f48080        	extui	a8, a8, 0, 16
4203988a:	f4a0a0        	extui	a10, a10, 0, 16
4203988d:	2098a0        	or	a9, a8, a10
42039890:	239980        	sext	a9, a9, 15
42039893:	2459d6        	bgez	a9, 42039adc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x638>
42039896:	c08580        	sub	a8, a5, a8
42039899:	40f890        	nsau	a9, a8
4203989c:	419590        	srli	a9, a9, 5
4203989f:	988c      	beqz.n	a8, 420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
420398a1:	c0a5a0        	sub	a10, a5, a10
420398a4:	180c      	movi.n	a8, 1
420398a6:	9389a0        	movnez	a8, a9, a10
420398a9:	cd4816        	beqz	a8, 42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
420398ac:	9198      	l32i.n	a9, a1, 36
420398ae:	1189c0        	slli	a8, a9, 4
420398b1:	898a      	add.n	a8, a9, a8
420398b3:	744080        	extui	a4, a8, 0, 8
420398b6:	ffa082        	movi	a8, 255
420398b9:	029487        	bne	a4, a8, 420398bf <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41b>
420398bc:	ff3046        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
420398bf:	ff3246        	j	4203958c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe8>
420398c2:	040c      	movi.n	a4, 0
420398c4:	ff3106        	j	4203958c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe8>
420398c7:	07a200        	ee.stf.64.xp	f10, f0, a0, a2
420398ca:	045a161e 	ee.vmulas.s16.accx.ld.ip.qup	q1, a1, 0x160, q0, q2, q5, q2
420398ce:	c05ba0        	sub	a5, a11, a10
420398d1:	3f2657        	blt	a6, a5, 42039914 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x470>
420398d4:	c05ca0        	sub	a5, a12, a10
420398d7:	392257        	blt	a2, a5, 42039914 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x470>
420398da:	99aa      	add.n	a9, a9, a10
420398dc:	c0e9e0        	sub	a14, a9, a14
420398df:	f48060        	extui	a8, a6, 0, 16
420398e2:	e8ea      	add.n	a14, a8, a14
420398e4:	b188      	l32i.n	a8, a1, 44
420398e6:	c09940        	sub	a9, a9, a4
420398e9:	989a      	add.n	a9, a8, a9
420398eb:	f4e0e0        	extui	a14, a14, 0, 16
420398ee:	f49090        	extui	a9, a9, 0, 16
420398f1:	208e90        	or	a8, a14, a9
420398f4:	238880        	sext	a8, a8, 15
420398f7:	1ea8d6        	bgez	a8, 42039ae5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x641>
420398fa:	c0eae0        	sub	a14, a10, a14
420398fd:	40fe80        	nsau	a8, a14
42039900:	418580        	srli	a8, a8, 5
42039903:	fa5e16        	beqz	a14, 420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039906:	c09a90        	sub	a9, a10, a9
42039909:	1a0c      	movi.n	a10, 1
4203990b:	838a90        	moveqz	a8, a10, a9
4203990e:	c6f816        	beqz	a8, 42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039911:	ffe5c6        	j	420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039914:	1f07a2        	l8ui	a10, a7, 31
42039917:	04da16        	beqz	a10, 42039968 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c4>
4203991a:	edaa      	add.n	a14, a13, a10
4203991c:	482e67        	blt	a14, a6, 42039968 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c4>
4203991f:	c0eca0        	sub	a14, a12, a10
42039922:	4222e7        	blt	a2, a14, 42039968 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c4>
42039925:	99aa      	add.n	a9, a9, a10
42039927:	b8aa      	add.n	a11, a8, a10
42039929:	c08940        	sub	a8, a9, a4
4203992c:	f49060        	extui	a9, a6, 0, 16
4203992f:	c09b90        	sub	a9, a11, a9
42039932:	b1b8      	l32i.n	a11, a1, 44
42039934:	f49090        	extui	a9, a9, 0, 16
42039937:	8b8a      	add.n	a8, a11, a8
42039939:	f48080        	extui	a8, a8, 0, 16
4203993c:	20b980        	or	a11, a9, a8
4203993f:	23bb80        	sext	a11, a11, 15
42039942:	35ebd6        	bgez	a11, 42039ca4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x800>
42039945:	c09a90        	sub	a9, a10, a9
42039948:	40f9b0        	nsau	a11, a9
4203994b:	41b5b0        	srli	a11, a11, 5
4203994e:	f5a916        	beqz	a9, 420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039951:	c08a80        	sub	a8, a10, a8
42039954:	190c      	movi.n	a9, 1
42039956:	939b80        	movnez	a9, a11, a8
42039959:	c24916        	beqz	a9, 42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
4203995c:	ffd306        	j	420398ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
4203995f:	00          	.byte	00
42039960:	1bec51        	l32r	a5, 42000910 <_stext+0x8f0> (ff000000 <_rtc_reserved_end+0x9ef00000>)
42039963:	6188      	l32i.n	a8, a1, 24
42039965:	ff71c6        	j	42039730 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28c>
42039968:	9198      	l32i.n	a9, a1, 36
4203996a:	e80c      	movi.n	a8, 14
4203996c:	02b897        	bgeu	a8, a9, 42039972 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4ce>
4203996f:	ff0386        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039972:	c0d6d0        	sub	a13, a6, a13
42039975:	40fd80        	nsau	a8, a13
42039978:	190c      	movi.n	a9, 1
4203997a:	418580        	srli	a8, a8, 5
4203997d:	7d9c      	beqz.n	a13, 42039998 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4f4>
4203997f:	c0b6b0        	sub	a11, a6, a11
42039982:	8389b0        	moveqz	a8, a9, a11
42039985:	f8cc      	bnez.n	a8, 42039998 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4f4>
42039987:	c0f2f0        	sub	a15, a2, a15
4203998a:	8389f0        	moveqz	a8, a9, a15
4203998d:	78cc      	bnez.n	a8, 42039998 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4f4>
4203998f:	c0c2c0        	sub	a12, a2, a12
42039992:	9398c0        	movnez	a9, a8, a12
42039995:	be8916        	beqz	a9, 42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039998:	9198      	l32i.n	a9, a1, 36
4203999a:	1189c0        	slli	a8, a9, 4
4203999d:	498a      	add.n	a4, a9, a8
4203999f:	744040        	extui	a4, a4, 0, 8
420399a2:	fef986        	j	4203958c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe8>
420399a5:	5188      	l32i.n	a8, a1, 20
420399a7:	7198      	l32i.n	a9, a1, 28
420399a9:	881b      	addi.n	a8, a8, 1
420399ab:	0b5d      	mov.n	a5, a11
420399ad:	0959      	s32i.n	a5, a9, 0
420399af:	236880        	sext	a6, a8, 15
420399b2:	894b      	addi.n	a8, a9, 4
420399b4:	8198      	l32i.n	a9, a1, 32
420399b6:	022967        	blt	a9, a6, 420399bc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x518>
420399b9:	ffa8c6        	j	42039860 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3bc>
420399bc:	b198      	l32i.n	a9, a1, 44
420399be:	991b      	addi.n	a9, a9, 1
420399c0:	232980        	sext	a2, a9, 15
420399c3:	e198      	l32i.n	a9, a1, 56
420399c5:	02a927        	bge	a9, a2, 420399cb <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x527>
420399c8:	00b486        	j	42039c9e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
420399cb:	086d      	mov.n	a6, a8
420399cd:	fec1c6        	j	420394d8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x34>
420399d0:	20c220        	or	a12, a2, a2
420399d3:	20b660        	or	a11, a6, a6
420399d6:	0661e2        	s32i	a14, a1, 24
420399d9:	ed50a5        	call8	42026ee4 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
420399dc:	ff6806        	j	42039780 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2dc>
420399df:	00          	.byte	00
420399e0:	0b5d      	mov.n	a5, a11
420399e2:	ff5346        	j	42039733 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
420399e5:	5aa8      	l32i.n	a10, a10, 20
420399e7:	ff65c6        	j	42039782 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2de>
420399ea:	05ad      	mov.n	a10, a5
420399ec:	4eb2a5        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
420399ef:	0a5d      	mov.n	a5, a10
420399f1:	ff9546        	j	4203984a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
420399f4:	0b5d      	mov.n	a5, a11
420399f6:	ff1846        	j	4203965b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420399f9:	4aa8      	l32i.n	a10, a10, 16
420399fb:	ff60c6        	j	42039782 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2de>
420399fe:	20a550        	or	a10, a5, a5
42039a01:	0661e2        	s32i	a14, a1, 24
42039a04:	4eb125        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039a07:	0621e2        	l32i	a14, a1, 24
42039a0a:	0a5d      	mov.n	a5, a10
42039a0c:	ff48c6        	j	42039733 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
42039a0f:	00          	.byte	00
42039a10:	e51926        	beqi	a9, 1, 420399f9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039a13:	ffc9c2        	addi	a12, a9, -1
42039a16:	23cc80        	sext	a12, a12, 15
42039a19:	11cce0        	slli	a12, a12, 2
42039a1c:	212c87        	blt	a12, a8, 42039a41 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x59d>
42039a1f:	042ab2        	l32i	a11, a10, 16
42039a22:	052aa2        	l32i	a10, a10, 20
42039a25:	0c4c      	movi.n	a12, 64
42039a27:	61e9      	s32i.n	a14, a1, 24
42039a29:	4d4e25        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039a2c:	ff5406        	j	42039780 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2dc>
42039a2f:	a55000        	extui	a5, a0, 16, 11
42039a32:	20          	.byte	0x20
42039a33:	0961e2        	s32i	a14, a1, 36
42039a36:	4eae25        	call8	42088518 <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039a39:	0921e2        	l32i	a14, a1, 36
42039a3c:	0a5d      	mov.n	a5, a10
42039a3e:	ff0646        	j	4203965b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
42039a41:	b42926        	beqi	a9, 2, 420399f9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039a44:	80bb90        	add	a11, a11, a9
42039a47:	80bbb0        	add	a11, a11, a11
42039a4a:	f8cbb2        	addi	a11, a11, -8
42039a4d:	a8a8b7        	bge	a8, a11, 420399f9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039a50:	042ab2        	l32i	a11, a10, 16
42039a53:	5aa8      	l32i.n	a10, a10, 20
42039a55:	0c2c      	movi.n	a12, 32
42039a57:	61e9      	s32i.n	a14, a1, 24
42039a59:	4d4b25        	call8	42086f0c <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039a5c:	ff4806        	j	42039780 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2dc>
42039a5f:	740884        	mul.aa.ll	a8, a8
42039a62:	780994        	mula.aa.ll	a9, a9
42039a65:	031080        	rsr.acclo	a8
42039a68:	119880        	slli	a9, a8, 8
42039a6b:	0249e6        	bgei	a9, 4, 42039a71 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5cd>
42039a6e:	fec3c6        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039a71:	40f980        	nsau	a8, a9
42039a74:	dfc882        	addi	a8, a8, -33
42039a77:	608080        	neg	a8, a8
42039a7a:	218180        	srai	a8, a8, 1
42039a7d:	3a0c      	movi.n	a10, 3
42039a7f:	fec882        	addi	a8, a8, -2
42039a82:	401800        	ssl	a8
42039a85:	a18a00        	sll	a8, a10
42039a88:	c2a980        	quou	a10, a9, a8
42039a8b:	aa8a      	add.n	a10, a10, a8
42039a8d:	41a1a0        	srli	a10, a10, 1
42039a90:	c289a0        	quou	a8, a9, a10
42039a93:	88aa      	add.n	a8, a8, a10
42039a95:	418180        	srli	a8, a8, 1
42039a98:	c2a980        	quou	a10, a9, a8
42039a9b:	aa8a      	add.n	a10, a10, a8
42039a9d:	1b9a81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42039aa0:	41a1a0        	srli	a10, a10, 1
42039aa3:	029a87        	bne	a10, a8, 42039aa9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x605>
42039aa6:	feb5c6        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039aa9:	82baa0        	mull	a11, a10, a10
42039aac:	1185c0        	slli	a8, a5, 4
42039aaf:	6299b0        	saltu	a9, a9, a11
42039ab2:	c0aa90        	sub	a10, a10, a9
42039ab5:	9198      	l32i.n	a9, a1, 36
42039ab7:	898a      	add.n	a8, a9, a8
42039ab9:	c088a0        	sub	a8, a8, a10
42039abc:	f49080        	extui	a9, a8, 0, 16
42039abf:	23a880        	sext	a10, a8, 15
42039ac2:	0268f7        	bbci	a8, 15, 42039ac8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x624>
42039ac5:	ff7e46        	j	420398c2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039ac8:	f80c      	movi.n	a8, 15
42039aca:	02a8a7        	bge	a8, a10, 42039ad0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x62c>
42039acd:	feac06        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039ad0:	1189c0        	slli	a8, a9, 4
42039ad3:	998a      	add.n	a9, a9, a8
42039ad5:	744090        	extui	a4, a9, 0, 8
42039ad8:	ff7686        	j	420398b6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x412>
42039adb:	0aa400        	add.s	f10, f4, f0
42039ade:	74          	.byte	0x74
42039adf:	780884        	mula.aa.ll	a8, a8
42039ae2:	ffdfc6        	j	42039a65 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5c1>
42039ae5:	740994        	mul.aa.ll	a9, a9
42039ae8:	780ee4        	mula.aa.ll	a14, a14
42039aeb:	031080        	rsr.acclo	a8
42039aee:	119880        	slli	a9, a8, 8
42039af1:	0249e6        	bgei	a9, 4, 42039af7 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x653>
42039af4:	fea246        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039af7:	40f980        	nsau	a8, a9
42039afa:	dfc882        	addi	a8, a8, -33
42039afd:	608080        	neg	a8, a8
42039b00:	218180        	srai	a8, a8, 1
42039b03:	3b0c      	movi.n	a11, 3
42039b05:	fec882        	addi	a8, a8, -2
42039b08:	401800        	ssl	a8
42039b0b:	a18b00        	sll	a8, a11
42039b0e:	c2b980        	quou	a11, a9, a8
42039b11:	bb8a      	add.n	a11, a11, a8
42039b13:	41b1b0        	srli	a11, a11, 1
42039b16:	c289b0        	quou	a8, a9, a11
42039b19:	88ba      	add.n	a8, a8, a11
42039b1b:	418180        	srli	a8, a8, 1
42039b1e:	c2b980        	quou	a11, a9, a8
42039b21:	bb8a      	add.n	a11, a11, a8
42039b23:	1b7981        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42039b26:	41b1b0        	srli	a11, a11, 1
42039b29:	029b87        	bne	a11, a8, 42039b2f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x68b>
42039b2c:	fe9446        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039b2f:	82cbb0        	mull	a12, a11, a11
42039b32:	118ac0        	slli	a8, a10, 4
42039b35:	6299c0        	saltu	a9, a9, a12
42039b38:	c0bb90        	sub	a11, a11, a9
42039b3b:	9198      	l32i.n	a9, a1, 36
42039b3d:	898a      	add.n	a8, a9, a8
42039b3f:	c088b0        	sub	a8, a8, a11
42039b42:	ffdd86        	j	42039abc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x618>
42039b45:	08ed      	mov.n	a14, a8
42039b47:	0788      	l32i.n	a8, a7, 0
42039b49:	6129      	s32i.n	a2, a1, 24
42039b4b:	b888      	l32i.n	a8, a8, 44
42039b4d:	0cdd      	mov.n	a13, a12
42039b4f:	0c3d      	mov.n	a3, a12
42039b51:	06fd      	mov.n	a15, a6
42039b53:	02cd      	mov.n	a12, a2
42039b55:	07ad      	mov.n	a10, a7
42039b57:	0b5d      	mov.n	a5, a11
42039b59:	0008e0        	callx8	a8
42039b5c:	024d      	mov.n	a4, a2
42039b5e:	0a2d      	mov.n	a2, a10
42039b60:	13ca56        	bnez	a10, 42039ca0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fc>
42039b63:	249782        	l16si	a8, a7, 72
42039b66:	1a9792        	l16si	a9, a7, 52
42039b69:	2107a2        	l8ui	a10, a7, 33
42039b6c:	539980        	max	a9, a9, a8
42039b6f:	1907f2        	l8ui	a15, a7, 25
42039b72:	180782        	l8ui	a8, a7, 24
42039b75:	eb0c      	movi.n	a11, 14
42039b77:	c9fb      	addi.n	a12, a9, 15
42039b79:	1a07e2        	l8ui	a14, a7, 26
42039b7c:	62bba0        	saltu	a11, a11, a10
42039b7f:	1ec992        	addi	a9, a9, 30
42039b82:	2007a2        	l8ui	a10, a7, 32
42039b85:	1b07d2        	l8ui	a13, a7, 27
42039b88:	b39cc0        	movgez	a9, a12, a12
42039b8b:	7388f0        	maxu	a8, a8, a15
42039b8e:	bb0b      	addi.n	a11, a11, -1
42039b90:	60b0b0        	neg	a11, a11
42039b93:	219490        	srai	a9, a9, 4
42039b96:	7388e0        	maxu	a8, a8, a14
42039b99:	caba      	add.n	a12, a10, a11
42039b9b:	7388d0        	maxu	a8, a8, a13
42039b9e:	a9aa      	add.n	a10, a9, a10
42039ba0:	5388c0        	max	a8, a8, a12
42039ba3:	0297f2        	l16si	a15, a7, 4
42039ba6:	aaba      	add.n	a10, a10, a11
42039ba8:	53aa80        	max	a10, a10, a8
42039bab:	bfaa      	add.n	a11, a15, a10
42039bad:	0397e2        	l16si	a14, a7, 6
42039bb0:	0497d2        	l16si	a13, a7, 8
42039bb3:	1261b2        	s32i	a11, a1, 72
42039bb6:	c0dda0        	sub	a13, a13, a10
42039bb9:	be8a      	add.n	a11, a14, a8
42039bbb:	03fd      	mov.n	a15, a3
42039bbd:	c0a350        	sub	a10, a3, a5
42039bc0:	0597c2        	l16si	a12, a7, 10
42039bc3:	122132        	l32i	a3, a1, 72
42039bc6:	43ddf0        	min	a13, a13, a15
42039bc9:	bb9a      	add.n	a11, a11, a9
42039bcb:	53e530        	max	a14, a5, a3
42039bce:	b1d9      	s32i.n	a13, a1, 44
42039bd0:	f1b9      	s32i.n	a11, a1, 60
42039bd2:	c08c80        	sub	a8, a12, a8
42039bd5:	ba1b      	addi.n	a11, a10, 1
42039bd7:	e1d8      	l32i.n	a13, a1, 56
42039bd9:	71e9      	s32i.n	a14, a1, 28
42039bdb:	106182        	s32i	a8, a1, 64
42039bde:	1761b2        	s32i	a11, a1, 92
42039be1:	02ad47        	bge	a13, a4, 42039be7 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x743>
42039be4:	002d86        	j	42039c9e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
42039be7:	b198      	l32i.n	a9, a1, 44
42039be9:	c08e50        	sub	a8, a14, a5
42039bec:	72a530        	salt	a10, a5, a3
42039bef:	880b      	addi.n	a8, a8, -1
42039bf1:	0ebd      	mov.n	a11, a14
42039bf3:	c51b      	addi.n	a12, a5, 1
42039bf5:	e91b      	addi.n	a14, a9, 1
42039bf7:	1198e0        	slli	a9, a8, 2
42039bfa:	8382a0        	moveqz	a8, a2, a10
42039bfd:	c0beb0        	sub	a11, a14, a11
42039c00:	8392a0        	moveqz	a9, a2, a10
42039c03:	88ca      	add.n	a8, a8, a12
42039c05:	1021a2        	l32i	a10, a1, 64
42039c08:	11cbe0        	slli	a12, a11, 2
42039c0b:	238880        	sext	a8, a8, 15
42039c0e:	1721b2        	l32i	a11, a1, 92
42039c11:	43ada0        	min	a10, a13, a10
42039c14:	156182        	s32i	a8, a1, 84
42039c17:	894b      	addi.n	a8, a9, 4
42039c19:	11bbe0        	slli	a11, a11, 2
42039c1c:	166182        	s32i	a8, a1, 88
42039c1f:	8a0b      	addi.n	a8, a10, -1
42039c21:	1461e2        	s32i	a14, a1, 80
42039c24:	a1a9      	s32i.n	a10, a1, 40
42039c26:	1361c2        	s32i	a12, a1, 76
42039c29:	c1b9      	s32i.n	a11, a1, 48
42039c2b:	4f1b      	addi.n	a4, a15, 1
42039c2d:	116182        	s32i	a8, a1, 68
42039c30:	d188      	l32i.n	a8, a1, 52
42039c32:	7198      	l32i.n	a9, a1, 28
42039c34:	9189      	s32i.n	a8, a1, 36
42039c36:	b188      	l32i.n	a8, a1, 44
42039c38:	062d      	mov.n	a2, a6
42039c3a:	022897        	blt	a8, a9, 42039c40 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x79c>
42039c3d:	004906        	j	42039d65 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8c1>
42039c40:	8188      	l32i.n	a8, a1, 32
42039c42:	9138      	l32i.n	a3, a1, 36
42039c44:	262837        	blt	a8, a3, 42039c6e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ca>
42039c47:	0c9182        	l16si	a8, a1, 24
42039c4a:	025d      	mov.n	a5, a2
42039c4c:	5189      	s32i.n	a8, a1, 20
42039c4e:	57d8      	l32i.n	a13, a7, 20
42039c50:	51c8      	l32i.n	a12, a1, 20
42039c52:	03bd      	mov.n	a11, a3
42039c54:	07ad      	mov.n	a10, a7
42039c56:	ff2665        	call8	42038ebc <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039c59:	05a9      	s32i.n	a10, a5, 0
42039c5b:	331b      	addi.n	a3, a3, 1
42039c5d:	554b      	addi.n	a5, a5, 4
42039c5f:	eb9347        	bne	a3, a4, 42039c4e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7aa>
42039c62:	8188      	l32i.n	a8, a1, 32
42039c64:	9198      	l32i.n	a9, a1, 36
42039c66:	c03890        	sub	a3, a8, a9
42039c69:	824b      	addi.n	a8, a2, 4
42039c6b:	a02380        	addx4	a2, a3, a8
42039c6e:	6198      	l32i.n	a9, a1, 24
42039c70:	f188      	l32i.n	a8, a1, 60
42039c72:	728980        	salt	a8, a9, a8
42039c75:	880b      	addi.n	a8, a8, -1
42039c77:	608080        	neg	a8, a8
42039c7a:	748080        	extui	a8, a8, 0, 8
42039c7d:	f88c      	beqz.n	a8, 42039c90 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ec>
42039c7f:	102182        	l32i	a8, a1, 64
42039c82:	728890        	salt	a8, a8, a9
42039c85:	880b      	addi.n	a8, a8, -1
42039c87:	608080        	neg	a8, a8
42039c8a:	748080        	extui	a8, a8, 0, 8
42039c8d:	075856        	bnez	a8, 42039d06 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x862>
42039c90:	026d      	mov.n	a6, a2
42039c92:	6188      	l32i.n	a8, a1, 24
42039c94:	e198      	l32i.n	a9, a1, 56
42039c96:	881b      	addi.n	a8, a8, 1
42039c98:	066182        	s32i	a8, a1, 24
42039c9b:	91a987        	bge	a9, a8, 42039c30 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x78c>
42039c9e:	020c      	movi.n	a2, 0
42039ca0:	f01d      	retw.n
42039ca2:	840000        	extui	a0, a0, 0, 9
42039ca5:	7408      	l32i.n	a0, a4, 28
42039ca7:	780994        	mula.aa.ll	a9, a9
42039caa:	031080        	rsr.acclo	a8
42039cad:	118880        	slli	a8, a8, 8
42039cb0:	0248e6        	bgei	a8, 4, 42039cb6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x812>
42039cb3:	fe3286        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039cb6:	40f890        	nsau	a9, a8
42039cb9:	dfc992        	addi	a9, a9, -33
42039cbc:	609090        	neg	a9, a9
42039cbf:	219190        	srai	a9, a9, 1
42039cc2:	3b0c      	movi.n	a11, 3
42039cc4:	fec992        	addi	a9, a9, -2
42039cc7:	401900        	ssl	a9
42039cca:	a19b00        	sll	a9, a11
42039ccd:	c2b890        	quou	a11, a8, a9
42039cd0:	bb9a      	add.n	a11, a11, a9
42039cd2:	41b1b0        	srli	a11, a11, 1
42039cd5:	c298b0        	quou	a9, a8, a11
42039cd8:	99ba      	add.n	a9, a9, a11
42039cda:	419190        	srli	a9, a9, 1
42039cdd:	c2b890        	quou	a11, a8, a9
42039ce0:	9b9a      	add.n	a9, a11, a9
42039ce2:	1b09b1        	l32r	a11, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42039ce5:	419190        	srli	a9, a9, 1
42039ce8:	0299b7        	bne	a9, a11, 42039cee <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x84a>
42039ceb:	fe2486        	j	42039581 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039cee:	82b990        	mull	a11, a9, a9
42039cf1:	11aac0        	slli	a10, a10, 4
42039cf4:	6288b0        	saltu	a8, a8, a11
42039cf7:	c09980        	sub	a9, a9, a8
42039cfa:	9188      	l32i.n	a8, a1, 36
42039cfc:	88aa      	add.n	a8, a8, a10
42039cfe:	c08890        	sub	a8, a8, a9
42039d01:	ff6dc6        	j	42039abc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x618>
42039d04:	00          	.byte	00
42039d05:	a18800        	sll	a8, a8
42039d08:	095d      	mov.n	a5, a9
42039d0a:	82a987        	bge	a9, a8, 42039c90 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ec>
42039d0d:	172182        	l32i	a8, a1, 92
42039d10:	3818a6        	blti	a8, 1, 42039d4c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8a8>
42039d13:	c198      	l32i.n	a9, a1, 48
42039d15:	208220        	or	a8, a2, a2
42039d18:	024966        	bnei	a9, 4, 42039d1e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x87a>
42039d1b:	002746        	j	42039dbc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x918>
42039d1e:	c138      	l32i.n	a3, a1, 48
42039d20:	08ad      	mov.n	a10, a8
42039d22:	03cd      	mov.n	a12, a3
42039d24:	06bd      	mov.n	a11, a6
42039d26:	1afc81        	l32r	a8, 42000918 <_stext+0x8f8> (40001200 <memmove>)
42039d29:	0008e0        	callx8	a8
42039d2c:	a198      	l32i.n	a9, a1, 40
42039d2e:	551b      	addi.n	a5, a5, 1
42039d30:	8a3a      	add.n	a8, a10, a3
42039d32:	e89957        	bne	a9, a5, 42039d1e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x87a>
42039d35:	6188      	l32i.n	a8, a1, 24
42039d37:	690b      	addi.n	a6, a9, -1
42039d39:	c06680        	sub	a6, a6, a8
42039d3c:	c188      	l32i.n	a8, a1, 48
42039d3e:	6199      	s32i.n	a9, a1, 24
42039d40:	826680        	mull	a6, a6, a8
42039d43:	828a      	add.n	a8, a2, a8
42039d45:	668a      	add.n	a6, a6, a8
42039d47:	ffd1c6        	j	42039c92 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ee>
42039d4a:	820000        	mull	a0, a0, a0
42039d4d:	901121        	l32r	a2, 4201dd94 <roo_display::(anonymous namespace)::GetSmoothRoundRectCornersPixelColor(roo_display::SmoothShape::RoundRectCorners const&, short, short)+0x30c> (5089f081 <_rtc_force_slow_end+0x89ee61>)
42039d50:	c068      	l32i.n	a6, a0, 48
42039d52:	c188      	l32i.n	a8, a1, 48
42039d54:	826680        	mull	a6, a6, a8
42039d57:	828a      	add.n	a8, a2, a8
42039d59:	668a      	add.n	a6, a6, a8
42039d5b:	a188      	l32i.n	a8, a1, 40
42039d5d:	6189      	s32i.n	a8, a1, 24
42039d5f:	ffcbc6        	j	42039c92 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ee>
42039d62:	000000        	ill
42039d65:	d138      	l32i.n	a3, a1, 52
42039d67:	122182        	l32i	a8, a1, 72
42039d6a:	0c9152        	l16si	a5, a1, 24
42039d6d:	44a387        	bge	a3, a8, 42039db5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x911>
42039d70:	0527d2        	l32i	a13, a7, 20
42039d73:	20b330        	or	a11, a3, a3
42039d76:	05cd      	mov.n	a12, a5
42039d78:	07ad      	mov.n	a10, a7
42039d7a:	ff1425        	call8	42038ebc <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039d7d:	7188      	l32i.n	a8, a1, 28
42039d7f:	02a9      	s32i.n	a10, a2, 0
42039d81:	331b      	addi.n	a3, a3, 1
42039d83:	224b      	addi.n	a2, a2, 4
42039d85:	e72387        	blt	a3, a8, 42039d70 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8cc>
42039d88:	162182        	l32i	a8, a1, 88
42039d8b:	1521b2        	l32i	a11, a1, 84
42039d8e:	368a      	add.n	a3, a6, a8
42039d90:	57d8      	l32i.n	a13, a7, 20
42039d92:	05cd      	mov.n	a12, a5
42039d94:	07ad      	mov.n	a10, a7
42039d96:	ff1265        	call8	42038ebc <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039d99:	132192        	l32i	a9, a1, 76
42039d9c:	fcc982        	addi	a8, a9, -4
42039d9f:	418280        	srli	a8, a8, 2
42039da2:	239a      	add.n	a2, a3, a9
42039da4:	881b      	addi.n	a8, a8, 1
42039da6:	038876        	loop	a8, 42039dad <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x909>
42039da9:	03a9      	s32i.n	a10, a3, 0
42039dab:	334b      	addi.n	a3, a3, 4
42039dad:	142182        	l32i	a8, a1, 80
42039db0:	9189      	s32i.n	a8, a1, 36
42039db2:	ffa286        	j	42039c40 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x79c>
42039db5:	03bd      	mov.n	a11, a3
42039db7:	063d      	mov.n	a3, a6
42039db9:	fff4c6        	j	42039d90 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8ec>
42039dbc:	a188      	l32i.n	a8, a1, 40
42039dbe:	029d      	mov.n	a9, a2
42039dc0:	c08850        	sub	a8, a8, a5
42039dc3:	058876        	loop	a8, 42039dcc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x928>
42039dc6:	06a8      	l32i.n	a10, a6, 0
42039dc8:	09a9      	s32i.n	a10, a9, 0
42039dca:	994b      	addi.n	a9, a9, 4
42039dcc:	a198      	l32i.n	a9, a1, 40
42039dce:	6188      	l32i.n	a8, a1, 24
42039dd0:	690b      	addi.n	a6, a9, -1
42039dd2:	c06680        	sub	a6, a6, a8
42039dd5:	824b      	addi.n	a8, a2, 4
42039dd7:	a06680        	addx4	a6, a6, a8
42039dda:	6199      	s32i.n	a9, a1, 24
42039ddc:	ffac86        	j	42039c92 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ee>
