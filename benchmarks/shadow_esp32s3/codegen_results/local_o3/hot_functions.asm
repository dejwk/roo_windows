
/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033194 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42033194:	006136        	entry	a1, 48
42033197:	002282        	l32i	a8, a2, 0
4203319a:	20f770        	or	a15, a7, a7
4203319d:	0b2882        	l32i	a8, a8, 44
420331a0:	233380        	sext	a3, a3, 15
420331a3:	234480        	sext	a4, a4, 15
420331a6:	235580        	sext	a5, a5, 15
420331a9:	236680        	sext	a6, a6, 15
420331ac:	02ad      	mov.n	a10, a2
420331ae:	31f9      	s32i.n	a15, a1, 12
420331b0:	06ed      	mov.n	a14, a6
420331b2:	05dd      	mov.n	a13, a5
420331b4:	04cd      	mov.n	a12, a4
420331b6:	03bd      	mov.n	a11, a3
420331b8:	0008e0        	callx8	a8
420331bb:	027d      	mov.n	a7, a2
420331bd:	0a2d      	mov.n	a2, a10
420331bf:	facc      	bnez.n	a10, 420331d2 <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
420331c1:	31f8      	l32i.n	a15, a1, 12
420331c3:	06ed      	mov.n	a14, a6
420331c5:	05dd      	mov.n	a13, a5
420331c7:	04cd      	mov.n	a12, a4
420331c9:	03bd      	mov.n	a11, a3
420331cb:	07ad      	mov.n	a10, a7
420331cd:	dc5565        	call8	4200f724 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
420331d0:	0a2d      	mov.n	a2, a10
420331d2:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033f08 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033f08:	00a136        	entry	a1, 80
42033f0b:	3169      	s32i.n	a6, a1, 12
42033f0d:	2159      	s32i.n	a5, a1, 8
42033f0f:	060c      	movi.n	a6, 0
42033f11:	1cc172        	addi	a7, a1, 28
42033f14:	2188      	l32i.n	a8, a1, 8
42033f16:	029687        	bne	a6, a8, 42033f1c <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033f19:	f01d      	retw.n
42033f1b:	938200        	movnez	a8, a2, a0
42033f1e:	22d200        	orb	b13, b2, b0
42033f21:	018917        	bany	a9, a1, 42033f26 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033f24:	009482        	l16si	a8, a4, 0
42033f27:	01b8      	l32i.n	a11, a1, 0
42033f29:	1622a2        	l32i	a10, a2, 88
42033f2c:	08cd      	mov.n	a12, a8
42033f2e:	0067d2        	s32i	a13, a7, 0
42033f31:	20e770        	or	a14, a7, a7
42033f34:	016182        	s32i	a8, a1, 4
42033f37:	201110        	or	a1, a1, a1
42033f3a:	fffa25        	call8	42033edc <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033f3d:	182282        	l32i	a8, a2, 96
42033f40:	1922b2        	l32i	a11, a2, 100
42033f43:	8189      	s32i.n	a8, a1, 32
42033f45:	20c152        	addi	a5, a1, 32
42033f48:	050b16        	beqz	a11, 42033f9c <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033f4b:	0bc8      	l32i.n	a12, a11, 0
42033f4d:	24c1a2        	addi	a10, a1, 36
42033f50:	201110        	or	a1, a1, a1
42033f53:	ff3665        	call8	420332b8 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f56:	07d8      	l32i.n	a13, a7, 0
42033f58:	11c8      	l32i.n	a12, a1, 4
42033f5a:	0021b2        	l32i	a11, a1, 0
42033f5d:	24c1a2        	addi	a10, a1, 36
42033f60:	201110        	or	a1, a1, a1
42033f63:	fe1865        	call8	420320e8 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f66:	07a9      	s32i.n	a10, a7, 0
42033f68:	24c1a2        	addi	a10, a1, 36
42033f6b:	201110        	or	a1, a1, a1
42033f6e:	f894a5        	call8	4202c8b8 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f71:	1922b2        	l32i	a11, a2, 100
42033f74:	24c1a2        	addi	a10, a1, 36
42033f77:	002bc2        	l32i	a12, a11, 0
42033f7a:	201110        	or	a1, a1, a1
42033f7d:	ff33a5        	call8	420332b8 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f80:	05d8      	l32i.n	a13, a5, 0
42033f82:	0094c2        	l16si	a12, a4, 0
42033f85:	0093b2        	l16si	a11, a3, 0
42033f88:	24c1a2        	addi	a10, a1, 36
42033f8b:	201110        	or	a1, a1, a1
42033f8e:	fe15a5        	call8	420320e8 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f91:	05a9      	s32i.n	a10, a5, 0
42033f93:	24c1a2        	addi	a10, a1, 36
42033f96:	201110        	or	a1, a1, a1
42033f99:	f891e5        	call8	4202c8b8 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f9c:	0094c2        	l16si	a12, a4, 0
42033f9f:	0093b2        	l16si	a11, a3, 0
42033fa2:	07d8      	l32i.n	a13, a7, 0
42033fa4:	05ed      	mov.n	a14, a5
42033fa6:	a24b      	addi.n	a10, a2, 4
42033fa8:	201110        	or	a1, a1, a1
42033fab:	069165        	call8	4203a8c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033fae:	3188      	l32i.n	a8, a1, 12
42033fb0:	332b      	addi.n	a3, a3, 2
42033fb2:	a08680        	addx4	a8, a6, a8
42033fb5:	08a9      	s32i.n	a10, a8, 0
42033fb7:	661b      	addi.n	a6, a6, 1
42033fb9:	442b      	addi.n	a4, a4, 2
42033fbb:	ffd546        	j	42033f14 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033fbe:	c1a200        	mul16u	a10, a2, a0
42033fc1:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033fc4:	20          	.byte	0x20
42033fc5:	f88f25        	call8	4202c8b8 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033fc8:	02ad      	mov.n	a10, a2
42033fca:	201110        	or	a1, a1, a1
42033fcd:	3eea25        	call8	42072e70 <_Unwind_Resume>
42033fd0:	fffac6        	j	42033fbf <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038780 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
42038780:	006136        	entry	a1, 48
42038783:	1891a2        	l16si	a10, a1, 48
42038786:	3169      	s32i.n	a6, a1, 12
42038788:	91cb      	addi.n	a9, a1, 12
4203878a:	1a91b2        	l16si	a11, a1, 52
4203878d:	028d      	mov.n	a8, a2
4203878f:	747070        	extui	a7, a7, 0, 8
42038792:	02aa27        	bge	a10, a2, 42038798 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
42038795:	002c46        	j	4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
42038798:	02a4a7        	bge	a4, a10, 4203879e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
4203879b:	002ac6        	j	4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
4203879e:	02ab37        	bge	a11, a3, 420387a4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
420387a1:	002946        	j	4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
420387a4:	72e5b0        	salt	a14, a5, a11
420387a7:	02a5b7        	bge	a5, a11, 420387ad <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2d>
420387aa:	002706        	j	4203884a <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xca>
420387ad:	0009c2        	l8ui	a12, a9, 0
420387b0:	09ac16        	beqz	a12, 4203884e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xce>
420387b3:	d2ca      	add.n	a13, a2, a12
420387b5:	02ada7        	bge	a13, a10, 420387bb <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3b>
420387b8:	002486        	j	4203884e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xce>
420387bb:	f3ca      	add.n	a15, a3, a12
420387bd:	02afb7        	bge	a15, a11, 420387c3 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x43>
420387c0:	002286        	j	4203884e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xce>
420387c3:	c0dda0        	sub	a13, a13, a10
420387c6:	c0ffb0        	sub	a15, a15, a11
420387c9:	f4d0d0        	extui	a13, a13, 0, 16
420387cc:	f4f0f0        	extui	a15, a15, 0, 16
420387cf:	208df0        	or	a8, a13, a15
420387d2:	238880        	sext	a8, a8, 15
420387d5:	177896        	bltz	a8, 42038950 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1d0>
420387d8:	740ff4        	mul.aa.ll	a15, a15
420387db:	780dd4        	mula.aa.ll	a13, a13
420387de:	031080        	rsr.acclo	a8
420387e1:	ffa022        	movi	a2, 255
420387e4:	11f880        	slli	a15, a8, 8
420387e7:	5d4fa6        	blti	a15, 4, 42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
420387ea:	40ff80        	nsau	a8, a15
420387ed:	dfc882        	addi	a8, a8, -33
420387f0:	608080        	neg	a8, a8
420387f3:	218180        	srai	a8, a8, 1
420387f6:	390c      	movi.n	a9, 3
420387f8:	fec882        	addi	a8, a8, -2
420387fb:	401800        	ssl	a8
420387fe:	a18900        	sll	a8, a9
42038801:	c29f80        	quou	a9, a15, a8
42038804:	998a      	add.n	a9, a9, a8
42038806:	419190        	srli	a9, a9, 1
42038809:	c28f90        	quou	a8, a15, a9
4203880c:	889a      	add.n	a8, a8, a9
4203880e:	418180        	srli	a8, a8, 1
42038811:	c29f80        	quou	a9, a15, a8
42038814:	998a      	add.n	a9, a9, a8
42038816:	203c81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42038819:	419190        	srli	a9, a9, 1
4203881c:	281987        	beq	a9, a8, 42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
4203881f:	82a990        	mull	a10, a9, a9
42038822:	118cc0        	slli	a8, a12, 4
42038825:	62ffa0        	saltu	a15, a15, a10
42038828:	c099f0        	sub	a9, a9, a15
4203882b:	878a      	add.n	a8, a7, a8
4203882d:	c08890        	sub	a8, a8, a9
42038830:	f49080        	extui	a9, a8, 0, 16
42038833:	23a880        	sext	a10, a8, 15
42038836:	020c      	movi.n	a2, 0
42038838:	0ce8f7        	bbsi	a8, 15, 42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
4203883b:	f80c      	movi.n	a8, 15
4203883d:	4b28a7        	blt	a8, a10, 4203888c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x10c>
42038840:	1189c0        	slli	a8, a9, 4
42038843:	998a      	add.n	a9, a9, a8
42038845:	742090        	extui	a2, a9, 0, 8
42038848:	f01d      	retw.n
4203884a:	020c      	movi.n	a2, 0
4203884c:	f01d      	retw.n
4203884e:	0109c2        	l8ui	a12, a9, 1
42038851:	ccbc      	beqz.n	a12, 42038891 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x111>
42038853:	c0d4c0        	sub	a13, a4, a12
42038856:	372ad7        	blt	a10, a13, 42038891 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x111>
42038859:	d3ca      	add.n	a13, a3, a12
4203885b:	322db7        	blt	a13, a11, 42038891 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x111>
4203885e:	c08c40        	sub	a8, a12, a4
42038861:	88aa      	add.n	a8, a8, a10
42038863:	c0ddb0        	sub	a13, a13, a11
42038866:	f48080        	extui	a8, a8, 0, 16
42038869:	f4d0d0        	extui	a13, a13, 0, 16
4203886c:	2098d0        	or	a9, a8, a13
4203886f:	239980        	sext	a9, a9, 15
42038872:	0f39d6        	bgez	a9, 42038969 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e9>
42038875:	c08c80        	sub	a8, a12, a8
42038878:	40f890        	nsau	a9, a8
4203887b:	419590        	srli	a9, a9, 5
4203887e:	052816        	beqz	a8, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038881:	c0dcd0        	sub	a13, a12, a13
42038884:	180c      	movi.n	a8, 1
42038886:	9389d0        	movnez	a8, a9, a13
42038889:	047856        	bnez	a8, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203888c:	ffa022        	movi	a2, 255
4203888f:	f01d      	retw.n
42038891:	0209c2        	l8ui	a12, a9, 2
42038894:	048c16        	beqz	a12, 420388e0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038897:	c0d4c0        	sub	a13, a4, a12
4203889a:	422ad7        	blt	a10, a13, 420388e0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
4203889d:	c0d5c0        	sub	a13, a5, a12
420388a0:	3c2bd7        	blt	a11, a13, 420388e0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
420388a3:	c09c40        	sub	a9, a12, a4
420388a6:	c08c50        	sub	a8, a12, a5
420388a9:	99aa      	add.n	a9, a9, a10
420388ab:	88ba      	add.n	a8, a8, a11
420388ad:	f49090        	extui	a9, a9, 0, 16
420388b0:	f48080        	extui	a8, a8, 0, 16
420388b3:	20a980        	or	a10, a9, a8
420388b6:	23aa80        	sext	a10, a10, 15
420388b9:	108ad6        	bgez	a10, 420389c5 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x245>
420388bc:	c09c90        	sub	a9, a12, a9
420388bf:	40f9a0        	nsau	a10, a9
420388c2:	41a5a0        	srli	a10, a10, 5
420388c5:	c08c80        	sub	a8, a12, a8
420388c8:	008916        	beqz	a9, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
420388cb:	01a092        	movi	a9, 1
420388ce:	939a80        	movnez	a9, a10, a8
420388d1:	fb7916        	beqz	a9, 4203888c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x10c>
420388d4:	1127c0        	slli	a2, a7, 4
420388d7:	272a      	add.n	a2, a7, a2
420388d9:	742020        	extui	a2, a2, 0, 8
420388dc:	f01d      	retw.n
420388de:	00          	.byte	00
420388df:	099200        	l32e	a0, a2, -28
420388e2:	03          	.byte	0x3
420388e3:	29bc      	beqz.n	a9, 42038919 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x199>
420388e5:	c89a      	add.n	a12, a8, a9
420388e7:	2e2ca7        	blt	a12, a10, 42038919 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x199>
420388ea:	c0d590        	sub	a13, a5, a9
420388ed:	282bd7        	blt	a11, a13, 42038919 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x199>
420388f0:	c08950        	sub	a8, a9, a5
420388f3:	c0cca0        	sub	a12, a12, a10
420388f6:	88ba      	add.n	a8, a8, a11
420388f8:	f4c0c0        	extui	a12, a12, 0, 16
420388fb:	f48080        	extui	a8, a8, 0, 16
420388fe:	20ac80        	or	a10, a12, a8
42038901:	23aa80        	sext	a10, a10, 15
42038904:	120ad6        	bgez	a10, 42038a28 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a8>
42038907:	c0c9c0        	sub	a12, a9, a12
4203890a:	40fca0        	nsau	a10, a12
4203890d:	41a5a0        	srli	a10, a10, 5
42038910:	fc0c16        	beqz	a12, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038913:	c08980        	sub	a8, a9, a8
42038916:	ffec46        	j	420388cb <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x14b>
42038919:	e90c      	movi.n	a9, 14
4203891b:	ffa022        	movi	a2, 255
4203891e:	02b977        	bgeu	a9, a7, 42038924 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1a4>
42038921:	ffc8c6        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038924:	c09a80        	sub	a9, a10, a8
42038927:	40f9c0        	nsau	a12, a9
4203892a:	1d0c      	movi.n	a13, 1
4203892c:	41c5c0        	srli	a12, a12, 5
4203892f:	fa1916        	beqz	a9, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038932:	c08a40        	sub	a8, a10, a4
42038935:	83cd80        	moveqz	a12, a13, a8
42038938:	f98c56        	bnez	a12, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203893b:	c03b30        	sub	a3, a11, a3
4203893e:	83cd30        	moveqz	a12, a13, a3
42038941:	f8fc56        	bnez	a12, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038944:	c08b50        	sub	a8, a11, a5
42038947:	93dc80        	movnez	a13, a12, a8
4203894a:	f86d56        	bnez	a13, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203894d:	f01d      	retw.n
4203894f:	00          	.byte	00
42038950:	c0dcd0        	sub	a13, a12, a13
42038953:	180c      	movi.n	a8, 1
42038955:	83e8d0        	moveqz	a14, a8, a13
42038958:	f78e56        	bnez	a14, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
4203895b:	c0fcf0        	sub	a15, a12, a15
4203895e:	938ef0        	movnez	a8, a14, a15
42038961:	ffa022        	movi	a2, 255
42038964:	f6c856        	bnez	a8, 420388d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x154>
42038967:	f01d      	retw.n
42038969:	740dd4        	mul.aa.ll	a13, a13
4203896c:	780884        	mula.aa.ll	a8, a8
4203896f:	031080        	rsr.acclo	a8
42038972:	ffa022        	movi	a2, 255
42038975:	11d880        	slli	a13, a8, 8
42038978:	024de6        	bgei	a13, 4, 4203897e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1fe>
4203897b:	ffb246        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
4203897e:	40fd80        	nsau	a8, a13
42038981:	dfc882        	addi	a8, a8, -33
42038984:	608080        	neg	a8, a8
42038987:	218180        	srai	a8, a8, 1
4203898a:	390c      	movi.n	a9, 3
4203898c:	fec882        	addi	a8, a8, -2
4203898f:	401800        	ssl	a8
42038992:	a18900        	sll	a8, a9
42038995:	c29d80        	quou	a9, a13, a8
42038998:	998a      	add.n	a9, a9, a8
4203899a:	419190        	srli	a9, a9, 1
4203899d:	c28d90        	quou	a8, a13, a9
420389a0:	889a      	add.n	a8, a8, a9
420389a2:	418180        	srli	a8, a8, 1
420389a5:	c29d80        	quou	a9, a13, a8
420389a8:	998a      	add.n	a9, a9, a8
420389aa:	1fd781        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
420389ad:	419190        	srli	a9, a9, 1
420389b0:	029987        	bne	a9, a8, 420389b6 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x236>
420389b3:	ffa446        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
420389b6:	82a990        	mull	a10, a9, a9
420389b9:	118cc0        	slli	a8, a12, 4
420389bc:	62dda0        	saltu	a13, a13, a10
420389bf:	c099d0        	sub	a9, a9, a13
420389c2:	ff9946        	j	4203882b <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xab>
420389c5:	740884        	mul.aa.ll	a8, a8
420389c8:	780994        	mula.aa.ll	a9, a9
420389cb:	031080        	rsr.acclo	a8
420389ce:	ffa022        	movi	a2, 255
420389d1:	119880        	slli	a9, a8, 8
420389d4:	0249e6        	bgei	a9, 4, 420389da <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x25a>
420389d7:	ff9b46        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
420389da:	40f980        	nsau	a8, a9
420389dd:	dfc882        	addi	a8, a8, -33
420389e0:	608080        	neg	a8, a8
420389e3:	218180        	srai	a8, a8, 1
420389e6:	3a0c      	movi.n	a10, 3
420389e8:	fec882        	addi	a8, a8, -2
420389eb:	401800        	ssl	a8
420389ee:	a18a00        	sll	a8, a10
420389f1:	c2a980        	quou	a10, a9, a8
420389f4:	aa8a      	add.n	a10, a10, a8
420389f6:	41a1a0        	srli	a10, a10, 1
420389f9:	c289a0        	quou	a8, a9, a10
420389fc:	88aa      	add.n	a8, a8, a10
420389fe:	418180        	srli	a8, a8, 1
42038a01:	c2a980        	quou	a10, a9, a8
42038a04:	aa8a      	add.n	a10, a10, a8
42038a06:	1fc081        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42038a09:	41a1a0        	srli	a10, a10, 1
42038a0c:	029a87        	bne	a10, a8, 42038a12 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x292>
42038a0f:	ff8d46        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038a12:	82baa0        	mull	a11, a10, a10
42038a15:	118cc0        	slli	a8, a12, 4
42038a18:	6299b0        	saltu	a9, a9, a11
42038a1b:	c0aa90        	sub	a10, a10, a9
42038a1e:	878a      	add.n	a8, a7, a8
42038a20:	c088a0        	sub	a8, a8, a10
42038a23:	ff8246        	j	42038830 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xb0>
42038a26:	840000        	extui	a0, a0, 0, 9
42038a29:	7408      	l32i.n	a0, a4, 28
42038a2b:	780cc4        	mula.aa.ll	a12, a12
42038a2e:	031080        	rsr.acclo	a8
42038a31:	ffa022        	movi	a2, 255
42038a34:	11a880        	slli	a10, a8, 8
42038a37:	024ae6        	bgei	a10, 4, 42038a3d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2bd>
42038a3a:	ff8286        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038a3d:	40fa80        	nsau	a8, a10
42038a40:	dfc882        	addi	a8, a8, -33
42038a43:	608080        	neg	a8, a8
42038a46:	218180        	srai	a8, a8, 1
42038a49:	3b0c      	movi.n	a11, 3
42038a4b:	fec882        	addi	a8, a8, -2
42038a4e:	401800        	ssl	a8
42038a51:	a18b00        	sll	a8, a11
42038a54:	c2ba80        	quou	a11, a10, a8
42038a57:	bb8a      	add.n	a11, a11, a8
42038a59:	41b1b0        	srli	a11, a11, 1
42038a5c:	c28ab0        	quou	a8, a10, a11
42038a5f:	88ba      	add.n	a8, a8, a11
42038a61:	418180        	srli	a8, a8, 1
42038a64:	c2ba80        	quou	a11, a10, a8
42038a67:	bb8a      	add.n	a11, a11, a8
42038a69:	1fa781        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42038a6c:	41b1b0        	srli	a11, a11, 1
42038a6f:	029b87        	bne	a11, a8, 42038a75 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2f5>
42038a72:	ff7486        	j	42038848 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xc8>
42038a75:	82cbb0        	mull	a12, a11, a11
42038a78:	1189c0        	slli	a8, a9, 4
42038a7b:	62aac0        	saltu	a10, a10, a12
42038a7e:	c0bba0        	sub	a11, a11, a10
42038a81:	878a      	add.n	a8, a7, a8
42038a83:	c088b0        	sub	a8, a8, a11
42038a86:	ff6986        	j	42038830 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xb0>


