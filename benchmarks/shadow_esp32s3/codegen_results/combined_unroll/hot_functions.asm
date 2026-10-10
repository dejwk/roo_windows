
/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203316c <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
4203316c:	006136        	entry	a1, 48
4203316f:	002282        	l32i	a8, a2, 0
42033172:	20f770        	or	a15, a7, a7
42033175:	0b2882        	l32i	a8, a8, 44
42033178:	233380        	sext	a3, a3, 15
4203317b:	234480        	sext	a4, a4, 15
4203317e:	235580        	sext	a5, a5, 15
42033181:	236680        	sext	a6, a6, 15
42033184:	02ad      	mov.n	a10, a2
42033186:	31f9      	s32i.n	a15, a1, 12
42033188:	06ed      	mov.n	a14, a6
4203318a:	05dd      	mov.n	a13, a5
4203318c:	04cd      	mov.n	a12, a4
4203318e:	03bd      	mov.n	a11, a3
42033190:	0008e0        	callx8	a8
42033193:	027d      	mov.n	a7, a2
42033195:	0a2d      	mov.n	a2, a10
42033197:	facc      	bnez.n	a10, 420331aa <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
42033199:	31f8      	l32i.n	a15, a1, 12
4203319b:	06ed      	mov.n	a14, a6
4203319d:	05dd      	mov.n	a13, a5
4203319f:	04cd      	mov.n	a12, a4
420331a1:	03bd      	mov.n	a11, a3
420331a3:	07ad      	mov.n	a10, a7
420331a5:	dc56e5        	call8	4200f714 <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
420331a8:	0a2d      	mov.n	a2, a10
420331aa:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033ee0 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033ee0:	00a136        	entry	a1, 80
42033ee3:	3169      	s32i.n	a6, a1, 12
42033ee5:	2159      	s32i.n	a5, a1, 8
42033ee7:	060c      	movi.n	a6, 0
42033ee9:	1cc172        	addi	a7, a1, 28
42033eec:	2188      	l32i.n	a8, a1, 8
42033eee:	029687        	bne	a6, a8, 42033ef4 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033ef1:	f01d      	retw.n
42033ef3:	938200        	movnez	a8, a2, a0
42033ef6:	22d200        	orb	b13, b2, b0
42033ef9:	018917        	bany	a9, a1, 42033efe <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033efc:	009482        	l16si	a8, a4, 0
42033eff:	01b8      	l32i.n	a11, a1, 0
42033f01:	1622a2        	l32i	a10, a2, 88
42033f04:	08cd      	mov.n	a12, a8
42033f06:	0067d2        	s32i	a13, a7, 0
42033f09:	20e770        	or	a14, a7, a7
42033f0c:	016182        	s32i	a8, a1, 4
42033f0f:	201110        	or	a1, a1, a1
42033f12:	fffa25        	call8	42033eb4 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033f15:	182282        	l32i	a8, a2, 96
42033f18:	1922b2        	l32i	a11, a2, 100
42033f1b:	8189      	s32i.n	a8, a1, 32
42033f1d:	20c152        	addi	a5, a1, 32
42033f20:	050b16        	beqz	a11, 42033f74 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033f23:	0bc8      	l32i.n	a12, a11, 0
42033f25:	24c1a2        	addi	a10, a1, 36
42033f28:	201110        	or	a1, a1, a1
42033f2b:	ff3665        	call8	42033290 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f2e:	07d8      	l32i.n	a13, a7, 0
42033f30:	11c8      	l32i.n	a12, a1, 4
42033f32:	0021b2        	l32i	a11, a1, 0
42033f35:	24c1a2        	addi	a10, a1, 36
42033f38:	201110        	or	a1, a1, a1
42033f3b:	fe1865        	call8	420320c0 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f3e:	07a9      	s32i.n	a10, a7, 0
42033f40:	24c1a2        	addi	a10, a1, 36
42033f43:	201110        	or	a1, a1, a1
42033f46:	f894a5        	call8	4202c890 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f49:	1922b2        	l32i	a11, a2, 100
42033f4c:	24c1a2        	addi	a10, a1, 36
42033f4f:	002bc2        	l32i	a12, a11, 0
42033f52:	201110        	or	a1, a1, a1
42033f55:	ff33a5        	call8	42033290 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f58:	05d8      	l32i.n	a13, a5, 0
42033f5a:	0094c2        	l16si	a12, a4, 0
42033f5d:	0093b2        	l16si	a11, a3, 0
42033f60:	24c1a2        	addi	a10, a1, 36
42033f63:	201110        	or	a1, a1, a1
42033f66:	fe15a5        	call8	420320c0 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f69:	05a9      	s32i.n	a10, a5, 0
42033f6b:	24c1a2        	addi	a10, a1, 36
42033f6e:	201110        	or	a1, a1, a1
42033f71:	f891e5        	call8	4202c890 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f74:	0094c2        	l16si	a12, a4, 0
42033f77:	0093b2        	l16si	a11, a3, 0
42033f7a:	07d8      	l32i.n	a13, a7, 0
42033f7c:	05ed      	mov.n	a14, a5
42033f7e:	a24b      	addi.n	a10, a2, 4
42033f80:	201110        	or	a1, a1, a1
42033f83:	0630e5        	call8	4203a290 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f86:	3188      	l32i.n	a8, a1, 12
42033f88:	332b      	addi.n	a3, a3, 2
42033f8a:	a08680        	addx4	a8, a6, a8
42033f8d:	08a9      	s32i.n	a10, a8, 0
42033f8f:	661b      	addi.n	a6, a6, 1
42033f91:	442b      	addi.n	a4, a4, 2
42033f93:	ffd546        	j	42033eec <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f96:	c1a200        	mul16u	a10, a2, a0
42033f99:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f9c:	20          	.byte	0x20
42033f9d:	f88f25        	call8	4202c890 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033fa0:	02ad      	mov.n	a10, a2
42033fa2:	201110        	or	a1, a1, a1
42033fa5:	3e61a5        	call8	420725c0 <_Unwind_Resume>
42033fa8:	fffac6        	j	42033f97 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
42038758:	006136        	entry	a1, 48
4203875b:	189192        	l16si	a9, a1, 48
4203875e:	3169      	s32i.n	a6, a1, 12
42038760:	a1cb      	addi.n	a10, a1, 12
42038762:	1a91b2        	l16si	a11, a1, 52
42038765:	028d      	mov.n	a8, a2
42038767:	747070        	extui	a7, a7, 0, 8
4203876a:	02a927        	bge	a9, a2, 42038770 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
4203876d:	002f46        	j	4203882e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd6>
42038770:	02a497        	bge	a4, a9, 42038776 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
42038773:	002dc6        	j	4203882e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd6>
42038776:	02ab37        	bge	a11, a3, 4203877c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
42038779:	002c46        	j	4203882e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd6>
4203877c:	7225b0        	salt	a2, a5, a11
4203877f:	02a5b7        	bge	a5, a11, 42038785 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2d>
42038782:	002a06        	j	4203882e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd6>
42038785:	000ac2        	l8ui	a12, a10, 0
42038788:	0a6c16        	beqz	a12, 42038832 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xda>
4203878b:	d8ca      	add.n	a13, a8, a12
4203878d:	02ad97        	bge	a13, a9, 42038793 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3b>
42038790:	002786        	j	42038832 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xda>
42038793:	e3ca      	add.n	a14, a3, a12
42038795:	02aeb7        	bge	a14, a11, 4203879b <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x43>
42038798:	002586        	j	42038832 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xda>
4203879b:	c0dd90        	sub	a13, a13, a9
4203879e:	c0eeb0        	sub	a14, a14, a11
420387a1:	f4d0d0        	extui	a13, a13, 0, 16
420387a4:	f4e0e0        	extui	a14, a14, 0, 16
420387a7:	208de0        	or	a8, a13, a14
420387aa:	238880        	sext	a8, a8, 15
420387ad:	18b896        	bltz	a8, 4203893c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e4>
420387b0:	c1ddd0        	mul16u	a13, a13, a13
420387b3:	c1eee0        	mul16u	a14, a14, a14
420387b6:	11ccc0        	slli	a12, a12, 4
420387b9:	cc7a      	add.n	a12, a12, a7
420387bb:	ddea      	add.n	a13, a13, a14
420387bd:	c18cc0        	mul16u	a8, a12, a12
420387c0:	11dd80        	slli	a13, a13, 8
420387c3:	65bd87        	bgeu	a13, a8, 4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
420387c6:	f1cc82        	addi	a8, a12, -15
420387c9:	c18880        	mul16u	a8, a8, a8
420387cc:	ffa022        	movi	a2, 255
420387cf:	593d87        	bltu	a13, a8, 4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
420387d2:	40fd90        	nsau	a9, a13
420387d5:	dfc992        	addi	a9, a9, -33
420387d8:	609090        	neg	a9, a9
420387db:	219190        	srai	a9, a9, 1
420387de:	fec992        	addi	a9, a9, -2
420387e1:	380c      	movi.n	a8, 3
420387e3:	401900        	ssl	a9
420387e6:	a18800        	sll	a8, a8
420387e9:	c29d80        	quou	a9, a13, a8
420387ec:	998a      	add.n	a9, a9, a8
420387ee:	419190        	srli	a9, a9, 1
420387f1:	c28d90        	quou	a8, a13, a9
420387f4:	889a      	add.n	a8, a8, a9
420387f6:	418180        	srli	a8, a8, 1
420387f9:	c29d80        	quou	a9, a13, a8
420387fc:	998a      	add.n	a9, a9, a8
420387fe:	204181        	l32r	a8, 42000904 <_stext+0x8e4> (10000 <UserFrameTotalSize+0xff00>)
42038801:	419190        	srli	a9, a9, 1
42038804:	241987        	beq	a9, a8, 4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
42038807:	828990        	mull	a8, a9, a9
4203880a:	020c      	movi.n	a2, 0
4203880c:	628d80        	saltu	a8, a13, a8
4203880f:	c09980        	sub	a9, a9, a8
42038812:	c0cc90        	sub	a12, a12, a9
42038815:	f480c0        	extui	a8, a12, 0, 16
42038818:	239c80        	sext	a9, a12, 15
4203881b:	0decf7        	bbsi	a12, 15, 4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
4203881e:	fa0c      	movi.n	a10, 15
42038820:	4c2a97        	blt	a10, a9, 42038870 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x118>
42038823:	1198c0        	slli	a9, a8, 4
42038826:	808890        	add	a8, a8, a9
42038829:	742080        	extui	a2, a8, 0, 8
4203882c:	f01d      	retw.n
4203882e:	020c      	movi.n	a2, 0
42038830:	f01d      	retw.n
42038832:	010ac2        	l8ui	a12, a10, 1
42038835:	ccbc      	beqz.n	a12, 42038875 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x11d>
42038837:	c0d4c0        	sub	a13, a4, a12
4203883a:	3729d7        	blt	a9, a13, 42038875 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x11d>
4203883d:	d3ca      	add.n	a13, a3, a12
4203883f:	322db7        	blt	a13, a11, 42038875 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x11d>
42038842:	c08c40        	sub	a8, a12, a4
42038845:	889a      	add.n	a8, a8, a9
42038847:	c0ddb0        	sub	a13, a13, a11
4203884a:	f48080        	extui	a8, a8, 0, 16
4203884d:	f4d0d0        	extui	a13, a13, 0, 16
42038850:	2098d0        	or	a9, a8, a13
42038853:	239980        	sext	a9, a9, 15
42038856:	0fb9d6        	bgez	a9, 42038955 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1fd>
42038859:	c08c80        	sub	a8, a12, a8
4203885c:	40f890        	nsau	a9, a8
4203885f:	419590        	srli	a9, a9, 5
42038862:	052816        	beqz	a8, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038865:	c0dcd0        	sub	a13, a12, a13
42038868:	180c      	movi.n	a8, 1
4203886a:	9389d0        	movnez	a8, a9, a13
4203886d:	047856        	bnez	a8, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038870:	ffa022        	movi	a2, 255
42038873:	f01d      	retw.n
42038875:	020ac2        	l8ui	a12, a10, 2
42038878:	048c16        	beqz	a12, 420388c4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x16c>
4203887b:	c0d4c0        	sub	a13, a4, a12
4203887e:	4229d7        	blt	a9, a13, 420388c4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x16c>
42038881:	c0d5c0        	sub	a13, a5, a12
42038884:	3c2bd7        	blt	a11, a13, 420388c4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x16c>
42038887:	c04c40        	sub	a4, a12, a4
4203888a:	c05c50        	sub	a5, a12, a5
4203888d:	949a      	add.n	a9, a4, a9
4203888f:	85ba      	add.n	a8, a5, a11
42038891:	f49090        	extui	a9, a9, 0, 16
42038894:	f48080        	extui	a8, a8, 0, 16
42038897:	20a980        	or	a10, a9, a8
4203889a:	23aa80        	sext	a10, a10, 15
4203889d:	123ad6        	bgez	a10, 420389c4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x26c>
420388a0:	c09c90        	sub	a9, a12, a9
420388a3:	40f9a0        	nsau	a10, a9
420388a6:	41a5a0        	srli	a10, a10, 5
420388a9:	00b916        	beqz	a9, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
420388ac:	c08c80        	sub	a8, a12, a8
420388af:	01a092        	movi	a9, 1
420388b2:	939a80        	movnez	a9, a10, a8
420388b5:	fb7916        	beqz	a9, 42038870 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x118>
420388b8:	1127c0        	slli	a2, a7, 4
420388bb:	272a      	add.n	a2, a7, a2
420388bd:	742020        	extui	a2, a2, 0, 8
420388c0:	f01d      	retw.n
420388c2:	a20000        	muluh	a0, a0, a0
420388c5:	030a      	add.n	a0, a3, a0
420388c7:	aabc      	beqz.n	a10, 42038905 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1ad>
420388c9:	c8aa      	add.n	a12, a8, a10
420388cb:	362c97        	blt	a12, a9, 42038905 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1ad>
420388ce:	c0d5a0        	sub	a13, a5, a10
420388d1:	302bd7        	blt	a11, a13, 42038905 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1ad>
420388d4:	c05a50        	sub	a5, a10, a5
420388d7:	c0cc90        	sub	a12, a12, a9
420388da:	85ba      	add.n	a8, a5, a11
420388dc:	f4c0c0        	extui	a12, a12, 0, 16
420388df:	f48080        	extui	a8, a8, 0, 16
420388e2:	209c80        	or	a9, a12, a8
420388e5:	239980        	sext	a9, a9, 15
420388e8:	0e89d6        	bgez	a9, 420389d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x27c>
420388eb:	c0cac0        	sub	a12, a10, a12
420388ee:	40fc90        	nsau	a9, a12
420388f1:	419590        	srli	a9, a9, 5
420388f4:	fc0c16        	beqz	a12, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
420388f7:	c08a80        	sub	a8, a10, a8
420388fa:	1a0c      	movi.n	a10, 1
420388fc:	839a80        	moveqz	a9, a10, a8
420388ff:	f6d916        	beqz	a9, 42038870 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x118>
42038902:	ffec86        	j	420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038905:	ea0c      	movi.n	a10, 14
42038907:	ffa022        	movi	a2, 255
4203890a:	02ba77        	bgeu	a10, a7, 42038910 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1b8>
4203890d:	ffc6c6        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
42038910:	c08980        	sub	a8, a9, a8
42038913:	40f8a0        	nsau	a10, a8
42038916:	1c0c      	movi.n	a12, 1
42038918:	41a5a0        	srli	a10, a10, 5
4203891b:	f99816        	beqz	a8, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
4203891e:	c08940        	sub	a8, a9, a4
42038921:	83ac80        	moveqz	a10, a12, a8
42038924:	f90a56        	bnez	a10, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038927:	c03b30        	sub	a3, a11, a3
4203892a:	83ac30        	moveqz	a10, a12, a3
4203892d:	f87a56        	bnez	a10, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038930:	c05b50        	sub	a5, a11, a5
42038933:	93ca50        	movnez	a12, a10, a5
42038936:	f7ec56        	bnez	a12, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038939:	f01d      	retw.n
4203893b:	00          	.byte	00
4203893c:	c0dcd0        	sub	a13, a12, a13
4203893f:	180c      	movi.n	a8, 1
42038941:	8328d0        	moveqz	a2, a8, a13
42038944:	f70256        	bnez	a2, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038947:	c0ece0        	sub	a14, a12, a14
4203894a:	9382e0        	movnez	a8, a2, a14
4203894d:	ffa022        	movi	a2, 255
42038950:	f64856        	bnez	a8, 420388b8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x160>
42038953:	f01d      	retw.n
42038955:	c18880        	mul16u	a8, a8, a8
42038958:	c1ddd0        	mul16u	a13, a13, a13
4203895b:	11ccc0        	slli	a12, a12, 4
4203895e:	cc7a      	add.n	a12, a12, a7
42038960:	88da      	add.n	a8, a8, a13
42038962:	c19cc0        	mul16u	a9, a12, a12
42038965:	118880        	slli	a8, a8, 8
42038968:	020c      	movi.n	a2, 0
4203896a:	023897        	bltu	a8, a9, 42038970 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x218>
4203896d:	ffaec6        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
42038970:	f1cc92        	addi	a9, a12, -15
42038973:	c19990        	mul16u	a9, a9, a9
42038976:	ffa022        	movi	a2, 255
42038979:	02b897        	bgeu	a8, a9, 4203897f <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x227>
4203897c:	ffab06        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
4203897f:	40f890        	nsau	a9, a8
42038982:	dfc992        	addi	a9, a9, -33
42038985:	609090        	neg	a9, a9
42038988:	219190        	srai	a9, a9, 1
4203898b:	fec992        	addi	a9, a9, -2
4203898e:	3a0c      	movi.n	a10, 3
42038990:	401900        	ssl	a9
42038993:	a1aa00        	sll	a10, a10
42038996:	c298a0        	quou	a9, a8, a10
42038999:	99aa      	add.n	a9, a9, a10
4203899b:	419190        	srli	a9, a9, 1
4203899e:	c2a890        	quou	a10, a8, a9
420389a1:	aa9a      	add.n	a10, a10, a9
420389a3:	41a1a0        	srli	a10, a10, 1
420389a6:	c298a0        	quou	a9, a8, a10
420389a9:	99aa      	add.n	a9, a9, a10
420389ab:	1fd6a1        	l32r	a10, 42000904 <_stext+0x8e4> (10000 <UserFrameTotalSize+0xff00>)
420389ae:	419190        	srli	a9, a9, 1
420389b1:	0299a7        	bne	a9, a10, 420389b7 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x25f>
420389b4:	ff9d06        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
420389b7:	82a990        	mull	a10, a9, a9
420389ba:	020c      	movi.n	a2, 0
420389bc:	6288a0        	saltu	a8, a8, a10
420389bf:	ff9306        	j	4203880f <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xb7>
420389c2:	800000        	add	a0, a0, a0
420389c5:	c188      	l32i.n	a8, a1, 48
420389c7:	c19990        	mul16u	a9, a9, a9
420389ca:	11ccc0        	slli	a12, a12, 4
420389cd:	cc7a      	add.n	a12, a12, a7
420389cf:	889a      	add.n	a8, a8, a9
420389d1:	ffe346        	j	42038962 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x20a>
420389d4:	c19880        	mul16u	a9, a8, a8
420389d7:	c1ccc0        	mul16u	a12, a12, a12
420389da:	118ac0        	slli	a8, a10, 4
420389dd:	887a      	add.n	a8, a8, a7
420389df:	9c9a      	add.n	a9, a12, a9
420389e1:	c1a880        	mul16u	a10, a8, a8
420389e4:	119980        	slli	a9, a9, 8
420389e7:	020c      	movi.n	a2, 0
420389e9:	0239a7        	bltu	a9, a10, 420389ef <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x297>
420389ec:	ff8f06        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
420389ef:	f1c8a2        	addi	a10, a8, -15
420389f2:	c1aaa0        	mul16u	a10, a10, a10
420389f5:	ffa022        	movi	a2, 255
420389f8:	02b9a7        	bgeu	a9, a10, 420389fe <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a6>
420389fb:	ff8b46        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
420389fe:	40f9a0        	nsau	a10, a9
42038a01:	dfcaa2        	addi	a10, a10, -33
42038a04:	60a0a0        	neg	a10, a10
42038a07:	21a1a0        	srai	a10, a10, 1
42038a0a:	fecaa2        	addi	a10, a10, -2
42038a0d:	3b0c      	movi.n	a11, 3
42038a0f:	401a00        	ssl	a10
42038a12:	a1bb00        	sll	a11, a11
42038a15:	c2a9b0        	quou	a10, a9, a11
42038a18:	aaba      	add.n	a10, a10, a11
42038a1a:	41a1a0        	srli	a10, a10, 1
42038a1d:	c2b9a0        	quou	a11, a9, a10
42038a20:	bbaa      	add.n	a11, a11, a10
42038a22:	41b1b0        	srli	a11, a11, 1
42038a25:	c2a9b0        	quou	a10, a9, a11
42038a28:	aaba      	add.n	a10, a10, a11
42038a2a:	1fb6b1        	l32r	a11, 42000904 <_stext+0x8e4> (10000 <UserFrameTotalSize+0xff00>)
42038a2d:	41a1a0        	srli	a10, a10, 1
42038a30:	029ab7        	bne	a10, a11, 42038a36 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2de>
42038a33:	ff7d46        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
42038a36:	82baa0        	mull	a11, a10, a10
42038a39:	020c      	movi.n	a2, 0
42038a3b:	6299b0        	saltu	a9, a9, a11
42038a3e:	c0aa90        	sub	a10, a10, a9
42038a41:	c088a0        	sub	a8, a8, a10
42038a44:	f49080        	extui	a9, a8, 0, 16
42038a47:	23a880        	sext	a10, a8, 15
42038a4a:	0268f7        	bbci	a8, 15, 42038a50 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2f8>
42038a4d:	ff76c6        	j	4203882c <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd4>
42038a50:	f80c      	movi.n	a8, 15
42038a52:	02a8a7        	bge	a8, a10, 42038a58 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x300>
42038a55:	ff85c6        	j	42038870 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x118>
42038a58:	1189c0        	slli	a8, a9, 4
42038a5b:	998a      	add.n	a9, a9, a8
42038a5d:	742090        	extui	a2, a9, 0, 8
42038a60:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203a8a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
4203a8a8:	006136        	entry	a1, 48
4203a8ab:	02bd      	mov.n	a11, a2
4203a8ad:	0f0222        	l8ui	a2, a2, 15
4203a8b0:	239480        	sext	a9, a4, 15
4203a8b3:	235580        	sext	a5, a5, 15
4203a8b6:	11a216        	beqz	a2, 4203a9d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12c>
4203a8b9:	019b82        	l16si	a8, a11, 2
4203a8bc:	069be2        	l16si	a14, a11, 12
4203a8bf:	c05580        	sub	a5, a5, a8
4203a8c2:	1185c0        	slli	a8, a5, 4
4203a8c5:	888b      	addi.n	a8, a8, 8
4203a8c7:	039bc2        	l16si	a12, a11, 6
4203a8ca:	c058e0        	sub	a5, a8, a14
4203a8cd:	0693d2        	l16si	a13, a3, 12
4203a8d0:	11ccc0        	slli	a12, a12, 4
4203a8d3:	009ba2        	l16si	a10, a11, 0
4203a8d6:	029b62        	l16si	a6, a11, 4
4203a8d9:	455a      	add.n	a4, a5, a5
4203a8db:	c088d0        	sub	a8, a8, a13
4203a8de:	72f4c0        	salt	a15, a4, a12
4203a8e1:	11d6c0        	slli	a13, a6, 4
4203a8e4:	c099a0        	sub	a9, a9, a10
4203a8e7:	ff0b      	addi.n	a15, a15, -1
4203a8e9:	a88a      	add.n	a10, a8, a8
4203a8eb:	01d9      	s32i.n	a13, a1, 0
4203a8ed:	c07c80        	sub	a7, a12, a8
4203a8f0:	31e9      	s32i.n	a14, a1, 12
4203a8f2:	1199c0        	slli	a9, a9, 4
4203a8f5:	72eac0        	salt	a14, a10, a12
4203a8f8:	60f0f0        	neg	a15, a15
4203a8fb:	998b      	addi.n	a9, a9, 8
4203a8fd:	ee0b      	addi.n	a14, a14, -1
4203a8ff:	74f0f0        	extui	a15, a15, 0, 8
4203a902:	437780        	min	a7, a7, a8
4203a905:	0188      	l32i.n	a8, a1, 0
4203a907:	11a9      	s32i.n	a10, a1, 4
4203a909:	60e0e0        	neg	a14, a14
4203a90c:	c0ad90        	sub	a10, a13, a9
4203a90f:	699a      	add.n	a6, a9, a9
4203a911:	c0dc50        	sub	a13, a12, a5
4203a914:	0261f2        	s32i	a15, a1, 8
4203a917:	74e0e0        	extui	a14, a14, 0, 8
4203a91a:	43aa90        	min	a10, a10, a9
4203a91d:	43dd50        	min	a13, a13, a5
4203a920:	02a687        	bge	a6, a8, 4203a926 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x7e>
4203a923:	002c46        	j	4203a9d8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x130>
4203a926:	02a4c7        	bge	a4, a12, 4203a92c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x84>
4203a929:	0038c6        	j	4203aa10 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x168>
4203a92c:	012182        	l32i	a8, a1, 4
4203a92f:	0a0bf2        	l8ui	a15, a11, 10
4203a932:	0228c7        	blt	a8, a12, 4203a938 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x90>
4203a935:	00a2c6        	j	4203abc4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x31c>
4203a938:	119fc0        	slli	a9, a15, 4
4203a93b:	090b82        	l8ui	a8, a11, 9
4203a93e:	02ad97        	bge	a13, a9, 4203a944 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9c>
4203a941:	002c06        	j	4203a9f5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x14d>
4203a944:	236880        	sext	a6, a8, 15
4203a947:	1166c0        	slli	a6, a6, 4
4203a94a:	022767        	blt	a7, a6, 4203a950 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xa8>
4203a94d:	008cc6        	j	4203ab84 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2dc>
4203a950:	022162        	l32i	a6, a1, 8
4203a953:	0216e7        	beq	a6, a14, 4203a959 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb1>
4203a956:	003586        	j	4203aa30 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x188>
4203a959:	53e8f0        	max	a14, a8, a15
4203a95c:	23ee80        	sext	a14, a14, 15
4203a95f:	11eec0        	slli	a14, a14, 4
4203a962:	02aae7        	bge	a10, a14, 4203a968 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc0>
4203a965:	002506        	j	4203a9fd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x155>
4203a968:	53ed70        	max	a14, a13, a7
4203a96b:	02aae7        	bge	a10, a14, 4203a971 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc9>
4203a96e:	009886        	j	4203abd4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x32c>
4203a971:	f8cdd2        	addi	a13, a13, -8
4203a974:	60d0d0        	neg	a13, a13
4203a977:	0224c7        	blt	a4, a12, 4203a97d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd5>
4203a97a:	009d86        	j	4203abf4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x34c>
4203a97d:	3188      	l32i.n	a8, a1, 12
4203a97f:	1b0c      	movi.n	a11, 1
4203a981:	c0dd80        	sub	a13, a13, a8
4203a984:	21c4d0        	srai	a12, a13, 4
4203a987:	0082c6        	j	4203ab96 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2ee>
4203a98a:	a70000        	ee.stf.64.xp	f0, f10, a0, a0
4203a98d:	9d01a7        	bnone	a1, a10, 4203a92e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x86>
4203a990:	0d          	.byte	0xd
4203a991:	0e03c2        	l8ui	a12, a3, 14
4203a994:	c08980        	sub	a8, a9, a8
4203a997:	0813a2        	l16ui	a10, a3, 16
4203a99a:	c088c0        	sub	a8, a8, a12
4203a99d:	17e591        	l32r	a9, 42000934 <_stext+0x914> (80000 <UserFrameTotalSize+0x7ff00>)
4203a9a0:	8288a0        	mull	a8, a8, a10
4203a9a3:	889a      	add.n	a8, a8, a9
4203a9a5:	218c80        	srai	a8, a8, 12
4203a9a8:	ffa092        	movi	a9, 255
4203a9ab:	438890        	min	a8, a8, a9
4203a9ae:	090c      	movi.n	a9, 0
4203a9b0:	538890        	max	a8, a8, a9
4203a9b3:	17e191        	l32r	a9, 42000938 <_stext+0x918> (3c0c141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
4203a9b6:	898a      	add.n	a8, a9, a8
4203a9b8:	000882        	l8ui	a8, a8, 0
4203a9bb:	17d591        	l32r	a9, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
4203a9be:	d1bb80        	mul16s	a11, a11, a8
4203a9c1:	882a      	add.n	a8, a8, a2
4203a9c3:	7fcbb2        	addi	a11, a11, 127
4203a9c6:	b29b90        	mulsh	a9, a11, a9
4203a9c9:	bb9a      	add.n	a11, a11, a9
4203a9cb:	21b7b0        	srai	a11, a11, 7
4203a9ce:	c088b0        	sub	a8, a8, a11
4203a9d1:	742080        	extui	a2, a8, 0, 8
4203a9d4:	f01d      	retw.n
4203a9d6:	c70000        	ee.stf.64.xp	f0, f12, a0, a0
4203a9d9:	4602a4        	ee.ld.qacc_h.l.128.ip	a10, 0xfffff820
4203a9dc:	880061        	l32r	a6, 4201c9dc <roo_display::(anonymous namespace)::GetSmoothArcPixelColor(roo_display::SmoothShape::Arc const&, short, short)+0x260> (db81ff96 <_rtc_reserved_end+0x7b71ff96>)
4203a9df:	0bf211        	l32r	a1, 41ffd9a8 <_coredump_iram_end+0x1c775a8>
4203a9e2:	c70b      	addi.n	a12, a7, -1
4203a9e4:	0228      	l32i.n	a2, a2, 0
4203a9e6:	007086        	j	4203abac <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x304>
4203a9e9:	119fc0        	slli	a9, a15, 4
4203a9ec:	080b82        	l8ui	a8, a11, 8
4203a9ef:	022d97        	blt	a13, a9, 4203a9f5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x14d>
4203a9f2:	ffd386        	j	4203a944 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9c>
4203a9f5:	2168      	l32i.n	a6, a1, 8
4203a9f7:	0296e7        	bne	a6, a14, 4203a9fd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x155>
4203a9fa:	ffd6c6        	j	4203a959 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb1>
4203a9fd:	091be2        	l16ui	a14, a11, 18
4203aa00:	60c0e0        	neg	a12, a14
4203aa03:	022ac7        	blt	a10, a12, 4203aa09 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x161>
4203aa06:	2fadc7        	bge	a13, a12, 4203aa39 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x191>
4203aa09:	0b0c      	movi.n	a11, 0
4203aa0b:	0b2d      	mov.n	a2, a11
4203aa0d:	003186        	j	4203aad7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x22f>
4203aa10:	1188      	l32i.n	a8, a1, 4
4203aa12:	090bf2        	l8ui	a15, a11, 9
4203aa15:	02a8c7        	bge	a8, a12, 4203aa1b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x173>
4203aa18:	005646        	j	4203ab75 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2cd>
4203aa1b:	0a0b82        	l8ui	a8, a11, 10
4203aa1e:	119fc0        	slli	a9, a15, 4
4203aa21:	d82d97        	blt	a13, a9, 4203a9fd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x155>
4203aa24:	236880        	sext	a6, a8, 15
4203aa27:	1166c0        	slli	a6, a6, 4
4203aa2a:	022767        	blt	a7, a6, 4203aa30 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x188>
4203aa2d:	0054c6        	j	4203ab84 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2dc>
4203aa30:	091be2        	l16ui	a14, a11, 18
4203aa33:	60c0e0        	neg	a12, a14
4203aa36:	cf2ac7        	blt	a10, a12, 4203aa09 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x161>
4203aa39:	c0c9a0        	sub	a12, a9, a10
4203aa3c:	0f0c      	movi.n	a15, 0
4203aa3e:	c069d0        	sub	a6, a9, a13
4203aa41:	022fc7        	blt	a15, a12, 4203aa47 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x19f>
4203aa44:	005646        	j	4203aba1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2f9>
4203aa47:	022f67        	blt	a15, a6, 4203aa4d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a5>
4203aa4a:	0054c6        	j	4203aba1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2f9>
4203aa4d:	e9ea      	add.n	a14, a9, a14
4203aa4f:	82ccc0        	mull	a12, a12, a12
4203aa52:	826660        	mull	a6, a6, a6
4203aa55:	ee1b      	addi.n	a14, a14, 1
4203aa57:	82eee0        	mull	a14, a14, a14
4203aa5a:	6c6a      	add.n	a6, a12, a6
4203aa5c:	0236e7        	bltu	a6, a14, 4203aa62 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ba>
4203aa5f:	007086        	j	4203ac25 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x37d>
4203aa62:	f4c060        	extui	a12, a6, 0, 16
4203aa65:	4246b6        	bltui	a6, 4, 4203aaab <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x203>
4203aa68:	40f6c0        	nsau	a12, a6
4203aa6b:	dfccc2        	addi	a12, a12, -33
4203aa6e:	60c0c0        	neg	a12, a12
4203aa71:	21c1c0        	srai	a12, a12, 1
4203aa74:	feccc2        	addi	a12, a12, -2
4203aa77:	3d0c      	movi.n	a13, 3
4203aa79:	401c00        	ssl	a12
4203aa7c:	a1dd00        	sll	a13, a13
4203aa7f:	c2c6d0        	quou	a12, a6, a13
4203aa82:	80ccd0        	add	a12, a12, a13
4203aa85:	41c1c0        	srli	a12, a12, 1
4203aa88:	c2d6c0        	quou	a13, a6, a12
4203aa8b:	80ddc0        	add	a13, a13, a12
4203aa8e:	41d1d0        	srli	a13, a13, 1
4203aa91:	c2c6d0        	quou	a12, a6, a13
4203aa94:	80ccd0        	add	a12, a12, a13
4203aa97:	179bd1        	l32r	a13, 42000904 <_stext+0x8e4> (10000 <UserFrameTotalSize+0xff00>)
4203aa9a:	41c1c0        	srli	a12, a12, 1
4203aa9d:	051cd7        	beq	a12, a13, 4203aaa6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1fe>
4203aaa0:	82dcc0        	mull	a13, a12, a12
4203aaa3:	01b6d7        	bgeu	a6, a13, 4203aaa8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x200>
4203aaa6:	cc0b      	addi.n	a12, a12, -1
4203aaa8:	f4c0c0        	extui	a12, a12, 0, 16
4203aaab:	0e0bd2        	l8ui	a13, a11, 14
4203aaae:	c09c90        	sub	a9, a12, a9
4203aab1:	081bb2        	l16ui	a11, a11, 16
4203aab4:	c099d0        	sub	a9, a9, a13
4203aab7:	8299b0        	mull	a9, a9, a11
4203aaba:	179eb1        	l32r	a11, 42000934 <_stext+0x914> (80000 <UserFrameTotalSize+0x7ff00>)
4203aabd:	99ba      	add.n	a9, a9, a11
4203aabf:	219c90        	srai	a9, a9, 12
4203aac2:	ffa0b2        	movi	a11, 255
4203aac5:	4399b0        	min	a9, a9, a11
4203aac8:	0b0c      	movi.n	a11, 0
4203aaca:	5399b0        	max	a9, a9, a11
4203aacd:	1798b1        	l32r	a11, 42000930 <_stext+0x910> (3c0c151f <roo_windows::internal::(anonymous namespace)::kKey>)
4203aad0:	9b9a      	add.n	a9, a11, a9
4203aad2:	000922        	l8ui	a2, a9, 0
4203aad5:	02bd      	mov.n	a11, a2
4203aad7:	0913c2        	l16ui	a12, a3, 18
4203aada:	439a70        	min	a9, a10, a7
4203aadd:	60d0c0        	neg	a13, a12
4203aae0:	02a9d7        	bge	a9, a13, 4203aae6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23e>
4203aae3:	ffbb46        	j	4203a9d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12c>
4203aae6:	238880        	sext	a8, a8, 15
4203aae9:	1188c0        	slli	a8, a8, 4
4203aaec:	c0d870        	sub	a13, a8, a7
4203aaef:	0e0c      	movi.n	a14, 0
4203aaf1:	c098a0        	sub	a9, a8, a10
4203aaf4:	022ed7        	blt	a14, a13, 4203aafa <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x252>
4203aaf7:	ffa446        	j	4203a98c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
4203aafa:	022e97        	blt	a14, a9, 4203ab00 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x258>
4203aafd:	ffa2c6        	j	4203a98c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
4203ab00:	c8ca      	add.n	a12, a8, a12
4203ab02:	82ddd0        	mull	a13, a13, a13
4203ab05:	829990        	mull	a9, a9, a9
4203ab08:	cc1b      	addi.n	a12, a12, 1
4203ab0a:	82ccc0        	mull	a12, a12, a12
4203ab0d:	dd9a      	add.n	a13, a13, a9
4203ab0f:	023dc7        	bltu	a13, a12, 4203ab15 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x26d>
4203ab12:	ffaf86        	j	4203a9d4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12c>
4203ab15:	f490d0        	extui	a9, a13, 0, 16
4203ab18:	024df6        	bgeui	a13, 4, 4203ab1e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
4203ab1b:	ff9c86        	j	4203a991 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe9>
4203ab1e:	40fd90        	nsau	a9, a13
4203ab21:	dfc992        	addi	a9, a9, -33
4203ab24:	609090        	neg	a9, a9
4203ab27:	219190        	srai	a9, a9, 1
4203ab2a:	fec992        	addi	a9, a9, -2
4203ab2d:	3a0c      	movi.n	a10, 3
4203ab2f:	401900        	ssl	a9
4203ab32:	a1aa00        	sll	a10, a10
4203ab35:	c29da0        	quou	a9, a13, a10
4203ab38:	99aa      	add.n	a9, a9, a10
4203ab3a:	419190        	srli	a9, a9, 1
4203ab3d:	c2ad90        	quou	a10, a13, a9
4203ab40:	aa9a      	add.n	a10, a10, a9
4203ab42:	41a1a0        	srli	a10, a10, 1
4203ab45:	c29da0        	quou	a9, a13, a10
4203ab48:	8099a0        	add	a9, a9, a10
4203ab4b:	176ea1        	l32r	a10, 42000904 <_stext+0x8e4> (10000 <UserFrameTotalSize+0xff00>)
4203ab4e:	419190        	srli	a9, a9, 1
4203ab51:	0519a7        	beq	a9, a10, 4203ab5a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2b2>
4203ab54:	82a990        	mull	a10, a9, a9
4203ab57:	01bda7        	bgeu	a13, a10, 4203ab5c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2b4>
4203ab5a:	990b      	addi.n	a9, a9, -1
4203ab5c:	f49090        	extui	a9, a9, 0, 16
4203ab5f:	ff8b86        	j	4203a991 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe9>
4203ab62:	f20000        	rems	a0, a0, a0
4203ab65:	080b      	addi.n	a0, a8, -1
4203ab67:	1188      	l32i.n	a8, a1, 4
4203ab69:	119fc0        	slli	a9, a15, 4
4203ab6c:	0828c7        	blt	a8, a12, 4203ab78 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2d0>
4203ab6f:	0b0b82        	l8ui	a8, a11, 11
4203ab72:	ffaac6        	j	4203aa21 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x179>
4203ab75:	119fc0        	slli	a9, a15, 4
4203ab78:	362d97        	blt	a13, a9, 4203abb2 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x30a>
4203ab7b:	208ff0        	or	a8, a15, a15
4203ab7e:	02a797        	bge	a7, a9, 4203ab84 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2dc>
4203ab81:	ff7506        	j	4203a959 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xb1>
4203ab84:	436d70        	min	a6, a13, a7
4203ab87:	2e26a7        	blt	a6, a10, 4203abb9 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x311>
4203ab8a:	f8caa2        	addi	a10, a10, -8
4203ab8d:	60a0a0        	neg	a10, a10
4203ab90:	21c4a0        	srai	a12, a10, 4
4203ab93:	00a0b2        	movi	a11, 0
4203ab96:	20a220        	or	a10, a2, a2
4203ab99:	ffada5        	call8	4203a674 <roo_windows::internal::(anonymous namespace)::SideGradient(int, int, int)>
4203ab9c:	202aa0        	or	a2, a10, a10
4203ab9f:	f01d      	retw.n
4203aba1:	022da7        	blt	a13, a10, 4203aba7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2ff>
4203aba4:	ffc0c6        	j	4203aaab <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x203>
4203aba7:	06cd      	mov.n	a12, a6
4203aba9:	ffbf86        	j	4203aaab <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x203>
4203abac:	119fc0        	slli	a9, a15, 4
4203abaf:	17ad97        	bge	a13, a9, 4203abca <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x322>
4203abb2:	09ed      	mov.n	a14, a9
4203abb4:	0f8d      	mov.n	a8, a15
4203abb6:	ff6a06        	j	4203a962 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xba>
4203abb9:	2168      	l32i.n	a6, a1, 8
4203abbb:	4116e7        	beq	a6, a14, 4203ac00 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x358>
4203abbe:	091be2        	l16ui	a14, a11, 18
4203abc1:	ff9d06        	j	4203aa39 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x191>
4203abc4:	119fc0        	slli	a9, a15, 4
4203abc7:	1a2d97        	blt	a13, a9, 4203abe5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x33d>
4203abca:	096d      	mov.n	a6, a9
4203abcc:	0f8d      	mov.n	a8, a15
4203abce:	ff5e06        	j	4203a94a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xa2>
4203abd1:	00          	.byte	00
4203abd2:	0f8d      	mov.n	a8, a15
4203abd4:	091be2        	l16ui	a14, a11, 18
4203abd7:	60c0e0        	neg	a12, a14
4203abda:	022dc7        	blt	a13, a12, 4203abe0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x338>
4203abdd:	ff9606        	j	4203aa39 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x191>
4203abe0:	ff8946        	j	4203aa09 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x161>
4203abe3:	970000        	ee.stf.64.xp	f0, f9, a0, a0
4203abe6:	432a      	add.n	a4, a3, a2
4203abe8:	538d70        	max	a8, a13, a7
4203abeb:	e32a87        	blt	a10, a8, 4203abd2 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x32a>
4203abee:	f8cdd2        	addi	a13, a13, -8
4203abf1:	60d0d0        	neg	a13, a13
4203abf4:	3188      	l32i.n	a8, a1, 12
4203abf6:	2b0c      	movi.n	a11, 2
4203abf8:	d8da      	add.n	a13, a8, a13
4203abfa:	21c4d0        	srai	a12, a13, 4
4203abfd:	ffe546        	j	4203ab96 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x2ee>
4203ac00:	53e8f0        	max	a14, a8, a15
4203ac03:	23ee80        	sext	a14, a14, 15
4203ac06:	11eec0        	slli	a14, a14, 4
4203ac09:	022ae7        	blt	a10, a14, 4203ac0f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x367>
4203ac0c:	ff5606        	j	4203a968 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xc0>
4203ac0f:	091be2        	l16ui	a14, a11, 18
4203ac12:	60c0e0        	neg	a12, a14
4203ac15:	022dc7        	blt	a13, a12, 4203ac1b <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x373>
4203ac18:	ff8746        	j	4203aa39 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x191>
4203ac1b:	020c      	movi.n	a2, 0
4203ac1d:	0913c2        	l16ui	a12, a3, 18
4203ac20:	02bd      	mov.n	a11, a2
4203ac22:	ffb006        	j	4203aae6 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x23e>
4203ac25:	0fbd      	mov.n	a11, a15
4203ac27:	0b2d      	mov.n	a2, a11
4203ac29:	ffaa86        	j	4203aad7 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x22f>
4203ac2c:	0f8d      	mov.n	a8, a15
4203ac2e:	ff72c6        	j	4203a9fd <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x155>


/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203a290 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
4203a290:	008136        	entry	a1, 64
4203a293:	021282        	l16ui	a8, a2, 4
4203a296:	233380        	sext	a3, a3, 15
4203a299:	239880        	sext	a9, a8, 15
4203a29c:	027d      	mov.n	a7, a2
4203a29e:	234480        	sext	a4, a4, 15
4203a2a1:	082397        	blt	a3, a9, 4203a2ad <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1d>
4203a2a4:	041292        	l16ui	a9, a2, 8
4203a2a7:	23a980        	sext	a10, a9, 15
4203a2aa:	12aa37        	bge	a10, a3, 4203a2c0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x30>
4203a2ad:	04dd      	mov.n	a13, a4
4203a2af:	03cd      	mov.n	a12, a3
4203a2b1:	3cc7b2        	addi	a11, a7, 60
4203a2b4:	28c7a2        	addi	a10, a7, 40
4203a2b7:	005f25        	call8	4203a8a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
4203a2ba:	012a80        	slli	a2, a10, 24
4203a2bd:	f01d      	retw.n
4203a2bf:	12b200        	andbc	b11, b2, b0
4203a2c2:	ab8003        	lsip	f0, a0, 0x2ac
4203a2c5:	23          	.byte	0x23
4203a2c6:	e324a7        	blt	a4, a10, 4203a2ad <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1d>
4203a2c9:	0512d2        	l16ui	a13, a2, 10
4203a2cc:	23ad80        	sext	a10, a13, 15
4203a2cf:	da2a47        	blt	a10, a4, 4203a2ad <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x1d>
4203a2d2:	2002a2        	l8ui	a10, a2, 32
4203a2d5:	2102f2        	l8ui	a15, a2, 33
4203a2d8:	c099a0        	sub	a9, a9, a10
4203a2db:	72e8      	l32i.n	a14, a2, 28
4203a2dd:	c0dda0        	sub	a13, a13, a10
4203a2e0:	baba      	add.n	a11, a10, a11
4203a2e2:	aa8a      	add.n	a10, a10, a8
4203a2e4:	23c980        	sext	a12, a9, 15
4203a2e7:	1149      	s32i.n	a4, a1, 4
4203a2e9:	006132        	s32i	a3, a1, 0
4203a2ec:	23dd80        	sext	a13, a13, 15
4203a2ef:	23bb80        	sext	a11, a11, 15
4203a2f2:	23aa80        	sext	a10, a10, 15
4203a2f5:	fe4625        	call8	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
4203a2f8:	ffa092        	movi	a9, 255
4203a2fb:	0a8d      	mov.n	a8, a10
4203a2fd:	079a97        	bne	a10, a9, 4203a308 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x78>
4203a300:	142292        	l32i	a9, a2, 80
4203a303:	052d      	mov.n	a2, a5
4203a305:	fb4916        	beqz	a9, 4203a2bd <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d>
4203a308:	04dd      	mov.n	a13, a4
4203a30a:	3cc7b2        	addi	a11, a7, 60
4203a30d:	03cd      	mov.n	a12, a3
4203a30f:	28c7a2        	addi	a10, a7, 40
4203a312:	6189      	s32i.n	a8, a1, 24
4203a314:	005925        	call8	4203a8a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
4203a317:	6188      	l32i.n	a8, a1, 24
4203a319:	2007b2        	l8ui	a11, a7, 32
4203a31c:	71a9      	s32i.n	a10, a1, 28
4203a31e:	012a80        	slli	a2, a10, 24
4203a321:	08dd      	mov.n	a13, a8
4203a323:	0c0b16        	beqz	a11, 4203a3e7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x157>
4203a326:	0597d2        	l16si	a13, a7, 10
4203a329:	67e8      	l32i.n	a14, a7, 24
4203a32b:	0497c2        	l16si	a12, a7, 8
4203a32e:	0397b2        	l16si	a11, a7, 6
4203a331:	0297a2        	l16si	a10, a7, 4
4203a334:	ff0c      	movi.n	a15, 15
4203a336:	1149      	s32i.n	a4, a1, 4
4203a338:	0139      	s32i.n	a3, a1, 0
4203a33a:	fe41e5        	call8	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
4203a33d:	6188      	l32i.n	a8, a1, 24
4203a33f:	0add      	mov.n	a13, a10
4203a341:	0a2a16        	beqz	a10, 4203a3e7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x157>
4203a344:	24c7a2        	addi	a10, a7, 36
4203a347:	836a60        	moveqz	a6, a10, a6
4203a34a:	06b8      	l32i.n	a11, a6, 0
4203a34c:	01cd80        	slli	a12, a13, 24
4203a34f:	11bb80        	slli	a11, a11, 8
4203a352:	41b8b0        	srli	a11, a11, 8
4203a355:	75a8c0        	extui	a10, a12, 24, 8
4203a358:	ffa0e2        	movi	a14, 255
4203a35b:	20bbc0        	or	a11, a11, a12
4203a35e:	029ae7        	bne	a10, a14, 4203a364 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xd4>
4203a361:	00a746        	j	4203a602 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x372>
4203a364:	7198      	l32i.n	a9, a1, 28
4203a366:	0299e7        	bne	a9, a14, 4203a36c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xdc>
4203a369:	00b4c6        	j	4203a640 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x3b0>
4203a36c:	0b2d      	mov.n	a2, a11
4203a36e:	075916        	beqz	a9, 4203a3e7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x157>
4203a371:	82ca90        	mull	a12, a10, a9
4203a374:	80a0b2        	movi	a11, 128
4203a377:	ccba      	add.n	a12, a12, a11
4203a379:	f4c0c0        	extui	a12, a12, 0, 16
4203a37c:	11ec80        	slli	a14, a12, 8
4203a37f:	ceca      	add.n	a12, a14, a12
4203a381:	9a9a      	add.n	a9, a10, a9
4203a383:	31c0c0        	srai	a12, a12, 16
4203a386:	c099c0        	sub	a9, a9, a12
4203a389:	11ca80        	slli	a12, a10, 8
4203a38c:	f49090        	extui	a9, a9, 0, 16
4203a38f:	c0aca0        	sub	a10, a12, a10
4203a392:	d2aa90        	quos	a10, a10, a9
4203a395:	f4a0a0        	extui	a10, a10, 0, 16
4203a398:	75c020        	extui	a12, a2, 16, 8
4203a39b:	c1cca0        	mul16u	a12, a12, a10
4203a39e:	74f820        	extui	a15, a2, 8, 8
4203a3a1:	c1ffa0        	mul16u	a15, a15, a10
4203a3a4:	74e020        	extui	a14, a2, 0, 8
4203a3a7:	c1eea0        	mul16u	a14, a14, a10
4203a3aa:	acba      	add.n	a10, a12, a11
4203a3ac:	f4a0a0        	extui	a10, a10, 0, 16
4203a3af:	ffba      	add.n	a15, a15, a11
4203a3b1:	f4f0f0        	extui	a15, a15, 0, 16
4203a3b4:	11ca80        	slli	a12, a10, 8
4203a3b7:	eeba      	add.n	a14, a14, a11
4203a3b9:	f4e0e0        	extui	a14, a14, 0, 16
4203a3bc:	acaa      	add.n	a10, a12, a10
4203a3be:	11bf80        	slli	a11, a15, 8
4203a3c1:	bbfa      	add.n	a11, a11, a15
4203a3c3:	75a0a0        	extui	a10, a10, 16, 8
4203a3c6:	11ce80        	slli	a12, a14, 8
4203a3c9:	01f980        	slli	a15, a9, 24
4203a3cc:	11aa00        	slli	a10, a10, 16
4203a3cf:	4198b0        	srli	a9, a11, 8
4203a3d2:	80cce0        	add	a12, a12, a14
4203a3d5:	20aaf0        	or	a10, a10, a15
4203a3d8:	75b0c0        	extui	a11, a12, 16, 8
4203a3db:	749890        	extui	a9, a9, 8, 8
4203a3de:	20aab0        	or	a10, a10, a11
4203a3e1:	119980        	slli	a9, a9, 8
4203a3e4:	202a90        	or	a2, a10, a9
4203a3e7:	ffa092        	movi	a9, 255
4203a3ea:	029897        	bne	a8, a9, 4203a3f0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x160>
4203a3ed:	0077c6        	j	4203a5d0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x340>
4203a3f0:	0c7816        	beqz	a8, 4203a4bb <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x22b>
4203a3f3:	75a850        	extui	a10, a5, 24, 8
4203a3f6:	d18a80        	mul16s	a8, a10, a8
4203a3f9:	1945a1        	l32r	a10, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
4203a3fc:	7fc882        	addi	a8, a8, 127
4203a3ff:	a288a0        	muluh	a8, a8, a10
4203a402:	115580        	slli	a5, a5, 8
4203a405:	0188f0        	slli	a8, a8, 17
4203a408:	75a880        	extui	a10, a8, 24, 8
4203a40b:	0a8d      	mov.n	a8, a10
4203a40d:	404800        	ssai	8
4203a410:	81ba50        	src	a11, a10, a5
4203a413:	029a97        	bne	a10, a9, 4203a419 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x189>
4203a416:	007486        	j	4203a5ec <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x35c>
4203a419:	75a820        	extui	a10, a2, 24, 8
4203a41c:	029a97        	bne	a10, a9, 4203a422 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x192>
4203a41f:	007ac6        	j	4203a60e <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x37e>
4203a422:	095816        	beqz	a8, 4203a4bb <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x22b>
4203a425:	1c3a16        	beqz	a10, 4203a5ec <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x35c>
4203a428:	82e8a0        	mull	a14, a8, a10
4203a42b:	80a0c2        	movi	a12, 128
4203a42e:	eeca      	add.n	a14, a14, a12
4203a430:	f4e0e0        	extui	a14, a14, 0, 16
4203a433:	11fe80        	slli	a15, a14, 8
4203a436:	efea      	add.n	a14, a15, a14
4203a438:	a8aa      	add.n	a10, a8, a10
4203a43a:	31e0e0        	srai	a14, a14, 16
4203a43d:	c0aae0        	sub	a10, a10, a14
4203a440:	11f880        	slli	a15, a8, 8
4203a443:	f4a0a0        	extui	a10, a10, 0, 16
4203a446:	c0ff80        	sub	a15, a15, a8
4203a449:	d2ffa0        	quos	a15, a15, a10
4203a44c:	f4f0f0        	extui	a15, a15, 0, 16
4203a44f:	c089f0        	sub	a8, a9, a15
4203a452:	75e0b0        	extui	a14, a11, 16, 8
4203a455:	f48080        	extui	a8, a8, 0, 16
4203a458:	755020        	extui	a5, a2, 16, 8
4203a45b:	7498b0        	extui	a9, a11, 8, 8
4203a45e:	c1eef0        	mul16u	a14, a14, a15
4203a461:	c15580        	mul16u	a5, a5, a8
4203a464:	746820        	extui	a6, a2, 8, 8
4203a467:	c199f0        	mul16u	a9, a9, a15
4203a46a:	74b0b0        	extui	a11, a11, 0, 8
4203a46d:	c16680        	mul16u	a6, a6, a8
4203a470:	eeca      	add.n	a14, a14, a12
4203a472:	c1bbf0        	mul16u	a11, a11, a15
4203a475:	742020        	extui	a2, a2, 0, 8
4203a478:	c12280        	mul16u	a2, a2, a8
4203a47b:	99ca      	add.n	a9, a9, a12
4203a47d:	ee5a      	add.n	a14, a14, a5
4203a47f:	bbca      	add.n	a11, a11, a12
4203a481:	f4e0e0        	extui	a14, a14, 0, 16
4203a484:	c96a      	add.n	a12, a9, a6
4203a486:	f4c0c0        	extui	a12, a12, 0, 16
4203a489:	9b2a      	add.n	a9, a11, a2
4203a48b:	118e80        	slli	a8, a14, 8
4203a48e:	f49090        	extui	a9, a9, 0, 16
4203a491:	88ea      	add.n	a8, a8, a14
4203a493:	112c80        	slli	a2, a12, 8
4203a496:	758080        	extui	a8, a8, 16, 8
4203a499:	11b980        	slli	a11, a9, 8
4203a49c:	22ca      	add.n	a2, a2, a12
4203a49e:	118800        	slli	a8, a8, 16
4203a4a1:	01aa80        	slli	a10, a10, 24
4203a4a4:	9b9a      	add.n	a9, a11, a9
4203a4a6:	412820        	srli	a2, a2, 8
4203a4a9:	2088a0        	or	a8, a8, a10
4203a4ac:	759090        	extui	a9, a9, 16, 8
4203a4af:	742820        	extui	a2, a2, 8, 8
4203a4b2:	208890        	or	a8, a8, a9
4203a4b5:	112280        	slli	a2, a2, 8
4203a4b8:	202820        	or	a2, a8, a2
4203a4bb:	1427a2        	l32i	a10, a7, 80
4203a4be:	dfba16        	beqz	a10, 4203a2bd <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d>
4203a4c1:	180a82        	l8ui	a8, a10, 24
4203a4c4:	115856        	bnez	a8, 4203a5dd <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x34d>
4203a4c7:	031a82        	l16ui	a8, a10, 6
4203a4ca:	021a92        	l16ui	a9, a10, 4
4203a4cd:	c04480        	sub	a4, a4, a8
4203a4d0:	c03390        	sub	a3, a3, a9
4203a4d3:	740444        	mul.aa.ll	a4, a4
4203a4d6:	780334        	mula.aa.ll	a3, a3
4203a4d9:	3a88      	l32i.n	a8, a10, 12
4203a4db:	031090        	rsr.acclo	a9
4203a4de:	c08890        	sub	a8, a8, a9
4203a4e1:	0218e6        	bgei	a8, 1, 4203a4e7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x257>
4203a4e4:	004246        	j	4203a5f1 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x361>
4203a4e7:	049a92        	l16si	a9, a10, 8
4203a4ea:	0219e6        	bgei	a9, 1, 4203a4f0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x260>
4203a4ed:	0045c6        	j	4203a608 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x378>
4203a4f0:	b99a      	add.n	a11, a9, a9
4203a4f2:	0228b7        	blt	a8, a11, 4203a4f8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x268>
4203a4f5:	0049c6        	j	4203a620 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x390>
4203a4f8:	4ab8      	l32i.n	a11, a10, 16
4203a4fa:	5aa8      	l32i.n	a10, a10, 20
4203a4fc:	60a0c2        	movi	a12, 96
4203a4ff:	61d9      	s32i.n	a13, a1, 24
4203a501:	4c32e5        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a504:	61d8      	l32i.n	a13, a1, 24
4203a506:	7588a0        	extui	a8, a10, 24, 8
4203a509:	190191        	l32r	a9, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
4203a50c:	d188d0        	mul16s	a8, a8, a13
4203a50f:	11aa80        	slli	a10, a10, 8
4203a512:	a28890        	muluh	a8, a8, a9
4203a515:	ffa0c2        	movi	a12, 255
4203a518:	0188f0        	slli	a8, a8, 17
4203a51b:	759880        	extui	a9, a8, 24, 8
4203a51e:	098d      	mov.n	a8, a9
4203a520:	404800        	ssai	8
4203a523:	81b9a0        	src	a11, a9, a10
4203a526:	0299c7        	bne	a9, a12, 4203a52c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x29c>
4203a529:	002b06        	j	4203a5d9 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x349>
4203a52c:	759820        	extui	a9, a2, 24, 8
4203a52f:	0299c7        	bne	a9, a12, 4203a535 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2a5>
4203a532:	003006        	j	4203a5f6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x366>
4203a535:	d84816        	beqz	a8, 4203a2bd <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x2d>
4203a538:	09d916        	beqz	a9, 4203a5d9 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x349>
4203a53b:	82a890        	mull	a10, a8, a9
4203a53e:	80a0d2        	movi	a13, 128
4203a541:	aada      	add.n	a10, a10, a13
4203a543:	f4a0a0        	extui	a10, a10, 0, 16
4203a546:	11ea80        	slli	a14, a10, 8
4203a549:	aeaa      	add.n	a10, a14, a10
4203a54b:	989a      	add.n	a9, a8, a9
4203a54d:	31a0a0        	srai	a10, a10, 16
4203a550:	c099a0        	sub	a9, a9, a10
4203a553:	11f880        	slli	a15, a8, 8
4203a556:	f49090        	extui	a9, a9, 0, 16
4203a559:	c0ff80        	sub	a15, a15, a8
4203a55c:	d2ff90        	quos	a15, a15, a9
4203a55f:	f4f0f0        	extui	a15, a15, 0, 16
4203a562:	c08cf0        	sub	a8, a12, a15
4203a565:	75e0b0        	extui	a14, a11, 16, 8
4203a568:	f48080        	extui	a8, a8, 0, 16
4203a56b:	c1eef0        	mul16u	a14, a14, a15
4203a56e:	756020        	extui	a6, a2, 16, 8
4203a571:	74c8b0        	extui	a12, a11, 8, 8
4203a574:	c1ccf0        	mul16u	a12, a12, a15
4203a577:	c16680        	mul16u	a6, a6, a8
4203a57a:	747820        	extui	a7, a2, 8, 8
4203a57d:	74a0b0        	extui	a10, a11, 0, 8
4203a580:	c17780        	mul16u	a7, a7, a8
4203a583:	c1aaf0        	mul16u	a10, a10, a15
4203a586:	742020        	extui	a2, a2, 0, 8
4203a589:	eeda      	add.n	a14, a14, a13
4203a58b:	c1f280        	mul16u	a15, a2, a8
4203a58e:	bcda      	add.n	a11, a12, a13
4203a590:	8e6a      	add.n	a8, a14, a6
4203a592:	f4c080        	extui	a12, a8, 0, 16
4203a595:	aada      	add.n	a10, a10, a13
4203a597:	bb7a      	add.n	a11, a11, a7
4203a599:	f4b0b0        	extui	a11, a11, 0, 16
4203a59c:	118c80        	slli	a8, a12, 8
4203a59f:	aafa      	add.n	a10, a10, a15
4203a5a1:	f4a0a0        	extui	a10, a10, 0, 16
4203a5a4:	88ca      	add.n	a8, a8, a12
4203a5a6:	112b80        	slli	a2, a11, 8
4203a5a9:	11ca80        	slli	a12, a10, 8
4203a5ac:	22ba      	add.n	a2, a2, a11
4203a5ae:	758080        	extui	a8, a8, 16, 8
4203a5b1:	019980        	slli	a9, a9, 24
4203a5b4:	acaa      	add.n	a10, a12, a10
4203a5b6:	118800        	slli	a8, a8, 16
4203a5b9:	412820        	srli	a2, a2, 8
4203a5bc:	208890        	or	a8, a8, a9
4203a5bf:	742820        	extui	a2, a2, 8, 8
4203a5c2:	7590a0        	extui	a9, a10, 16, 8
4203a5c5:	208890        	or	a8, a8, a9
4203a5c8:	112280        	slli	a2, a2, 8
4203a5cb:	202820        	or	a2, a8, a2
4203a5ce:	f01d      	retw.n
4203a5d0:	18cf21        	l32r	a2, 4200090c <_stext+0x8ec> (ff000000 <_rtc_reserved_end+0x9ef00000>)
4203a5d3:	202520        	or	a2, a5, a2
4203a5d6:	ffb846        	j	4203a4bb <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x22b>
4203a5d9:	0b2d      	mov.n	a2, a11
4203a5db:	f01d      	retw.n
4203a5dd:	20c440        	or	a12, a4, a4
4203a5e0:	20b330        	or	a11, a3, a3
4203a5e3:	0661d2        	s32i	a13, a1, 24
4203a5e6:	ec8da5        	call8	42026ec0 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
4203a5e9:	ffc5c6        	j	4203a504 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x274>
4203a5ec:	0b2d      	mov.n	a2, a11
4203a5ee:	ffb246        	j	4203a4bb <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x22b>
4203a5f1:	5aa8      	l32i.n	a10, a10, 20
4203a5f3:	ffc3c6        	j	4203a506 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x276>
4203a5f6:	20a220        	or	a10, a2, a2
4203a5f9:	4d8425        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a5fc:	202aa0        	or	a2, a10, a10
4203a5ff:	000090        	retw
4203a602:	0b2d      	mov.n	a2, a11
4203a604:	ff77c6        	j	4203a3e7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x157>
4203a607:	4aa800        	madd.s	f10, f8, f0
4203a60a:	ffbe06        	j	4203a506 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x276>
4203a60d:	a22000        	muluh	a2, a0, a0
4203a610:	61d220        	xsr.excsave2	a2
4203a613:	826506        	j	4201afab <roo_display::DrawableRawStreamable<roo_display::SimpleStreamable<roo_display::ProgMemPtr, roo_display::internal::Indexed<(unsigned char)4>, roo_display::internal::RleStream4bppxBiased<roo_display::ProgMemPtr, roo_display::internal::Indexed<(unsigned char)4>, (signed char)4> > >::drawTo(roo_display::Surface const&) const+0x4f>
4203a616:	4d          	.byte	0x4d
4203a617:	0621d2        	l32i	a13, a1, 24
4203a61a:	0a2d      	mov.n	a2, a10
4203a61c:	ffa6c6        	j	4203a4bb <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x22b>
4203a61f:	00          	.byte	00
4203a620:	e41926        	beqi	a9, 1, 4203a608 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x378>
4203a623:	ffc9c2        	addi	a12, a9, -1
4203a626:	23cc80        	sext	a12, a12, 15
4203a629:	11cce0        	slli	a12, a12, 2
4203a62c:	252c87        	blt	a12, a8, 4203a655 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x3c5>
4203a62f:	042ab2        	l32i	a11, a10, 16
4203a632:	052aa2        	l32i	a10, a10, 20
4203a635:	0c4c      	movi.n	a12, 64
4203a637:	61d9      	s32i.n	a13, a1, 24
4203a639:	4c1f65        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a63c:	ffb106        	j	4203a504 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x274>
4203a63f:	a22000        	muluh	a2, a0, a0
4203a642:	20          	.byte	0x20
4203a643:	076182        	s32i	a8, a1, 28
4203a646:	0661d2        	s32i	a13, a1, 24
4203a649:	4d7f25        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
4203a64c:	61d8      	l32i.n	a13, a1, 24
4203a64e:	7188      	l32i.n	a8, a1, 28
4203a650:	0a2d      	mov.n	a2, a10
4203a652:	ff6446        	j	4203a3e7 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x157>
4203a655:	af2926        	beqi	a9, 2, 4203a608 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x378>
4203a658:	80bb90        	add	a11, a11, a9
4203a65b:	80bbb0        	add	a11, a11, a11
4203a65e:	f8cb92        	addi	a9, a11, -8
4203a661:	a3a897        	bge	a8, a9, 4203a608 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x378>
4203a664:	042ab2        	l32i	a11, a10, 16
4203a667:	5aa8      	l32i.n	a10, a10, 20
4203a669:	0c2c      	movi.n	a12, 32
4203a66b:	61d9      	s32i.n	a13, a1, 24
4203a66d:	4c1c25        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
4203a670:	ffa406        	j	4203a504 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x274>


/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039934 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42039934:	00c136        	entry	a1, 96
42039937:	b159      	s32i.n	a5, a1, 44
42039939:	7139      	s32i.n	a3, a1, 28
4203993b:	8149      	s32i.n	a4, a1, 32
4203993d:	9169      	s32i.n	a6, a1, 36
4203993f:	038d      	mov.n	a8, a3
42039941:	067d      	mov.n	a7, a6
42039943:	0245f6        	bgeui	a5, 4, 42039949 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x15>
42039946:	00a406        	j	42039bda <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2a6>
42039949:	053d      	mov.n	a3, a5
4203994b:	085d      	mov.n	a5, a8
4203994d:	3cc282        	addi	a8, a2, 60
42039950:	046d      	mov.n	a6, a4
42039952:	6189      	s32i.n	a8, a1, 24
42039954:	001582        	l16ui	a8, a5, 0
42039957:	001692        	l16ui	a9, a6, 0
4203995a:	0212e2        	l16ui	a14, a2, 4
4203995d:	085182        	s16i	a8, a1, 16
42039960:	0a5192        	s16i	a9, a1, 20
42039963:	238880        	sext	a8, a8, 15
42039966:	23ae80        	sext	a10, a14, 15
42039969:	239980        	sext	a9, a9, 15
4203996c:	02a8a7        	bge	a8, a10, 42039972 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3e>
4203996f:	005fc6        	j	42039af2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1be>
42039972:	0412c2        	l16ui	a12, a2, 8
42039975:	23ac80        	sext	a10, a12, 15
42039978:	02aa87        	bge	a10, a8, 4203997e <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4a>
4203997b:	005cc6        	j	42039af2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1be>
4203997e:	0312b2        	l16ui	a11, a2, 6
42039981:	23ab80        	sext	a10, a11, 15
42039984:	02a9a7        	bge	a9, a10, 4203998a <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x56>
42039987:	0059c6        	j	42039af2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1be>
4203998a:	0512d2        	l16ui	a13, a2, 10
4203998d:	23ad80        	sext	a10, a13, 15
42039990:	02aa97        	bge	a10, a9, 42039996 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x62>
42039993:	0056c6        	j	42039af2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1be>
42039996:	2002a2        	l8ui	a10, a2, 32
42039999:	2102f2        	l8ui	a15, a2, 33
4203999c:	c0dda0        	sub	a13, a13, a10
4203999f:	c0cca0        	sub	a12, a12, a10
420399a2:	80bab0        	add	a11, a10, a11
420399a5:	80aae0        	add	a10, a10, a14
420399a8:	0722e2        	l32i	a14, a2, 28
420399ab:	1199      	s32i.n	a9, a1, 4
420399ad:	0189      	s32i.n	a8, a1, 0
420399af:	5248      	l32i.n	a4, a2, 20
420399b1:	23bb80        	sext	a11, a11, 15
420399b4:	23dd80        	sext	a13, a13, 15
420399b7:	23cc80        	sext	a12, a12, 15
420399ba:	23aa80        	sext	a10, a10, 15
420399bd:	d199      	s32i.n	a9, a1, 52
420399bf:	c189      	s32i.n	a8, a1, 48
420399c1:	f149      	s32i.n	a4, a1, 60
420399c3:	fed965        	call8	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
420399c6:	0abd      	mov.n	a11, a10
420399c8:	a1a9      	s32i.n	a10, a1, 40
420399ca:	ffa0a2        	movi	a10, 255
420399cd:	c188      	l32i.n	a8, a1, 48
420399cf:	d198      	l32i.n	a9, a1, 52
420399d1:	059ba7        	bne	a11, a10, 420399da <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xa6>
420399d4:	1422a2        	l32i	a10, a2, 80
420399d7:	129a16        	beqz	a10, 42039b04 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d0>
420399da:	09dd      	mov.n	a13, a9
420399dc:	08cd      	mov.n	a12, a8
420399de:	3cc2b2        	addi	a11, a2, 60
420399e1:	28c2a2        	addi	a10, a2, 40
420399e4:	e199      	s32i.n	a9, a1, 56
420399e6:	d189      	s32i.n	a8, a1, 52
420399e8:	00ebe5        	call8	4203a8a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
420399eb:	2002b2        	l8ui	a11, a2, 32
420399ee:	c1a9      	s32i.n	a10, a1, 48
420399f0:	a1f8      	l32i.n	a15, a1, 40
420399f2:	d188      	l32i.n	a8, a1, 52
420399f4:	e198      	l32i.n	a9, a1, 56
420399f6:	014a80        	slli	a4, a10, 24
420399f9:	048b16        	beqz	a11, 42039a45 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x111>
420399fc:	62e8      	l32i.n	a14, a2, 24
420399fe:	0592d2        	l16si	a13, a2, 10
42039a01:	0492c2        	l16si	a12, a2, 8
42039a04:	0392b2        	l16si	a11, a2, 6
42039a07:	0292a2        	l16si	a10, a2, 4
42039a0a:	ff0c      	movi.n	a15, 15
42039a0c:	1199      	s32i.n	a9, a1, 4
42039a0e:	0189      	s32i.n	a8, a1, 0
42039a10:	fed465        	call8	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42039a13:	d188      	l32i.n	a8, a1, 52
42039a15:	e198      	l32i.n	a9, a1, 56
42039a17:	0afd      	mov.n	a15, a10
42039a19:	8aac      	beqz.n	a10, 42039a45 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x111>
42039a1b:	92a8      	l32i.n	a10, a2, 36
42039a1d:	01bf80        	slli	a11, a15, 24
42039a20:	11aa80        	slli	a10, a10, 8
42039a23:	41a8a0        	srli	a10, a10, 8
42039a26:	75d8b0        	extui	a13, a11, 24, 8
42039a29:	ffa0c2        	movi	a12, 255
42039a2c:	20bab0        	or	a11, a10, a11
42039a2f:	029dc7        	bne	a13, a12, 42039a35 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x101>
42039a32:	00e886        	j	42039dd8 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4a4>
42039a35:	c1a8      	l32i.n	a10, a1, 48
42039a37:	029ac7        	bne	a10, a12, 42039a3d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x109>
42039a3a:	00ef86        	j	42039dfc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4c8>
42039a3d:	0aed      	mov.n	a14, a10
42039a3f:	204bb0        	or	a4, a11, a11
42039a42:	31aa56        	bnez	a10, 42039d60 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x42c>
42039a45:	a1a8      	l32i.n	a10, a1, 40
42039a47:	ffa0c2        	movi	a12, 255
42039a4a:	029ac7        	bne	a10, a12, 42039a50 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x11c>
42039a4d:	0051c6        	j	42039b98 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x264>
42039a50:	0abd      	mov.n	a11, a10
42039a52:	5abc      	beqz.n	a10, 42039a8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x157>
42039a54:	f1d8      	l32i.n	a13, a1, 60
42039a56:	75a8d0        	extui	a10, a13, 24, 8
42039a59:	d1aab0        	mul16s	a10, a10, a11
42039a5c:	1badb1        	l32r	a11, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
42039a5f:	7fcaa2        	addi	a10, a10, 127
42039a62:	a2aab0        	muluh	a10, a10, a11
42039a65:	11bd80        	slli	a11, a13, 8
42039a68:	01aaf0        	slli	a10, a10, 17
42039a6b:	75d8a0        	extui	a13, a10, 24, 8
42039a6e:	404800        	ssai	8
42039a71:	81bdb0        	src	a11, a13, a11
42039a74:	a1b9      	s32i.n	a11, a1, 40
42039a76:	20add0        	or	a10, a13, a13
42039a79:	029dc7        	bne	a13, a12, 42039a7f <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14b>
42039a7c:	005c46        	j	42039bf1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2bd>
42039a7f:	75d840        	extui	a13, a4, 24, 8
42039a82:	029dc7        	bne	a13, a12, 42039a88 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x154>
42039a85:	00d646        	j	42039de2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4ae>
42039a88:	174a56        	bnez	a10, 42039c00 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2cc>
42039a8b:	1422a2        	l32i	a10, a2, 80
42039a8e:	072a16        	beqz	a10, 42039b04 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d0>
42039a91:	180ab2        	l8ui	a11, a10, 24
42039a94:	148b56        	bnez	a11, 42039be0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2ac>
42039a97:	5198      	l32i.n	a9, a1, 20
42039a99:	031a82        	l16ui	a8, a10, 6
42039a9c:	021ab2        	l16ui	a11, a10, 4
42039a9f:	c08980        	sub	a8, a9, a8
42039aa2:	740884        	mul.aa.ll	a8, a8
42039aa5:	4188      	l32i.n	a8, a1, 16
42039aa7:	3a98      	l32i.n	a9, a10, 12
42039aa9:	c088b0        	sub	a8, a8, a11
42039aac:	780884        	mula.aa.ll	a8, a8
42039aaf:	031080        	rsr.acclo	a8
42039ab2:	c08980        	sub	a8, a9, a8
42039ab5:	0218e6        	bgei	a8, 1, 42039abb <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x187>
42039ab8:	003a06        	j	42039ba4 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x270>
42039abb:	049a92        	l16si	a9, a10, 8
42039abe:	0219e6        	bgei	a9, 1, 42039ac4 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x190>
42039ac1:	00c606        	j	42039ddd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4a9>
42039ac4:	b99a      	add.n	a11, a9, a9
42039ac6:	02a8b7        	bge	a8, a11, 42039acc <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x198>
42039ac9:	007906        	j	42039cb1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x37d>
42039acc:	021966        	bnei	a9, 1, 42039ad2 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x19e>
42039acf:	00c286        	j	42039ddd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4a9>
42039ad2:	c90b      	addi.n	a12, a9, -1
42039ad4:	23cc80        	sext	a12, a12, 15
42039ad7:	11cce0        	slli	a12, a12, 2
42039ada:	02ac87        	bge	a12, a8, 42039ae0 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1ac>
42039add:	00cd06        	j	42039e15 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4e1>
42039ae0:	4ab8      	l32i.n	a11, a10, 16
42039ae2:	5aa8      	l32i.n	a10, a10, 20
42039ae4:	0c4c      	movi.n	a12, 64
42039ae6:	0461f2        	s32i	a15, a1, 16
42039ae9:	4cd465        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039aec:	41f8      	l32i.n	a15, a1, 16
42039aee:	002d06        	j	42039ba6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x272>
42039af1:	21b200        	srai	a11, a0, 2
42039af4:	d99006        	j	42030138 <roo_display::BlendingFilter<roo_display::BlendOp<(roo_display::BlendingMode)5> >::fillRectIntersectingRaster(roo_display::BlendingMode, short, short, short, short, roo_display::Color)+0x20>
42039af7:	20          	.byte	0x20
42039af8:	20c880        	or	a12, a8, a8
42039afb:	28c2a2        	addi	a10, a2, 40
42039afe:	00daa5        	call8	4203a8a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42039b01:	014a80        	slli	a4, a10, 24
42039b04:	006742        	s32i	a4, a7, 0
42039b07:	0196c2        	l16si	a12, a6, 2
42039b0a:	0195b2        	l16si	a11, a5, 2
42039b0d:	52d8      	l32i.n	a13, a2, 20
42039b0f:	02ad      	mov.n	a10, a2
42039b11:	ff3865        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039b14:	17a9      	s32i.n	a10, a7, 4
42039b16:	0296c2        	l16si	a12, a6, 4
42039b19:	0295b2        	l16si	a11, a5, 4
42039b1c:	52d8      	l32i.n	a13, a2, 20
42039b1e:	02ad      	mov.n	a10, a2
42039b20:	ff3765        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039b23:	27a9      	s32i.n	a10, a7, 8
42039b25:	0396c2        	l16si	a12, a6, 6
42039b28:	0395b2        	l16si	a11, a5, 6
42039b2b:	52d8      	l32i.n	a13, a2, 20
42039b2d:	02ad      	mov.n	a10, a2
42039b2f:	ff36a5        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039b32:	37a9      	s32i.n	a10, a7, 12
42039b34:	fcc332        	addi	a3, a3, -4
42039b37:	558b      	addi.n	a5, a5, 8
42039b39:	668b      	addi.n	a6, a6, 8
42039b3b:	10c772        	addi	a7, a7, 16
42039b3e:	0243b6        	bltui	a3, 4, 42039b44 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x210>
42039b41:	ff83c6        	j	42039954 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x20>
42039b44:	9198      	l32i.n	a9, a1, 36
42039b46:	b1d8      	l32i.n	a13, a1, 44
42039b48:	10c9c2        	addi	a12, a9, 16
42039b4b:	8198      	l32i.n	a9, a1, 32
42039b4d:	fccd82        	addi	a8, a13, -4
42039b50:	a98b      	addi.n	a10, a9, 8
42039b52:	7198      	l32i.n	a9, a1, 28
42039b54:	418280        	srli	a8, a8, 2
42039b57:	11b8c0        	slli	a11, a8, 4
42039b5a:	b09890        	addx8	a9, a8, a9
42039b5d:	b088a0        	addx8	a8, a8, a10
42039b60:	bcba      	add.n	a11, a12, a11
42039b62:	8189      	s32i.n	a8, a1, 32
42039b64:	898b      	addi.n	a8, a9, 8
42039b66:	1440d0        	extui	a4, a13, 0, 2
42039b69:	91b9      	s32i.n	a11, a1, 36
42039b6b:	7189      	s32i.n	a8, a1, 28
42039b6d:	34ac      	beqz.n	a4, 42039b94 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x260>
42039b6f:	9178      	l32i.n	a7, a1, 36
42039b71:	f44040        	extui	a4, a4, 0, 16
42039b74:	8158      	l32i.n	a5, a1, 32
42039b76:	072162        	l32i	a6, a1, 28
42039b79:	a04470        	addx4	a4, a4, a7
42039b7c:	0095c2        	l16si	a12, a5, 0
42039b7f:	0096b2        	l16si	a11, a6, 0
42039b82:	52d8      	l32i.n	a13, a2, 20
42039b84:	02ad      	mov.n	a10, a2
42039b86:	ff3125        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039b89:	07a9      	s32i.n	a10, a7, 0
42039b8b:	774b      	addi.n	a7, a7, 4
42039b8d:	552b      	addi.n	a5, a5, 2
42039b8f:	662b      	addi.n	a6, a6, 2
42039b91:	e79747        	bne	a7, a4, 42039b7c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x248>
42039b94:	f01d      	retw.n
42039b96:	410000        	srli	a0, a0, 0
42039b99:	5d          	.byte	0x5d
42039b9a:	a81b      	addi.n	a10, a8, 1
42039b9c:	44a0f1        	l32r	a15, 4200ae1c <roo_display::RasterizableStack::createStream(roo_display::Box const&) const+0x1cc> (199ab7d6 <UserFrameTotalSize+0x199ab6d6>)
42039b9f:	20          	.byte	0x20
42039ba0:	ffb9c6        	j	42039a8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x157>
42039ba3:	5aa800        	msub.s	f10, f8, f0
42039ba6:	7588a0        	extui	a8, a10, 24, 8
42039ba9:	1b5991        	l32r	a9, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
42039bac:	d188f0        	mul16s	a8, a8, a15
42039baf:	11ba80        	slli	a11, a10, 8
42039bb2:	a28890        	muluh	a8, a8, a9
42039bb5:	ffa0a2        	movi	a10, 255
42039bb8:	0188f0        	slli	a8, a8, 17
42039bbb:	759880        	extui	a9, a8, 24, 8
42039bbe:	098d      	mov.n	a8, a9
42039bc0:	404800        	ssai	8
42039bc3:	81b9b0        	src	a11, a9, a11
42039bc6:	0b19a7        	beq	a9, a10, 42039bd5 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2a1>
42039bc9:	759840        	extui	a9, a4, 24, 8
42039bcc:	2619a7        	beq	a9, a10, 42039bf6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2c2>
42039bcf:	f31816        	beqz	a8, 42039b04 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d0>
42039bd2:	0ee956        	bnez	a9, 42039cc4 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x390>
42039bd5:	0b4d      	mov.n	a4, a11
42039bd7:	ffca46        	j	42039b04 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d0>
42039bda:	054d      	mov.n	a4, a5
42039bdc:	ffe346        	j	42039b6d <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x239>
42039bdf:	00          	.byte	00
42039be0:	20c990        	or	a12, a9, a9
42039be3:	20b880        	or	a11, a8, a8
42039be6:	0461f2        	s32i	a15, a1, 16
42039be9:	ed2d65        	call8	42026ec0 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
42039bec:	41f8      	l32i.n	a15, a1, 16
42039bee:	ffed06        	j	42039ba6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x272>
42039bf1:	0b4d      	mov.n	a4, a11
42039bf3:	ffa506        	j	42039a8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x157>
42039bf6:	04ad      	mov.n	a10, a4
42039bf8:	4e2425        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039bfb:	0a4d      	mov.n	a4, a10
42039bfd:	ffc0c6        	j	42039b04 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d0>
42039c00:	fedd16        	beqz	a13, 42039bf1 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x2bd>
42039c03:	82ead0        	mull	a14, a10, a13
42039c06:	80a0b2        	movi	a11, 128
42039c09:	eeba      	add.n	a14, a14, a11
42039c0b:	f4e0e0        	extui	a14, a14, 0, 16
42039c0e:	11be80        	slli	a11, a14, 8
42039c11:	ebea      	add.n	a14, a11, a14
42039c13:	dada      	add.n	a13, a10, a13
42039c15:	31e0e0        	srai	a14, a14, 16
42039c18:	c0ede0        	sub	a14, a13, a14
42039c1b:	11da80        	slli	a13, a10, 8
42039c1e:	f4e0e0        	extui	a14, a14, 0, 16
42039c21:	c0dda0        	sub	a13, a13, a10
42039c24:	a1b8      	l32i.n	a11, a1, 40
42039c26:	d2dde0        	quos	a13, a13, a14
42039c29:	f4d0d0        	extui	a13, a13, 0, 16
42039c2c:	c0acd0        	sub	a10, a12, a13
42039c2f:	75c0b0        	extui	a12, a11, 16, 8
42039c32:	74b8b0        	extui	a11, a11, 8, 8
42039c35:	c1bbd0        	mul16u	a11, a11, a13
42039c38:	c1ccd0        	mul16u	a12, a12, a13
42039c3b:	e1b9      	s32i.n	a11, a1, 56
42039c3d:	2801b2        	l8ui	a11, a1, 40
42039c40:	f4a0a0        	extui	a10, a10, 0, 16
42039c43:	c1bbd0        	mul16u	a11, a11, a13
42039c46:	d1c9      	s32i.n	a12, a1, 52
42039c48:	74c840        	extui	a12, a4, 8, 8
42039c4b:	c1dca0        	mul16u	a13, a12, a10
42039c4e:	c1e9      	s32i.n	a14, a1, 48
42039c50:	d1c8      	l32i.n	a12, a1, 52
42039c52:	75e040        	extui	a14, a4, 16, 8
42039c55:	744040        	extui	a4, a4, 0, 8
42039c58:	c1eea0        	mul16u	a14, a14, a10
42039c5b:	c144a0        	mul16u	a4, a4, a10
42039c5e:	a1b9      	s32i.n	a11, a1, 40
42039c60:	e1a8      	l32i.n	a10, a1, 56
42039c62:	80a0b2        	movi	a11, 128
42039c65:	ccba      	add.n	a12, a12, a11
42039c67:	ccea      	add.n	a12, a12, a14
42039c69:	aaba      	add.n	a10, a10, a11
42039c6b:	0bed      	mov.n	a14, a11
42039c6d:	a1b8      	l32i.n	a11, a1, 40
42039c6f:	f4c0c0        	extui	a12, a12, 0, 16
42039c72:	aada      	add.n	a10, a10, a13
42039c74:	f4d0a0        	extui	a13, a10, 0, 16
42039c77:	bbea      	add.n	a11, a11, a14
42039c79:	11ac80        	slli	a10, a12, 8
42039c7c:	bb4a      	add.n	a11, a11, a4
42039c7e:	aaca      	add.n	a10, a10, a12
42039c80:	11cd80        	slli	a12, a13, 8
42039c83:	f4b0b0        	extui	a11, a11, 0, 16
42039c86:	ccda      	add.n	a12, a12, a13
42039c88:	c1d8      	l32i.n	a13, a1, 48
42039c8a:	75a0a0        	extui	a10, a10, 16, 8
42039c8d:	11eb80        	slli	a14, a11, 8
42039c90:	11aa00        	slli	a10, a10, 16
42039c93:	01dd80        	slli	a13, a13, 24
42039c96:	eeba      	add.n	a14, a14, a11
42039c98:	41c8c0        	srli	a12, a12, 8
42039c9b:	204ad0        	or	a4, a10, a13
42039c9e:	75e0e0        	extui	a14, a14, 16, 8
42039ca1:	74a8c0        	extui	a10, a12, 8, 8
42039ca4:	2044e0        	or	a4, a4, a14
42039ca7:	11aa80        	slli	a10, a10, 8
42039caa:	2044a0        	or	a4, a4, a10
42039cad:	ff7686        	j	42039a8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x157>
42039cb0:	2ab200        	mul.s	f11, f2, f0
42039cb3:	04          	.byte	0x4
42039cb4:	052aa2        	l32i	a10, a10, 20
42039cb7:	60a0c2        	movi	a12, 96
42039cba:	0461f2        	s32i	a15, a1, 16
42039cbd:	4cb725        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039cc0:	ff8a06        	j	42039aec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1b8>
42039cc3:	00          	.byte	00
42039cc4:	82c890        	mull	a12, a8, a9
42039cc7:	80a0f2        	movi	a15, 128
42039cca:	ccfa      	add.n	a12, a12, a15
42039ccc:	f4c0c0        	extui	a12, a12, 0, 16
42039ccf:	11dc80        	slli	a13, a12, 8
42039cd2:	cdca      	add.n	a12, a13, a12
42039cd4:	989a      	add.n	a9, a8, a9
42039cd6:	31c0c0        	srai	a12, a12, 16
42039cd9:	c0c9c0        	sub	a12, a9, a12
42039cdc:	119880        	slli	a9, a8, 8
42039cdf:	f4c0c0        	extui	a12, a12, 0, 16
42039ce2:	c09980        	sub	a9, a9, a8
42039ce5:	d299c0        	quos	a9, a9, a12
42039ce8:	f49090        	extui	a9, a9, 0, 16
42039ceb:	c08a90        	sub	a8, a10, a9
42039cee:	41c9      	s32i.n	a12, a1, 16
42039cf0:	75c0b0        	extui	a12, a11, 16, 8
42039cf3:	f48080        	extui	a8, a8, 0, 16
42039cf6:	c1ec90        	mul16u	a14, a12, a9
42039cf9:	75d040        	extui	a13, a4, 16, 8
42039cfc:	74c8b0        	extui	a12, a11, 8, 8
42039cff:	c1cc90        	mul16u	a12, a12, a9
42039d02:	c1dd80        	mul16u	a13, a13, a8
42039d05:	74a840        	extui	a10, a4, 8, 8
42039d08:	74b0b0        	extui	a11, a11, 0, 8
42039d0b:	c1aa80        	mul16u	a10, a10, a8
42039d0e:	c1bb90        	mul16u	a11, a11, a9
42039d11:	744040        	extui	a4, a4, 0, 8
42039d14:	eefa      	add.n	a14, a14, a15
42039d16:	c14480        	mul16u	a4, a4, a8
42039d19:	eeda      	add.n	a14, a14, a13
42039d1b:	8cfa      	add.n	a8, a12, a15
42039d1d:	f4e0e0        	extui	a14, a14, 0, 16
42039d20:	88aa      	add.n	a8, a8, a10
42039d22:	bbfa      	add.n	a11, a11, a15
42039d24:	f4a080        	extui	a10, a8, 0, 16
42039d27:	bb4a      	add.n	a11, a11, a4
42039d29:	118e80        	slli	a8, a14, 8
42039d2c:	f490b0        	extui	a9, a11, 0, 16
42039d2f:	114a80        	slli	a4, a10, 8
42039d32:	41c8      	l32i.n	a12, a1, 16
42039d34:	88ea      	add.n	a8, a8, a14
42039d36:	44aa      	add.n	a4, a4, a10
42039d38:	758080        	extui	a8, a8, 16, 8
42039d3b:	11b980        	slli	a11, a9, 8
42039d3e:	118800        	slli	a8, a8, 16
42039d41:	01ac80        	slli	a10, a12, 24
42039d44:	9b9a      	add.n	a9, a11, a9
42039d46:	414840        	srli	a4, a4, 8
42039d49:	2088a0        	or	a8, a8, a10
42039d4c:	759090        	extui	a9, a9, 16, 8
42039d4f:	744840        	extui	a4, a4, 8, 8
42039d52:	208890        	or	a8, a8, a9
42039d55:	114480        	slli	a4, a4, 8
42039d58:	204840        	or	a4, a8, a4
42039d5b:	ff6946        	j	42039b04 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1d0>
42039d5e:	a00000        	addx4	a0, a0, a0
42039d61:	ad          	.byte	0xad
42039d62:	a0c282        	addi	a8, a2, -96
42039d65:	aaca80        	floor.s	a12, f10, 8
42039d68:	f4a0a0        	extui	a10, a10, 0, 16
42039d6b:	11ba80        	slli	a11, a10, 8
42039d6e:	bbaa      	add.n	a11, a11, a10
42039d70:	31b0b0        	srai	a11, a11, 16
42039d73:	adea      	add.n	a10, a13, a14
42039d75:	c0aab0        	sub	a10, a10, a11
42039d78:	11bd80        	slli	a11, a13, 8
42039d7b:	f4a0a0        	extui	a10, a10, 0, 16
42039d7e:	c0bbd0        	sub	a11, a11, a13
42039d81:	d2bba0        	quos	a11, a11, a10
42039d84:	f4b0b0        	extui	a11, a11, 0, 16
42039d87:	75e040        	extui	a14, a4, 16, 8
42039d8a:	c1eeb0        	mul16u	a14, a14, a11
42039d8d:	74d840        	extui	a13, a4, 8, 8
42039d90:	c1ddb0        	mul16u	a13, a13, a11
42039d93:	744040        	extui	a4, a4, 0, 8
42039d96:	c144b0        	mul16u	a4, a4, a11
42039d99:	eeca      	add.n	a14, a14, a12
42039d9b:	f4e0e0        	extui	a14, a14, 0, 16
42039d9e:	ddca      	add.n	a13, a13, a12
42039da0:	f4d0d0        	extui	a13, a13, 0, 16
42039da3:	11be80        	slli	a11, a14, 8
42039da6:	c4ca      	add.n	a12, a4, a12
42039da8:	f4c0c0        	extui	a12, a12, 0, 16
42039dab:	bbea      	add.n	a11, a11, a14
42039dad:	11ed80        	slli	a14, a13, 8
42039db0:	114c80        	slli	a4, a12, 8
42039db3:	75b0b0        	extui	a11, a11, 16, 8
42039db6:	eeda      	add.n	a14, a14, a13
42039db8:	01aa80        	slli	a10, a10, 24
42039dbb:	c4ca      	add.n	a12, a4, a12
42039dbd:	11bb00        	slli	a11, a11, 16
42039dc0:	41e8e0        	srli	a14, a14, 8
42039dc3:	204ba0        	or	a4, a11, a10
42039dc6:	75c0c0        	extui	a12, a12, 16, 8
42039dc9:	74a8e0        	extui	a10, a14, 8, 8
42039dcc:	2044c0        	or	a4, a4, a12
42039dcf:	11aa80        	slli	a10, a10, 8
42039dd2:	2044a0        	or	a4, a4, a10
42039dd5:	ff1b06        	j	42039a45 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x111>
42039dd8:	0b4d      	mov.n	a4, a11
42039dda:	ff19c6        	j	42039a45 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x111>
42039ddd:	4aa8      	l32i.n	a10, a10, 16
42039ddf:	ff70c6        	j	42039ba6 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x272>
42039de2:	20a440        	or	a10, a4, a4
42039de5:	0e61f2        	s32i	a15, a1, 56
42039de8:	0d6192        	s32i	a9, a1, 52
42039deb:	c189      	s32i.n	a8, a1, 48
42039ded:	4e04e5        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039df0:	c188      	l32i.n	a8, a1, 48
42039df2:	d198      	l32i.n	a9, a1, 52
42039df4:	e1f8      	l32i.n	a15, a1, 56
42039df6:	0a4d      	mov.n	a4, a10
42039df8:	ff23c6        	j	42039a8b <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x157>
42039dfb:	a44000        	extui	a4, a0, 0, 11
42039dfe:	61f220        	xsr.ccompare2	a2
42039e01:	0d61920e 	ee.vmulas.s16.accx.ld.ip.qup	q2, a0, -224, q2, q2, q6, q1
42039e05:	c189      	s32i.n	a8, a1, 48
42039e07:	4e0365        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039e0a:	c188      	l32i.n	a8, a1, 48
42039e0c:	d198      	l32i.n	a9, a1, 52
42039e0e:	e1f8      	l32i.n	a15, a1, 56
42039e10:	0a4d      	mov.n	a4, a10
42039e12:	ff0bc6        	j	42039a45 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x111>
42039e15:	c42926        	beqi	a9, 2, 42039ddd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4a9>
42039e18:	80bb90        	add	a11, a11, a9
42039e1b:	80bbb0        	add	a11, a11, a11
42039e1e:	f8cb92        	addi	a9, a11, -8
42039e21:	b8a897        	bge	a8, a9, 42039ddd <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x4a9>
42039e24:	042ab2        	l32i	a11, a10, 16
42039e27:	5aa8      	l32i.n	a10, a10, 20
42039e29:	0c2c      	movi.n	a12, 32
42039e2b:	41f9      	s32i.n	a15, a1, 16
42039e2d:	4ca025        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039e30:	ff2e06        	j	42039aec <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1b8>


/tmp/roo-shadow-review/codegen/combined_unroll/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039248 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42039248:	010136        	entry	a1, 128
4203924b:	069d      	mov.n	a9, a6
4203924d:	23a380        	sext	a10, a3, 15
42039250:	23b580        	sext	a11, a5, 15
42039253:	239980        	sext	a9, a9, 15
42039256:	142282        	l32i	a8, a2, 80
42039259:	81a9      	s32i.n	a10, a1, 32
4203925b:	51b9      	s32i.n	a11, a1, 20
4203925d:	9199      	s32i.n	a9, a1, 36
4203925f:	076d      	mov.n	a6, a7
42039261:	234480        	sext	a4, a4, 15
42039264:	2e4816        	beqz	a8, 4203954c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x304>
42039267:	02a947        	bge	a9, a4, 4203926d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x25>
4203926a:	008e06        	j	420394a6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x25e>
4203926d:	3cc282        	addi	a8, a2, 60
42039270:	066182        	s32i	a8, a1, 24
42039273:	02aba7        	bge	a11, a10, 42039279 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x31>
42039276:	008b06        	j	420394a6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x25e>
42039279:	f48040        	extui	a8, a4, 0, 16
4203927c:	8178      	l32i.n	a7, a1, 32
4203927e:	076182        	s32i	a8, a1, 28
42039281:	021282        	l16ui	a8, a2, 4
42039284:	f43070        	extui	a3, a7, 0, 16
42039287:	239880        	sext	a9, a8, 15
4203928a:	02a797        	bge	a7, a9, 42039290 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x48>
4203928d:	008706        	j	420394ad <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x265>
42039290:	0412c2        	l16ui	a12, a2, 8
42039293:	239c80        	sext	a9, a12, 15
42039296:	02a977        	bge	a9, a7, 4203929c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x54>
42039299:	008406        	j	420394ad <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x265>
4203929c:	0312b2        	l16ui	a11, a2, 6
4203929f:	239b80        	sext	a9, a11, 15
420392a2:	02a497        	bge	a4, a9, 420392a8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x60>
420392a5:	008106        	j	420394ad <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x265>
420392a8:	0512d2        	l16ui	a13, a2, 10
420392ab:	239d80        	sext	a9, a13, 15
420392ae:	02a947        	bge	a9, a4, 420392b4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6c>
420392b1:	007e06        	j	420394ad <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x265>
420392b4:	2002a2        	l8ui	a10, a2, 32
420392b7:	2102f2        	l8ui	a15, a2, 33
420392ba:	0722e2        	l32i	a14, a2, 28
420392bd:	016142        	s32i	a4, a1, 4
420392c0:	006172        	s32i	a7, a1, 0
420392c3:	5258      	l32i.n	a5, a2, 20
420392c5:	c0dda0        	sub	a13, a13, a10
420392c8:	c0cca0        	sub	a12, a12, a10
420392cb:	baba      	add.n	a11, a10, a11
420392cd:	aa8a      	add.n	a10, a10, a8
420392cf:	23dd80        	sext	a13, a13, 15
420392d2:	23cc80        	sext	a12, a12, 15
420392d5:	23bb80        	sext	a11, a11, 15
420392d8:	23aa80        	sext	a10, a10, 15
420392db:	b159      	s32i.n	a5, a1, 44
420392dd:	ff47a5        	call8	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
420392e0:	ffa092        	movi	a9, 255
420392e3:	0a8d      	mov.n	a8, a10
420392e5:	059a97        	bne	a10, a9, 420392ee <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xa6>
420392e8:	142292        	l32i	a9, a2, 80
420392eb:	197916        	beqz	a9, 42039486 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x23e>
420392ee:	0621b2        	l32i	a11, a1, 24
420392f1:	20d440        	or	a13, a4, a4
420392f4:	07cd      	mov.n	a12, a7
420392f6:	28c2a2        	addi	a10, a2, 40
420392f9:	0a6182        	s32i	a8, a1, 40
420392fc:	015aa5        	call8	4203a8a8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
420392ff:	a188      	l32i.n	a8, a1, 40
42039301:	2002b2        	l8ui	a11, a2, 32
42039304:	c1a9      	s32i.n	a10, a1, 48
42039306:	015a80        	slli	a5, a10, 24
42039309:	08dd      	mov.n	a13, a8
4203930b:	0b9b16        	beqz	a11, 420393c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x180>
4203930e:	0592d2        	l16si	a13, a2, 10
42039311:	62e8      	l32i.n	a14, a2, 24
42039313:	0492c2        	l16si	a12, a2, 8
42039316:	0392b2        	l16si	a11, a2, 6
42039319:	0292a2        	l16si	a10, a2, 4
4203931c:	ff0c      	movi.n	a15, 15
4203931e:	1149      	s32i.n	a4, a1, 4
42039320:	0179      	s32i.n	a7, a1, 0
42039322:	ff4365        	call8	42038758 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42039325:	a188      	l32i.n	a8, a1, 40
42039327:	0add      	mov.n	a13, a10
42039329:	09ba16        	beqz	a10, 420393c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x180>
4203932c:	92b8      	l32i.n	a11, a2, 36
4203932e:	01ca80        	slli	a12, a10, 24
42039331:	11bb80        	slli	a11, a11, 8
42039334:	41b8b0        	srli	a11, a11, 8
42039337:	75a8c0        	extui	a10, a12, 24, 8
4203933a:	ffa0e2        	movi	a14, 255
4203933d:	20bbc0        	or	a11, a11, a12
42039340:	029ae7        	bne	a10, a14, 42039346 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xfe>
42039343:	00ed86        	j	420396fd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4b5>
42039346:	c198      	l32i.n	a9, a1, 48
42039348:	0299e7        	bne	a9, a14, 4203934e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x106>
4203934b:	012346        	j	420397dc <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x594>
4203934e:	0b5d      	mov.n	a5, a11
42039350:	074916        	beqz	a9, 420393c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x180>
42039353:	82ca90        	mull	a12, a10, a9
42039356:	80a0b2        	movi	a11, 128
42039359:	ccba      	add.n	a12, a12, a11
4203935b:	f4c0c0        	extui	a12, a12, 0, 16
4203935e:	11ec80        	slli	a14, a12, 8
42039361:	ceca      	add.n	a12, a14, a12
42039363:	9a9a      	add.n	a9, a10, a9
42039365:	31c0c0        	srai	a12, a12, 16
42039368:	c099c0        	sub	a9, a9, a12
4203936b:	11ca80        	slli	a12, a10, 8
4203936e:	f49090        	extui	a9, a9, 0, 16
42039371:	c0aca0        	sub	a10, a12, a10
42039374:	d2aa90        	quos	a10, a10, a9
42039377:	f4a0a0        	extui	a10, a10, 0, 16
4203937a:	75e050        	extui	a14, a5, 16, 8
4203937d:	c1eea0        	mul16u	a14, a14, a10
42039380:	74c850        	extui	a12, a5, 8, 8
42039383:	c1cca0        	mul16u	a12, a12, a10
42039386:	745050        	extui	a5, a5, 0, 8
42039389:	c1f5a0        	mul16u	a15, a5, a10
4203938c:	eeba      	add.n	a14, a14, a11
4203938e:	f4e0e0        	extui	a14, a14, 0, 16
42039391:	ccba      	add.n	a12, a12, a11
42039393:	f4c0c0        	extui	a12, a12, 0, 16
42039396:	11ae80        	slli	a10, a14, 8
42039399:	bfba      	add.n	a11, a15, a11
4203939b:	f4b0b0        	extui	a11, a11, 0, 16
4203939e:	aaea      	add.n	a10, a10, a14
420393a0:	11ec80        	slli	a14, a12, 8
420393a3:	75a0a0        	extui	a10, a10, 16, 8
420393a6:	11fb80        	slli	a15, a11, 8
420393a9:	eeca      	add.n	a14, a14, a12
420393ab:	019980        	slli	a9, a9, 24
420393ae:	11aa00        	slli	a10, a10, 16
420393b1:	bfba      	add.n	a11, a15, a11
420393b3:	41e8e0        	srli	a14, a14, 8
420393b6:	205a90        	or	a5, a10, a9
420393b9:	75b0b0        	extui	a11, a11, 16, 8
420393bc:	7498e0        	extui	a9, a14, 8, 8
420393bf:	2055b0        	or	a5, a5, a11
420393c2:	119980        	slli	a9, a9, 8
420393c5:	205590        	or	a5, a5, a9
420393c8:	ffa092        	movi	a9, 255
420393cb:	029897        	bne	a8, a9, 420393d1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x189>
420393ce:	004086        	j	420394d4 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x28c>
420393d1:	38bc      	beqz.n	a8, 42039408 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
420393d3:	b1b8      	l32i.n	a11, a1, 44
420393d5:	75a8b0        	extui	a10, a11, 24, 8
420393d8:	d18a80        	mul16s	a8, a10, a8
420393db:	1d4da1        	l32r	a10, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
420393de:	7fc882        	addi	a8, a8, 127
420393e1:	a288a0        	muluh	a8, a8, a10
420393e4:	11ab80        	slli	a10, a11, 8
420393e7:	0188f0        	slli	a8, a8, 17
420393ea:	75b880        	extui	a11, a8, 24, 8
420393ed:	0b8d      	mov.n	a8, a11
420393ef:	404800        	ssai	8
420393f2:	81eba0        	src	a14, a11, a10
420393f5:	0d1b97        	beq	a11, a9, 42039406 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1be>
420393f8:	75a850        	extui	a10, a5, 24, 8
420393fb:	029a97        	bne	a10, a9, 42039401 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1b9>
420393fe:	004486        	j	42039514 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2cc>
42039401:	3b8c      	beqz.n	a11, 42039408 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
42039403:	445a56        	bnez	a10, 4203984c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x604>
42039406:	0e5d      	mov.n	a5, a14
42039408:	1422a2        	l32i	a10, a2, 80
4203940b:	077a16        	beqz	a10, 42039486 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x23e>
4203940e:	180a82        	l8ui	a8, a10, 24
42039411:	0cb856        	bnez	a8, 420394e0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x298>
42039414:	031a82        	l16ui	a8, a10, 6
42039417:	71b8      	l32i.n	a11, a1, 28
42039419:	021a92        	l16ui	a9, a10, 4
4203941c:	c08b80        	sub	a8, a11, a8
4203941f:	740884        	mul.aa.ll	a8, a8
42039422:	c09390        	sub	a9, a3, a9
42039425:	780994        	mula.aa.ll	a9, a9
42039428:	3a88      	l32i.n	a8, a10, 12
4203942a:	031090        	rsr.acclo	a9
4203942d:	c08890        	sub	a8, a8, a9
42039430:	0218e6        	bgei	a8, 1, 42039436 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ee>
42039433:	003c46        	j	42039528 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2e0>
42039436:	049a92        	l16si	a9, a10, 8
42039439:	0219e6        	bgei	a9, 1, 4203943f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f7>
4203943c:	003386        	j	4203950e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c6>
4203943f:	b99a      	add.n	a11, a9, a9
42039441:	0228b7        	blt	a8, a11, 42039447 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ff>
42039444:	003006        	j	42039508 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c0>
42039447:	4ab8      	l32i.n	a11, a10, 16
42039449:	5aa8      	l32i.n	a10, a10, 20
4203944b:	60a0c2        	movi	a12, 96
4203944e:	0a61d2        	s32i	a13, a1, 40
42039451:	4d3de5        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039454:	a1d8      	l32i.n	a13, a1, 40
42039456:	7588a0        	extui	a8, a10, 24, 8
42039459:	1d2d91        	l32r	a9, 42000910 <_stext+0x8f0> (80808081 <_rtc_reserved_end+0x20708081>)
4203945c:	d188d0        	mul16s	a8, a8, a13
4203945f:	11ba80        	slli	a11, a10, 8
42039462:	a28890        	muluh	a8, a8, a9
42039465:	ffa0a2        	movi	a10, 255
42039468:	0188f0        	slli	a8, a8, 17
4203946b:	759880        	extui	a9, a8, 24, 8
4203946e:	208990        	or	a8, a9, a9
42039471:	404800        	ssai	8
42039474:	81b9b0        	src	a11, a9, a11
42039477:	7719a7        	beq	a9, a10, 420394f2 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2aa>
4203947a:	759850        	extui	a9, a5, 24, 8
4203947d:	0299a7        	bne	a9, a10, 42039483 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x23b>
42039480:	002a46        	j	4203952d <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2e5>
42039483:	068856        	bnez	a8, 420394ef <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2a7>
42039486:	331b      	addi.n	a3, a3, 1
42039488:	5188      	l32i.n	a8, a1, 20
4203948a:	0659      	s32i.n	a5, a6, 0
4203948c:	237380        	sext	a7, a3, 15
4203948f:	664b      	addi.n	a6, a6, 4
42039491:	022877        	blt	a8, a7, 42039497 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x24f>
42039494:	ff7a46        	j	42039281 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x39>
42039497:	7188      	l32i.n	a8, a1, 28
42039499:	481b      	addi.n	a4, a8, 1
4203949b:	9188      	l32i.n	a8, a1, 36
4203949d:	234480        	sext	a4, a4, 15
420394a0:	022847        	blt	a8, a4, 420394a6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x25e>
420394a3:	ff7486        	j	42039279 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x31>
420394a6:	0a0c      	movi.n	a10, 0
420394a8:	0a2d      	mov.n	a2, a10
420394aa:	f01d      	retw.n
420394ac:	21b200        	srai	a11, a0, 2
420394af:	c77006        	j	4202b273 <roo_windows::(anonymous namespace)::FindDirectionalFocusable(roo_windows::Widget&, roo_windows::Rect const&, roo_windows::FocusDirection, int&, roo_windows::(anonymous namespace)::DirectionalCandidate&)+0xf3>
420394b2:	d44020        	extui	a4, a2, 0, 14
420394b5:	c2a220        	quou	a10, a2, a2
420394b8:	e528      	l32i.n	a2, a5, 56
420394ba:	5a80013e 	ee.vmulas.u16.qacc.ld.ip.qup	q4, a3, 0xfffffe10, q0, q1, q0, q0
420394be:	331b01        	l32r	a0, 4200612c <roo_io::LittlefsFs::mountImpl(std::function<void ()>)+0xf4> (1020a660 <UserFrameTotalSize+0x1020a560>)
420394c1:	5188      	l32i.n	a8, a1, 20
420394c3:	0659      	s32i.n	a5, a6, 0
420394c5:	237380        	sext	a7, a3, 15
420394c8:	664b      	addi.n	a6, a6, 4
420394ca:	022877        	blt	a8, a7, 420394d0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x288>
420394cd:	ff6c06        	j	42039281 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x39>
420394d0:	fff0c6        	j	42039497 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x24f>
420394d3:	00          	.byte	00
420394d4:	1d0e51        	l32r	a5, 4200090c <_stext+0x8ec> (ff000000 <_rtc_reserved_end+0x9ef00000>)
420394d7:	b188      	l32i.n	a8, a1, 44
420394d9:	205580        	or	a5, a5, a8
420394dc:	ffca06        	j	42039408 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
420394df:	c44000        	extui	a4, a0, 0, 13
420394e2:	b77020        	ee.stf.64.xp	f7, f11, a2, a0
420394e5:	61d220        	xsr.excsave2	a2
420394e8:	650a      	add.n	a6, a5, a0
420394ea:	9d          	.byte	0x9d
420394eb:	06ed      	mov.n	a14, a6
420394ed:	ffd9      	s32i.n	a13, a15, 60
420394ef:	241956        	bnez	a9, 42039734 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4ec>
420394f2:	0b5d      	mov.n	a5, a11
420394f4:	331b      	addi.n	a3, a3, 1
420394f6:	5188      	l32i.n	a8, a1, 20
420394f8:	0659      	s32i.n	a5, a6, 0
420394fa:	237380        	sext	a7, a3, 15
420394fd:	664b      	addi.n	a6, a6, 4
420394ff:	022877        	blt	a8, a7, 42039505 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2bd>
42039502:	ff5ec6        	j	42039281 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x39>
42039505:	ffe386        	j	42039497 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x24f>
42039508:	021926        	beqi	a9, 1, 4203950e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c6>
4203950b:	007cc6        	j	42039702 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4ba>
4203950e:	4aa8      	l32i.n	a10, a10, 16
42039510:	ffd086        	j	42039456 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x20e>
42039513:	a55000        	extui	a5, a0, 16, 11
42039516:	20          	.byte	0x20
42039517:	20bee0        	or	a11, a14, a14
4203951a:	0a61d2        	s32i	a13, a1, 40
4203951d:	4e91e5        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039520:	a1d8      	l32i.n	a13, a1, 40
42039522:	0a5d      	mov.n	a5, a10
42039524:	ffb806        	j	42039408 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
42039527:	5aa800        	msub.s	f10, f8, f0
4203952a:	ffca06        	j	42039456 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x20e>
4203952d:	20a550        	or	a10, a5, a5
42039530:	4e90a5        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
42039533:	205aa0        	or	a5, a10, a10
42039536:	01c332        	addi	a3, a3, 1
42039539:	5188      	l32i.n	a8, a1, 20
4203953b:	0659      	s32i.n	a5, a6, 0
4203953d:	237380        	sext	a7, a3, 15
42039540:	664b      	addi.n	a6, a6, 4
42039542:	022877        	blt	a8, a7, 42039548 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x300>
42039545:	ff4e06        	j	42039281 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x39>
42039548:	ffd2c6        	j	42039497 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x24f>
4203954b:	028800        	andb	b8, b8, b0
4203954e:	0b3d      	mov.n	a3, a11
42039550:	b888      	l32i.n	a8, a8, 44
42039552:	0a5d      	mov.n	a5, a10
42039554:	0bdd      	mov.n	a13, a11
42039556:	09ed      	mov.n	a14, a9
42039558:	0abd      	mov.n	a11, a10
4203955a:	7149      	s32i.n	a4, a1, 28
4203955c:	07fd      	mov.n	a15, a7
4203955e:	04cd      	mov.n	a12, a4
42039560:	02ad      	mov.n	a10, a2
42039562:	0008e0        	callx8	a8
42039565:	f3fa56        	bnez	a10, 420394a8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x260>
42039568:	249282        	l16si	a8, a2, 72
4203956b:	1a9292        	l16si	a9, a2, 52
4203956e:	2102b2        	l8ui	a11, a2, 33
42039571:	539980        	max	a9, a9, a8
42039574:	190272        	l8ui	a7, a2, 25
42039577:	180282        	l8ui	a8, a2, 24
4203957a:	ec0c      	movi.n	a12, 14
4203957c:	d9fb      	addi.n	a13, a9, 15
4203957e:	1a02f2        	l8ui	a15, a2, 26
42039581:	62ccb0        	saltu	a12, a12, a11
42039584:	1ec992        	addi	a9, a9, 30
42039587:	2002b2        	l8ui	a11, a2, 32
4203958a:	1b02e2        	l8ui	a14, a2, 27
4203958d:	b39dd0        	movgez	a9, a13, a13
42039590:	738870        	maxu	a8, a8, a7
42039593:	cc0b      	addi.n	a12, a12, -1
42039595:	60c0c0        	neg	a12, a12
42039598:	219490        	srai	a9, a9, 4
4203959b:	7388f0        	maxu	a8, a8, a15
4203959e:	dbca      	add.n	a13, a11, a12
420395a0:	7388e0        	maxu	a8, a8, a14
420395a3:	b9ba      	add.n	a11, a9, a11
420395a5:	5388d0        	max	a8, a8, a13
420395a8:	0292f2        	l16si	a15, a2, 4
420395ab:	0492e2        	l16si	a14, a2, 8
420395ae:	bbca      	add.n	a11, a11, a12
420395b0:	0392d2        	l16si	a13, a2, 6
420395b3:	53cb80        	max	a12, a11, a8
420395b6:	0592b2        	l16si	a11, a2, 10
420395b9:	7fca      	add.n	a7, a15, a12
420395bb:	dd8a      	add.n	a13, a13, a8
420395bd:	c0cec0        	sub	a12, a14, a12
420395c0:	03fd      	mov.n	a15, a3
420395c2:	c0e350        	sub	a14, a3, a5
420395c5:	c08b80        	sub	a8, a11, a8
420395c8:	533570        	max	a3, a5, a7
420395cb:	be1b      	addi.n	a11, a14, 1
420395cd:	43ccf0        	min	a12, a12, a15
420395d0:	dd9a      	add.n	a13, a13, a9
420395d2:	91e8      	l32i.n	a14, a1, 36
420395d4:	126172        	s32i	a7, a1, 72
420395d7:	a139      	s32i.n	a3, a1, 40
420395d9:	d1c9      	s32i.n	a12, a1, 52
420395db:	f1d9      	s32i.n	a13, a1, 60
420395dd:	106182        	s32i	a8, a1, 64
420395e0:	1761b2        	s32i	a11, a1, 92
420395e3:	02ae47        	bge	a14, a4, 420395e9 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3a1>
420395e6:	ffaf06        	j	420394a6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x25e>
420395e9:	c08350        	sub	a8, a3, a5
420395ec:	72b570        	salt	a11, a5, a7
420395ef:	880b      	addi.n	a8, a8, -1
420395f1:	1198e0        	slli	a9, a8, 2
420395f4:	d51b      	addi.n	a13, a5, 1
420395f6:	838ab0        	moveqz	a8, a10, a11
420395f9:	839ab0        	moveqz	a9, a10, a11
420395fc:	88da      	add.n	a8, a8, a13
420395fe:	1021a2        	l32i	a10, a1, 64
42039601:	238880        	sext	a8, a8, 15
42039604:	43aea0        	min	a10, a14, a10
42039607:	156182        	s32i	a8, a1, 84
4203960a:	894b      	addi.n	a8, a9, 4
4203960c:	166182        	s32i	a8, a1, 88
4203960f:	8a0b      	addi.n	a8, a10, -1
42039611:	7c1b      	addi.n	a7, a12, 1
42039613:	1721b2        	l32i	a11, a1, 92
42039616:	116182        	s32i	a8, a1, 68
42039619:	8188      	l32i.n	a8, a1, 32
4203961b:	c0c730        	sub	a12, a7, a3
4203961e:	11cce0        	slli	a12, a12, 2
42039621:	11bbe0        	slli	a11, a11, 2
42039624:	b189      	s32i.n	a8, a1, 44
42039626:	a198      	l32i.n	a9, a1, 40
42039628:	d188      	l32i.n	a8, a1, 52
4203962a:	146172        	s32i	a7, a1, 80
4203962d:	c1a9      	s32i.n	a10, a1, 48
4203962f:	1361c2        	s32i	a12, a1, 76
42039632:	e1b9      	s32i.n	a11, a1, 56
42039634:	01cf52        	addi	a5, a15, 1
42039637:	063d      	mov.n	a3, a6
42039639:	6da897        	bge	a8, a9, 420396aa <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x462>
4203963c:	5188      	l32i.n	a8, a1, 20
4203963e:	b148      	l32i.n	a4, a1, 44
42039640:	262847        	blt	a8, a4, 4203966a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x422>
42039643:	0e9182        	l16si	a8, a1, 28
42039646:	037d      	mov.n	a7, a3
42039648:	6189      	s32i.n	a8, a1, 24
4203964a:	52d8      	l32i.n	a13, a2, 20
4203964c:	61c8      	l32i.n	a12, a1, 24
4203964e:	04bd      	mov.n	a11, a4
42039650:	02ad      	mov.n	a10, a2
42039652:	ff8465        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
42039655:	07a9      	s32i.n	a10, a7, 0
42039657:	441b      	addi.n	a4, a4, 1
42039659:	774b      	addi.n	a7, a7, 4
4203965b:	eb9457        	bne	a4, a5, 4203964a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x402>
4203965e:	5188      	l32i.n	a8, a1, 20
42039660:	b198      	l32i.n	a9, a1, 44
42039662:	c04890        	sub	a4, a8, a9
42039665:	834b      	addi.n	a8, a3, 4
42039667:	a03480        	addx4	a3, a4, a8
4203966a:	7198      	l32i.n	a9, a1, 28
4203966c:	f188      	l32i.n	a8, a1, 60
4203966e:	728980        	salt	a8, a9, a8
42039671:	880b      	addi.n	a8, a8, -1
42039673:	608080        	neg	a8, a8
42039676:	748080        	extui	a8, a8, 0, 8
42039679:	f88c      	beqz.n	a8, 4203968c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x444>
4203967b:	102182        	l32i	a8, a1, 64
4203967e:	728890        	salt	a8, a8, a9
42039681:	880b      	addi.n	a8, a8, -1
42039683:	608080        	neg	a8, a8
42039686:	748080        	extui	a8, a8, 0, 8
42039689:	164856        	bnez	a8, 420397f1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5a9>
4203968c:	036d      	mov.n	a6, a3
4203968e:	7188      	l32i.n	a8, a1, 28
42039690:	9198      	l32i.n	a9, a1, 36
42039692:	881b      	addi.n	a8, a8, 1
42039694:	7189      	s32i.n	a8, a1, 28
42039696:	02a987        	bge	a9, a8, 4203969c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x454>
42039699:	ff8246        	j	420394a6 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x25e>
4203969c:	8188      	l32i.n	a8, a1, 32
4203969e:	a198      	l32i.n	a9, a1, 40
420396a0:	b189      	s32i.n	a8, a1, 44
420396a2:	d188      	l32i.n	a8, a1, 52
420396a4:	203660        	or	a3, a6, a6
420396a7:	912897        	blt	a8, a9, 4203963c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3f4>
420396aa:	8148      	l32i.n	a4, a1, 32
420396ac:	122182        	l32i	a8, a1, 72
420396af:	0e9172        	l16si	a7, a1, 28
420396b2:	022487        	blt	a4, a8, 420396b8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x470>
420396b5:	009346        	j	42039906 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6be>
420396b8:	0522d2        	l32i	a13, a2, 20
420396bb:	20b440        	or	a11, a4, a4
420396be:	07cd      	mov.n	a12, a7
420396c0:	02ad      	mov.n	a10, a2
420396c2:	ff7d65        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
420396c5:	a188      	l32i.n	a8, a1, 40
420396c7:	03a9      	s32i.n	a10, a3, 0
420396c9:	441b      	addi.n	a4, a4, 1
420396cb:	334b      	addi.n	a3, a3, 4
420396cd:	e72487        	blt	a4, a8, 420396b8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x470>
420396d0:	162182        	l32i	a8, a1, 88
420396d3:	1521b2        	l32i	a11, a1, 84
420396d6:	468a      	add.n	a4, a6, a8
420396d8:	52d8      	l32i.n	a13, a2, 20
420396da:	07cd      	mov.n	a12, a7
420396dc:	02ad      	mov.n	a10, a2
420396de:	ff7ba5        	call8	42038e98 <_ZNK11roo_windows10Decoration15readWithContentEssN11roo_display5ColorEPKS2_$constprop$0>
420396e1:	132192        	l32i	a9, a1, 76
420396e4:	fcc982        	addi	a8, a9, -4
420396e7:	418280        	srli	a8, a8, 2
420396ea:	349a      	add.n	a3, a4, a9
420396ec:	881b      	addi.n	a8, a8, 1
420396ee:	038876        	loop	a8, 420396f5 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4ad>
420396f1:	04a9      	s32i.n	a10, a4, 0
420396f3:	444b      	addi.n	a4, a4, 4
420396f5:	142182        	l32i	a8, a1, 80
420396f8:	b189      	s32i.n	a8, a1, 44
420396fa:	ffcf86        	j	4203963c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3f4>
420396fd:	0b5d      	mov.n	a5, a11
420396ff:	ff3146        	j	420393c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x180>
42039702:	ffc9c2        	addi	a12, a9, -1
42039705:	23cc80        	sext	a12, a12, 15
42039708:	11cce0        	slli	a12, a12, 2
4203970b:	022c87        	blt	a12, a8, 42039711 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4c9>
4203970e:	004a06        	j	4203983a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5f2>
42039711:	022966        	bnei	a9, 2, 42039717 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4cf>
42039714:	ff7d86        	j	4203950e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c6>
42039717:	80bb90        	add	a11, a11, a9
4203971a:	80bbb0        	add	a11, a11, a11
4203971d:	f8cb92        	addi	a9, a11, -8
42039720:	022897        	blt	a8, a9, 42039726 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x4de>
42039723:	ff79c6        	j	4203950e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x2c6>
42039726:	4ab8      	l32i.n	a11, a10, 16
42039728:	5aa8      	l32i.n	a10, a10, 20
4203972a:	0c2c      	movi.n	a12, 32
4203972c:	a1d9      	s32i.n	a13, a1, 40
4203972e:	4d1025        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039731:	ff47c6        	j	42039454 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x20c>
42039734:	82c890        	mull	a12, a8, a9
42039737:	80a0f2        	movi	a15, 128
4203973a:	ccfa      	add.n	a12, a12, a15
4203973c:	f4c0c0        	extui	a12, a12, 0, 16
4203973f:	11dc80        	slli	a13, a12, 8
42039742:	cdca      	add.n	a12, a13, a12
42039744:	989a      	add.n	a9, a8, a9
42039746:	31c0c0        	srai	a12, a12, 16
42039749:	c0c9c0        	sub	a12, a9, a12
4203974c:	119880        	slli	a9, a8, 8
4203974f:	f470c0        	extui	a7, a12, 0, 16
42039752:	c09980        	sub	a9, a9, a8
42039755:	d29970        	quos	a9, a9, a7
42039758:	f49090        	extui	a9, a9, 0, 16
4203975b:	c08a90        	sub	a8, a10, a9
4203975e:	75c0b0        	extui	a12, a11, 16, 8
42039761:	f48080        	extui	a8, a8, 0, 16
42039764:	c1ec90        	mul16u	a14, a12, a9
42039767:	75d050        	extui	a13, a5, 16, 8
4203976a:	74c8b0        	extui	a12, a11, 8, 8
4203976d:	c1dd80        	mul16u	a13, a13, a8
42039770:	c1cc90        	mul16u	a12, a12, a9
42039773:	74a850        	extui	a10, a5, 8, 8
42039776:	74b0b0        	extui	a11, a11, 0, 8
42039779:	c1aa80        	mul16u	a10, a10, a8
4203977c:	c19b90        	mul16u	a9, a11, a9
4203977f:	745050        	extui	a5, a5, 0, 8
42039782:	eefa      	add.n	a14, a14, a15
42039784:	c15580        	mul16u	a5, a5, a8
42039787:	eeda      	add.n	a14, a14, a13
42039789:	8cfa      	add.n	a8, a12, a15
4203978b:	f4e0e0        	extui	a14, a14, 0, 16
4203978e:	88aa      	add.n	a8, a8, a10
42039790:	99fa      	add.n	a9, a9, a15
42039792:	f4a080        	extui	a10, a8, 0, 16
42039795:	995a      	add.n	a9, a9, a5
42039797:	118e80        	slli	a8, a14, 8
4203979a:	f49090        	extui	a9, a9, 0, 16
4203979d:	115a80        	slli	a5, a10, 8
420397a0:	88ea      	add.n	a8, a8, a14
420397a2:	55aa      	add.n	a5, a5, a10
420397a4:	758080        	extui	a8, a8, 16, 8
420397a7:	11b980        	slli	a11, a9, 8
420397aa:	01a780        	slli	a10, a7, 24
420397ad:	118800        	slli	a8, a8, 16
420397b0:	9b9a      	add.n	a9, a11, a9
420397b2:	415850        	srli	a5, a5, 8
420397b5:	2088a0        	or	a8, a8, a10
420397b8:	759090        	extui	a9, a9, 16, 8
420397bb:	745850        	extui	a5, a5, 8, 8
420397be:	208890        	or	a8, a8, a9
420397c1:	115580        	slli	a5, a5, 8
420397c4:	205850        	or	a5, a8, a5
420397c7:	331b      	addi.n	a3, a3, 1
420397c9:	5188      	l32i.n	a8, a1, 20
420397cb:	0659      	s32i.n	a5, a6, 0
420397cd:	237380        	sext	a7, a3, 15
420397d0:	664b      	addi.n	a6, a6, 4
420397d2:	022877        	blt	a8, a7, 420397d8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x590>
420397d5:	feaa06        	j	42039281 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x39>
420397d8:	ff2ec6        	j	42039497 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x24f>
420397db:	a55000        	extui	a5, a0, 16, 11
420397de:	20          	.byte	0x20
420397df:	0c6182        	s32i	a8, a1, 48
420397e2:	0a61d2        	s32i	a13, a1, 40
420397e5:	4e6565        	call8	42087e3c <_ZNK11roo_display7BlendOpILNS_12BlendingModeE12EE5blendENS_5ColorES3_$isra$0>
420397e8:	a1d8      	l32i.n	a13, a1, 40
420397ea:	c188      	l32i.n	a8, a1, 48
420397ec:	0a5d      	mov.n	a5, a10
420397ee:	fef586        	j	420393c8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x180>
420397f1:	c188      	l32i.n	a8, a1, 48
420397f3:	097d      	mov.n	a7, a9
420397f5:	022987        	blt	a9, a8, 420397fb <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5b3>
420397f8:	ffa406        	j	4203968c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x444>
420397fb:	172182        	l32i	a8, a1, 92
420397fe:	0218e6        	bgei	a8, 1, 42039804 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5bc>
42039801:	003ac6        	j	420398f0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6a8>
42039804:	e198      	l32i.n	a9, a1, 56
42039806:	038d      	mov.n	a8, a3
42039808:	024966        	bnei	a9, 4, 4203980e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5c6>
4203980b:	003fc6        	j	4203990e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6c6>
4203980e:	e148      	l32i.n	a4, a1, 56
42039810:	08ad      	mov.n	a10, a8
42039812:	04cd      	mov.n	a12, a4
42039814:	06bd      	mov.n	a11, a6
42039816:	1c3f81        	l32r	a8, 42000914 <_stext+0x8f4> (40001200 <memmove>)
42039819:	0008e0        	callx8	a8
4203981c:	c198      	l32i.n	a9, a1, 48
4203981e:	771b      	addi.n	a7, a7, 1
42039820:	8a4a      	add.n	a8, a10, a4
42039822:	e89977        	bne	a9, a7, 4203980e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x5c6>
42039825:	7188      	l32i.n	a8, a1, 28
42039827:	690b      	addi.n	a6, a9, -1
42039829:	c06680        	sub	a6, a6, a8
4203982c:	e188      	l32i.n	a8, a1, 56
4203982e:	7199      	s32i.n	a9, a1, 28
42039830:	826680        	mull	a6, a6, a8
42039833:	838a      	add.n	a8, a3, a8
42039835:	668a      	add.n	a6, a6, a8
42039837:	ff94c6        	j	4203968e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x446>
4203983a:	042ab2        	l32i	a11, a10, 16
4203983d:	052aa2        	l32i	a10, a10, 20
42039840:	40a0c2        	movi	a12, 64
42039843:	a1d9      	s32i.n	a13, a1, 40
42039845:	4cfea5        	call8	42086830 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42039848:	ff0206        	j	42039454 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x20c>
4203984b:	bba000        	movgez.s	f10, f0, a0
4203984e:	a0f282        	s32ri	a8, a2, 0x280
42039851:	bbfa80        	movgez.s	f15, f10, a8
42039854:	f4b0b0        	extui	a11, a11, 0, 16
42039857:	11cb80        	slli	a12, a11, 8
4203985a:	bcba      	add.n	a11, a12, a11
4203985c:	a8aa      	add.n	a10, a8, a10
4203985e:	31b0b0        	srai	a11, a11, 16
42039861:	c0bab0        	sub	a11, a10, a11
42039864:	11a880        	slli	a10, a8, 8
42039867:	f4b0b0        	extui	a11, a11, 0, 16
4203986a:	c0aa80        	sub	a10, a10, a8
4203986d:	d2aab0        	quos	a10, a10, a11
42039870:	f4a0a0        	extui	a10, a10, 0, 16
42039873:	75c0e0        	extui	a12, a14, 16, 8
42039876:	c089a0        	sub	a8, a9, a10
42039879:	c19ca0        	mul16u	a9, a12, a10
4203987c:	f48080        	extui	a8, a8, 0, 16
4203987f:	b199      	s32i.n	a9, a1, 44
42039881:	7498e0        	extui	a9, a14, 8, 8
42039884:	c199a0        	mul16u	a9, a9, a10
42039887:	a1b9      	s32i.n	a11, a1, 40
42039889:	c199      	s32i.n	a9, a1, 48
4203988b:	7490e0        	extui	a9, a14, 0, 8
4203988e:	c199a0        	mul16u	a9, a9, a10
42039891:	75b050        	extui	a11, a5, 16, 8
42039894:	74c850        	extui	a12, a5, 8, 8
42039897:	b1a8      	l32i.n	a10, a1, 44
42039899:	745050        	extui	a5, a5, 0, 8
4203989c:	c1bb80        	mul16u	a11, a11, a8
4203989f:	c1cc80        	mul16u	a12, a12, a8
420398a2:	c1e580        	mul16u	a14, a5, a8
420398a5:	c188      	l32i.n	a8, a1, 48
420398a7:	aafa      	add.n	a10, a10, a15
420398a9:	aaba      	add.n	a10, a10, a11
420398ab:	88fa      	add.n	a8, a8, a15
420398ad:	f4a0a0        	extui	a10, a10, 0, 16
420398b0:	88ca      	add.n	a8, a8, a12
420398b2:	f4b080        	extui	a11, a8, 0, 16
420398b5:	99fa      	add.n	a9, a9, a15
420398b7:	118a80        	slli	a8, a10, 8
420398ba:	88aa      	add.n	a8, a8, a10
420398bc:	99ea      	add.n	a9, a9, a14
420398be:	11ab80        	slli	a10, a11, 8
420398c1:	f49090        	extui	a9, a9, 0, 16
420398c4:	aaba      	add.n	a10, a10, a11
420398c6:	a1b8      	l32i.n	a11, a1, 40
420398c8:	758080        	extui	a8, a8, 16, 8
420398cb:	11c980        	slli	a12, a9, 8
420398ce:	118800        	slli	a8, a8, 16
420398d1:	01bb80        	slli	a11, a11, 24
420398d4:	cc9a      	add.n	a12, a12, a9
420398d6:	41a8a0        	srli	a10, a10, 8
420398d9:	2058b0        	or	a5, a8, a11
420398dc:	75c0c0        	extui	a12, a12, 16, 8
420398df:	7488a0        	extui	a8, a10, 8, 8
420398e2:	2055c0        	or	a5, a5, a12
420398e5:	118880        	slli	a8, a8, 8
420398e8:	205580        	or	a5, a5, a8
420398eb:	fec646        	j	42039408 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1c0>
420398ee:	820000        	mull	a0, a0, a0
420398f1:	901121        	l32r	a2, 4201d938 <roo_display::(anonymous namespace)::RoundRectStream::PrepareRow()+0x214> (a0aa0b00 <_rtc_reserved_end+0x409a0b00>)
420398f4:	c068      	l32i.n	a6, a0, 48
420398f6:	e188      	l32i.n	a8, a1, 56
420398f8:	826680        	mull	a6, a6, a8
420398fb:	838a      	add.n	a8, a3, a8
420398fd:	668a      	add.n	a6, a6, a8
420398ff:	c188      	l32i.n	a8, a1, 48
42039901:	7189      	s32i.n	a8, a1, 28
42039903:	ff61c6        	j	4203968e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x446>
42039906:	04bd      	mov.n	a11, a4
42039908:	064d      	mov.n	a4, a6
4203990a:	ff7286        	j	420396d8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x490>
4203990d:	c18800        	mul16u	a8, a8, a0
42039910:	039d      	mov.n	a9, a3
42039912:	c08870        	sub	a8, a8, a7
42039915:	058876        	loop	a8, 4203991e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x6d6>
42039918:	06a8      	l32i.n	a10, a6, 0
4203991a:	09a9      	s32i.n	a10, a9, 0
4203991c:	994b      	addi.n	a9, a9, 4
4203991e:	c198      	l32i.n	a9, a1, 48
42039920:	7188      	l32i.n	a8, a1, 28
42039922:	690b      	addi.n	a6, a9, -1
42039924:	c06680        	sub	a6, a6, a8
42039927:	834b      	addi.n	a8, a3, 4
42039929:	a06680        	addx4	a6, a6, a8
4203992c:	7199      	s32i.n	a9, a1, 28
4203992e:	ff5706        	j	4203968e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x446>