/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203b114 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
4203b114:	00a136        	entry	a1, 80
4203b117:	028d      	mov.n	a8, a2
4203b119:	23d480        	sext	a13, a4, 15
4203b11c:	239580        	sext	a9, a5, 15
4203b11f:	0f0222        	l8ui	a2, a2, 15
4203b122:	21d9      	s32i.n	a13, a1, 8
4203b124:	3199      	s32i.n	a9, a1, 12
4203b126:	246216        	beqz	a2, 4203b370 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
4203b129:	0198a2        	l16si	a10, a8, 2
4203b12c:	0698c2        	l16si	a12, a8, 12
4203b12f:	91a9      	s32i.n	a10, a1, 36
4203b131:	c0a9a0        	sub	a10, a9, a10
4203b134:	11bac0        	slli	a11, a10, 4
4203b137:	069392        	l16si	a9, a3, 12
4203b13a:	bb8b      	addi.n	a11, a11, 8
4203b13c:	a1a9      	s32i.n	a10, a1, 40
4203b13e:	0398a2        	l16si	a10, a8, 6
4203b141:	c0cbc0        	sub	a12, a11, a12
4203b144:	0098e2        	l16si	a14, a8, 0
4203b147:	c0bb90        	sub	a11, a11, a9
4203b14a:	b1a9      	s32i.n	a10, a1, 44
4203b14c:	4cca      	add.n	a4, a12, a12
4203b14e:	11aac0        	slli	a10, a10, 4
4203b151:	c09de0        	sub	a9, a13, a14
4203b154:	61e9      	s32i.n	a14, a1, 24
4203b156:	dbba      	add.n	a13, a11, a11
4203b158:	72e4a0        	salt	a14, a4, a10
4203b15b:	11d9      	s32i.n	a13, a1, 4
4203b15d:	ee0b      	addi.n	a14, a14, -1
4203b15f:	72dda0        	salt	a13, a13, a10
4203b162:	0298f2        	l16si	a15, a8, 4
4203b165:	dd0b      	addi.n	a13, a13, -1
4203b167:	60e0e0        	neg	a14, a14
4203b16a:	60d0d0        	neg	a13, a13
4203b16d:	74e0e0        	extui	a14, a14, 0, 8
4203b170:	7199      	s32i.n	a9, a1, 28
4203b172:	1199c0        	slli	a9, a9, 4
4203b175:	998b      	addi.n	a9, a9, 8
4203b177:	116fc0        	slli	a6, a15, 4
4203b17a:	41e9      	s32i.n	a14, a1, 16
4203b17c:	74e0d0        	extui	a14, a13, 0, 8
4203b17f:	81f9      	s32i.n	a15, a1, 32
4203b181:	c07ab0        	sub	a7, a10, a11
4203b184:	c0fac0        	sub	a15, a10, a12
4203b187:	599a      	add.n	a5, a9, a9
4203b189:	c0d690        	sub	a13, a6, a9
4203b18c:	0561e2        	s32i	a14, a1, 20
4203b18f:	439d90        	min	a9, a13, a9
4203b192:	43ffc0        	min	a15, a15, a12
4203b195:	4377b0        	min	a7, a7, a11
4203b198:	5c2567        	blt	a5, a6, 4203b1f8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
4203b19b:	02a4a7        	bge	a4, a10, 4203b1a1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x8d>
4203b19e:	007406        	j	4203b372 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25e>
4203b1a1:	0121b2        	l32i	a11, a1, 4
4203b1a4:	0a08d2        	l8ui	a13, a8, 10
4203b1a7:	022ba7        	blt	a11, a10, 4203b1ad <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x99>
4203b1aa:	00a186        	j	4203b434 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x320>
4203b1ad:	11cdc0        	slli	a12, a13, 4
4203b1b0:	0908b2        	l8ui	a11, a8, 9
4203b1b3:	02afc7        	bge	a15, a12, 4203b1b9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xa5>
4203b1b6:	007d86        	j	4203b3b0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x29c>
4203b1b9:	23eb80        	sext	a14, a11, 15
4203b1bc:	11eec0        	slli	a14, a14, 4
4203b1bf:	0227e7        	blt	a7, a14, 4203b1c5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb1>
4203b1c2:	008486        	j	4203b3d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2c4>
4203b1c5:	41e8      	l32i.n	a14, a1, 16
4203b1c7:	052162        	l32i	a6, a1, 20
4203b1ca:	539e67        	bne	a14, a6, 4203b221 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10d>
4203b1cd:	53ebd0        	max	a14, a11, a13
4203b1d0:	23ee80        	sext	a14, a14, 15
4203b1d3:	11eec0        	slli	a14, a14, 4
4203b1d6:	02a9e7        	bge	a9, a14, 4203b1dc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc8>
4203b1d9:	006a46        	j	4203b386 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x272>
4203b1dc:	53df70        	max	a13, a15, a7
4203b1df:	02a9d7        	bge	a9, a13, 4203b1e5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd1>
4203b1e2:	009b86        	j	4203b454 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x340>
4203b1e5:	0224a7        	blt	a4, a10, 4203b1eb <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd7>
4203b1e8:	00b386        	j	4203b4ba <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x3a6>
4203b1eb:	9188      	l32i.n	a8, a1, 36
4203b1ed:	3198      	l32i.n	a9, a1, 12
4203b1ef:	1b0c      	movi.n	a11, 1
4203b1f1:	c0c890        	sub	a12, a8, a9
4203b1f4:	007d46        	j	4203b3ed <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2d9>
4203b1f7:	24a700        	extui	a10, a0, 7, 3
4203b1fa:	674602        	s8i	a0, a6, 103
4203b1fd:	08d200        	lsx	f13, a2, a0
4203b200:	b808      	l32i.n	a0, a8, 44
4203b202:	cdc011        	l32r	a1, 4202e904 <void roo_windows::internal::ExclusionFilter::subtractMaskedRect<roo_display::BufferedRectFiller>(roo_display::Box const&, unsigned int, unsigned char, roo_display::BufferedRectFiller*)+0x120> (94a238c1 <_rtc_reserved_end+0x349238c1>)
4203b205:	aba711        	l32r	a1, 420260a4 <roo_display::internal::TileBase::TileBase(roo_display::Drawable const&, roo_display::Box, roo_display::Alignment, roo_display::Color)+0x50> (da803160 <_rtc_reserved_end+0x7a703160>)
4203b208:	6fc602        	addi	a0, a6, 111
4203b20b:	08b200        	lsx	f11, a2, a0
4203b20e:	c70b      	addi.n	a12, a7, -1
4203b210:	5c0602af 	ee.vmulas.u16.qacc.ld.ip.qup	q0, a10, -224, q4, q0, q0, q6
4203b214:	00          	.byte	00
4203b215:	23eb80        	sext	a14, a11, 15
4203b218:	11eec0        	slli	a14, a14, 4
4203b21b:	0227e7        	blt	a7, a14, 4203b221 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10d>
4203b21e:	006d86        	j	4203b3d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2c4>
4203b221:	0918a2        	l16ui	a10, a8, 18
4203b224:	60a0a0        	neg	a10, a10
4203b227:	02a9a7        	bge	a9, a10, 4203b22d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x119>
4203b22a:	0059c6        	j	4203b395 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x281>
4203b22d:	c0dcf0        	sub	a13, a12, a15
4203b230:	0e0c      	movi.n	a14, 0
4203b232:	c0ac90        	sub	a10, a12, a9
4203b235:	022ed7        	blt	a14, a13, 4203b23b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x127>
4203b238:	007406        	j	4203b40c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2f8>
4203b23b:	022ea7        	blt	a14, a10, 4203b241 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12d>
4203b23e:	007286        	j	4203b40c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2f8>
4203b241:	82aaa0        	mull	a10, a10, a10
4203b244:	82ddd0        	mull	a13, a13, a13
4203b247:	dada      	add.n	a13, a10, a13
4203b249:	f4a0d0        	extui	a10, a13, 0, 16
4203b24c:	3c4db6        	bltui	a13, 4, 4203b28c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x178>
4203b24f:	40fda0        	nsau	a10, a13
4203b252:	dfcaa2        	addi	a10, a10, -33
4203b255:	60a0a0        	neg	a10, a10
4203b258:	21a1a0        	srai	a10, a10, 1
4203b25b:	fecaa2        	addi	a10, a10, -2
4203b25e:	3e0c      	movi.n	a14, 3
4203b260:	401a00        	ssl	a10
4203b263:	a1ee00        	sll	a14, a14
4203b266:	c2ade0        	quou	a10, a13, a14
4203b269:	aaea      	add.n	a10, a10, a14
4203b26b:	41a1a0        	srli	a10, a10, 1
4203b26e:	c2eda0        	quou	a14, a13, a10
4203b271:	eeaa      	add.n	a14, a14, a10
4203b273:	41e1e0        	srli	a14, a14, 1
4203b276:	c2ade0        	quou	a10, a13, a14
4203b279:	aaea      	add.n	a10, a10, a14
4203b27b:	15a3e1        	l32r	a14, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203b27e:	41a1a0        	srli	a10, a10, 1
4203b281:	021ae7        	beq	a10, a14, 4203b287 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x173>
4203b284:	008006        	j	4203b488 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x374>
4203b287:	aa0b      	addi.n	a10, a10, -1
4203b289:	f4a0a0        	extui	a10, a10, 0, 16
4203b28c:	0e08e2        	l8ui	a14, a8, 14
4203b28f:	0818d2        	l16ui	a13, a8, 16
4203b292:	c08ac0        	sub	a8, a10, a12
4203b295:	c088e0        	sub	a8, a8, a14
4203b298:	15a8a1        	l32r	a10, 42000938 <_stext+0x918> (80000 <UserFrameTotalSize+0x7ff00>)
4203b29b:	8288d0        	mull	a8, a8, a13
4203b29e:	88aa      	add.n	a8, a8, a10
4203b2a0:	218c80        	srai	a8, a8, 12
4203b2a3:	ffa0a2        	movi	a10, 255
4203b2a6:	4388a0        	min	a8, a8, a10
4203b2a9:	0a0c      	movi.n	a10, 0
4203b2ab:	5388a0        	max	a8, a8, a10
4203b2ae:	15a1a1        	l32r	a10, 42000934 <_stext+0x914> (3c0c151f <roo_windows::internal::(anonymous namespace)::kKey>)
4203b2b1:	8a8a      	add.n	a8, a10, a8
4203b2b3:	000822        	l8ui	a2, a8, 0
4203b2b6:	02ad      	mov.n	a10, a2
4203b2b8:	091382        	l16ui	a8, a3, 18
4203b2bb:	43c970        	min	a12, a9, a7
4203b2be:	608080        	neg	a8, a8
4203b2c1:	02ac87        	bge	a12, a8, 4203b2c7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1b3>
4203b2c4:	002a06        	j	4203b370 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
4203b2c7:	23bb80        	sext	a11, a11, 15
4203b2ca:	11bbc0        	slli	a11, a11, 4
4203b2cd:	c08b90        	sub	a8, a11, a9
4203b2d0:	0d0c      	movi.n	a13, 0
4203b2d2:	c0cb70        	sub	a12, a11, a7
4203b2d5:	022d87        	blt	a13, a8, 4203b2db <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1c7>
4203b2d8:	004f06        	j	4203b418 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x304>
4203b2db:	022dc7        	blt	a13, a12, 4203b2e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1cd>
4203b2de:	004d86        	j	4203b418 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x304>
4203b2e1:	828880        	mull	a8, a8, a8
4203b2e4:	82ccc0        	mull	a12, a12, a12
4203b2e7:	c8ca      	add.n	a12, a8, a12
4203b2e9:	f480c0        	extui	a8, a12, 0, 16
4203b2ec:	3c4cb6        	bltui	a12, 4, 4203b32c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x218>
4203b2ef:	40fc80        	nsau	a8, a12
4203b2f2:	dfc882        	addi	a8, a8, -33
4203b2f5:	608080        	neg	a8, a8
4203b2f8:	218180        	srai	a8, a8, 1
4203b2fb:	fec882        	addi	a8, a8, -2
4203b2fe:	390c      	movi.n	a9, 3
4203b300:	401800        	ssl	a8
4203b303:	a19900        	sll	a9, a9
4203b306:	c28c90        	quou	a8, a12, a9
4203b309:	889a      	add.n	a8, a8, a9
4203b30b:	418180        	srli	a8, a8, 1
4203b30e:	c29c80        	quou	a9, a12, a8
4203b311:	998a      	add.n	a9, a9, a8
4203b313:	419190        	srli	a9, a9, 1
4203b316:	c28c90        	quou	a8, a12, a9
4203b319:	889a      	add.n	a8, a8, a9
4203b31b:	157b91        	l32r	a9, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203b31e:	418180        	srli	a8, a8, 1
4203b321:	021897        	beq	a8, a9, 4203b327 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x213>
4203b324:	005406        	j	4203b478 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x364>
4203b327:	880b      	addi.n	a8, a8, -1
4203b329:	f48080        	extui	a8, a8, 0, 16
4203b32c:	0e03c2        	l8ui	a12, a3, 14
4203b32f:	081392        	l16ui	a9, a3, 16
4203b332:	c088b0        	sub	a8, a8, a11
4203b335:	c088c0        	sub	a8, a8, a12
4203b338:	828890        	mull	a8, a8, a9
4203b33b:	157f91        	l32r	a9, 42000938 <_stext+0x918> (80000 <UserFrameTotalSize+0x7ff00>)
4203b33e:	889a      	add.n	a8, a8, a9
4203b340:	218c80        	srai	a8, a8, 12
4203b343:	ffa092        	movi	a9, 255
4203b346:	438890        	min	a8, a8, a9
4203b349:	090c      	movi.n	a9, 0
4203b34b:	538890        	max	a8, a8, a9
4203b34e:	157b91        	l32r	a9, 4200093c <_stext+0x91c> (3c0c141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
4203b351:	898a      	add.n	a8, a9, a8
4203b353:	000882        	l8ui	a8, a8, 0
4203b356:	156f91        	l32r	a9, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203b359:	d1aa80        	mul16s	a10, a10, a8
4203b35c:	882a      	add.n	a8, a8, a2
4203b35e:	7fcaa2        	addi	a10, a10, 127
4203b361:	b29a90        	mulsh	a9, a10, a9
4203b364:	80aa90        	add	a10, a10, a9
4203b367:	21a7a0        	srai	a10, a10, 7
4203b36a:	c088a0        	sub	a8, a8, a10
4203b36d:	742080        	extui	a2, a8, 0, 8
4203b370:	f01d      	retw.n
4203b372:	11b8      	l32i.n	a11, a1, 4
4203b374:	0908d2        	l8ui	a13, a8, 9
4203b377:	4e2ba7        	blt	a11, a10, 4203b3c9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2b5>
4203b37a:	11cdc0        	slli	a12, a13, 4
4203b37d:	0a08b2        	l8ui	a11, a8, 10
4203b380:	022fc7        	blt	a15, a12, 4203b386 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x272>
4203b383:	ffa386        	j	4203b215 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x101>
4203b386:	0918a2        	l16ui	a10, a8, 18
4203b389:	60a0a0        	neg	a10, a10
4203b38c:	0529a7        	blt	a9, a10, 4203b395 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x281>
4203b38f:	022fa7        	blt	a15, a10, 4203b395 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x281>
4203b392:	ffa5c6        	j	4203b22d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x119>
4203b395:	0a0c      	movi.n	a10, 0
4203b397:	0a2d      	mov.n	a2, a10
4203b399:	ffc6c6        	j	4203b2b8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a4>
4203b39c:	11b8      	l32i.n	a11, a1, 4
4203b39e:	0b08d2        	l8ui	a13, a8, 11
4203b3a1:	7eaba7        	bge	a11, a10, 4203b423 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x30f>
4203b3a4:	11cdc0        	slli	a12, a13, 4
4203b3a7:	0808b2        	l8ui	a11, a8, 8
4203b3aa:	022fc7        	blt	a15, a12, 4203b3b0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x29c>
4203b3ad:	ff8206        	j	4203b1b9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xa5>
4203b3b0:	41e8      	l32i.n	a14, a1, 16
4203b3b2:	5168      	l32i.n	a6, a1, 20
4203b3b4:	029e67        	bne	a14, a6, 4203b3ba <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2a6>
4203b3b7:	ff8486        	j	4203b1cd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb9>
4203b3ba:	0918a2        	l16ui	a10, a8, 18
4203b3bd:	60a0a0        	neg	a10, a10
4203b3c0:	cba9a7        	bge	a9, a10, 4203b38f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27b>
4203b3c3:	fff386        	j	4203b395 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x281>
4203b3c6:	000000        	ill
4203b3c9:	11cdc0        	slli	a12, a13, 4
4203b3cc:	712fc7        	blt	a15, a12, 4203b441 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x32d>
4203b3cf:	20bdd0        	or	a11, a13, a13
4203b3d2:	02a7c7        	bge	a7, a12, 4203b3d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2c4>
4203b3d5:	ff7d06        	j	4203b1cd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb9>
4203b3d8:	43ef70        	min	a14, a15, a7
4203b3db:	172e97        	blt	a14, a9, 4203b3f6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2e2>
4203b3de:	062182        	l32i	a8, a1, 24
4203b3e1:	022192        	l32i	a9, a1, 8
4203b3e4:	c0c890        	sub	a12, a8, a9
4203b3e7:	72a567        	bge	a5, a6, 4203b45d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x349>
4203b3ea:	00a0b2        	movi	a11, 0
4203b3ed:	02ad      	mov.n	a10, a2
4203b3ef:	ffaf25        	call8	4203aee0 <roo_windows::internal::(anonymous namespace)::SideGradient(int, int, int)>
4203b3f2:	0a2d      	mov.n	a2, a10
4203b3f4:	f01d      	retw.n
4203b3f6:	41e8      	l32i.n	a14, a1, 16
4203b3f8:	5168      	l32i.n	a6, a1, 20
4203b3fa:	029e67        	bne	a14, a6, 4203b400 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2ec>
4203b3fd:	0025c6        	j	4203b498 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x384>
4203b400:	c0ac90        	sub	a10, a12, a9
4203b403:	00a0e2        	movi	a14, 0
4203b406:	c0dcf0        	sub	a13, a12, a15
4203b409:	5c2ea7        	blt	a14, a10, 4203b469 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x355>
4203b40c:	022f97        	blt	a15, a9, 4203b412 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2fe>
4203b40f:	ff9e46        	j	4203b28c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x178>
4203b412:	0dad      	mov.n	a10, a13
4203b414:	ff9d06        	j	4203b28c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x178>
4203b417:	279700        	ee.stf.64.xp	f9, f2, a0, a7
4203b41a:	c34602        	s8i	a0, a6, 195
4203b41d:	060c8dff 	ee.vmulas.s16.accx.ld.ip.qup	q5, a15, 0x1d0, q6, q0, q0, q4
4203b421:	c0ffc2        	s32ri	a12, a15, 0x300
4203b424:	cd          	.byte	0xcd
4203b425:	0ced11        	l32r	a1, 41ffe7dc <_coredump_iram_end+0x1c783dc>
4203b428:	0dbd      	mov.n	a11, a13
4203b42a:	022fc7        	blt	a15, a12, 4203b430 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x31c>
4203b42d:	ff6386        	j	4203b1bf <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
4203b430:	ff6886        	j	4203b1d6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc2>
4203b433:	00          	.byte	00
4203b434:	11cdc0        	slli	a12, a13, 4
4203b437:	0e2fc7        	blt	a15, a12, 4203b449 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x335>
4203b43a:	0ced      	mov.n	a14, a12
4203b43c:	0dbd      	mov.n	a11, a13
4203b43e:	ff5f46        	j	4203b1bf <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
4203b441:	0ced      	mov.n	a14, a12
4203b443:	0dbd      	mov.n	a11, a13
4203b445:	ff6346        	j	4203b1d6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc2>
4203b448:	00          	.byte	00
4203b449:	7b29c7        	blt	a9, a12, 4203b4c8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x3b4>
4203b44c:	53af70        	max	a10, a15, a7
4203b44f:	67a9a7        	bge	a9, a10, 4203b4ba <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x3a6>
4203b452:	0dbd      	mov.n	a11, a13
4203b454:	0918a2        	l16ui	a10, a8, 18
4203b457:	60a0a0        	neg	a10, a10
4203b45a:	ffcc46        	j	4203b38f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27b>
4203b45d:	7188      	l32i.n	a8, a1, 28
4203b45f:	8198      	l32i.n	a9, a1, 32
4203b461:	c0c890        	sub	a12, a8, a9
4203b464:	cc1b      	addi.n	a12, a12, 1
4203b466:	ffe006        	j	4203b3ea <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2d6>
4203b469:	9faed7        	bge	a14, a13, 4203b40c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2f8>
4203b46c:	c1aaa0        	mul16u	a10, a10, a10
4203b46f:	82ddd0        	mull	a13, a13, a13
4203b472:	dada      	add.n	a13, a10, a13
4203b474:	ff75c6        	j	4203b24f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x13b>
4203b477:	00          	.byte	00
4203b478:	829880        	mull	a9, a8, a8
4203b47b:	023c97        	bltu	a12, a9, 4203b481 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x36d>
4203b47e:	ffa9c6        	j	4203b329 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x215>
4203b481:	880b      	addi.n	a8, a8, -1
4203b483:	ffa886        	j	4203b329 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x215>
4203b486:	a00000        	addx4	a0, a0, a0
4203b489:	82ea      	add.n	a8, a2, a14
4203b48b:	023de7        	bltu	a13, a14, 4203b491 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x37d>
4203b48e:	ff7dc6        	j	4203b289 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x175>
4203b491:	aa0b      	addi.n	a10, a10, -1
4203b493:	ff7c86        	j	4203b289 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x175>
4203b496:	d00000        	subx2	a0, a0, a0
4203b499:	53db      	addi.n	a5, a3, 13
4203b49b:	23dd80        	sext	a13, a13, 15
4203b49e:	11ddc0        	slli	a13, a13, 4
4203b4a1:	0229d7        	blt	a9, a13, 4203b4a7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x393>
4203b4a4:	ff4d06        	j	4203b1dc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc8>
4203b4a7:	0918a2        	l16ui	a10, a8, 18
4203b4aa:	60a0a0        	neg	a10, a10
4203b4ad:	022fa7        	blt	a15, a10, 4203b4b3 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x39f>
4203b4b0:	ff5e46        	j	4203b22d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x119>
4203b4b3:	020c      	movi.n	a2, 0
4203b4b5:	02ad      	mov.n	a10, a2
4203b4b7:	ff8306        	j	4203b2c7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1b3>
4203b4ba:	a188      	l32i.n	a8, a1, 40
4203b4bc:	b198      	l32i.n	a9, a1, 44
4203b4be:	2b0c      	movi.n	a11, 2
4203b4c0:	c0c890        	sub	a12, a8, a9
4203b4c3:	cc1b      	addi.n	a12, a12, 1
4203b4c5:	ffc906        	j	4203b3ed <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2d9>
4203b4c8:	0dbd      	mov.n	a11, a13
4203b4ca:	ffae06        	j	4203b386 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x272>


/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203a8c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
4203a8c0:	008136        	entry	a1, 64
4203a8c3:	200282        	l8ui	a8, a2, 32
4203a8c6:	0212a2        	l16ui	a10, a2, 4
4203a8c9:	0412c2        	l16ui	a12, a2, 8
4203a8cc:	a8aa      	add.n	a10, a8, a10
4203a8ce:	c09c80        	sub	a9, a12, a8
4203a8d1:	4169      	s32i.n	a6, a1, 16
4203a8d3:	233380        	sext	a3, a3, 15
4203a8d6:	23ba80        	sext	a11, a10, 15
4203a8d9:	027d      	mov.n	a7, a2
4203a8db:	234480        	sext	a4, a4, 15
4203a8de:	f460a0        	extui	a6, a10, 0, 16
4203a8e1:	239980        	sext	a9, a9, 15
4203a8e4:	02a3b7        	bge	a3, a11, 4203a8ea <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2a>
4203a8e7:	003946        	j	4203a9d0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a8ea:	02a937        	bge	a9, a3, 4203a8f0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30>
4203a8ed:	0037c6        	j	4203a9d0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a8f0:	0312e2        	l16ui	a14, a2, 6
4203a8f3:	051222        	l16ui	a2, a2, 10
4203a8f6:	e8ea      	add.n	a14, a8, a14
4203a8f8:	23de80        	sext	a13, a14, 15
4203a8fb:	f4e0e0        	extui	a14, a14, 0, 16
4203a8fe:	c0a280        	sub	a10, a2, a8
4203a901:	61e9      	s32i.n	a14, a1, 24
4203a903:	23aa80        	sext	a10, a10, 15
4203a906:	02a4d7        	bge	a4, a13, 4203a90c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x4c>
4203a909:	0030c6        	j	4203a9d0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a90c:	72fa40        	salt	a15, a10, a4
4203a90f:	71f9      	s32i.n	a15, a1, 28
4203a911:	02aa47        	bge	a10, a4, 4203a917 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x57>
4203a914:	002e06        	j	4203a9d0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a917:	2107f2        	l8ui	a15, a7, 33
4203a91a:	1c07e2        	l8ui	a14, a7, 28
4203a91d:	51f9      	s32i.n	a15, a1, 20
4203a91f:	1dbe16        	beqz	a14, 4203aafe <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23e>
4203a922:	fbea      	add.n	a15, a11, a14
4203a924:	02af37        	bge	a15, a3, 4203a92a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x6a>
4203a927:	0074c6        	j	4203aafe <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23e>
4203a92a:	fdea      	add.n	a15, a13, a14
4203a92c:	02af47        	bge	a15, a4, 4203a932 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x72>
4203a92f:	0072c6        	j	4203aafe <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23e>
4203a932:	6198      	l32i.n	a9, a1, 24
4203a934:	c0ae30        	sub	a10, a14, a3
4203a937:	c08e40        	sub	a8, a14, a4
4203a93a:	898a      	add.n	a8, a9, a8
4203a93c:	a6aa      	add.n	a10, a6, a10
4203a93e:	f4a0a0        	extui	a10, a10, 0, 16
4203a941:	f48080        	extui	a8, a8, 0, 16
4203a944:	209a80        	or	a9, a10, a8
4203a947:	239980        	sext	a9, a9, 15
4203a94a:	478996        	bltz	a9, 4203adc6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x506>
4203a94d:	740884        	mul.aa.ll	a8, a8
4203a950:	780aa4        	mula.aa.ll	a10, a10
4203a953:	031080        	rsr.acclo	a8
4203a956:	119880        	slli	a9, a8, 8
4203a959:	6349a6        	blti	a9, 4, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203a95c:	40f980        	nsau	a8, a9
4203a95f:	dfc882        	addi	a8, a8, -33
4203a962:	608080        	neg	a8, a8
4203a965:	218180        	srai	a8, a8, 1
4203a968:	3a0c      	movi.n	a10, 3
4203a96a:	fec882        	addi	a8, a8, -2
4203a96d:	401800        	ssl	a8
4203a970:	a18a00        	sll	a8, a10
4203a973:	c2a980        	quou	a10, a9, a8
4203a976:	aa8a      	add.n	a10, a10, a8
4203a978:	41a1a0        	srli	a10, a10, 1
4203a97b:	c289a0        	quou	a8, a9, a10
4203a97e:	88aa      	add.n	a8, a8, a10
4203a980:	418180        	srli	a8, a8, 1
4203a983:	c2a980        	quou	a10, a9, a8
4203a986:	aa8a      	add.n	a10, a10, a8
4203a988:	17e081        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a98b:	41a1a0        	srli	a10, a10, 1
4203a98e:	2e1a87        	beq	a10, a8, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203a991:	82baa0        	mull	a11, a10, a10
4203a994:	118ec0        	slli	a8, a14, 4
4203a997:	6299b0        	saltu	a9, a9, a11
4203a99a:	c0aa90        	sub	a10, a10, a9
4203a99d:	5198      	l32i.n	a9, a1, 20
4203a99f:	898a      	add.n	a8, a9, a8
4203a9a1:	c088a0        	sub	a8, a8, a10
4203a9a4:	f49080        	extui	a9, a8, 0, 16
4203a9a7:	23a880        	sext	a10, a8, 15
4203a9aa:	22e8f7        	bbsi	a8, 15, 4203a9d0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x110>
4203a9ad:	f80c      	movi.n	a8, 15
4203a9af:	0d28a7        	blt	a8, a10, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203a9b2:	1189c0        	slli	a8, a9, 4
4203a9b5:	898a      	add.n	a8, a9, a8
4203a9b7:	746080        	extui	a6, a8, 0, 8
4203a9ba:	ffa092        	movi	a9, 255
4203a9bd:	119697        	bne	a6, a9, 4203a9d2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x112>
4203a9c0:	142782        	l32i	a8, a7, 80
4203a9c3:	052d      	mov.n	a2, a5
4203a9c5:	133816        	beqz	a8, 4203aafc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203a9c8:	ffa062        	movi	a6, 255
4203a9cb:	0000c6        	j	4203a9d2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x112>
4203a9ce:	00          	.byte	00
4203a9cf:	060c00        	ee.ldf.64.xp	f0, f0, a0, a12
4203a9d2:	04dd      	mov.n	a13, a4
4203a9d4:	3cc7b2        	addi	a11, a7, 60
4203a9d7:	03cd      	mov.n	a12, a3
4203a9d9:	28c7a2        	addi	a10, a7, 40
4203a9dc:	007365        	call8	4203b114 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
4203a9df:	2007b2        	l8ui	a11, a7, 32
4203a9e2:	51a9      	s32i.n	a10, a1, 20
4203a9e4:	012a80        	slli	a2, a10, 24
4203a9e7:	06dd      	mov.n	a13, a6
4203a9e9:	04cb16        	beqz	a11, 4203aa39 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203a9ec:	0597d2        	l16si	a13, a7, 10
4203a9ef:	67e8      	l32i.n	a14, a7, 24
4203a9f1:	0497c2        	l16si	a12, a7, 8
4203a9f4:	0397b2        	l16si	a11, a7, 6
4203a9f7:	0297a2        	l16si	a10, a7, 4
4203a9fa:	0fa0f2        	movi	a15, 15
4203a9fd:	1149      	s32i.n	a4, a1, 4
4203a9ff:	0139      	s32i.n	a3, a1, 0
4203aa01:	fdd7e5        	call8	42038780 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
4203aa04:	0add      	mov.n	a13, a10
4203aa06:	faac      	beqz.n	a10, 4203aa39 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203aa08:	4198      	l32i.n	a9, a1, 16
4203aa0a:	24c7a2        	addi	a10, a7, 36
4203aa0d:	098d      	mov.n	a8, a9
4203aa0f:	838a90        	moveqz	a8, a10, a9
4203aa12:	08b8      	l32i.n	a11, a8, 0
4203aa14:	01cd80        	slli	a12, a13, 24
4203aa17:	11bb80        	slli	a11, a11, 8
4203aa1a:	41b8b0        	srli	a11, a11, 8
4203aa1d:	75a8c0        	extui	a10, a12, 24, 8
4203aa20:	ffa0e2        	movi	a14, 255
4203aa23:	20bbc0        	or	a11, a11, a12
4203aa26:	029ae7        	bne	a10, a14, 4203aa2c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x16c>
4203aa29:	007bc6        	j	4203ac1c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x35c>
4203aa2c:	5198      	l32i.n	a9, a1, 20
4203aa2e:	0299e7        	bne	a9, a14, 4203aa34 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
4203aa31:	0108c6        	j	4203ae58 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x598>
4203aa34:	0b2d      	mov.n	a2, a11
4203aa36:	3a6956        	bnez	a9, 4203ade0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x520>
4203aa39:	ffa0a2        	movi	a10, 255
4203aa3c:	0296a7        	bne	a6, a10, 4203aa42 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x182>
4203aa3f:	007246        	j	4203ac0c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x34c>
4203aa42:	36bc      	beqz.n	a6, 4203aa79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203aa44:	759850        	extui	a9, a5, 24, 8
4203aa47:	d18960        	mul16s	a8, a9, a6
4203aa4a:	17b291        	l32r	a9, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203aa4d:	7fc882        	addi	a8, a8, 127
4203aa50:	a28890        	muluh	a8, a8, a9
4203aa53:	115580        	slli	a5, a5, 8
4203aa56:	0188f0        	slli	a8, a8, 17
4203aa59:	759880        	extui	a9, a8, 24, 8
4203aa5c:	098d      	mov.n	a8, a9
4203aa5e:	404800        	ssai	8
4203aa61:	81b950        	src	a11, a9, a5
4203aa64:	0e19a7        	beq	a9, a10, 4203aa76 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b6>
4203aa67:	759820        	extui	a9, a2, 24, 8
4203aa6a:	0299a7        	bne	a9, a10, 4203aa70 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b0>
4203aa6d:	007646        	j	4203ac4a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x38a>
4203aa70:	005816        	beqz	a8, 4203aa79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203aa73:	2b9956        	bnez	a9, 4203ad30 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x470>
4203aa76:	202bb0        	or	a2, a11, a11
4203aa79:	1427a2        	l32i	a10, a7, 80
4203aa7c:	07ca16        	beqz	a10, 4203aafc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203aa7f:	180a82        	l8ui	a8, a10, 24
4203aa82:	19b856        	bnez	a8, 4203ac21 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x361>
4203aa85:	031a82        	l16ui	a8, a10, 6
4203aa88:	021a92        	l16ui	a9, a10, 4
4203aa8b:	c04480        	sub	a4, a4, a8
4203aa8e:	c03390        	sub	a3, a3, a9
4203aa91:	740444        	mul.aa.ll	a4, a4
4203aa94:	780334        	mula.aa.ll	a3, a3
4203aa97:	032a82        	l32i	a8, a10, 12
4203aa9a:	031090        	rsr.acclo	a9
4203aa9d:	c08890        	sub	a8, a8, a9
4203aaa0:	2218a6        	blti	a8, 1, 4203aac6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x206>
4203aaa3:	049a92        	l16si	a9, a10, 8
4203aaa6:	0219e6        	bgei	a9, 1, 4203aaac <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1ec>
4203aaa9:	0065c6        	j	4203ac44 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x384>
4203aaac:	80b990        	add	a11, a9, a9
4203aaaf:	0228b7        	blt	a8, a11, 4203aab5 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1f5>
4203aab2:	0062c6        	j	4203ac41 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x381>
4203aab5:	4ab8      	l32i.n	a11, a10, 16
4203aab7:	5aa8      	l32i.n	a10, a10, 20
4203aab9:	60a0c2        	movi	a12, 96
4203aabc:	41d9      	s32i.n	a13, a1, 16
4203aabe:	4c6225        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203aac1:	41d8      	l32i.n	a13, a1, 16
4203aac3:	000046        	j	4203aac8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x208>
4203aac6:	5aa8      	l32i.n	a10, a10, 20
4203aac8:	7588a0        	extui	a8, a10, 24, 8
4203aacb:	179291        	l32r	a9, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203aace:	d188d0        	mul16s	a8, a8, a13
4203aad1:	11aa80        	slli	a10, a10, 8
4203aad4:	a28890        	muluh	a8, a8, a9
4203aad7:	ffa0c2        	movi	a12, 255
4203aada:	0188f0        	slli	a8, a8, 17
4203aadd:	759880        	extui	a9, a8, 24, 8
4203aae0:	098d      	mov.n	a8, a9
4203aae2:	404800        	ssai	8
4203aae5:	81b9a0        	src	a11, a9, a10
4203aae8:	0e19c7        	beq	a9, a12, 4203aafa <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23a>
4203aaeb:	759820        	extui	a9, a2, 24, 8
4203aaee:	0299c7        	bne	a9, a12, 4203aaf4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x234>
4203aaf1:	004f46        	j	4203ac32 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x372>
4203aaf4:	004816        	beqz	a8, 4203aafc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203aaf7:	18d956        	bnez	a9, 4203ac88 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x3c8>
4203aafa:	0b2d      	mov.n	a2, a11
4203aafc:	f01d      	retw.n
4203aafe:	1d07e2        	l8ui	a14, a7, 29
4203ab01:	043e16        	beqz	a14, 4203ab48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x288>
4203ab04:	c0f9e0        	sub	a15, a9, a14
4203ab07:	3d23f7        	blt	a3, a15, 4203ab48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x288>
4203ab0a:	fdea      	add.n	a15, a13, a14
4203ab0c:	382f47        	blt	a15, a4, 4203ab48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x288>
4203ab0f:	61a8      	l32i.n	a10, a1, 24
4203ab11:	98ea      	add.n	a9, a8, a14
4203ab13:	993a      	add.n	a9, a9, a3
4203ab15:	c08e40        	sub	a8, a14, a4
4203ab18:	8a8a      	add.n	a8, a10, a8
4203ab1a:	c099c0        	sub	a9, a9, a12
4203ab1d:	f49090        	extui	a9, a9, 0, 16
4203ab20:	f48080        	extui	a8, a8, 0, 16
4203ab23:	20a980        	or	a10, a9, a8
4203ab26:	23aa80        	sext	a10, a10, 15
4203ab29:	33cad6        	bgez	a10, 4203ae69 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5a9>
4203ab2c:	c09e90        	sub	a9, a14, a9
4203ab2f:	40f9a0        	nsau	a10, a9
4203ab32:	41a5a0        	srli	a10, a10, 5
4203ab35:	c08e80        	sub	a8, a14, a8
4203ab38:	698c      	beqz.n	a9, 4203ab42 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203ab3a:	190c      	movi.n	a9, 1
4203ab3c:	939a80        	movnez	a9, a10, a8
4203ab3f:	e7d916        	beqz	a9, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203ab42:	5198      	l32i.n	a9, a1, 20
4203ab44:	ff9a86        	j	4203a9b2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf2>
4203ab47:	07e200        	ee.stf.64.xp	f14, f0, a0, a2
4203ab4a:	042e161e 	ee.vmulas.s16.accx.ld.ip.qup	q1, a1, 0x160, q0, q2, q2, q6
4203ab4e:	c0f9e0        	sub	a15, a9, a14
4203ab51:	3c23f7        	blt	a3, a15, 4203ab91 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d1>
4203ab54:	c0fae0        	sub	a15, a10, a14
4203ab57:	3624f7        	blt	a4, a15, 4203ab91 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d1>
4203ab5a:	88ea      	add.n	a8, a8, a14
4203ab5c:	c09820        	sub	a9, a8, a2
4203ab5f:	c0c8c0        	sub	a12, a8, a12
4203ab62:	8c3a      	add.n	a8, a12, a3
4203ab64:	994a      	add.n	a9, a9, a4
4203ab66:	f48080        	extui	a8, a8, 0, 16
4203ab69:	f49090        	extui	a9, a9, 0, 16
4203ab6c:	20a890        	or	a10, a8, a9
4203ab6f:	23aa80        	sext	a10, a10, 15
4203ab72:	2fead6        	bgez	a10, 4203ae74 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5b4>
4203ab75:	c08e80        	sub	a8, a14, a8
4203ab78:	40f8a0        	nsau	a10, a8
4203ab7b:	41a5a0        	srli	a10, a10, 5
4203ab7e:	fc0816        	beqz	a8, 4203ab42 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203ab81:	c09e90        	sub	a9, a14, a9
4203ab84:	180c      	movi.n	a8, 1
4203ab86:	938a90        	movnez	a8, a10, a9
4203ab89:	e33816        	beqz	a8, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203ab8c:	5198      	l32i.n	a9, a1, 20
4203ab8e:	ff8806        	j	4203a9b2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf2>
4203ab91:	1f07c2        	l8ui	a12, a7, 31
4203ab94:	6cbc      	beqz.n	a12, 4203abce <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30e>
4203ab96:	ebca      	add.n	a14, a11, a12
4203ab98:	322e37        	blt	a14, a3, 4203abce <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30e>
4203ab9b:	c0eac0        	sub	a14, a10, a12
4203ab9e:	2c24e7        	blt	a4, a14, 4203abce <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30e>
4203aba1:	88ca      	add.n	a8, a8, a12
4203aba3:	c09c30        	sub	a9, a12, a3
4203aba6:	884a      	add.n	a8, a8, a4
4203aba8:	969a      	add.n	a9, a6, a9
4203abaa:	c08820        	sub	a8, a8, a2
4203abad:	f49090        	extui	a9, a9, 0, 16
4203abb0:	f48080        	extui	a8, a8, 0, 16
4203abb3:	20a980        	or	a10, a9, a8
4203abb6:	23aa80        	sext	a10, a10, 15
4203abb9:	2c0ad6        	bgez	a10, 4203ae7d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5bd>
4203abbc:	c09c90        	sub	a9, a12, a9
4203abbf:	40f9a0        	nsau	a10, a9
4203abc2:	41a5a0        	srli	a10, a10, 5
4203abc5:	f79916        	beqz	a9, 4203ab42 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203abc8:	c08c80        	sub	a8, a12, a8
4203abcb:	ffdac6        	j	4203ab3a <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x27a>
4203abce:	51c8      	l32i.n	a12, a1, 20
4203abd0:	e80c      	movi.n	a8, 14
4203abd2:	02b8c7        	bgeu	a8, a12, 4203abd8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x318>
4203abd5:	ff79c6        	j	4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203abd8:	c0bb30        	sub	a11, a11, a3
4203abdb:	40fbc0        	nsau	a12, a11
4203abde:	180c      	movi.n	a8, 1
4203abe0:	41c5c0        	srli	a12, a12, 5
4203abe3:	7b9c      	beqz.n	a11, 4203abfe <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x33e>
4203abe5:	c09930        	sub	a9, a9, a3
4203abe8:	83c890        	moveqz	a12, a8, a9
4203abeb:	fccc      	bnez.n	a12, 4203abfe <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x33e>
4203abed:	c0dd40        	sub	a13, a13, a4
4203abf0:	83c8d0        	moveqz	a12, a8, a13
4203abf3:	7ccc      	bnez.n	a12, 4203abfe <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x33e>
4203abf5:	c0aa40        	sub	a10, a10, a4
4203abf8:	938ca0        	movnez	a8, a12, a10
4203abfb:	dc1816        	beqz	a8, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203abfe:	5198      	l32i.n	a9, a1, 20
4203ac00:	1189c0        	slli	a8, a9, 4
4203ac03:	898a      	add.n	a8, a9, a8
4203ac05:	746080        	extui	a6, a8, 0, 8
4203ac08:	ff7186        	j	4203a9d2 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x112>
4203ac0b:	412100        	srli	a2, a0, 1
4203ac0e:	27a217        	bge	a2, a1, 4203ac39 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x379>
4203ac11:	14          	.byte	0x14
4203ac12:	202520        	or	a2, a5, a2
4203ac15:	e66a56        	bnez	a10, 4203aa7f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1bf>
4203ac18:	ffb806        	j	4203aafc <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x23c>
4203ac1b:	00          	.byte	00
4203ac1c:	0b2d      	mov.n	a2, a11
4203ac1e:	ff85c6        	j	4203aa39 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203ac21:	20c440        	or	a12, a4, a4
4203ac24:	20b330        	or	a11, a3, a3
4203ac27:	0461d2        	s32i	a13, a1, 16
4203ac2a:	ec2be5        	call8	42026ee8 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
4203ac2d:	41d8      	l32i.n	a13, a1, 16
4203ac2f:	ffa546        	j	4203aac8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x208>
4203ac32:	20a220        	or	a10, a2, a2
4203ac35:	4dab65        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203ac38:	202aa0        	or	a2, a10, a10
4203ac3b:	000090        	retw
4203ac3e:	000000        	ill
4203ac41:	171966        	bnei	a9, 1, 4203ac5c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x39c>
4203ac44:	4aa8      	l32i.n	a10, a10, 16
4203ac46:	ff9f86        	j	4203aac8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x208>
4203ac49:	a22000        	muluh	a2, a0, a0
4203ac4c:	61d220        	xsr.excsave2	a2
4203ac4f:	a9a504        	ee.vld.l.64.ip	q5, a0, 0x128
4203ac52:	4d          	.byte	0x4d
4203ac53:	0421d2        	l32i	a13, a1, 16
4203ac56:	0a2d      	mov.n	a2, a10
4203ac58:	ff8746        	j	4203aa79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203ac5b:	00          	.byte	00
4203ac5c:	ffc9c2        	addi	a12, a9, -1
4203ac5f:	23cc80        	sext	a12, a12, 15
4203ac62:	11cce0        	slli	a12, a12, 2
4203ac65:	022c87        	blt	a12, a8, 4203ac6b <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x3ab>
4203ac68:	002c86        	j	4203ad1e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x45e>
4203ac6b:	d52926        	beqi	a9, 2, 4203ac44 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x384>
4203ac6e:	80bb90        	add	a11, a11, a9
4203ac71:	80bbb0        	add	a11, a11, a11
4203ac74:	f8cbb2        	addi	a11, a11, -8
4203ac77:	c9a8b7        	bge	a8, a11, 4203ac44 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x384>
4203ac7a:	4ab8      	l32i.n	a11, a10, 16
4203ac7c:	5aa8      	l32i.n	a10, a10, 20
4203ac7e:	0c2c      	movi.n	a12, 32
4203ac80:	41d9      	s32i.n	a13, a1, 16
4203ac82:	4c45e5        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203ac85:	ff8e06        	j	4203aac1 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x201>
4203ac88:	82a890        	mull	a10, a8, a9
4203ac8b:	80a0d2        	movi	a13, 128
4203ac8e:	aada      	add.n	a10, a10, a13
4203ac90:	f4a0a0        	extui	a10, a10, 0, 16
4203ac93:	11ea80        	slli	a14, a10, 8
4203ac96:	aeaa      	add.n	a10, a14, a10
4203ac98:	989a      	add.n	a9, a8, a9
4203ac9a:	31a0a0        	srai	a10, a10, 16
4203ac9d:	c099a0        	sub	a9, a9, a10
4203aca0:	11f880        	slli	a15, a8, 8
4203aca3:	f49090        	extui	a9, a9, 0, 16
4203aca6:	c0ff80        	sub	a15, a15, a8
4203aca9:	d2ff90        	quos	a15, a15, a9
4203acac:	f4f0f0        	extui	a15, a15, 0, 16
4203acaf:	c08cf0        	sub	a8, a12, a15
4203acb2:	75e0b0        	extui	a14, a11, 16, 8
4203acb5:	f48080        	extui	a8, a8, 0, 16
4203acb8:	c1eef0        	mul16u	a14, a14, a15
4203acbb:	757020        	extui	a7, a2, 16, 8
4203acbe:	74c8b0        	extui	a12, a11, 8, 8
4203acc1:	c17780        	mul16u	a7, a7, a8
4203acc4:	c1ccf0        	mul16u	a12, a12, a15
4203acc7:	746820        	extui	a6, a2, 8, 8
4203acca:	74a0b0        	extui	a10, a11, 0, 8
4203accd:	c1aaf0        	mul16u	a10, a10, a15
4203acd0:	eeda      	add.n	a14, a14, a13
4203acd2:	c1f680        	mul16u	a15, a6, a8
4203acd5:	742020        	extui	a2, a2, 0, 8
4203acd8:	c12280        	mul16u	a2, a2, a8
4203acdb:	bcda      	add.n	a11, a12, a13
4203acdd:	ee7a      	add.n	a14, a14, a7
4203acdf:	f4c0e0        	extui	a12, a14, 0, 16
4203ace2:	aada      	add.n	a10, a10, a13
4203ace4:	bbfa      	add.n	a11, a11, a15
4203ace6:	f4b0b0        	extui	a11, a11, 0, 16
4203ace9:	118c80        	slli	a8, a12, 8
4203acec:	aa2a      	add.n	a10, a10, a2
4203acee:	f4a0a0        	extui	a10, a10, 0, 16
4203acf1:	88ca      	add.n	a8, a8, a12
4203acf3:	112b80        	slli	a2, a11, 8
4203acf6:	11ca80        	slli	a12, a10, 8
4203acf9:	22ba      	add.n	a2, a2, a11
4203acfb:	758080        	extui	a8, a8, 16, 8
4203acfe:	019980        	slli	a9, a9, 24
4203ad01:	acaa      	add.n	a10, a12, a10
4203ad03:	118800        	slli	a8, a8, 16
4203ad06:	412820        	srli	a2, a2, 8
4203ad09:	208890        	or	a8, a8, a9
4203ad0c:	742820        	extui	a2, a2, 8, 8
4203ad0f:	7590a0        	extui	a9, a10, 16, 8
4203ad12:	208890        	or	a8, a8, a9
4203ad15:	112280        	slli	a2, a2, 8
4203ad18:	202820        	or	a2, a8, a2
4203ad1b:	f01d      	retw.n
4203ad1d:	2ab200        	mul.s	f11, f2, f0
4203ad20:	04          	.byte	0x4
4203ad21:	052aa2        	l32i	a10, a10, 20
4203ad24:	40a0c2        	movi	a12, 64
4203ad27:	41d9      	s32i.n	a13, a1, 16
4203ad29:	4c3b65        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203ad2c:	ff6446        	j	4203aac1 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x201>
4203ad2f:	00          	.byte	00
4203ad30:	82e890        	mull	a14, a8, a9
4203ad33:	80a0c2        	movi	a12, 128
4203ad36:	eeca      	add.n	a14, a14, a12
4203ad38:	f4e0e0        	extui	a14, a14, 0, 16
4203ad3b:	11fe80        	slli	a15, a14, 8
4203ad3e:	efea      	add.n	a14, a15, a14
4203ad40:	989a      	add.n	a9, a8, a9
4203ad42:	31e0e0        	srai	a14, a14, 16
4203ad45:	c099e0        	sub	a9, a9, a14
4203ad48:	11f880        	slli	a15, a8, 8
4203ad4b:	f49090        	extui	a9, a9, 0, 16
4203ad4e:	c0ff80        	sub	a15, a15, a8
4203ad51:	d2ff90        	quos	a15, a15, a9
4203ad54:	f4f0f0        	extui	a15, a15, 0, 16
4203ad57:	75e0b0        	extui	a14, a11, 16, 8
4203ad5a:	c0aaf0        	sub	a10, a10, a15
4203ad5d:	f4a0a0        	extui	a10, a10, 0, 16
4203ad60:	755020        	extui	a5, a2, 16, 8
4203ad63:	7488b0        	extui	a8, a11, 8, 8
4203ad66:	c1eef0        	mul16u	a14, a14, a15
4203ad69:	c155a0        	mul16u	a5, a5, a10
4203ad6c:	746820        	extui	a6, a2, 8, 8
4203ad6f:	c188f0        	mul16u	a8, a8, a15
4203ad72:	74b0b0        	extui	a11, a11, 0, 8
4203ad75:	c166a0        	mul16u	a6, a6, a10
4203ad78:	eeca      	add.n	a14, a14, a12
4203ad7a:	c1bbf0        	mul16u	a11, a11, a15
4203ad7d:	742020        	extui	a2, a2, 0, 8
4203ad80:	c122a0        	mul16u	a2, a2, a10
4203ad83:	88ca      	add.n	a8, a8, a12
4203ad85:	ee5a      	add.n	a14, a14, a5
4203ad87:	bbca      	add.n	a11, a11, a12
4203ad89:	f4e0e0        	extui	a14, a14, 0, 16
4203ad8c:	c86a      	add.n	a12, a8, a6
4203ad8e:	ab2a      	add.n	a10, a11, a2
4203ad90:	f4c0c0        	extui	a12, a12, 0, 16
4203ad93:	118e80        	slli	a8, a14, 8
4203ad96:	f4a0a0        	extui	a10, a10, 0, 16
4203ad99:	88ea      	add.n	a8, a8, a14
4203ad9b:	112c80        	slli	a2, a12, 8
4203ad9e:	758080        	extui	a8, a8, 16, 8
4203ada1:	11ba80        	slli	a11, a10, 8
4203ada4:	22ca      	add.n	a2, a2, a12
4203ada6:	019980        	slli	a9, a9, 24
4203ada9:	118800        	slli	a8, a8, 16
4203adac:	abaa      	add.n	a10, a11, a10
4203adae:	412820        	srli	a2, a2, 8
4203adb1:	208890        	or	a8, a8, a9
4203adb4:	742820        	extui	a2, a2, 8, 8
4203adb7:	7590a0        	extui	a9, a10, 16, 8
4203adba:	208890        	or	a8, a8, a9
4203adbd:	112280        	slli	a2, a2, 8
4203adc0:	202820        	or	a2, a8, a2
4203adc3:	ff2c86        	j	4203aa79 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1b9>
4203adc6:	71f8      	l32i.n	a15, a1, 28
4203adc8:	c0aea0        	sub	a10, a14, a10
4203adcb:	190c      	movi.n	a9, 1
4203adcd:	83f9a0        	moveqz	a15, a9, a10
4203add0:	d6ef56        	bnez	a15, 4203ab42 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203add3:	c08e80        	sub	a8, a14, a8
4203add6:	939f80        	movnez	a9, a15, a8
4203add9:	be3916        	beqz	a9, 4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203addc:	ff5886        	j	4203ab42 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x282>
4203addf:	ca9000        	float.s	f9, a0, 0
4203ade2:	a0b282        	l32ai	a8, a2, 0x280
4203ade5:	80          	.byte	0x80
4203ade6:	ccba      	add.n	a12, a12, a11
4203ade8:	f4c0c0        	extui	a12, a12, 0, 16
4203adeb:	11ec80        	slli	a14, a12, 8
4203adee:	ceca      	add.n	a12, a14, a12
4203adf0:	9a9a      	add.n	a9, a10, a9
4203adf2:	31c0c0        	srai	a12, a12, 16
4203adf5:	c099c0        	sub	a9, a9, a12
4203adf8:	11ca80        	slli	a12, a10, 8
4203adfb:	f49090        	extui	a9, a9, 0, 16
4203adfe:	c0aca0        	sub	a10, a12, a10
4203ae01:	d2aa90        	quos	a10, a10, a9
4203ae04:	f4a0a0        	extui	a10, a10, 0, 16
4203ae07:	75c020        	extui	a12, a2, 16, 8
4203ae0a:	c1cca0        	mul16u	a12, a12, a10
4203ae0d:	74f820        	extui	a15, a2, 8, 8
4203ae10:	c1ffa0        	mul16u	a15, a15, a10
4203ae13:	74e020        	extui	a14, a2, 0, 8
4203ae16:	c1eea0        	mul16u	a14, a14, a10
4203ae19:	ccba      	add.n	a12, a12, a11
4203ae1b:	f4c0c0        	extui	a12, a12, 0, 16
4203ae1e:	ffba      	add.n	a15, a15, a11
4203ae20:	f4f0f0        	extui	a15, a15, 0, 16
4203ae23:	11ac80        	slli	a10, a12, 8
4203ae26:	eeba      	add.n	a14, a14, a11
4203ae28:	f4e0e0        	extui	a14, a14, 0, 16
4203ae2b:	aaca      	add.n	a10, a10, a12
4203ae2d:	11bf80        	slli	a11, a15, 8
4203ae30:	bbfa      	add.n	a11, a11, a15
4203ae32:	75a0a0        	extui	a10, a10, 16, 8
4203ae35:	11ce80        	slli	a12, a14, 8
4203ae38:	01f980        	slli	a15, a9, 24
4203ae3b:	11aa00        	slli	a10, a10, 16
4203ae3e:	4198b0        	srli	a9, a11, 8
4203ae41:	ccea      	add.n	a12, a12, a14
4203ae43:	20aaf0        	or	a10, a10, a15
4203ae46:	75b0c0        	extui	a11, a12, 16, 8
4203ae49:	749890        	extui	a9, a9, 8, 8
4203ae4c:	20aab0        	or	a10, a10, a11
4203ae4f:	119980        	slli	a9, a9, 8
4203ae52:	202a90        	or	a2, a10, a9
4203ae55:	fef806        	j	4203aa39 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203ae58:	20a220        	or	a10, a2, a2
4203ae5b:	0461d2        	s32i	a13, a1, 16
4203ae5e:	4d88e5        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203ae61:	0421d2        	l32i	a13, a1, 16
4203ae64:	0a2d      	mov.n	a2, a10
4203ae66:	fef3c6        	j	4203aa39 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x179>
4203ae69:	740884        	mul.aa.ll	a8, a8
4203ae6c:	780994        	mula.aa.ll	a9, a9
4203ae6f:	feb806        	j	4203a953 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x93>
4203ae72:	940000        	extui	a0, a0, 0, 10
4203ae75:	7409      	s32i.n	a0, a4, 28
4203ae77:	780884        	mula.aa.ll	a8, a8
4203ae7a:	feb546        	j	4203a953 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x93>
4203ae7d:	740884        	mul.aa.ll	a8, a8
4203ae80:	780994        	mula.aa.ll	a9, a9
4203ae83:	031080        	rsr.acclo	a8
4203ae86:	118880        	slli	a8, a8, 8
4203ae89:	0248e6        	bgei	a8, 4, 4203ae8f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x5cf>
4203ae8c:	fecc06        	j	4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203ae8f:	40f890        	nsau	a9, a8
4203ae92:	dfc992        	addi	a9, a9, -33
4203ae95:	609090        	neg	a9, a9
4203ae98:	219190        	srai	a9, a9, 1
4203ae9b:	fec992        	addi	a9, a9, -2
4203ae9e:	3a0c      	movi.n	a10, 3
4203aea0:	401900        	ssl	a9
4203aea3:	a1aa00        	sll	a10, a10
4203aea6:	c298a0        	quou	a9, a8, a10
4203aea9:	99aa      	add.n	a9, a9, a10
4203aeab:	419190        	srli	a9, a9, 1
4203aeae:	c2a890        	quou	a10, a8, a9
4203aeb1:	aa9a      	add.n	a10, a10, a9
4203aeb3:	41a1a0        	srli	a10, a10, 1
4203aeb6:	c298a0        	quou	a9, a8, a10
4203aeb9:	99aa      	add.n	a9, a9, a10
4203aebb:	1693a1        	l32r	a10, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203aebe:	419190        	srli	a9, a9, 1
4203aec1:	0299a7        	bne	a9, a10, 4203aec7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x607>
4203aec4:	febe06        	j	4203a9c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x100>
4203aec7:	82a990        	mull	a10, a9, a9
4203aeca:	11ccc0        	slli	a12, a12, 4
4203aecd:	6288a0        	saltu	a8, a8, a10
4203aed0:	c09980        	sub	a9, a9, a8
4203aed3:	5188      	l32i.n	a8, a1, 20
4203aed5:	88ca      	add.n	a8, a8, a12
4203aed7:	c08890        	sub	a8, a8, a9
4203aeda:	feb186        	j	4203a9a4 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xe4>


/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039de4 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42039de4:	00c136        	entry	a1, 96
42039de7:	850b      	addi.n	a8, a5, -1
42039de9:	a189      	s32i.n	a8, a1, 40
42039deb:	3cc282        	addi	a8, a2, 60
42039dee:	6149      	s32i.n	a4, a1, 24
42039df0:	b169      	s32i.n	a6, a1, 44
42039df2:	7139      	s32i.n	a3, a1, 28
42039df4:	d189      	s32i.n	a8, a1, 52
42039df6:	252516        	beqz	a5, 4203a04c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x268>
42039df9:	61b8      	l32i.n	a11, a1, 24
42039dfb:	71a8      	l32i.n	a10, a1, 28
42039dfd:	200292        	l8ui	a9, a2, 32
42039e00:	021282        	l16ui	a8, a2, 4
42039e03:	001bc2        	l16ui	a12, a11, 0
42039e06:	001aa2        	l16ui	a10, a10, 0
42039e09:	0412e2        	l16ui	a14, a2, 8
42039e0c:	52f8      	l32i.n	a15, a2, 20
42039e0e:	898a      	add.n	a8, a9, a8
42039e10:	c0be90        	sub	a11, a14, a9
42039e13:	23d880        	sext	a13, a8, 15
42039e16:	1051a2        	s16i	a10, a1, 32
42039e19:	237a80        	sext	a7, a10, 15
42039e1c:	c1f9      	s32i.n	a15, a1, 48
42039e1e:	1251c2        	s16i	a12, a1, 36
42039e21:	233c80        	sext	a3, a12, 15
42039e24:	f48080        	extui	a8, a8, 0, 16
42039e27:	23bb80        	sext	a11, a11, 15
42039e2a:	02a7d7        	bge	a7, a13, 42039e30 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4c>
42039e2d:	008ac6        	j	4203a05c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e30:	02ab77        	bge	a11, a7, 42039e36 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x52>
42039e33:	008946        	j	4203a05c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e36:	0312a2        	l16ui	a10, a2, 6
42039e39:	051252        	l16ui	a5, a2, 10
42039e3c:	a9aa      	add.n	a10, a9, a10
42039e3e:	c0c590        	sub	a12, a5, a9
42039e41:	23fa80        	sext	a15, a10, 15
42039e44:	23cc80        	sext	a12, a12, 15
42039e47:	f4a0a0        	extui	a10, a10, 0, 16
42039e4a:	02a3f7        	bge	a3, a15, 42039e50 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x6c>
42039e4d:	0082c6        	j	4203a05c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e50:	724c30        	salt	a4, a12, a3
42039e53:	f149      	s32i.n	a4, a1, 60
42039e55:	02ac37        	bge	a12, a3, 42039e5b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x77>
42039e58:	008006        	j	4203a05c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
42039e5b:	210242        	l8ui	a4, a2, 33
42039e5e:	1c0262        	l8ui	a6, a2, 28
42039e61:	e149      	s32i.n	a4, a1, 56
42039e63:	00e616        	beqz	a6, 42039e75 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x91>
42039e66:	804d60        	add	a4, a13, a6
42039e69:	082477        	blt	a4, a7, 42039e75 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x91>
42039e6c:	804f60        	add	a4, a15, a6
42039e6f:	022437        	blt	a4, a3, 42039e75 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x91>
42039e72:	00b3c6        	j	4203a145 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x361>
42039e75:	1d0262        	l8ui	a6, a2, 29
42039e78:	d68c      	beqz.n	a6, 42039e89 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa5>
42039e7a:	c04b60        	sub	a4, a11, a6
42039e7d:	082747        	blt	a7, a4, 42039e89 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa5>
42039e80:	804f60        	add	a4, a15, a6
42039e83:	022437        	blt	a4, a3, 42039e89 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa5>
42039e86:	00eb06        	j	4203a236 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x452>
42039e89:	1e02a2        	l8ui	a10, a2, 30
42039e8c:	da8c      	beqz.n	a10, 42039e9d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb9>
42039e8e:	c06ba0        	sub	a6, a11, a10
42039e91:	082767        	blt	a7, a6, 42039e9d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb9>
42039e94:	c06ca0        	sub	a6, a12, a10
42039e97:	022367        	blt	a3, a6, 42039e9d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb9>
42039e9a:	00f486        	j	4203a270 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x48c>
42039e9d:	1f02a2        	l8ui	a10, a2, 31
42039ea0:	da8c      	beqz.n	a10, 42039eb1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xcd>
42039ea2:	80eda0        	add	a14, a13, a10
42039ea5:	082e77        	blt	a14, a7, 42039eb1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xcd>
42039ea8:	c0eca0        	sub	a14, a12, a10
42039eab:	0223e7        	blt	a3, a14, 42039eb1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xcd>
42039eae:	010f86        	j	4203a2f0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x50c>
42039eb1:	e198      	l32i.n	a9, a1, 56
42039eb3:	e80c      	movi.n	a8, 14
42039eb5:	283897        	bltu	a8, a9, 42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
42039eb8:	c0d7d0        	sub	a13, a7, a13
42039ebb:	40fd80        	nsau	a8, a13
42039ebe:	190c      	movi.n	a9, 1
42039ec0:	418580        	srli	a8, a8, 5
42039ec3:	187d16        	beqz	a13, 4203a04e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ec6:	c0b7b0        	sub	a11, a7, a11
42039ec9:	8389b0        	moveqz	a8, a9, a11
42039ecc:	17e856        	bnez	a8, 4203a04e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ecf:	c0f3f0        	sub	a15, a3, a15
42039ed2:	8389f0        	moveqz	a8, a9, a15
42039ed5:	175856        	bnez	a8, 4203a04e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ed8:	c0c3c0        	sub	a12, a3, a12
42039edb:	9398c0        	movnez	a9, a8, a12
42039ede:	16c956        	bnez	a9, 4203a04e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x26a>
42039ee1:	142282        	l32i	a8, a2, 80
42039ee4:	c168      	l32i.n	a6, a1, 48
42039ee6:	142816        	beqz	a8, 4203a02c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
42039ee9:	ffa052        	movi	a5, 255
42039eec:	d1b8      	l32i.n	a11, a1, 52
42039eee:	03dd      	mov.n	a13, a3
42039ef0:	07cd      	mov.n	a12, a7
42039ef2:	28c2a2        	addi	a10, a2, 40
42039ef5:	0121e5        	call8	4203b114 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42039ef8:	200282        	l8ui	a8, a2, 32
42039efb:	0a4d      	mov.n	a4, a10
42039efd:	016a80        	slli	a6, a10, 24
42039f00:	05ed      	mov.n	a14, a5
42039f02:	042816        	beqz	a8, 42039f48 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
42039f05:	62e8      	l32i.n	a14, a2, 24
42039f07:	0592d2        	l16si	a13, a2, 10
42039f0a:	0492c2        	l16si	a12, a2, 8
42039f0d:	0392b2        	l16si	a11, a2, 6
42039f10:	0292a2        	l16si	a10, a2, 4
42039f13:	ff0c      	movi.n	a15, 15
42039f15:	1139      	s32i.n	a3, a1, 4
42039f17:	0179      	s32i.n	a7, a1, 0
42039f19:	fe8665        	call8	42038780 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42039f1c:	20eaa0        	or	a14, a10, a10
42039f1f:	5aac      	beqz.n	a10, 42039f48 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
42039f21:	0922b2        	l32i	a11, a2, 36
42039f24:	019a80        	slli	a9, a10, 24
42039f27:	11bb80        	slli	a11, a11, 8
42039f2a:	41b8b0        	srli	a11, a11, 8
42039f2d:	758890        	extui	a8, a9, 24, 8
42039f30:	ffa0a2        	movi	a10, 255
42039f33:	20bb90        	or	a11, a11, a9
42039f36:	0298a7        	bne	a8, a10, 42039f3c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x158>
42039f39:	0054c6        	j	4203a090 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2ac>
42039f3c:	0294a7        	bne	a4, a10, 42039f42 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x15e>
42039f3f:	010b46        	j	4203a370 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x58c>
42039f42:	206bb0        	or	a6, a11, a11
42039f45:	4d0456        	bnez	a4, 4203a419 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x635>
42039f48:	ffa092        	movi	a9, 255
42039f4b:	029597        	bne	a5, a9, 42039f51 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x16d>
42039f4e:	0043c6        	j	4203a061 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x27d>
42039f51:	25bc      	beqz.n	a5, 42039f87 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
42039f53:	c1b8      	l32i.n	a11, a1, 48
42039f55:	1a6fa1        	l32r	a10, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
42039f58:	7588b0        	extui	a8, a11, 24, 8
42039f5b:	d18850        	mul16s	a8, a8, a5
42039f5e:	11bb80        	slli	a11, a11, 8
42039f61:	7fc882        	addi	a8, a8, 127
42039f64:	a288a0        	muluh	a8, a8, a10
42039f67:	0188f0        	slli	a8, a8, 17
42039f6a:	75a880        	extui	a10, a8, 24, 8
42039f6d:	0a8d      	mov.n	a8, a10
42039f6f:	404800        	ssai	8
42039f72:	81bab0        	src	a11, a10, a11
42039f75:	029a97        	bne	a10, a9, 42039f7b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x197>
42039f78:	004006        	j	4203a07c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x298>
42039f7b:	75a860        	extui	a10, a6, 24, 8
42039f7e:	029a97        	bne	a10, a9, 42039f84 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a0>
42039f81:	004546        	j	4203a09a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b6>
42039f84:	124856        	bnez	a8, 4203a0ac <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2c8>
42039f87:	1422a2        	l32i	a10, a2, 80
42039f8a:	09ea16        	beqz	a10, 4203a02c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
42039f8d:	180a82        	l8ui	a8, a10, 24
42039f90:	0d8856        	bnez	a8, 4203a06c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x288>
42039f93:	9198      	l32i.n	a9, a1, 36
42039f95:	031a82        	l16ui	a8, a10, 6
42039f98:	021ab2        	l16ui	a11, a10, 4
42039f9b:	c08980        	sub	a8, a9, a8
42039f9e:	740884        	mul.aa.ll	a8, a8
42039fa1:	8188      	l32i.n	a8, a1, 32
42039fa3:	3a98      	l32i.n	a9, a10, 12
42039fa5:	c088b0        	sub	a8, a8, a11
42039fa8:	780884        	mula.aa.ll	a8, a8
42039fab:	031080        	rsr.acclo	a8
42039fae:	c08980        	sub	a8, a9, a8
42039fb1:	0218e6        	bgei	a8, 1, 42039fb7 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d3>
42039fb4:	003246        	j	4203a081 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x29d>
42039fb7:	049a92        	l16si	a9, a10, 8
42039fba:	0219e6        	bgei	a9, 1, 42039fc0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1dc>
42039fbd:	003506        	j	4203a095 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fc0:	b99a      	add.n	a11, a9, a9
42039fc2:	02a8b7        	bge	a8, a11, 42039fc8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e4>
42039fc5:	007146        	j	4203a18e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3aa>
42039fc8:	021966        	bnei	a9, 1, 42039fce <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1ea>
42039fcb:	003186        	j	4203a095 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fce:	c90b      	addi.n	a12, a9, -1
42039fd0:	23cc80        	sext	a12, a12, 15
42039fd3:	11cce0        	slli	a12, a12, 2
42039fd6:	022c87        	blt	a12, a8, 42039fdc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1f8>
42039fd9:	00e946        	j	4203a382 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x59e>
42039fdc:	022966        	bnei	a9, 2, 42039fe2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1fe>
42039fdf:	002c86        	j	4203a095 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fe2:	bb9a      	add.n	a11, a11, a9
42039fe4:	bbba      	add.n	a11, a11, a11
42039fe6:	f8cbb2        	addi	a11, a11, -8
42039fe9:	0228b7        	blt	a8, a11, 42039fef <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x20b>
42039fec:	002946        	j	4203a095 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2b1>
42039fef:	4ab8      	l32i.n	a11, a10, 16
42039ff1:	5aa8      	l32i.n	a10, a10, 20
42039ff3:	0c2c      	movi.n	a12, 32
42039ff5:	81e9      	s32i.n	a14, a1, 32
42039ff7:	4d0ea5        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039ffa:	81e8      	l32i.n	a14, a1, 32
42039ffc:	7598a0        	extui	a9, a10, 24, 8
42039fff:	1a4581        	l32r	a8, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203a002:	d199e0        	mul16s	a9, a9, a14
4203a005:	11aa80        	slli	a10, a10, 8
4203a008:	a29980        	muluh	a9, a9, a8
4203a00b:	ffa082        	movi	a8, 255
4203a00e:	0199f0        	slli	a9, a9, 17
4203a011:	75b890        	extui	a11, a9, 24, 8
4203a014:	0bcd      	mov.n	a12, a11
4203a016:	404800        	ssai	8
4203a019:	81bba0        	src	a11, a11, a10
4203a01c:	0a1c87        	beq	a12, a8, 4203a02a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x246>
4203a01f:	759860        	extui	a9, a6, 24, 8
4203a022:	601987        	beq	a9, a8, 4203a086 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2a2>
4203a025:	3c8c      	beqz.n	a12, 4203a02c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
4203a027:	175956        	bnez	a9, 4203a1a0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3bc>
4203a02a:	0b6d      	mov.n	a6, a11
4203a02c:	b188      	l32i.n	a8, a1, 44
4203a02e:	a198      	l32i.n	a9, a1, 40
4203a030:	0869      	s32i.n	a6, a8, 0
4203a032:	884b      	addi.n	a8, a8, 4
4203a034:	61a8      	l32i.n	a10, a1, 24
4203a036:	b189      	s32i.n	a8, a1, 44
4203a038:	7188      	l32i.n	a8, a1, 28
4203a03a:	990b      	addi.n	a9, a9, -1
4203a03c:	aa2b      	addi.n	a10, a10, 2
4203a03e:	882b      	addi.n	a8, a8, 2
4203a040:	a199      	s32i.n	a9, a1, 40
4203a042:	61a9      	s32i.n	a10, a1, 24
4203a044:	7189      	s32i.n	a8, a1, 28
4203a046:	020926        	beqi	a9, -1, 4203a04c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x268>
4203a049:	ff6b06        	j	42039df9 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x15>
4203a04c:	f01d      	retw.n
4203a04e:	e198      	l32i.n	a9, a1, 56
4203a050:	1189c0        	slli	a8, a9, 4
4203a053:	598a      	add.n	a5, a9, a8
4203a055:	745050        	extui	a5, a5, 0, 8
4203a058:	ffa406        	j	42039eec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x108>
4203a05b:	050c00        	extui	a0, a0, 28, 1
4203a05e:	ffa286        	j	42039eec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x108>
4203a061:	1a2b61        	l32r	a6, 42000910 <_stext+0x8f0> (ff000000 <_rtc_reserved_end+0x9ef00000>)
4203a064:	c188      	l32i.n	a8, a1, 48
4203a066:	206680        	or	a6, a6, a8
4203a069:	ffc686        	j	42039f87 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a06c:	20c330        	or	a12, a3, a3
4203a06f:	20b770        	or	a11, a7, a7
4203a072:	0861e2        	s32i	a14, a1, 32
4203a075:	ece725        	call8	42026ee8 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
4203a078:	ffdf86        	j	42039ffa <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x216>
4203a07b:	00          	.byte	00
4203a07c:	0b6d      	mov.n	a6, a11
4203a07e:	ffc146        	j	42039f87 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a081:	5aa8      	l32i.n	a10, a10, 20
4203a083:	ffdd46        	j	42039ffc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x218>
4203a086:	06ad      	mov.n	a10, a6
4203a088:	4e6625        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a08b:	0a6d      	mov.n	a6, a10
4203a08d:	ffe6c6        	j	4203a02c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
4203a090:	0b6d      	mov.n	a6, a11
4203a092:	ffac86        	j	42039f48 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
4203a095:	4aa8      	l32i.n	a10, a10, 16
4203a097:	ffd846        	j	42039ffc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x218>
4203a09a:	20a660        	or	a10, a6, a6
4203a09d:	0c61e2        	s32i	a14, a1, 48
4203a0a0:	4e64a5        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a0a3:	0c21e2        	l32i	a14, a1, 48
4203a0a6:	0a6d      	mov.n	a6, a10
4203a0a8:	ffb6c6        	j	42039f87 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a0ab:	ca1600        	float.s	f1, a6, 0
4203a0ae:	a0fc      	bnez.n	a0, 4203a0ec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x308>
4203a0b0:	82c8      	l32i.n	a12, a2, 32
4203a0b2:	80a0f2        	movi	a15, 128
4203a0b5:	ccfa      	add.n	a12, a12, a15
4203a0b7:	f4c0c0        	extui	a12, a12, 0, 16
4203a0ba:	11dc80        	slli	a13, a12, 8
4203a0bd:	cdca      	add.n	a12, a13, a12
4203a0bf:	a8aa      	add.n	a10, a8, a10
4203a0c1:	31c0c0        	srai	a12, a12, 16
4203a0c4:	c0cac0        	sub	a12, a10, a12
4203a0c7:	11a880        	slli	a10, a8, 8
4203a0ca:	f450c0        	extui	a5, a12, 0, 16
4203a0cd:	c08a80        	sub	a8, a10, a8
4203a0d0:	d28850        	quos	a8, a8, a5
4203a0d3:	f48080        	extui	a8, a8, 0, 16
4203a0d6:	75a0b0        	extui	a10, a11, 16, 8
4203a0d9:	c09980        	sub	a9, a9, a8
4203a0dc:	f49090        	extui	a9, a9, 0, 16
4203a0df:	c14a80        	mul16u	a4, a10, a8
4203a0e2:	75d060        	extui	a13, a6, 16, 8
4203a0e5:	74c8b0        	extui	a12, a11, 8, 8
4203a0e8:	c1dd90        	mul16u	a13, a13, a9
4203a0eb:	c1cc80        	mul16u	a12, a12, a8
4203a0ee:	74a860        	extui	a10, a6, 8, 8
4203a0f1:	74b0b0        	extui	a11, a11, 0, 8
4203a0f4:	c1aa90        	mul16u	a10, a10, a9
4203a0f7:	c1bb80        	mul16u	a11, a11, a8
4203a0fa:	746060        	extui	a6, a6, 0, 8
4203a0fd:	84fa      	add.n	a8, a4, a15
4203a0ff:	c19690        	mul16u	a9, a6, a9
4203a102:	88da      	add.n	a8, a8, a13
4203a104:	ccfa      	add.n	a12, a12, a15
4203a106:	f48080        	extui	a8, a8, 0, 16
4203a109:	ccaa      	add.n	a12, a12, a10
4203a10b:	bbfa      	add.n	a11, a11, a15
4203a10d:	f4c0c0        	extui	a12, a12, 0, 16
4203a110:	11a880        	slli	a10, a8, 8
4203a113:	bb9a      	add.n	a11, a11, a9
4203a115:	f4b0b0        	extui	a11, a11, 0, 16
4203a118:	8a8a      	add.n	a8, a10, a8
4203a11a:	119c80        	slli	a9, a12, 8
4203a11d:	99ca      	add.n	a9, a9, a12
4203a11f:	758080        	extui	a8, a8, 16, 8
4203a122:	11ab80        	slli	a10, a11, 8
4203a125:	118800        	slli	a8, a8, 16
4203a128:	01c580        	slli	a12, a5, 24
4203a12b:	aaba      	add.n	a10, a10, a11
4203a12d:	419890        	srli	a9, a9, 8
4203a130:	2068c0        	or	a6, a8, a12
4203a133:	75a0a0        	extui	a10, a10, 16, 8
4203a136:	748890        	extui	a8, a9, 8, 8
4203a139:	2066a0        	or	a6, a6, a10
4203a13c:	118880        	slli	a8, a8, 8
4203a13f:	206680        	or	a6, a6, a8
4203a142:	ff9046        	j	42039f87 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1a3>
4203a145:	986a      	add.n	a9, a8, a6
4203a147:	8188      	l32i.n	a8, a1, 32
4203a149:	aa6a      	add.n	a10, a10, a6
4203a14b:	c09980        	sub	a9, a9, a8
4203a14e:	9188      	l32i.n	a8, a1, 36
4203a150:	f49090        	extui	a9, a9, 0, 16
4203a153:	c08a80        	sub	a8, a10, a8
4203a156:	f48080        	extui	a8, a8, 0, 16
4203a159:	20a980        	or	a10, a9, a8
4203a15c:	23aa80        	sext	a10, a10, 15
4203a15f:	231ad6        	bgez	a10, 4203a394 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5b0>
4203a162:	f148      	l32i.n	a4, a1, 60
4203a164:	c09690        	sub	a9, a6, a9
4203a167:	1a0c      	movi.n	a10, 1
4203a169:	834a90        	moveqz	a4, a10, a9
4203a16c:	008456        	bnez	a4, 4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a16f:	c08680        	sub	a8, a6, a8
4203a172:	93a480        	movnez	a10, a4, a8
4203a175:	d68a16        	beqz	a10, 42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a178:	e198      	l32i.n	a9, a1, 56
4203a17a:	1189c0        	slli	a8, a9, 4
4203a17d:	898a      	add.n	a8, a9, a8
4203a17f:	745080        	extui	a5, a8, 0, 8
4203a182:	ffa082        	movi	a8, 255
4203a185:	029587        	bne	a5, a8, 4203a18b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3a7>
4203a188:	ff5546        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a18b:	ff5746        	j	42039eec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x108>
4203a18e:	042ab2        	l32i	a11, a10, 16
4203a191:	052aa2        	l32i	a10, a10, 20
4203a194:	60a0c2        	movi	a12, 96
4203a197:	0861e2        	s32i	a14, a1, 32
4203a19a:	4cf465        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a19d:	ff9646        	j	42039ffa <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x216>
4203a1a0:	82ac90        	mull	a10, a12, a9
4203a1a3:	80a0e2        	movi	a14, 128
4203a1a6:	aaea      	add.n	a10, a10, a14
4203a1a8:	f4a0a0        	extui	a10, a10, 0, 16
4203a1ab:	11da80        	slli	a13, a10, 8
4203a1ae:	adaa      	add.n	a10, a13, a10
4203a1b0:	9c9a      	add.n	a9, a12, a9
4203a1b2:	31a0a0        	srai	a10, a10, 16
4203a1b5:	c099a0        	sub	a9, a9, a10
4203a1b8:	11dc80        	slli	a13, a12, 8
4203a1bb:	f49090        	extui	a9, a9, 0, 16
4203a1be:	c0ddc0        	sub	a13, a13, a12
4203a1c1:	d2dd90        	quos	a13, a13, a9
4203a1c4:	f4d0d0        	extui	a13, a13, 0, 16
4203a1c7:	c088d0        	sub	a8, a8, a13
4203a1ca:	75c0b0        	extui	a12, a11, 16, 8
4203a1cd:	f48080        	extui	a8, a8, 0, 16
4203a1d0:	c1ccd0        	mul16u	a12, a12, a13
4203a1d3:	757060        	extui	a7, a6, 16, 8
4203a1d6:	74a8b0        	extui	a10, a11, 8, 8
4203a1d9:	c17780        	mul16u	a7, a7, a8
4203a1dc:	c1aad0        	mul16u	a10, a10, a13
4203a1df:	74f860        	extui	a15, a6, 8, 8
4203a1e2:	74b0b0        	extui	a11, a11, 0, 8
4203a1e5:	c1bbd0        	mul16u	a11, a11, a13
4203a1e8:	c1ff80        	mul16u	a15, a15, a8
4203a1eb:	ccea      	add.n	a12, a12, a14
4203a1ed:	746060        	extui	a6, a6, 0, 8
4203a1f0:	c16680        	mul16u	a6, a6, a8
4203a1f3:	aaea      	add.n	a10, a10, a14
4203a1f5:	cc7a      	add.n	a12, a12, a7
4203a1f7:	bbea      	add.n	a11, a11, a14
4203a1f9:	f4c0c0        	extui	a12, a12, 0, 16
4203a1fc:	eafa      	add.n	a14, a10, a15
4203a1fe:	db6a      	add.n	a13, a11, a6
4203a200:	f4e0e0        	extui	a14, a14, 0, 16
4203a203:	118c80        	slli	a8, a12, 8
4203a206:	f4d0d0        	extui	a13, a13, 0, 16
4203a209:	88ca      	add.n	a8, a8, a12
4203a20b:	116e80        	slli	a6, a14, 8
4203a20e:	758080        	extui	a8, a8, 16, 8
4203a211:	11ad80        	slli	a10, a13, 8
4203a214:	66ea      	add.n	a6, a6, a14
4203a216:	01b980        	slli	a11, a9, 24
4203a219:	118800        	slli	a8, a8, 16
4203a21c:	9ada      	add.n	a9, a10, a13
4203a21e:	416860        	srli	a6, a6, 8
4203a221:	2088b0        	or	a8, a8, a11
4203a224:	759090        	extui	a9, a9, 16, 8
4203a227:	746860        	extui	a6, a6, 8, 8
4203a22a:	208890        	or	a8, a8, a9
4203a22d:	116680        	slli	a6, a6, 8
4203a230:	206860        	or	a6, a8, a6
4203a233:	ff7d46        	j	4203a02c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
4203a236:	896a      	add.n	a8, a9, a6
4203a238:	8198      	l32i.n	a9, a1, 32
4203a23a:	c088e0        	sub	a8, a8, a14
4203a23d:	889a      	add.n	a8, a8, a9
4203a23f:	9198      	l32i.n	a9, a1, 36
4203a241:	aa6a      	add.n	a10, a10, a6
4203a243:	c0aa90        	sub	a10, a10, a9
4203a246:	f48080        	extui	a8, a8, 0, 16
4203a249:	f4a0a0        	extui	a10, a10, 0, 16
4203a24c:	2098a0        	or	a9, a8, a10
4203a24f:	239980        	sext	a9, a9, 15
4203a252:	1ba9d6        	bgez	a9, 4203a410 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x62c>
4203a255:	c08680        	sub	a8, a6, a8
4203a258:	40f890        	nsau	a9, a8
4203a25b:	419590        	srli	a9, a9, 5
4203a25e:	f16816        	beqz	a8, 4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a261:	c0a6a0        	sub	a10, a6, a10
4203a264:	180c      	movi.n	a8, 1
4203a266:	9389a0        	movnez	a8, a9, a10
4203a269:	c74816        	beqz	a8, 42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a26c:	ffc206        	j	4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a26f:	818800        	src	a8, a8, a0
4203a272:	99aa      	add.n	a9, a9, a10
4203a274:	c0e9e0        	sub	a14, a9, a14
4203a277:	ee8a      	add.n	a14, a14, a8
4203a279:	9188      	l32i.n	a8, a1, 36
4203a27b:	c09950        	sub	a9, a9, a5
4203a27e:	998a      	add.n	a9, a9, a8
4203a280:	f4e0e0        	extui	a14, a14, 0, 16
4203a283:	f49090        	extui	a9, a9, 0, 16
4203a286:	208e90        	or	a8, a14, a9
4203a289:	238880        	sext	a8, a8, 15
4203a28c:	201896        	bltz	a8, 4203a491 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x6ad>
4203a28f:	740994        	mul.aa.ll	a9, a9
4203a292:	780ee4        	mula.aa.ll	a14, a14
4203a295:	031080        	rsr.acclo	a8
4203a298:	119880        	slli	a9, a8, 8
4203a29b:	0249e6        	bgei	a9, 4, 4203a2a1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4bd>
4203a29e:	ff0fc6        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a2a1:	40f980        	nsau	a8, a9
4203a2a4:	dfc882        	addi	a8, a8, -33
4203a2a7:	608080        	neg	a8, a8
4203a2aa:	218180        	srai	a8, a8, 1
4203a2ad:	3b0c      	movi.n	a11, 3
4203a2af:	fec882        	addi	a8, a8, -2
4203a2b2:	401800        	ssl	a8
4203a2b5:	a18b00        	sll	a8, a11
4203a2b8:	c2b980        	quou	a11, a9, a8
4203a2bb:	bb8a      	add.n	a11, a11, a8
4203a2bd:	41b1b0        	srli	a11, a11, 1
4203a2c0:	c289b0        	quou	a8, a9, a11
4203a2c3:	88ba      	add.n	a8, a8, a11
4203a2c5:	418180        	srli	a8, a8, 1
4203a2c8:	c2b980        	quou	a11, a9, a8
4203a2cb:	bb8a      	add.n	a11, a11, a8
4203a2cd:	198e81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a2d0:	41b1b0        	srli	a11, a11, 1
4203a2d3:	029b87        	bne	a11, a8, 4203a2d9 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4f5>
4203a2d6:	ff01c6        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a2d9:	82cbb0        	mull	a12, a11, a11
4203a2dc:	118ac0        	slli	a8, a10, 4
4203a2df:	6299c0        	saltu	a9, a9, a12
4203a2e2:	c0bb90        	sub	a11, a11, a9
4203a2e5:	e198      	l32i.n	a9, a1, 56
4203a2e7:	898a      	add.n	a8, a9, a8
4203a2e9:	c088b0        	sub	a8, a8, a11
4203a2ec:	004046        	j	4203a3f1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x60d>
4203a2ef:	00          	.byte	00
4203a2f0:	99aa      	add.n	a9, a9, a10
4203a2f2:	b8aa      	add.n	a11, a8, a10
4203a2f4:	c08950        	sub	a8, a9, a5
4203a2f7:	8198      	l32i.n	a9, a1, 32
4203a2f9:	c09b90        	sub	a9, a11, a9
4203a2fc:	91b8      	l32i.n	a11, a1, 36
4203a2fe:	f49090        	extui	a9, a9, 0, 16
4203a301:	88ba      	add.n	a8, a8, a11
4203a303:	f48080        	extui	a8, a8, 0, 16
4203a306:	20b980        	or	a11, a9, a8
4203a309:	23bb80        	sext	a11, a11, 15
4203a30c:	19cb96        	bltz	a11, 4203a4ac <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x6c8>
4203a30f:	740884        	mul.aa.ll	a8, a8
4203a312:	780994        	mula.aa.ll	a9, a9
4203a315:	031080        	rsr.acclo	a8
4203a318:	118880        	slli	a8, a8, 8
4203a31b:	0248e6        	bgei	a8, 4, 4203a321 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x53d>
4203a31e:	feefc6        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a321:	40f890        	nsau	a9, a8
4203a324:	dfc992        	addi	a9, a9, -33
4203a327:	609090        	neg	a9, a9
4203a32a:	219190        	srai	a9, a9, 1
4203a32d:	3b0c      	movi.n	a11, 3
4203a32f:	fec992        	addi	a9, a9, -2
4203a332:	401900        	ssl	a9
4203a335:	a19b00        	sll	a9, a11
4203a338:	c2b890        	quou	a11, a8, a9
4203a33b:	bb9a      	add.n	a11, a11, a9
4203a33d:	41b1b0        	srli	a11, a11, 1
4203a340:	c298b0        	quou	a9, a8, a11
4203a343:	99ba      	add.n	a9, a9, a11
4203a345:	419190        	srli	a9, a9, 1
4203a348:	c2b890        	quou	a11, a8, a9
4203a34b:	9b9a      	add.n	a9, a11, a9
4203a34d:	196eb1        	l32r	a11, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a350:	419190        	srli	a9, a9, 1
4203a353:	0299b7        	bne	a9, a11, 4203a359 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x575>
4203a356:	fee1c6        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a359:	82b990        	mull	a11, a9, a9
4203a35c:	11aac0        	slli	a10, a10, 4
4203a35f:	6288b0        	saltu	a8, a8, a11
4203a362:	c09980        	sub	a9, a9, a8
4203a365:	e188      	l32i.n	a8, a1, 56
4203a367:	88aa      	add.n	a8, a8, a10
4203a369:	c08890        	sub	a8, a8, a9
4203a36c:	002046        	j	4203a3f1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x60d>
4203a36f:	a66000        	ee.ldf.64.xp	f6, f10, a0, a0
4203a372:	20          	.byte	0x20
4203a373:	0e61e2        	s32i	a14, a1, 56
4203a376:	4e3765        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a379:	0e21e2        	l32i	a14, a1, 56
4203a37c:	0a6d      	mov.n	a6, a10
4203a37e:	fef186        	j	42039f48 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
4203a381:	2ab200        	mul.s	f11, f2, f0
4203a384:	04          	.byte	0x4
4203a385:	052aa2        	l32i	a10, a10, 20
4203a388:	40a0c2        	movi	a12, 64
4203a38b:	81e9      	s32i.n	a14, a1, 32
4203a38d:	4cd525        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a390:	ff1986        	j	42039ffa <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x216>
4203a393:	088400        	lsx	f8, a4, a0
4203a396:	74          	.byte	0x74
4203a397:	780994        	mula.aa.ll	a9, a9
4203a39a:	031080        	rsr.acclo	a8
4203a39d:	119880        	slli	a9, a8, 8
4203a3a0:	0249e6        	bgei	a9, 4, 4203a3a6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5c2>
4203a3a3:	fece86        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a3a6:	40f980        	nsau	a8, a9
4203a3a9:	dfc882        	addi	a8, a8, -33
4203a3ac:	608080        	neg	a8, a8
4203a3af:	218180        	srai	a8, a8, 1
4203a3b2:	3a0c      	movi.n	a10, 3
4203a3b4:	fec882        	addi	a8, a8, -2
4203a3b7:	401800        	ssl	a8
4203a3ba:	a18a00        	sll	a8, a10
4203a3bd:	c2a980        	quou	a10, a9, a8
4203a3c0:	aa8a      	add.n	a10, a10, a8
4203a3c2:	41a1a0        	srli	a10, a10, 1
4203a3c5:	c289a0        	quou	a8, a9, a10
4203a3c8:	88aa      	add.n	a8, a8, a10
4203a3ca:	418180        	srli	a8, a8, 1
4203a3cd:	c2a980        	quou	a10, a9, a8
4203a3d0:	aa8a      	add.n	a10, a10, a8
4203a3d2:	194d81        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
4203a3d5:	41a1a0        	srli	a10, a10, 1
4203a3d8:	029a87        	bne	a10, a8, 4203a3de <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5fa>
4203a3db:	fec086        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a3de:	82baa0        	mull	a11, a10, a10
4203a3e1:	1186c0        	slli	a8, a6, 4
4203a3e4:	6299b0        	saltu	a9, a9, a11
4203a3e7:	c0aa90        	sub	a10, a10, a9
4203a3ea:	e198      	l32i.n	a9, a1, 56
4203a3ec:	898a      	add.n	a8, a9, a8
4203a3ee:	c088a0        	sub	a8, a8, a10
4203a3f1:	f49080        	extui	a9, a8, 0, 16
4203a3f4:	23a880        	sext	a10, a8, 15
4203a3f7:	0268f7        	bbci	a8, 15, 4203a3fd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x619>
4203a3fa:	ff1786        	j	4203a05c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x278>
4203a3fd:	f80c      	movi.n	a8, 15
4203a3ff:	02a8a7        	bge	a8, a10, 4203a405 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x621>
4203a402:	feb6c6        	j	42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a405:	1189c0        	slli	a8, a9, 4
4203a408:	998a      	add.n	a9, a9, a8
4203a40a:	745090        	extui	a5, a9, 0, 8
4203a40d:	ff5c46        	j	4203a182 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x39e>
4203a410:	740aa4        	mul.aa.ll	a10, a10
4203a413:	780884        	mula.aa.ll	a8, a8
4203a416:	ffe006        	j	4203a39a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x5b6>
4203a419:	829840        	mull	a9, a8, a4
4203a41c:	80a0c2        	movi	a12, 128
4203a41f:	99ca      	add.n	a9, a9, a12
4203a421:	f49090        	extui	a9, a9, 0, 16
4203a424:	11a980        	slli	a10, a9, 8
4203a427:	9a9a      	add.n	a9, a10, a9
4203a429:	484a      	add.n	a4, a8, a4
4203a42b:	319090        	srai	a9, a9, 16
4203a42e:	c04490        	sub	a4, a4, a9
4203a431:	119880        	slli	a9, a8, 8
4203a434:	f4d040        	extui	a13, a4, 0, 16
4203a437:	c08980        	sub	a8, a9, a8
4203a43a:	d288d0        	quos	a8, a8, a13
4203a43d:	f48080        	extui	a8, a8, 0, 16
4203a440:	7590b0        	extui	a9, a11, 16, 8
4203a443:	c19980        	mul16u	a9, a9, a8
4203a446:	74b8b0        	extui	a11, a11, 8, 8
4203a449:	c1bb80        	mul16u	a11, a11, a8
4203a44c:	74a060        	extui	a10, a6, 0, 8
4203a44f:	c1aa80        	mul16u	a10, a10, a8
4203a452:	99ca      	add.n	a9, a9, a12
4203a454:	f49090        	extui	a9, a9, 0, 16
4203a457:	bbca      	add.n	a11, a11, a12
4203a459:	f4b0b0        	extui	a11, a11, 0, 16
4203a45c:	118980        	slli	a8, a9, 8
4203a45f:	aaca      	add.n	a10, a10, a12
4203a461:	f4a0a0        	extui	a10, a10, 0, 16
4203a464:	889a      	add.n	a8, a8, a9
4203a466:	119b80        	slli	a9, a11, 8
4203a469:	99ba      	add.n	a9, a9, a11
4203a46b:	758080        	extui	a8, a8, 16, 8
4203a46e:	11ca80        	slli	a12, a10, 8
4203a471:	118800        	slli	a8, a8, 16
4203a474:	01bd80        	slli	a11, a13, 24
4203a477:	ccaa      	add.n	a12, a12, a10
4203a479:	419890        	srli	a9, a9, 8
4203a47c:	2088b0        	or	a8, a8, a11
4203a47f:	75c0c0        	extui	a12, a12, 16, 8
4203a482:	749890        	extui	a9, a9, 8, 8
4203a485:	2088c0        	or	a8, a8, a12
4203a488:	119980        	slli	a9, a9, 8
4203a48b:	206890        	or	a6, a8, a9
4203a48e:	fead86        	j	42039f48 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x164>
4203a491:	c0eae0        	sub	a14, a10, a14
4203a494:	40fe80        	nsau	a8, a14
4203a497:	418580        	srli	a8, a8, 5
4203a49a:	cdae16        	beqz	a14, 4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a49d:	c09a90        	sub	a9, a10, a9
4203a4a0:	1a0c      	movi.n	a10, 1
4203a4a2:	838a90        	moveqz	a8, a10, a9
4203a4a5:	a38816        	beqz	a8, 42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a4a8:	ff3306        	j	4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a4ab:	9a9000        	trunc.s	a9, f0, 0
4203a4ae:	c0          	.byte	0xc0
4203a4af:	40f9b0        	nsau	a11, a9
4203a4b2:	41b5b0        	srli	a11, a11, 5
4203a4b5:	cbf916        	beqz	a9, 4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>
4203a4b8:	c08a80        	sub	a8, a10, a8
4203a4bb:	190c      	movi.n	a9, 1
4203a4bd:	939b80        	movnez	a9, a11, a8
4203a4c0:	a1d916        	beqz	a9, 42039ee1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xfd>
4203a4c3:	ff2c46        	j	4203a178 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x394>


/tmp/roo-shadow-review/codegen/local_o3/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420394a8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
420394a8:	010136        	entry	a1, 128
420394ab:	078d      	mov.n	a8, a7
420394ad:	06ad      	mov.n	a10, a6
420394af:	142292        	l32i	a9, a2, 80
420394b2:	086d      	mov.n	a6, a8
420394b4:	23b380        	sext	a11, a3, 15
420394b7:	23c580        	sext	a12, a5, 15
420394ba:	238a80        	sext	a8, a10, 15
420394bd:	d1b9      	s32i.n	a11, a1, 52
420394bf:	81c9      	s32i.n	a12, a1, 32
420394c1:	e189      	s32i.n	a8, a1, 56
420394c3:	027d      	mov.n	a7, a2
420394c5:	232480        	sext	a2, a4, 15
420394c8:	67d916        	beqz	a9, 42039b49 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6a1>
420394cb:	02a827        	bge	a8, a2, 420394d1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x29>
420394ce:	01f406        	j	42039ca2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
420394d1:	3cc782        	addi	a8, a7, 60
420394d4:	a189      	s32i.n	a8, a1, 40
420394d6:	02acb7        	bge	a12, a11, 420394dc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x34>
420394d9:	01f146        	j	42039ca2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
420394dc:	f48020        	extui	a8, a2, 0, 16
420394df:	7169      	s32i.n	a6, a1, 28
420394e1:	d168      	l32i.n	a6, a1, 52
420394e3:	b189      	s32i.n	a8, a1, 44
420394e5:	200792        	l8ui	a9, a7, 32
420394e8:	57a8      	l32i.n	a10, a7, 20
420394ea:	021782        	l16ui	a8, a7, 4
420394ed:	0417e2        	l16ui	a14, a7, 8
420394f0:	898a      	add.n	a8, a9, a8
420394f2:	61a9      	s32i.n	a10, a1, 24
420394f4:	f4a060        	extui	a10, a6, 0, 16
420394f7:	c0be90        	sub	a11, a14, a9
420394fa:	23d880        	sext	a13, a8, 15
420394fd:	51a9      	s32i.n	a10, a1, 20
420394ff:	f48080        	extui	a8, a8, 0, 16
42039502:	23bb80        	sext	a11, a11, 15
42039505:	02a6d7        	bge	a6, a13, 4203950b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x63>
42039508:	00ee86        	j	420398c6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
4203950b:	02ab67        	bge	a11, a6, 42039511 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x69>
4203950e:	00ed06        	j	420398c6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039511:	0317a2        	l16ui	a10, a7, 6
42039514:	051742        	l16ui	a4, a7, 10
42039517:	a9aa      	add.n	a10, a9, a10
42039519:	c0c490        	sub	a12, a4, a9
4203951c:	23fa80        	sext	a15, a10, 15
4203951f:	23cc80        	sext	a12, a12, 15
42039522:	f4a0a0        	extui	a10, a10, 0, 16
42039525:	02a2f7        	bge	a2, a15, 4203952b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x83>
42039528:	00e686        	j	420398c6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
4203952b:	723c20        	salt	a3, a12, a2
4203952e:	c139      	s32i.n	a3, a1, 48
42039530:	02ac27        	bge	a12, a2, 42039536 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8e>
42039533:	00e3c6        	j	420398c6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039536:	210732        	l8ui	a3, a7, 33
42039539:	1c0752        	l8ui	a5, a7, 28
4203953c:	9139      	s32i.n	a3, a1, 36
4203953e:	327516        	beqz	a5, 42039869 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3c1>
42039541:	3d5a      	add.n	a3, a13, a5
42039543:	02a367        	bge	a3, a6, 42039549 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xa1>
42039546:	00c7c6        	j	42039869 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3c1>
42039549:	3f5a      	add.n	a3, a15, a5
4203954b:	02a327        	bge	a3, a2, 42039551 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xa9>
4203954e:	00c5c6        	j	42039869 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3c1>
42039551:	985a      	add.n	a9, a8, a5
42039553:	f48060        	extui	a8, a6, 0, 16
42039556:	c09980        	sub	a9, a9, a8
42039559:	b188      	l32i.n	a8, a1, 44
4203955b:	aa5a      	add.n	a10, a10, a5
4203955d:	c08a80        	sub	a8, a10, a8
42039560:	f49090        	extui	a9, a9, 0, 16
42039563:	f48080        	extui	a8, a8, 0, 16
42039566:	20a980        	or	a10, a9, a8
42039569:	23aa80        	sext	a10, a10, 15
4203956c:	4f3ad6        	bgez	a10, 42039a63 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5bb>
4203956f:	c138      	l32i.n	a3, a1, 48
42039571:	c09590        	sub	a9, a5, a9
42039574:	1a0c      	movi.n	a10, 1
42039576:	833a90        	moveqz	a3, a10, a9
42039579:	333356        	bnez	a3, 420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
4203957c:	c08580        	sub	a8, a5, a8
4203957f:	93a380        	movnez	a10, a3, a8
42039582:	32aa56        	bnez	a10, 420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039585:	142782        	l32i	a8, a7, 80
42039588:	6158      	l32i.n	a5, a1, 24
4203958a:	2c0816        	beqz	a8, 4203984e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
4203958d:	ffa042        	movi	a4, 255
42039590:	a1b8      	l32i.n	a11, a1, 40
42039592:	02dd      	mov.n	a13, a2
42039594:	06cd      	mov.n	a12, a6
42039596:	28c7a2        	addi	a10, a7, 40
42039599:	01b7a5        	call8	4203b114 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
4203959c:	200782        	l8ui	a8, a7, 32
4203959f:	0a3d      	mov.n	a3, a10
420395a1:	015a80        	slli	a5, a10, 24
420395a4:	04ed      	mov.n	a14, a4
420395a6:	0b5816        	beqz	a8, 4203965f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420395a9:	67e8      	l32i.n	a14, a7, 24
420395ab:	0597d2        	l16si	a13, a7, 10
420395ae:	0497c2        	l16si	a12, a7, 8
420395b1:	0397b2        	l16si	a11, a7, 6
420395b4:	0297a2        	l16si	a10, a7, 4
420395b7:	ff0c      	movi.n	a15, 15
420395b9:	1129      	s32i.n	a2, a1, 4
420395bb:	0169      	s32i.n	a6, a1, 0
420395bd:	ff1c25        	call8	42038780 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
420395c0:	0aed      	mov.n	a14, a10
420395c2:	099a16        	beqz	a10, 4203965f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420395c5:	97b8      	l32i.n	a11, a7, 36
420395c7:	018a80        	slli	a8, a10, 24
420395ca:	11bb80        	slli	a11, a11, 8
420395cd:	41b8b0        	srli	a11, a11, 8
420395d0:	759880        	extui	a9, a8, 24, 8
420395d3:	ffa0a2        	movi	a10, 255
420395d6:	20bb80        	or	a11, a11, a8
420395d9:	0299a7        	bne	a9, a10, 420395df <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x137>
420395dc:	010606        	j	420399f8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x550>
420395df:	0293a7        	bne	a3, a10, 420395e5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x13d>
420395e2:	011386        	j	42039a34 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x58c>
420395e5:	0b5d      	mov.n	a5, a11
420395e7:	074316        	beqz	a3, 4203965f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420395ea:	828930        	mull	a8, a9, a3
420395ed:	80a0c2        	movi	a12, 128
420395f0:	88ca      	add.n	a8, a8, a12
420395f2:	f48080        	extui	a8, a8, 0, 16
420395f5:	11a880        	slli	a10, a8, 8
420395f8:	aa8a      	add.n	a10, a10, a8
420395fa:	31a0a0        	srai	a10, a10, 16
420395fd:	893a      	add.n	a8, a9, a3
420395ff:	c088a0        	sub	a8, a8, a10
42039602:	11a980        	slli	a10, a9, 8
42039605:	f48080        	extui	a8, a8, 0, 16
42039608:	c09a90        	sub	a9, a10, a9
4203960b:	d29980        	quos	a9, a9, a8
4203960e:	f49090        	extui	a9, a9, 0, 16
42039611:	75b0b0        	extui	a11, a11, 16, 8
42039614:	c1bb90        	mul16u	a11, a11, a9
42039617:	74a850        	extui	a10, a5, 8, 8
4203961a:	c1aa90        	mul16u	a10, a10, a9
4203961d:	745050        	extui	a5, a5, 0, 8
42039620:	c15590        	mul16u	a5, a5, a9
42039623:	bbca      	add.n	a11, a11, a12
42039625:	f4b0b0        	extui	a11, a11, 0, 16
42039628:	aaca      	add.n	a10, a10, a12
4203962a:	f4a0a0        	extui	a10, a10, 0, 16
4203962d:	119b80        	slli	a9, a11, 8
42039630:	55ca      	add.n	a5, a5, a12
42039632:	f45050        	extui	a5, a5, 0, 16
42039635:	99ba      	add.n	a9, a9, a11
42039637:	11ba80        	slli	a11, a10, 8
4203963a:	11c580        	slli	a12, a5, 8
4203963d:	bbaa      	add.n	a11, a11, a10
4203963f:	759090        	extui	a9, a9, 16, 8
42039642:	01a880        	slli	a10, a8, 24
42039645:	cc5a      	add.n	a12, a12, a5
42039647:	119900        	slli	a9, a9, 16
4203964a:	41b8b0        	srli	a11, a11, 8
4203964d:	2099a0        	or	a9, a9, a10
42039650:	75c0c0        	extui	a12, a12, 16, 8
42039653:	7488b0        	extui	a8, a11, 8, 8
42039656:	2099c0        	or	a9, a9, a12
42039659:	118880        	slli	a8, a8, 8
4203965c:	205980        	or	a5, a9, a8
4203965f:	ffa082        	movi	a8, 255
42039662:	029487        	bne	a4, a8, 42039668 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
42039665:	00bec6        	j	42039964 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4bc>
42039668:	0cb416        	beqz	a4, 42039737 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
4203966b:	61b8      	l32i.n	a11, a1, 24
4203966d:	1ca9a1        	l32r	a10, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
42039670:	7598b0        	extui	a9, a11, 24, 8
42039673:	d19940        	mul16s	a9, a9, a4
42039676:	11bb80        	slli	a11, a11, 8
42039679:	7fc992        	addi	a9, a9, 127
4203967c:	a299a0        	muluh	a9, a9, a10
4203967f:	0199f0        	slli	a9, a9, 17
42039682:	75a890        	extui	a10, a9, 24, 8
42039685:	0a9d      	mov.n	a9, a10
42039687:	404800        	ssai	8
4203968a:	81bab0        	src	a11, a10, a11
4203968d:	029a87        	bne	a10, a8, 42039693 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1eb>
42039690:	00d406        	j	420399e4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x53c>
42039693:	75a850        	extui	a10, a5, 24, 8
42039696:	029a87        	bne	a10, a8, 4203969c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f4>
42039699:	00d946        	j	42039a02 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x55a>
4203969c:	097916        	beqz	a9, 42039737 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
4203969f:	341a16        	beqz	a10, 420399e4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x53c>
420396a2:	82c9a0        	mull	a12, a9, a10
420396a5:	80a0f2        	movi	a15, 128
420396a8:	ccfa      	add.n	a12, a12, a15
420396aa:	f4c0c0        	extui	a12, a12, 0, 16
420396ad:	11dc80        	slli	a13, a12, 8
420396b0:	cdca      	add.n	a12, a13, a12
420396b2:	a9aa      	add.n	a10, a9, a10
420396b4:	31c0c0        	srai	a12, a12, 16
420396b7:	c0cac0        	sub	a12, a10, a12
420396ba:	11a980        	slli	a10, a9, 8
420396bd:	f440c0        	extui	a4, a12, 0, 16
420396c0:	c0ca90        	sub	a12, a10, a9
420396c3:	d2cc40        	quos	a12, a12, a4
420396c6:	f4c0c0        	extui	a12, a12, 0, 16
420396c9:	c088c0        	sub	a8, a8, a12
420396cc:	75a0b0        	extui	a10, a11, 16, 8
420396cf:	f48080        	extui	a8, a8, 0, 16
420396d2:	c1aac0        	mul16u	a10, a10, a12
420396d5:	753050        	extui	a3, a5, 16, 8
420396d8:	7498b0        	extui	a9, a11, 8, 8
420396db:	c13380        	mul16u	a3, a3, a8
420396de:	c199c0        	mul16u	a9, a9, a12
420396e1:	74d850        	extui	a13, a5, 8, 8
420396e4:	74b0b0        	extui	a11, a11, 0, 8
420396e7:	c1bbc0        	mul16u	a11, a11, a12
420396ea:	c1dd80        	mul16u	a13, a13, a8
420396ed:	745050        	extui	a5, a5, 0, 8
420396f0:	aafa      	add.n	a10, a10, a15
420396f2:	c15580        	mul16u	a5, a5, a8
420396f5:	aa3a      	add.n	a10, a10, a3
420396f7:	99fa      	add.n	a9, a9, a15
420396f9:	f4a0a0        	extui	a10, a10, 0, 16
420396fc:	bbfa      	add.n	a11, a11, a15
420396fe:	99da      	add.n	a9, a9, a13
42039700:	f49090        	extui	a9, a9, 0, 16
42039703:	118a80        	slli	a8, a10, 8
42039706:	bb5a      	add.n	a11, a11, a5
42039708:	f4b0b0        	extui	a11, a11, 0, 16
4203970b:	88aa      	add.n	a8, a8, a10
4203970d:	11a980        	slli	a10, a9, 8
42039710:	80aa90        	add	a10, a10, a9
42039713:	758080        	extui	a8, a8, 16, 8
42039716:	11cb80        	slli	a12, a11, 8
42039719:	118800        	slli	a8, a8, 16
4203971c:	019480        	slli	a9, a4, 24
4203971f:	80ccb0        	add	a12, a12, a11
42039722:	4158a0        	srli	a5, a10, 8
42039725:	208890        	or	a8, a8, a9
42039728:	75c0c0        	extui	a12, a12, 16, 8
4203972b:	745850        	extui	a5, a5, 8, 8
4203972e:	2088c0        	or	a8, a8, a12
42039731:	115580        	slli	a5, a5, 8
42039734:	205850        	or	a5, a8, a5
42039737:	1427a2        	l32i	a10, a7, 80
4203973a:	110a16        	beqz	a10, 4203984e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
4203973d:	180a82        	l8ui	a8, a10, 24
42039740:	290856        	bnez	a8, 420399d4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x52c>
42039743:	b1b8      	l32i.n	a11, a1, 44
42039745:	031a82        	l16ui	a8, a10, 6
42039748:	021a92        	l16ui	a9, a10, 4
4203974b:	c08b80        	sub	a8, a11, a8
4203974e:	51b8      	l32i.n	a11, a1, 20
42039750:	740884        	mul.aa.ll	a8, a8
42039753:	c09b90        	sub	a9, a11, a9
42039756:	780994        	mula.aa.ll	a9, a9
42039759:	3a88      	l32i.n	a8, a10, 12
4203975b:	031090        	rsr.acclo	a9
4203975e:	c08890        	sub	a8, a8, a9
42039761:	0218e6        	bgei	a8, 1, 42039767 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2bf>
42039764:	00a046        	j	420399e9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x541>
42039767:	049a92        	l16si	a9, a10, 8
4203976a:	0219e6        	bgei	a9, 1, 42039770 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c8>
4203976d:	00a306        	j	420399fd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039770:	b99a      	add.n	a11, a9, a9
42039772:	0228b7        	blt	a8, a11, 42039778 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2d0>
42039775:	00a6c6        	j	42039a14 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x56c>
42039778:	4ab8      	l32i.n	a11, a10, 16
4203977a:	5aa8      	l32i.n	a10, a10, 20
4203977c:	60a0c2        	movi	a12, 96
4203977f:	61e9      	s32i.n	a14, a1, 24
42039781:	4d95e5        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039784:	61e8      	l32i.n	a14, a1, 24
42039786:	75c8a0        	extui	a12, a10, 24, 8
42039789:	1c6281        	l32r	a8, 42000914 <_stext+0x8f4> (80808081 <_rtc_reserved_end+0x20708081>)
4203978c:	d1cce0        	mul16s	a12, a12, a14
4203978f:	11aa80        	slli	a10, a10, 8
42039792:	a2cc80        	muluh	a12, a12, a8
42039795:	ffa082        	movi	a8, 255
42039798:	01ccf0        	slli	a12, a12, 17
4203979b:	7598c0        	extui	a9, a12, 24, 8
4203979e:	09cd      	mov.n	a12, a9
420397a0:	404800        	ssai	8
420397a3:	81b9a0        	src	a11, a9, a10
420397a6:	029987        	bne	a9, a8, 420397ac <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x304>
420397a9:	007f06        	j	420399a9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x501>
420397ac:	759850        	extui	a9, a5, 24, 8
420397af:	029987        	bne	a9, a8, 420397b5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x30d>
420397b2:	008e06        	j	420399ee <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x546>
420397b5:	095c16        	beqz	a12, 4203984e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
420397b8:	1ed916        	beqz	a9, 420399a9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x501>
420397bb:	82ac90        	mull	a10, a12, a9
420397be:	80a0e2        	movi	a14, 128
420397c1:	aaea      	add.n	a10, a10, a14
420397c3:	f4a0a0        	extui	a10, a10, 0, 16
420397c6:	11da80        	slli	a13, a10, 8
420397c9:	adaa      	add.n	a10, a13, a10
420397cb:	9c9a      	add.n	a9, a12, a9
420397cd:	31a0a0        	srai	a10, a10, 16
420397d0:	c099a0        	sub	a9, a9, a10
420397d3:	11dc80        	slli	a13, a12, 8
420397d6:	f49090        	extui	a9, a9, 0, 16
420397d9:	c0ddc0        	sub	a13, a13, a12
420397dc:	d2dd90        	quos	a13, a13, a9
420397df:	f4d0d0        	extui	a13, a13, 0, 16
420397e2:	75c0b0        	extui	a12, a11, 16, 8
420397e5:	c088d0        	sub	a8, a8, a13
420397e8:	f48080        	extui	a8, a8, 0, 16
420397eb:	756050        	extui	a6, a5, 16, 8
420397ee:	74a8b0        	extui	a10, a11, 8, 8
420397f1:	c1ccd0        	mul16u	a12, a12, a13
420397f4:	c16680        	mul16u	a6, a6, a8
420397f7:	74f850        	extui	a15, a5, 8, 8
420397fa:	c1aad0        	mul16u	a10, a10, a13
420397fd:	74b0b0        	extui	a11, a11, 0, 8
42039800:	c1ff80        	mul16u	a15, a15, a8
42039803:	c1bbd0        	mul16u	a11, a11, a13
42039806:	745050        	extui	a5, a5, 0, 8
42039809:	ccea      	add.n	a12, a12, a14
4203980b:	c15580        	mul16u	a5, a5, a8
4203980e:	cc6a      	add.n	a12, a12, a6
42039810:	aaea      	add.n	a10, a10, a14
42039812:	f4c0c0        	extui	a12, a12, 0, 16
42039815:	bbea      	add.n	a11, a11, a14
42039817:	aafa      	add.n	a10, a10, a15
42039819:	f4a0a0        	extui	a10, a10, 0, 16
4203981c:	118c80        	slli	a8, a12, 8
4203981f:	bb5a      	add.n	a11, a11, a5
42039821:	f4b0b0        	extui	a11, a11, 0, 16
42039824:	88ca      	add.n	a8, a8, a12
42039826:	115a80        	slli	a5, a10, 8
42039829:	55aa      	add.n	a5, a5, a10
4203982b:	758080        	extui	a8, a8, 16, 8
4203982e:	11cb80        	slli	a12, a11, 8
42039831:	01a980        	slli	a10, a9, 24
42039834:	118800        	slli	a8, a8, 16
42039837:	9cba      	add.n	a9, a12, a11
42039839:	415850        	srli	a5, a5, 8
4203983c:	2088a0        	or	a8, a8, a10
4203983f:	759090        	extui	a9, a9, 16, 8
42039842:	745850        	extui	a5, a5, 8, 8
42039845:	208890        	or	a8, a8, a9
42039848:	115580        	slli	a5, a5, 8
4203984b:	205850        	or	a5, a8, a5
4203984e:	5188      	l32i.n	a8, a1, 20
42039850:	7198      	l32i.n	a9, a1, 28
42039852:	881b      	addi.n	a8, a8, 1
42039854:	0959      	s32i.n	a5, a9, 0
42039856:	236880        	sext	a6, a8, 15
42039859:	894b      	addi.n	a8, a9, 4
4203985b:	082192        	l32i	a9, a1, 32
4203985e:	02a967        	bge	a9, a6, 42039864 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3bc>
42039861:	0056c6        	j	420399c0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x518>
42039864:	7189      	s32i.n	a8, a1, 28
42039866:	ff1ec6        	j	420394e5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3d>
42039869:	1d0752        	l8ui	a5, a7, 29
4203986c:	05c516        	beqz	a5, 420398cc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x424>
4203986f:	c03b50        	sub	a3, a11, a5
42039872:	562637        	blt	a6, a3, 420398cc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x424>
42039875:	3f5a      	add.n	a3, a15, a5
42039877:	512327        	blt	a3, a2, 420398cc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x424>
4203987a:	895a      	add.n	a8, a9, a5
4203987c:	c088e0        	sub	a8, a8, a14
4203987f:	f49060        	extui	a9, a6, 0, 16
42039882:	898a      	add.n	a8, a9, a8
42039884:	b198      	l32i.n	a9, a1, 44
42039886:	aa5a      	add.n	a10, a10, a5
42039888:	c0aa90        	sub	a10, a10, a9
4203988b:	f48080        	extui	a8, a8, 0, 16
4203988e:	f4a0a0        	extui	a10, a10, 0, 16
42039891:	2098a0        	or	a9, a8, a10
42039894:	239980        	sext	a9, a9, 15
42039897:	2459d6        	bgez	a9, 42039ae0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x638>
4203989a:	c08580        	sub	a8, a5, a8
4203989d:	40f890        	nsau	a9, a8
420398a0:	419590        	srli	a9, a9, 5
420398a3:	988c      	beqz.n	a8, 420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
420398a5:	c0a5a0        	sub	a10, a5, a10
420398a8:	180c      	movi.n	a8, 1
420398aa:	9389a0        	movnez	a8, a9, a10
420398ad:	cd4816        	beqz	a8, 42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
420398b0:	9198      	l32i.n	a9, a1, 36
420398b2:	1189c0        	slli	a8, a9, 4
420398b5:	898a      	add.n	a8, a9, a8
420398b7:	744080        	extui	a4, a8, 0, 8
420398ba:	ffa082        	movi	a8, 255
420398bd:	029487        	bne	a4, a8, 420398c3 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41b>
420398c0:	ff3046        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
420398c3:	ff3246        	j	42039590 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe8>
420398c6:	040c      	movi.n	a4, 0
420398c8:	ff3106        	j	42039590 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe8>
420398cb:	07a200        	ee.stf.64.xp	f10, f0, a0, a2
420398ce:	045a161e 	ee.vmulas.s16.accx.ld.ip.qup	q1, a1, 0x160, q0, q2, q5, q2
420398d2:	c05ba0        	sub	a5, a11, a10
420398d5:	3f2657        	blt	a6, a5, 42039918 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x470>
420398d8:	c05ca0        	sub	a5, a12, a10
420398db:	392257        	blt	a2, a5, 42039918 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x470>
420398de:	99aa      	add.n	a9, a9, a10
420398e0:	c0e9e0        	sub	a14, a9, a14
420398e3:	f48060        	extui	a8, a6, 0, 16
420398e6:	e8ea      	add.n	a14, a8, a14
420398e8:	b188      	l32i.n	a8, a1, 44
420398ea:	c09940        	sub	a9, a9, a4
420398ed:	989a      	add.n	a9, a8, a9
420398ef:	f4e0e0        	extui	a14, a14, 0, 16
420398f2:	f49090        	extui	a9, a9, 0, 16
420398f5:	208e90        	or	a8, a14, a9
420398f8:	238880        	sext	a8, a8, 15
420398fb:	1ea8d6        	bgez	a8, 42039ae9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x641>
420398fe:	c0eae0        	sub	a14, a10, a14
42039901:	40fe80        	nsau	a8, a14
42039904:	418580        	srli	a8, a8, 5
42039907:	fa5e16        	beqz	a14, 420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
4203990a:	c09a90        	sub	a9, a10, a9
4203990d:	1a0c      	movi.n	a10, 1
4203990f:	838a90        	moveqz	a8, a10, a9
42039912:	c6f816        	beqz	a8, 42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039915:	ffe5c6        	j	420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039918:	1f07a2        	l8ui	a10, a7, 31
4203991b:	04da16        	beqz	a10, 4203996c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c4>
4203991e:	edaa      	add.n	a14, a13, a10
42039920:	482e67        	blt	a14, a6, 4203996c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c4>
42039923:	c0eca0        	sub	a14, a12, a10
42039926:	4222e7        	blt	a2, a14, 4203996c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c4>
42039929:	99aa      	add.n	a9, a9, a10
4203992b:	b8aa      	add.n	a11, a8, a10
4203992d:	c08940        	sub	a8, a9, a4
42039930:	f49060        	extui	a9, a6, 0, 16
42039933:	c09b90        	sub	a9, a11, a9
42039936:	b1b8      	l32i.n	a11, a1, 44
42039938:	f49090        	extui	a9, a9, 0, 16
4203993b:	8b8a      	add.n	a8, a11, a8
4203993d:	f48080        	extui	a8, a8, 0, 16
42039940:	20b980        	or	a11, a9, a8
42039943:	23bb80        	sext	a11, a11, 15
42039946:	35ebd6        	bgez	a11, 42039ca8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x800>
42039949:	c09a90        	sub	a9, a10, a9
4203994c:	40f9b0        	nsau	a11, a9
4203994f:	41b5b0        	srli	a11, a11, 5
42039952:	f5a916        	beqz	a9, 420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039955:	c08a80        	sub	a8, a10, a8
42039958:	190c      	movi.n	a9, 1
4203995a:	939b80        	movnez	a9, a11, a8
4203995d:	c24916        	beqz	a9, 42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039960:	ffd306        	j	420398b0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x408>
42039963:	00          	.byte	00
42039964:	1beb51        	l32r	a5, 42000910 <_stext+0x8f0> (ff000000 <_rtc_reserved_end+0x9ef00000>)
42039967:	6188      	l32i.n	a8, a1, 24
42039969:	ff71c6        	j	42039734 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28c>
4203996c:	9198      	l32i.n	a9, a1, 36
4203996e:	e80c      	movi.n	a8, 14
42039970:	02b897        	bgeu	a8, a9, 42039976 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4ce>
42039973:	ff0386        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039976:	c0d6d0        	sub	a13, a6, a13
42039979:	40fd80        	nsau	a8, a13
4203997c:	190c      	movi.n	a9, 1
4203997e:	418580        	srli	a8, a8, 5
42039981:	7d9c      	beqz.n	a13, 4203999c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4f4>
42039983:	c0b6b0        	sub	a11, a6, a11
42039986:	8389b0        	moveqz	a8, a9, a11
42039989:	f8cc      	bnez.n	a8, 4203999c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4f4>
4203998b:	c0f2f0        	sub	a15, a2, a15
4203998e:	8389f0        	moveqz	a8, a9, a15
42039991:	78cc      	bnez.n	a8, 4203999c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4f4>
42039993:	c0c2c0        	sub	a12, a2, a12
42039996:	9398c0        	movnez	a9, a8, a12
42039999:	be8916        	beqz	a9, 42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
4203999c:	9198      	l32i.n	a9, a1, 36
4203999e:	1189c0        	slli	a8, a9, 4
420399a1:	498a      	add.n	a4, a9, a8
420399a3:	744040        	extui	a4, a4, 0, 8
420399a6:	fef986        	j	42039590 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe8>
420399a9:	5188      	l32i.n	a8, a1, 20
420399ab:	7198      	l32i.n	a9, a1, 28
420399ad:	881b      	addi.n	a8, a8, 1
420399af:	0b5d      	mov.n	a5, a11
420399b1:	0959      	s32i.n	a5, a9, 0
420399b3:	236880        	sext	a6, a8, 15
420399b6:	894b      	addi.n	a8, a9, 4
420399b8:	8198      	l32i.n	a9, a1, 32
420399ba:	022967        	blt	a9, a6, 420399c0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x518>
420399bd:	ffa8c6        	j	42039864 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3bc>
420399c0:	b198      	l32i.n	a9, a1, 44
420399c2:	991b      	addi.n	a9, a9, 1
420399c4:	232980        	sext	a2, a9, 15
420399c7:	e198      	l32i.n	a9, a1, 56
420399c9:	02a927        	bge	a9, a2, 420399cf <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x527>
420399cc:	00b486        	j	42039ca2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
420399cf:	086d      	mov.n	a6, a8
420399d1:	fec1c6        	j	420394dc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x34>
420399d4:	20c220        	or	a12, a2, a2
420399d7:	20b660        	or	a11, a6, a6
420399da:	0661e2        	s32i	a14, a1, 24
420399dd:	ed50a5        	call8	42026ee8 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
420399e0:	ff6806        	j	42039784 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2dc>
420399e3:	00          	.byte	00
420399e4:	0b5d      	mov.n	a5, a11
420399e6:	ff5346        	j	42039737 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
420399e9:	5aa8      	l32i.n	a10, a10, 20
420399eb:	ff65c6        	j	42039786 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2de>
420399ee:	05ad      	mov.n	a10, a5
420399f0:	4ecfa5        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
420399f3:	0a5d      	mov.n	a5, a10
420399f5:	ff9546        	j	4203984e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a6>
420399f8:	0b5d      	mov.n	a5, a11
420399fa:	ff1846        	j	4203965f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
420399fd:	4aa8      	l32i.n	a10, a10, 16
420399ff:	ff60c6        	j	42039786 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2de>
42039a02:	20a550        	or	a10, a5, a5
42039a05:	0661e2        	s32i	a14, a1, 24
42039a08:	4ece25        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039a0b:	0621e2        	l32i	a14, a1, 24
42039a0e:	0a5d      	mov.n	a5, a10
42039a10:	ff48c6        	j	42039737 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28f>
42039a13:	00          	.byte	00
42039a14:	e51926        	beqi	a9, 1, 420399fd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039a17:	ffc9c2        	addi	a12, a9, -1
42039a1a:	23cc80        	sext	a12, a12, 15
42039a1d:	11cce0        	slli	a12, a12, 2
42039a20:	212c87        	blt	a12, a8, 42039a45 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x59d>
42039a23:	042ab2        	l32i	a11, a10, 16
42039a26:	052aa2        	l32i	a10, a10, 20
42039a29:	0c4c      	movi.n	a12, 64
42039a2b:	61e9      	s32i.n	a14, a1, 24
42039a2d:	4d6b25        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039a30:	ff5406        	j	42039784 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2dc>
42039a33:	a55000        	extui	a5, a0, 16, 11
42039a36:	20          	.byte	0x20
42039a37:	0961e2        	s32i	a14, a1, 36
42039a3a:	4ecb25        	call8	420886ec <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039a3d:	0921e2        	l32i	a14, a1, 36
42039a40:	0a5d      	mov.n	a5, a10
42039a42:	ff0646        	j	4203965f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b7>
42039a45:	b42926        	beqi	a9, 2, 420399fd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039a48:	80bb90        	add	a11, a11, a9
42039a4b:	80bbb0        	add	a11, a11, a11
42039a4e:	f8cbb2        	addi	a11, a11, -8
42039a51:	a8a8b7        	bge	a8, a11, 420399fd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x555>
42039a54:	042ab2        	l32i	a11, a10, 16
42039a57:	5aa8      	l32i.n	a10, a10, 20
42039a59:	0c2c      	movi.n	a12, 32
42039a5b:	61e9      	s32i.n	a14, a1, 24
42039a5d:	4d6825        	call8	420870e0 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039a60:	ff4806        	j	42039784 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2dc>
42039a63:	740884        	mul.aa.ll	a8, a8
42039a66:	780994        	mula.aa.ll	a9, a9
42039a69:	031080        	rsr.acclo	a8
42039a6c:	119880        	slli	a9, a8, 8
42039a6f:	0249e6        	bgei	a9, 4, 42039a75 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5cd>
42039a72:	fec3c6        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039a75:	40f980        	nsau	a8, a9
42039a78:	dfc882        	addi	a8, a8, -33
42039a7b:	608080        	neg	a8, a8
42039a7e:	218180        	srai	a8, a8, 1
42039a81:	3a0c      	movi.n	a10, 3
42039a83:	fec882        	addi	a8, a8, -2
42039a86:	401800        	ssl	a8
42039a89:	a18a00        	sll	a8, a10
42039a8c:	c2a980        	quou	a10, a9, a8
42039a8f:	aa8a      	add.n	a10, a10, a8
42039a91:	41a1a0        	srli	a10, a10, 1
42039a94:	c289a0        	quou	a8, a9, a10
42039a97:	88aa      	add.n	a8, a8, a10
42039a99:	418180        	srli	a8, a8, 1
42039a9c:	c2a980        	quou	a10, a9, a8
42039a9f:	aa8a      	add.n	a10, a10, a8
42039aa1:	1b9981        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42039aa4:	41a1a0        	srli	a10, a10, 1
42039aa7:	029a87        	bne	a10, a8, 42039aad <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x605>
42039aaa:	feb5c6        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039aad:	82baa0        	mull	a11, a10, a10
42039ab0:	1185c0        	slli	a8, a5, 4
42039ab3:	6299b0        	saltu	a9, a9, a11
42039ab6:	c0aa90        	sub	a10, a10, a9
42039ab9:	9198      	l32i.n	a9, a1, 36
42039abb:	898a      	add.n	a8, a9, a8
42039abd:	c088a0        	sub	a8, a8, a10
42039ac0:	f49080        	extui	a9, a8, 0, 16
42039ac3:	23a880        	sext	a10, a8, 15
42039ac6:	0268f7        	bbci	a8, 15, 42039acc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x624>
42039ac9:	ff7e46        	j	420398c6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x41e>
42039acc:	f80c      	movi.n	a8, 15
42039ace:	02a8a7        	bge	a8, a10, 42039ad4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x62c>
42039ad1:	feac06        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039ad4:	1189c0        	slli	a8, a9, 4
42039ad7:	998a      	add.n	a9, a9, a8
42039ad9:	744090        	extui	a4, a9, 0, 8
42039adc:	ff7686        	j	420398ba <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x412>
42039adf:	0aa400        	add.s	f10, f4, f0
42039ae2:	74          	.byte	0x74
42039ae3:	780884        	mula.aa.ll	a8, a8
42039ae6:	ffdfc6        	j	42039a69 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5c1>
42039ae9:	740994        	mul.aa.ll	a9, a9
42039aec:	780ee4        	mula.aa.ll	a14, a14
42039aef:	031080        	rsr.acclo	a8
42039af2:	119880        	slli	a9, a8, 8
42039af5:	0249e6        	bgei	a9, 4, 42039afb <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x653>
42039af8:	fea246        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039afb:	40f980        	nsau	a8, a9
42039afe:	dfc882        	addi	a8, a8, -33
42039b01:	608080        	neg	a8, a8
42039b04:	218180        	srai	a8, a8, 1
42039b07:	3b0c      	movi.n	a11, 3
42039b09:	fec882        	addi	a8, a8, -2
42039b0c:	401800        	ssl	a8
42039b0f:	a18b00        	sll	a8, a11
42039b12:	c2b980        	quou	a11, a9, a8
42039b15:	bb8a      	add.n	a11, a11, a8
42039b17:	41b1b0        	srli	a11, a11, 1
42039b1a:	c289b0        	quou	a8, a9, a11
42039b1d:	88ba      	add.n	a8, a8, a11
42039b1f:	418180        	srli	a8, a8, 1
42039b22:	c2b980        	quou	a11, a9, a8
42039b25:	bb8a      	add.n	a11, a11, a8
42039b27:	1b7881        	l32r	a8, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42039b2a:	41b1b0        	srli	a11, a11, 1
42039b2d:	029b87        	bne	a11, a8, 42039b33 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x68b>
42039b30:	fe9446        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039b33:	82cbb0        	mull	a12, a11, a11
42039b36:	118ac0        	slli	a8, a10, 4
42039b39:	6299c0        	saltu	a9, a9, a12
42039b3c:	c0bb90        	sub	a11, a11, a9
42039b3f:	9198      	l32i.n	a9, a1, 36
42039b41:	898a      	add.n	a8, a9, a8
42039b43:	c088b0        	sub	a8, a8, a11
42039b46:	ffdd86        	j	42039ac0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x618>
42039b49:	08ed      	mov.n	a14, a8
42039b4b:	0788      	l32i.n	a8, a7, 0
42039b4d:	6129      	s32i.n	a2, a1, 24
42039b4f:	b888      	l32i.n	a8, a8, 44
42039b51:	0cdd      	mov.n	a13, a12
42039b53:	0c3d      	mov.n	a3, a12
42039b55:	06fd      	mov.n	a15, a6
42039b57:	02cd      	mov.n	a12, a2
42039b59:	07ad      	mov.n	a10, a7
42039b5b:	0b5d      	mov.n	a5, a11
42039b5d:	0008e0        	callx8	a8
42039b60:	024d      	mov.n	a4, a2
42039b62:	0a2d      	mov.n	a2, a10
42039b64:	13ca56        	bnez	a10, 42039ca4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fc>
42039b67:	249782        	l16si	a8, a7, 72
42039b6a:	1a9792        	l16si	a9, a7, 52
42039b6d:	2107a2        	l8ui	a10, a7, 33
42039b70:	539980        	max	a9, a9, a8
42039b73:	1907f2        	l8ui	a15, a7, 25
42039b76:	180782        	l8ui	a8, a7, 24
42039b79:	eb0c      	movi.n	a11, 14
42039b7b:	c9fb      	addi.n	a12, a9, 15
42039b7d:	1a07e2        	l8ui	a14, a7, 26
42039b80:	62bba0        	saltu	a11, a11, a10
42039b83:	1ec992        	addi	a9, a9, 30
42039b86:	2007a2        	l8ui	a10, a7, 32
42039b89:	1b07d2        	l8ui	a13, a7, 27
42039b8c:	b39cc0        	movgez	a9, a12, a12
42039b8f:	7388f0        	maxu	a8, a8, a15
42039b92:	bb0b      	addi.n	a11, a11, -1
42039b94:	60b0b0        	neg	a11, a11
42039b97:	219490        	srai	a9, a9, 4
42039b9a:	7388e0        	maxu	a8, a8, a14
42039b9d:	caba      	add.n	a12, a10, a11
42039b9f:	7388d0        	maxu	a8, a8, a13
42039ba2:	a9aa      	add.n	a10, a9, a10
42039ba4:	5388c0        	max	a8, a8, a12
42039ba7:	0297f2        	l16si	a15, a7, 4
42039baa:	aaba      	add.n	a10, a10, a11
42039bac:	53aa80        	max	a10, a10, a8
42039baf:	bfaa      	add.n	a11, a15, a10
42039bb1:	0397e2        	l16si	a14, a7, 6
42039bb4:	0497d2        	l16si	a13, a7, 8
42039bb7:	1261b2        	s32i	a11, a1, 72
42039bba:	c0dda0        	sub	a13, a13, a10
42039bbd:	be8a      	add.n	a11, a14, a8
42039bbf:	03fd      	mov.n	a15, a3
42039bc1:	c0a350        	sub	a10, a3, a5
42039bc4:	0597c2        	l16si	a12, a7, 10
42039bc7:	122132        	l32i	a3, a1, 72
42039bca:	43ddf0        	min	a13, a13, a15
42039bcd:	bb9a      	add.n	a11, a11, a9
42039bcf:	53e530        	max	a14, a5, a3
42039bd2:	b1d9      	s32i.n	a13, a1, 44
42039bd4:	f1b9      	s32i.n	a11, a1, 60
42039bd6:	c08c80        	sub	a8, a12, a8
42039bd9:	ba1b      	addi.n	a11, a10, 1
42039bdb:	e1d8      	l32i.n	a13, a1, 56
42039bdd:	71e9      	s32i.n	a14, a1, 28
42039bdf:	106182        	s32i	a8, a1, 64
42039be2:	1761b2        	s32i	a11, a1, 92
42039be5:	02ad47        	bge	a13, a4, 42039beb <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x743>
42039be8:	002d86        	j	42039ca2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7fa>
42039beb:	b198      	l32i.n	a9, a1, 44
42039bed:	c08e50        	sub	a8, a14, a5
42039bf0:	72a530        	salt	a10, a5, a3
42039bf3:	880b      	addi.n	a8, a8, -1
42039bf5:	0ebd      	mov.n	a11, a14
42039bf7:	c51b      	addi.n	a12, a5, 1
42039bf9:	e91b      	addi.n	a14, a9, 1
42039bfb:	1198e0        	slli	a9, a8, 2
42039bfe:	8382a0        	moveqz	a8, a2, a10
42039c01:	c0beb0        	sub	a11, a14, a11
42039c04:	8392a0        	moveqz	a9, a2, a10
42039c07:	88ca      	add.n	a8, a8, a12
42039c09:	1021a2        	l32i	a10, a1, 64
42039c0c:	11cbe0        	slli	a12, a11, 2
42039c0f:	238880        	sext	a8, a8, 15
42039c12:	1721b2        	l32i	a11, a1, 92
42039c15:	43ada0        	min	a10, a13, a10
42039c18:	156182        	s32i	a8, a1, 84
42039c1b:	894b      	addi.n	a8, a9, 4
42039c1d:	11bbe0        	slli	a11, a11, 2
42039c20:	166182        	s32i	a8, a1, 88
42039c23:	8a0b      	addi.n	a8, a10, -1
42039c25:	1461e2        	s32i	a14, a1, 80
42039c28:	a1a9      	s32i.n	a10, a1, 40
42039c2a:	1361c2        	s32i	a12, a1, 76
42039c2d:	c1b9      	s32i.n	a11, a1, 48
42039c2f:	4f1b      	addi.n	a4, a15, 1
42039c31:	116182        	s32i	a8, a1, 68
42039c34:	d188      	l32i.n	a8, a1, 52
42039c36:	7198      	l32i.n	a9, a1, 28
42039c38:	9189      	s32i.n	a8, a1, 36
42039c3a:	b188      	l32i.n	a8, a1, 44
42039c3c:	062d      	mov.n	a2, a6
42039c3e:	022897        	blt	a8, a9, 42039c44 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x79c>
42039c41:	004906        	j	42039d69 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8c1>
42039c44:	8188      	l32i.n	a8, a1, 32
42039c46:	9138      	l32i.n	a3, a1, 36
42039c48:	262837        	blt	a8, a3, 42039c72 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ca>
42039c4b:	0c9182        	l16si	a8, a1, 24
42039c4e:	025d      	mov.n	a5, a2
42039c50:	5189      	s32i.n	a8, a1, 20
42039c52:	57d8      	l32i.n	a13, a7, 20
42039c54:	51c8      	l32i.n	a12, a1, 20
42039c56:	03bd      	mov.n	a11, a3
42039c58:	07ad      	mov.n	a10, a7
42039c5a:	ff2665        	call8	42038ec0 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039c5d:	05a9      	s32i.n	a10, a5, 0
42039c5f:	331b      	addi.n	a3, a3, 1
42039c61:	554b      	addi.n	a5, a5, 4
42039c63:	eb9347        	bne	a3, a4, 42039c52 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7aa>
42039c66:	8188      	l32i.n	a8, a1, 32
42039c68:	9198      	l32i.n	a9, a1, 36
42039c6a:	c03890        	sub	a3, a8, a9
42039c6d:	824b      	addi.n	a8, a2, 4
42039c6f:	a02380        	addx4	a2, a3, a8
42039c72:	6198      	l32i.n	a9, a1, 24
42039c74:	f188      	l32i.n	a8, a1, 60
42039c76:	728980        	salt	a8, a9, a8
42039c79:	880b      	addi.n	a8, a8, -1
42039c7b:	608080        	neg	a8, a8
42039c7e:	748080        	extui	a8, a8, 0, 8
42039c81:	f88c      	beqz.n	a8, 42039c94 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ec>
42039c83:	102182        	l32i	a8, a1, 64
42039c86:	728890        	salt	a8, a8, a9
42039c89:	880b      	addi.n	a8, a8, -1
42039c8b:	608080        	neg	a8, a8
42039c8e:	748080        	extui	a8, a8, 0, 8
42039c91:	075856        	bnez	a8, 42039d0a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x862>
42039c94:	026d      	mov.n	a6, a2
42039c96:	6188      	l32i.n	a8, a1, 24
42039c98:	e198      	l32i.n	a9, a1, 56
42039c9a:	881b      	addi.n	a8, a8, 1
42039c9c:	066182        	s32i	a8, a1, 24
42039c9f:	91a987        	bge	a9, a8, 42039c34 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x78c>
42039ca2:	020c      	movi.n	a2, 0
42039ca4:	f01d      	retw.n
42039ca6:	840000        	extui	a0, a0, 0, 9
42039ca9:	7408      	l32i.n	a0, a4, 28
42039cab:	780994        	mula.aa.ll	a9, a9
42039cae:	031080        	rsr.acclo	a8
42039cb1:	118880        	slli	a8, a8, 8
42039cb4:	0248e6        	bgei	a8, 4, 42039cba <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x812>
42039cb7:	fe3286        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039cba:	40f890        	nsau	a9, a8
42039cbd:	dfc992        	addi	a9, a9, -33
42039cc0:	609090        	neg	a9, a9
42039cc3:	219190        	srai	a9, a9, 1
42039cc6:	3b0c      	movi.n	a11, 3
42039cc8:	fec992        	addi	a9, a9, -2
42039ccb:	401900        	ssl	a9
42039cce:	a19b00        	sll	a9, a11
42039cd1:	c2b890        	quou	a11, a8, a9
42039cd4:	bb9a      	add.n	a11, a11, a9
42039cd6:	41b1b0        	srli	a11, a11, 1
42039cd9:	c298b0        	quou	a9, a8, a11
42039cdc:	99ba      	add.n	a9, a9, a11
42039cde:	419190        	srli	a9, a9, 1
42039ce1:	c2b890        	quou	a11, a8, a9
42039ce4:	9b9a      	add.n	a9, a11, a9
42039ce6:	1b08b1        	l32r	a11, 42000908 <_stext+0x8e8> (10000 <UserFrameTotalSize+0xff00>)
42039ce9:	419190        	srli	a9, a9, 1
42039cec:	0299b7        	bne	a9, a11, 42039cf2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x84a>
42039cef:	fe2486        	j	42039585 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xdd>
42039cf2:	82b990        	mull	a11, a9, a9
42039cf5:	11aac0        	slli	a10, a10, 4
42039cf8:	6288b0        	saltu	a8, a8, a11
42039cfb:	c09980        	sub	a9, a9, a8
42039cfe:	9188      	l32i.n	a8, a1, 36
42039d00:	88aa      	add.n	a8, a8, a10
42039d02:	c08890        	sub	a8, a8, a9
42039d05:	ff6dc6        	j	42039ac0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x618>
42039d08:	00          	.byte	00
42039d09:	a18800        	sll	a8, a8
42039d0c:	095d      	mov.n	a5, a9
42039d0e:	82a987        	bge	a9, a8, 42039c94 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ec>
42039d11:	172182        	l32i	a8, a1, 92
42039d14:	3818a6        	blti	a8, 1, 42039d50 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8a8>
42039d17:	c198      	l32i.n	a9, a1, 48
42039d19:	208220        	or	a8, a2, a2
42039d1c:	024966        	bnei	a9, 4, 42039d22 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x87a>
42039d1f:	002746        	j	42039dc0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x918>
42039d22:	c138      	l32i.n	a3, a1, 48
42039d24:	08ad      	mov.n	a10, a8
42039d26:	03cd      	mov.n	a12, a3
42039d28:	06bd      	mov.n	a11, a6
42039d2a:	1afb81        	l32r	a8, 42000918 <_stext+0x8f8> (40001200 <memmove>)
42039d2d:	0008e0        	callx8	a8
42039d30:	a198      	l32i.n	a9, a1, 40
42039d32:	551b      	addi.n	a5, a5, 1
42039d34:	8a3a      	add.n	a8, a10, a3
42039d36:	e89957        	bne	a9, a5, 42039d22 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x87a>
42039d39:	6188      	l32i.n	a8, a1, 24
42039d3b:	690b      	addi.n	a6, a9, -1
42039d3d:	c06680        	sub	a6, a6, a8
42039d40:	c188      	l32i.n	a8, a1, 48
42039d42:	6199      	s32i.n	a9, a1, 24
42039d44:	826680        	mull	a6, a6, a8
42039d47:	828a      	add.n	a8, a2, a8
42039d49:	668a      	add.n	a6, a6, a8
42039d4b:	ffd1c6        	j	42039c96 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ee>
42039d4e:	820000        	mull	a0, a0, a0
42039d51:	901121        	l32r	a2, 4201dd98 <roo_display::(anonymous namespace)::GetSmoothRoundRectCornersPixelColor(roo_display::SmoothShape::RoundRectCorners const&, short, short)+0x30c> (5089ef81 <_rtc_force_slow_end+0x89ed61>)
42039d54:	c068      	l32i.n	a6, a0, 48
42039d56:	c188      	l32i.n	a8, a1, 48
42039d58:	826680        	mull	a6, a6, a8
42039d5b:	828a      	add.n	a8, a2, a8
42039d5d:	668a      	add.n	a6, a6, a8
42039d5f:	a188      	l32i.n	a8, a1, 40
42039d61:	6189      	s32i.n	a8, a1, 24
42039d63:	ffcbc6        	j	42039c96 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ee>
42039d66:	000000        	ill
42039d69:	d138      	l32i.n	a3, a1, 52
42039d6b:	122182        	l32i	a8, a1, 72
42039d6e:	0c9152        	l16si	a5, a1, 24
42039d71:	44a387        	bge	a3, a8, 42039db9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x911>
42039d74:	0527d2        	l32i	a13, a7, 20
42039d77:	20b330        	or	a11, a3, a3
42039d7a:	05cd      	mov.n	a12, a5
42039d7c:	07ad      	mov.n	a10, a7
42039d7e:	ff1425        	call8	42038ec0 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039d81:	7188      	l32i.n	a8, a1, 28
42039d83:	02a9      	s32i.n	a10, a2, 0
42039d85:	331b      	addi.n	a3, a3, 1
42039d87:	224b      	addi.n	a2, a2, 4
42039d89:	e72387        	blt	a3, a8, 42039d74 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8cc>
42039d8c:	162182        	l32i	a8, a1, 88
42039d8f:	1521b2        	l32i	a11, a1, 84
42039d92:	368a      	add.n	a3, a6, a8
42039d94:	57d8      	l32i.n	a13, a7, 20
42039d96:	05cd      	mov.n	a12, a5
42039d98:	07ad      	mov.n	a10, a7
42039d9a:	ff1265        	call8	42038ec0 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039d9d:	132192        	l32i	a9, a1, 76
42039da0:	fcc982        	addi	a8, a9, -4
42039da3:	418280        	srli	a8, a8, 2
42039da6:	239a      	add.n	a2, a3, a9
42039da8:	881b      	addi.n	a8, a8, 1
42039daa:	038876        	loop	a8, 42039db1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x909>
42039dad:	03a9      	s32i.n	a10, a3, 0
42039daf:	334b      	addi.n	a3, a3, 4
42039db1:	142182        	l32i	a8, a1, 80
42039db4:	9189      	s32i.n	a8, a1, 36
42039db6:	ffa286        	j	42039c44 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x79c>
42039db9:	03bd      	mov.n	a11, a3
42039dbb:	063d      	mov.n	a3, a6
42039dbd:	fff4c6        	j	42039d94 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x8ec>
42039dc0:	a188      	l32i.n	a8, a1, 40
42039dc2:	029d      	mov.n	a9, a2
42039dc4:	c08850        	sub	a8, a8, a5
42039dc7:	058876        	loop	a8, 42039dd0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x928>
42039dca:	06a8      	l32i.n	a10, a6, 0
42039dcc:	09a9      	s32i.n	a10, a9, 0
42039dce:	994b      	addi.n	a9, a9, 4
42039dd0:	a198      	l32i.n	a9, a1, 40
42039dd2:	6188      	l32i.n	a8, a1, 24
42039dd4:	690b      	addi.n	a6, a9, -1
42039dd6:	c06680        	sub	a6, a6, a8
42039dd9:	824b      	addi.n	a8, a2, 4
42039ddb:	a06680        	addx4	a6, a6, a8
42039dde:	6199      	s32i.n	a9, a1, 24
42039de0:	ffac86        	j	42039c96 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x7ee>
