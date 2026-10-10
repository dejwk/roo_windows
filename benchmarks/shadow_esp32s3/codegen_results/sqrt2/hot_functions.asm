
/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038d14 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42038d14:	004136        	entry	a1, 32
42038d17:	15cc      	bnez.n	a5, 42038d1c <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x8>
42038d19:	f01d      	retw.n
42038d1b:	94c200        	extui	a12, a0, 2, 10
42038d1e:	93b200        	movnez	a11, a2, a0
42038d21:	a22000        	muluh	a2, a0, a0
42038d24:	20          	.byte	0x20
42038d25:	fffd25        	call8	42038cf8 <roo_windows::Decoration::read(short, short) const>
42038d28:	0066a2        	s32i	a10, a6, 0
42038d2b:	02c332        	addi	a3, a3, 2
42038d2e:	442b      	addi.n	a4, a4, 2
42038d30:	550b      	addi.n	a5, a5, -1
42038d32:	664b      	addi.n	a6, a6, 4
42038d34:	fff7c6        	j	42038d17 <roo_windows::Decoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x3>


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

4203313c <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
4203313c:	006136        	entry	a1, 48
4203313f:	002282        	l32i	a8, a2, 0
42033142:	20f770        	or	a15, a7, a7
42033145:	0b2882        	l32i	a8, a8, 44
42033148:	233380        	sext	a3, a3, 15
4203314b:	234480        	sext	a4, a4, 15
4203314e:	235580        	sext	a5, a5, 15
42033151:	236680        	sext	a6, a6, 15
42033154:	02ad      	mov.n	a10, a2
42033156:	31f9      	s32i.n	a15, a1, 12
42033158:	06ed      	mov.n	a14, a6
4203315a:	05dd      	mov.n	a13, a5
4203315c:	04cd      	mov.n	a12, a4
4203315e:	03bd      	mov.n	a11, a3
42033160:	0008e0        	callx8	a8
42033163:	027d      	mov.n	a7, a2
42033165:	0a2d      	mov.n	a2, a10
42033167:	facc      	bnez.n	a10, 4203317a <roo_windows::internal::RoundedDecoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x3e>
42033169:	31f8      	l32i.n	a15, a1, 12
4203316b:	06ed      	mov.n	a14, a6
4203316d:	05dd      	mov.n	a13, a5
4203316f:	04cd      	mov.n	a12, a4
42033171:	03bd      	mov.n	a11, a3
42033173:	07ad      	mov.n	a10, a7
42033175:	dc5d65        	call8	4200f74c <roo_display::Rasterizable::readColorRect(short, short, short, short, roo_display::Color*) const>
42033178:	0a2d      	mov.n	a2, a10
4203317a:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038f68 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>:
42038f68:	004136        	entry	a1, 32
42038f6b:	029d      	mov.n	a9, a2
42038f6d:	120c      	movi.n	a2, 1
42038f6f:	832990        	moveqz	a2, a9, a9
42038f72:	4949b6        	bltui	a9, 4, 42038fbf <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x57>
42038f75:	40f980        	nsau	a8, a9
42038f78:	e1c882        	addi	a8, a8, -31
42038f7b:	608080        	neg	a8, a8
42038f7e:	21a180        	srai	a10, a8, 1
42038f81:	2b0c      	movi.n	a11, 2
42038f83:	400a00        	ssr	a10
42038f86:	918090        	srl	a8, a9
42038f89:	401a00        	ssl	a10
42038f8c:	a1ab00        	sll	a10, a11
42038f8f:	88aa      	add.n	a8, a8, a10
42038f91:	1e52a1        	l32r	a10, 420008dc <_stext+0x8bc> (aaaaaaab <_rtc_reserved_end+0x4a9aaaab>)
42038f94:	a288a0        	muluh	a8, a8, a10
42038f97:	418180        	srli	a8, a8, 1
42038f9a:	c2a980        	quou	a10, a9, a8
42038f9d:	aa8a      	add.n	a10, a10, a8
42038f9f:	41a1a0        	srli	a10, a10, 1
42038fa2:	c289a0        	quou	a8, a9, a10
42038fa5:	8088a0        	add	a8, a8, a10
42038fa8:	01a0a2        	movi	a10, 1
42038fab:	418180        	srli	a8, a8, 1
42038fae:	11aa00        	slli	a10, a10, 16
42038fb1:	0518a7        	beq	a8, a10, 42038fba <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x52>
42038fb4:	82a880        	mull	a10, a8, a8
42038fb7:	01b9a7        	bgeu	a9, a10, 42038fbc <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x54>
42038fba:	880b      	addi.n	a8, a8, -1
42038fbc:	f42080        	extui	a2, a8, 0, 16
42038fbf:	f01d      	retw.n


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038728 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>:
42038728:	004136        	entry	a1, 32
4203872b:	208230        	or	a8, a2, a3
4203872e:	0228d6        	bgez	a8, 42038754 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x2c>
42038731:	c02240        	sub	a2, a2, a4
42038734:	40f290        	nsau	a9, a2
42038737:	419590        	srli	a9, a9, 5
4203873a:	b28c      	beqz.n	a2, 42038749 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x21>
4203873c:	c03340        	sub	a3, a3, a4
4203873f:	180c      	movi.n	a8, 1
42038741:	938930        	movnez	a8, a9, a3
42038744:	ffa022        	movi	a2, 255
42038747:	688c      	beqz.n	a8, 42038751 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42038749:	1125c0        	slli	a2, a5, 4
4203874c:	252a      	add.n	a2, a5, a2
4203874e:	742020        	extui	a2, a2, 0, 8
42038751:	f01d      	retw.n
42038753:	222000        	orb	b2, b0, b0
42038756:	3330d1        	l32r	a13, 42005418 <roo_io::internal::SdSpiProbeCardEspIdf(spi_host_device_t, gpio_num_t)+0x138> (23d80000 <UserFrameTotalSize+0x23d7ff00>)
42038759:	823ad1        	l32r	a13, 42019044 <roo_display::SmoothFontV2::drawKernedGlyphsModeFill(roo_display::DisplayOutput&, short, short, short, roo_display::GlyphMetrics const&, bool, std::byte const*, short, roo_display::GlyphMetrics const&, bool, std::byte const*, short, roo_display::Box const&, roo_display::Color, roo_display::Color, roo_display::BlendingMode) const+0x270> (822182fa <_rtc_reserved_end+0x221182fa>)
4203875c:	118880        	slli	a8, a8, 8
4203875f:	ffa022        	movi	a2, 255
42038762:	eb48a6        	blti	a8, 4, 42038751 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
42038765:	40f890        	nsau	a9, a8
42038768:	e1c992        	addi	a9, a9, -31
4203876b:	609090        	neg	a9, a9
4203876e:	21a190        	srai	a10, a9, 1
42038771:	2b0c      	movi.n	a11, 2
42038773:	400a00        	ssr	a10
42038776:	919080        	srl	a9, a8
42038779:	401a00        	ssl	a10
4203877c:	a1ab00        	sll	a10, a11
4203877f:	99aa      	add.n	a9, a9, a10
42038781:	2056a1        	l32r	a10, 420008dc <_stext+0x8bc> (aaaaaaab <_rtc_reserved_end+0x4a9aaaab>)
42038784:	a299a0        	muluh	a9, a9, a10
42038787:	419190        	srli	a9, a9, 1
4203878a:	c2a890        	quou	a10, a8, a9
4203878d:	aa9a      	add.n	a10, a10, a9
4203878f:	41a1a0        	srli	a10, a10, 1
42038792:	c298a0        	quou	a9, a8, a10
42038795:	8099a0        	add	a9, a9, a10
42038798:	01a0a2        	movi	a10, 1
4203879b:	419190        	srli	a9, a9, 1
4203879e:	11aa00        	slli	a10, a10, 16
420387a1:	0519a7        	beq	a9, a10, 420387aa <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x82>
420387a4:	82a990        	mull	a10, a9, a9
420387a7:	01b8a7        	bgeu	a8, a10, 420387ac <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x84>
420387aa:	990b      	addi.n	a9, a9, -1
420387ac:	1144c0        	slli	a4, a4, 4
420387af:	554a      	add.n	a5, a5, a4
420387b1:	c05590        	sub	a5, a5, a9
420387b4:	f48050        	extui	a8, a5, 0, 16
420387b7:	239580        	sext	a9, a5, 15
420387ba:	020c      	movi.n	a2, 0
420387bc:	91e5f7        	bbsi	a5, 15, 42038751 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420387bf:	fa0c      	movi.n	a10, 15
420387c1:	ffa022        	movi	a2, 255
420387c4:	892a97        	blt	a10, a9, 42038751 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>
420387c7:	1198c0        	slli	a9, a8, 4
420387ca:	889a      	add.n	a8, a8, a9
420387cc:	742080        	extui	a2, a8, 0, 8
420387cf:	ffdf86        	j	42038751 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)+0x29>


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42033eb0 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const>:
42033eb0:	00a136        	entry	a1, 80
42033eb3:	3169      	s32i.n	a6, a1, 12
42033eb5:	2159      	s32i.n	a5, a1, 8
42033eb7:	060c      	movi.n	a6, 0
42033eb9:	1cc172        	addi	a7, a1, 28
42033ebc:	2188      	l32i.n	a8, a1, 8
42033ebe:	029687        	bne	a6, a8, 42033ec4 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x14>
42033ec1:	f01d      	retw.n
42033ec3:	938200        	movnez	a8, a2, a0
42033ec6:	22d200        	orb	b13, b2, b0
42033ec9:	018917        	bany	a9, a1, 42033ece <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x1e>
42033ecc:	009482        	l16si	a8, a4, 0
42033ecf:	01b8      	l32i.n	a11, a1, 0
42033ed1:	1622a2        	l32i	a10, a2, 88
42033ed4:	08cd      	mov.n	a12, a8
42033ed6:	0067d2        	s32i	a13, a7, 0
42033ed9:	20e770        	or	a14, a7, a7
42033edc:	016182        	s32i	a8, a1, 4
42033edf:	201110        	or	a1, a1, a1
42033ee2:	fffa25        	call8	42033e84 <roo_windows::internal::RoundedClip::contentAt(short, short, roo_display::Color, roo_display::Color&) const>
42033ee5:	182282        	l32i	a8, a2, 96
42033ee8:	1922b2        	l32i	a11, a2, 100
42033eeb:	8189      	s32i.n	a8, a1, 32
42033eed:	20c152        	addi	a5, a1, 32
42033ef0:	050b16        	beqz	a11, 42033f44 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0x94>
42033ef3:	0bc8      	l32i.n	a12, a11, 0
42033ef5:	24c1a2        	addi	a10, a1, 36
42033ef8:	201110        	or	a1, a1, a1
42033efb:	ff3665        	call8	42033260 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033efe:	07d8      	l32i.n	a13, a7, 0
42033f00:	11c8      	l32i.n	a12, a1, 4
42033f02:	0021b2        	l32i	a11, a1, 0
42033f05:	24c1a2        	addi	a10, a1, 36
42033f08:	201110        	or	a1, a1, a1
42033f0b:	fe1865        	call8	42032090 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f0e:	07a9      	s32i.n	a10, a7, 0
42033f10:	24c1a2        	addi	a10, a1, 36
42033f13:	201110        	or	a1, a1, a1
42033f16:	f894a5        	call8	4202c860 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f19:	1922b2        	l32i	a11, a2, 100
42033f1c:	24c1a2        	addi	a10, a1, 36
42033f1f:	002bc2        	l32i	a12, a11, 0
42033f22:	201110        	or	a1, a1, a1
42033f25:	ff33a5        	call8	42033260 <roo_windows::internal::PaintEffectStack::PaintEffectStack(roo_windows::internal::PaintEffect const*, roo_windows::internal::PaintEffect const*)>
42033f28:	05d8      	l32i.n	a13, a5, 0
42033f2a:	0094c2        	l16si	a12, a4, 0
42033f2d:	0093b2        	l16si	a11, a3, 0
42033f30:	24c1a2        	addi	a10, a1, 36
42033f33:	201110        	or	a1, a1, a1
42033f36:	fe15a5        	call8	42032090 <roo_windows::internal::PaintEffectStack::apply(short, short, roo_display::Color) const>
42033f39:	05a9      	s32i.n	a10, a5, 0
42033f3b:	24c1a2        	addi	a10, a1, 36
42033f3e:	201110        	or	a1, a1, a1
42033f41:	f891e5        	call8	4202c860 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f44:	0094c2        	l16si	a12, a4, 0
42033f47:	0093b2        	l16si	a11, a3, 0
42033f4a:	07d8      	l32i.n	a13, a7, 0
42033f4c:	05ed      	mov.n	a14, a5
42033f4e:	a24b      	addi.n	a10, a2, 4
42033f50:	201110        	or	a1, a1, a1
42033f53:	04c2a5        	call8	42038b7c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>
42033f56:	3188      	l32i.n	a8, a1, 12
42033f58:	332b      	addi.n	a3, a3, 2
42033f5a:	a08680        	addx4	a8, a6, a8
42033f5d:	08a9      	s32i.n	a10, a8, 0
42033f5f:	661b      	addi.n	a6, a6, 1
42033f61:	442b      	addi.n	a4, a4, 2
42033f63:	ffd546        	j	42033ebc <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xc>
42033f66:	c1a200        	mul16u	a10, a2, a0
42033f69:	111024        	ee.ldqa.s8.128.ip	a2, 0x100
42033f6c:	20          	.byte	0x20
42033f6d:	f88f25        	call8	4202c860 <roo_windows::internal::PaintEffectStack::~PaintEffectStack()>
42033f70:	02ad      	mov.n	a10, a2
42033f72:	201110        	or	a1, a1, a1
42033f75:	3cd9e5        	call8	42070d14 <_Unwind_Resume>
42033f78:	fffac6        	j	42033f67 <roo_windows::internal::RoundedDecoration::readColors(short const*, short const*, unsigned long, roo_display::Color*) const+0xb7>


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

420387d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>:
420387d4:	006136        	entry	a1, 48
420387d7:	189192        	l16si	a9, a1, 48
420387da:	3169      	s32i.n	a6, a1, 12
420387dc:	f1cb      	addi.n	a15, a1, 12
420387de:	1a9182        	l16si	a8, a1, 52
420387e1:	02ed      	mov.n	a14, a2
420387e3:	74d070        	extui	a13, a7, 0, 8
420387e6:	02a927        	bge	a9, a2, 420387ec <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x18>
420387e9:	0030c6        	j	420388b0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
420387ec:	02a497        	bge	a4, a9, 420387f2 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x1e>
420387ef:	002f46        	j	420388b0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
420387f2:	02a837        	bge	a8, a3, 420387f8 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x24>
420387f5:	002dc6        	j	420388b0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
420387f8:	02a587        	bge	a5, a8, 420387fe <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x2a>
420387fb:	002c46        	j	420388b0 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xdc>
420387fe:	000fc2        	l8ui	a12, a15, 0
42038801:	3c9c      	beqz.n	a12, 42038818 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038803:	a2ca      	add.n	a10, a2, a12
42038805:	0f2a97        	blt	a10, a9, 42038818 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
42038808:	b3ca      	add.n	a11, a3, a12
4203880a:	0a2b87        	blt	a11, a8, 42038818 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x44>
4203880d:	c0bb80        	sub	a11, a11, a8
42038810:	c0aa90        	sub	a10, a10, a9
42038813:	000706        	j	42038833 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x5f>
42038816:	c20000        	quou	a0, a0, a0
42038819:	1c16010f 	ee.vmulas.s16.qacc.ld.ip.qup	q0, a0, -240, q4, q0, q1, q6
4203881d:	a4c002        	addi	a0, a0, -92
42038820:	c0          	.byte	0xc0
42038821:	1b29a7        	blt	a9, a10, 42038840 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
42038824:	80b3c0        	add	a11, a3, a12
42038827:	152b87        	blt	a11, a8, 42038840 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6c>
4203882a:	c0bb80        	sub	a11, a11, a8
4203882d:	c0ac40        	sub	a10, a12, a4
42038830:	80aa90        	add	a10, a10, a9
42038833:	23bb80        	sext	a11, a11, 15
42038836:	23aa80        	sext	a10, a10, 15
42038839:	ffeee5        	call8	42038728 <roo_windows::(anonymous namespace)::calcRoundCornerAlpha(short, short, short, unsigned char)>
4203883c:	0a2d      	mov.n	a2, a10
4203883e:	f01d      	retw.n
42038840:	020fc2        	l8ui	a12, a15, 2
42038843:	2c9c      	beqz.n	a12, 42038859 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038845:	c0a4c0        	sub	a10, a4, a12
42038848:	0d29a7        	blt	a9, a10, 42038859 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
4203884b:	c0a5c0        	sub	a10, a5, a12
4203884e:	0728a7        	blt	a8, a10, 42038859 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x85>
42038851:	c0bc50        	sub	a11, a12, a5
42038854:	bb8a      	add.n	a11, a11, a8
42038856:	fff4c6        	j	4203882d <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x59>
42038859:	030fc2        	l8ui	a12, a15, 3
4203885c:	1c9c      	beqz.n	a12, 42038871 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
4203885e:	aeca      	add.n	a10, a14, a12
42038860:	0d2a97        	blt	a10, a9, 42038871 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
42038863:	c0b5c0        	sub	a11, a5, a12
42038866:	0728b7        	blt	a8, a11, 42038871 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x9d>
42038869:	c0bc50        	sub	a11, a12, a5
4203886c:	bb8a      	add.n	a11, a11, a8
4203886e:	ffe786        	j	42038810 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x3c>
42038871:	ea0c      	movi.n	a10, 14
42038873:	ffa022        	movi	a2, 255
42038876:	c43ad7        	bltu	a10, a13, 4203883e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
42038879:	c0e9e0        	sub	a14, a9, a14
4203887c:	40fea0        	nsau	a10, a14
4203887f:	1b0c      	movi.n	a11, 1
42038881:	41a5a0        	srli	a10, a10, 5
42038884:	ce9c      	beqz.n	a14, 420388a4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
42038886:	c09940        	sub	a9, a9, a4
42038889:	83ab90        	moveqz	a10, a11, a9
4203888c:	014a56        	bnez	a10, 420388a4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
4203888f:	c03830        	sub	a3, a8, a3
42038892:	40f390        	nsau	a9, a3
42038895:	419590        	srli	a9, a9, 5
42038898:	008316        	beqz	a3, 420388a4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0xd0>
4203889b:	c08850        	sub	a8, a8, a5
4203889e:	93b980        	movnez	a11, a9, a8
420388a1:	f99b16        	beqz	a11, 4203883e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420388a4:	118dc0        	slli	a8, a13, 4
420388a7:	dd8a      	add.n	a13, a13, a8
420388a9:	7420d0        	extui	a2, a13, 0, 8
420388ac:	ffe386        	j	4203883e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>
420388af:	020c00        	andb	b0, b12, b0
420388b2:	ffe206        	j	4203883e <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)+0x6a>


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038b7c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const>:
42038b7c:	008136        	entry	a1, 64
42038b7f:	2002a2        	l8ui	a10, a2, 32
42038b82:	021292        	l16ui	a9, a2, 4
42038b85:	0512d2        	l16ui	a13, a2, 10
42038b88:	0412c2        	l16ui	a12, a2, 8
42038b8b:	0312b2        	l16ui	a11, a2, 6
42038b8e:	233380        	sext	a3, a3, 15
42038b91:	234480        	sext	a4, a4, 15
42038b94:	1149      	s32i.n	a4, a1, 4
42038b96:	0139      	s32i.n	a3, a1, 0
42038b98:	c0dda0        	sub	a13, a13, a10
42038b9b:	c0cca0        	sub	a12, a12, a10
42038b9e:	baba      	add.n	a11, a10, a11
42038ba0:	2102f2        	l8ui	a15, a2, 33
42038ba3:	aa9a      	add.n	a10, a10, a9
42038ba5:	72e8      	l32i.n	a14, a2, 28
42038ba7:	23dd80        	sext	a13, a13, 15
42038baa:	23cc80        	sext	a12, a12, 15
42038bad:	23bb80        	sext	a11, a11, 15
42038bb0:	23aa80        	sext	a10, a10, 15
42038bb3:	7169      	s32i.n	a6, a1, 28
42038bb5:	ffc1e5        	call8	420387d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bb8:	ffa092        	movi	a9, 255
42038bbb:	027d      	mov.n	a7, a2
42038bbd:	0a6d      	mov.n	a6, a10
42038bbf:	079a97        	bne	a10, a9, 42038bca <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x4e>
42038bc2:	142292        	l32i	a9, a2, 80
42038bc5:	052d      	mov.n	a2, a5
42038bc7:	0a1916        	beqz	a9, 42038c6c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038bca:	04dd      	mov.n	a13, a4
42038bcc:	03cd      	mov.n	a12, a3
42038bce:	3cc7b2        	addi	a11, a7, 60
42038bd1:	28c7a2        	addi	a10, a7, 40
42038bd4:	005b25        	call8	42039188 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>
42038bd7:	200792        	l8ui	a9, a7, 32
42038bda:	6169      	s32i.n	a6, a1, 24
42038bdc:	012a80        	slli	a2, a10, 24
42038bdf:	03c916        	beqz	a9, 42038c1f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038be2:	1149      	s32i.n	a4, a1, 4
42038be4:	0139      	s32i.n	a3, a1, 0
42038be6:	67e8      	l32i.n	a14, a7, 24
42038be8:	0597d2        	l16si	a13, a7, 10
42038beb:	0497c2        	l16si	a12, a7, 8
42038bee:	0397b2        	l16si	a11, a7, 6
42038bf1:	0297a2        	l16si	a10, a7, 4
42038bf4:	ff0c      	movi.n	a15, 15
42038bf6:	ffbde5        	call8	420387d4 <roo_windows::(anonymous namespace)::calcRoundRectAlpha(short, short, short, short, roo_windows::BorderStyle::CornerRadii, unsigned char, short, short)>
42038bf9:	61a9      	s32i.n	a10, a1, 24
42038bfb:	0aac      	beqz.n	a10, 42038c1f <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xa3>
42038bfd:	7188      	l32i.n	a8, a1, 28
42038bff:	38cc      	bnez.n	a8, 42038c06 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x8a>
42038c01:	24c782        	addi	a8, a7, 36
42038c04:	7189      	s32i.n	a8, a1, 28
42038c06:	7188      	l32i.n	a8, a1, 28
42038c08:	02ad      	mov.n	a10, a2
42038c0a:	0898      	l32i.n	a9, a8, 0
42038c0c:	6188      	l32i.n	a8, a1, 24
42038c0e:	119980        	slli	a9, a9, 8
42038c11:	419890        	srli	a9, a9, 8
42038c14:	01b880        	slli	a11, a8, 24
42038c17:	20b9b0        	or	a11, a9, a11
42038c1a:	cec3e5        	call8	42007858 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c1d:	0a2d      	mov.n	a2, a10
42038c1f:	ffa092        	movi	a9, 255
42038c22:	4a9697        	bne	a6, a9, 42038c70 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf4>
42038c25:	fb7c      	movi.n	a11, -1
42038c27:	01bb80        	slli	a11, a11, 24
42038c2a:	20b5b0        	or	a11, a5, a11
42038c2d:	02ad      	mov.n	a10, a2
42038c2f:	cec2a5        	call8	42007858 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c32:	0a2d      	mov.n	a2, a10
42038c34:	1427a2        	l32i	a10, a7, 80
42038c37:	031a16        	beqz	a10, 42038c6c <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xf0>
42038c3a:	180a82        	l8ui	a8, a10, 24
42038c3d:	053816        	beqz	a8, 42038c94 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x118>
42038c40:	20c440        	or	a12, a4, a4
42038c43:	03bd      	mov.n	a11, a3
42038c45:	ee24a5        	call8	42026e90 <roo_windows::PressOverlay::get_with_clip_circle(short, short) const>
42038c48:	6188      	l32i.n	a8, a1, 24
42038c4a:	75b8a0        	extui	a11, a10, 24, 8
42038c4d:	d1bb80        	mul16s	a11, a11, a8
42038c50:	1f2581        	l32r	a8, 420008e4 <_stext+0x8c4> (80808081 <_rtc_reserved_end+0x20708081>)
42038c53:	11aa80        	slli	a10, a10, 8
42038c56:	a2bb80        	muluh	a11, a11, a8
42038c59:	01bbf0        	slli	a11, a11, 17
42038c5c:	75b8b0        	extui	a11, a11, 24, 8
42038c5f:	404800        	ssai	8
42038c62:	81bba0        	src	a11, a11, a10
42038c65:	02ad      	mov.n	a10, a2
42038c67:	cebf25        	call8	42007858 <roo_display::AlphaBlend(roo_display::Color, roo_display::Color)>
42038c6a:	0a2d      	mov.n	a2, a10
42038c6c:	f01d      	retw.n
42038c6e:	160000        	ee.ldf.64.xp	f0, f1, a0, a0
42038c71:	50fc06        	j	4204d065 <uart_module_disable+0x6d>
42038c74:	7598      	l32i.n	a9, a5, 28
42038c76:	d19960        	mul16s	a9, a9, a6
42038c79:	1f1aa1        	l32r	a10, 420008e4 <_stext+0x8c4> (80808081 <_rtc_reserved_end+0x20708081>)
42038c7c:	7fc992        	addi	a9, a9, 127
42038c7f:	a299a0        	muluh	a9, a9, a10
42038c82:	11b580        	slli	a11, a5, 8
42038c85:	0199f0        	slli	a9, a9, 17
42038c88:	759890        	extui	a9, a9, 24, 8
42038c8b:	404800        	ssai	8
42038c8e:	81b9b0        	src	a11, a9, a11
42038c91:	ffe606        	j	42038c2d <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xb1>
42038c94:	021a82        	l16ui	a8, a10, 4
42038c97:	3ab8      	l32i.n	a11, a10, 12
42038c99:	c03380        	sub	a3, a3, a8
42038c9c:	031a82        	l16ui	a8, a10, 6
42038c9f:	c04480        	sub	a4, a4, a8
42038ca2:	740444        	mul.aa.ll	a4, a4
42038ca5:	780334        	mula.aa.ll	a3, a3
42038ca8:	031080        	rsr.acclo	a8
42038cab:	c0bb80        	sub	a11, a11, a8
42038cae:	061be6        	bgei	a11, 1, 42038cb8 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x13c>
42038cb1:	5aa8      	l32i.n	a10, a10, 20
42038cb3:	ffe446        	j	42038c48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038cb6:	00          	.byte	00
42038cb7:	9a9200        	trunc.s	a9, f2, 0
42038cba:	04          	.byte	0x4
42038cbb:	3119a6        	blti	a9, 1, 42038cf0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038cbe:	899a      	add.n	a8, a9, a9
42038cc0:	60a0c2        	movi	a12, 96
42038cc3:	0f2b87        	blt	a11, a8, 42038cd6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038cc6:	261926        	beqi	a9, 1, 42038cf0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038cc9:	c90b      	addi.n	a12, a9, -1
42038ccb:	23cc80        	sext	a12, a12, 15
42038cce:	11cce0        	slli	a12, a12, 2
42038cd1:	0c2cb7        	blt	a12, a11, 42038ce1 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x165>
42038cd4:	0c4c      	movi.n	a12, 64
42038cd6:	4ab8      	l32i.n	a11, a10, 16
42038cd8:	5aa8      	l32i.n	a10, a10, 20
42038cda:	4c2aa5        	call8	42084f84 <roo_windows::internal::AverageColors(roo_display::Color, roo_display::Color, unsigned char)>
42038cdd:	ffd9c6        	j	42038c48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>
42038ce0:	00          	.byte	00
42038ce1:	0b2926        	beqi	a9, 2, 42038cf0 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x174>
42038ce4:	889a      	add.n	a8, a8, a9
42038ce6:	888a      	add.n	a8, a8, a8
42038ce8:	f8c882        	addi	a8, a8, -8
42038ceb:	0c2c      	movi.n	a12, 32
42038ced:	e52b87        	blt	a11, a8, 42038cd6 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0x15a>
42038cf0:	4aa8      	l32i.n	a10, a10, 16
42038cf2:	ffd486        	j	42038c48 <roo_windows::Decoration::readWithContent(short, short, roo_display::Color, roo_display::Color const*) const+0xcc>


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42038d38 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const>:
42038d38:	00c136        	entry	a1, 96
42038d3b:	238680        	sext	a8, a6, 15
42038d3e:	1189      	s32i.n	a8, a1, 4
42038d40:	142282        	l32i	a8, a2, 80
42038d43:	07fd      	mov.n	a15, a7
42038d45:	027d      	mov.n	a7, a2
42038d47:	233380        	sext	a3, a3, 15
42038d4a:	234480        	sext	a4, a4, 15
42038d4d:	235580        	sext	a5, a5, 15
42038d50:	204856        	bnez	a8, 42038f58 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x220>
42038d53:	0288      	l32i.n	a8, a2, 0
42038d55:	11e8      	l32i.n	a14, a1, 4
42038d57:	b888      	l32i.n	a8, a8, 44
42038d59:	02ad      	mov.n	a10, a2
42038d5b:	a1f9      	s32i.n	a15, a1, 40
42038d5d:	05dd      	mov.n	a13, a5
42038d5f:	04cd      	mov.n	a12, a4
42038d61:	03bd      	mov.n	a11, a3
42038d63:	0008e0        	callx8	a8
42038d66:	0a2d      	mov.n	a2, a10
42038d68:	098a56        	bnez	a10, 42038e04 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xcc>
42038d6b:	1a9792        	l16si	a9, a7, 52
42038d6e:	249782        	l16si	a8, a7, 72
42038d71:	ec0c      	movi.n	a12, 14
42038d73:	538890        	max	a8, a8, a9
42038d76:	98fb      	addi.n	a9, a8, 15
42038d78:	1ec882        	addi	a8, a8, 30
42038d7b:	b38990        	movgez	a8, a9, a9
42038d7e:	21b480        	srai	a11, a8, 4
42038d81:	210782        	l8ui	a8, a7, 33
42038d84:	1907a2        	l8ui	a10, a7, 25
42038d87:	62cc80        	saltu	a12, a12, a8
42038d8a:	180782        	l8ui	a8, a7, 24
42038d8d:	200792        	l8ui	a9, a7, 32
42038d90:	7388a0        	maxu	a8, a8, a10
42038d93:	1a07a2        	l8ui	a10, a7, 26
42038d96:	cc0b      	addi.n	a12, a12, -1
42038d98:	7388a0        	maxu	a8, a8, a10
42038d9b:	1b07a2        	l8ui	a10, a7, 27
42038d9e:	60c0c0        	neg	a12, a12
42038da1:	73a8a0        	maxu	a10, a8, a10
42038da4:	89ca      	add.n	a8, a9, a12
42038da6:	99ba      	add.n	a9, a9, a11
42038da8:	538a80        	max	a8, a10, a8
42038dab:	029762        	l16si	a6, a7, 4
42038dae:	0497a2        	l16si	a10, a7, 8
42038db1:	99ca      	add.n	a9, a9, a12
42038db3:	539980        	max	a9, a9, a8
42038db6:	669a      	add.n	a6, a6, a9
42038db8:	c09a90        	sub	a9, a10, a9
42038dbb:	439950        	min	a9, a9, a5
42038dbe:	3199      	s32i.n	a9, a1, 12
42038dc0:	039792        	l16si	a9, a7, 6
42038dc3:	536630        	max	a6, a6, a3
42038dc6:	998a      	add.n	a9, a9, a8
42038dc8:	99ba      	add.n	a9, a9, a11
42038dca:	7199      	s32i.n	a9, a1, 28
42038dcc:	059792        	l16si	a9, a7, 10
42038dcf:	a1f8      	l32i.n	a15, a1, 40
42038dd1:	c08980        	sub	a8, a9, a8
42038dd4:	2189      	s32i.n	a8, a1, 8
42038dd6:	c08530        	sub	a8, a5, a3
42038dd9:	881b      	addi.n	a8, a8, 1
42038ddb:	4189      	s32i.n	a8, a1, 16
42038ddd:	3188      	l32i.n	a8, a1, 12
42038ddf:	881b      	addi.n	a8, a8, 1
42038de1:	5189      	s32i.n	a8, a1, 20
42038de3:	c08860        	sub	a8, a8, a6
42038de6:	1188e0        	slli	a8, a8, 2
42038de9:	8189      	s32i.n	a8, a1, 32
42038deb:	4188      	l32i.n	a8, a1, 16
42038ded:	1128e0        	slli	a2, a8, 2
42038df0:	c08630        	sub	a8, a6, a3
42038df3:	1188e0        	slli	a8, a8, 2
42038df6:	9189      	s32i.n	a8, a1, 36
42038df8:	238680        	sext	a8, a6, 15
42038dfb:	6189      	s32i.n	a8, a1, 24
42038dfd:	1188      	l32i.n	a8, a1, 4
42038dff:	03a847        	bge	a8, a4, 42038e06 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xce>
42038e02:	020c      	movi.n	a2, 0
42038e04:	f01d      	retw.n
42038e06:	31a8      	l32i.n	a10, a1, 12
42038e08:	038d      	mov.n	a8, a3
42038e0a:	0f9d      	mov.n	a9, a15
42038e0c:	2b2a67        	blt	a10, a6, 42038e3b <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x103>
42038e0f:	0f8d      	mov.n	a8, a15
42038e11:	03bd      	mov.n	a11, a3
42038e13:	23c480        	sext	a12, a4, 15
42038e16:	02ab67        	bge	a11, a6, 42038e1c <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xe4>
42038e19:	0024c6        	j	42038eb0 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x178>
42038e1c:	9188      	l32i.n	a8, a1, 36
42038e1e:	61b8      	l32i.n	a11, a1, 24
42038e20:	9f8a      	add.n	a9, a15, a8
42038e22:	07ad      	mov.n	a10, a7
42038e24:	b1f9      	s32i.n	a15, a1, 44
42038e26:	a199      	s32i.n	a9, a1, 40
42038e28:	ffece5        	call8	42038cf8 <roo_windows::Decoration::read(short, short) const>
42038e2b:	8188      	l32i.n	a8, a1, 32
42038e2d:	a198      	l32i.n	a9, a1, 40
42038e2f:	b1f8      	l32i.n	a15, a1, 44
42038e31:	898a      	add.n	a8, a9, a8
42038e33:	021897        	beq	a8, a9, 42038e39 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x101>
42038e36:	0025c6        	j	42038ed1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x199>
42038e39:	5188      	l32i.n	a8, a1, 20
42038e3b:	09dd      	mov.n	a13, a9
42038e3d:	08bd      	mov.n	a11, a8
42038e3f:	23c480        	sext	a12, a4, 15
42038e42:	0225b7        	blt	a5, a11, 42038e48 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x110>
42038e45:	0023c6        	j	42038ed8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1a0>
42038e48:	b51b      	addi.n	a11, a5, 1
42038e4a:	72ab80        	salt	a10, a11, a8
42038e4d:	aa0b      	addi.n	a10, a10, -1
42038e4f:	60a0a0        	neg	a10, a10
42038e52:	74a0a0        	extui	a10, a10, 0, 8
42038e55:	0c0c      	movi.n	a12, 0
42038e57:	4a8c      	beqz.n	a10, 42038e5f <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x127>
42038e59:	c0bb80        	sub	a11, a11, a8
42038e5c:	11cbe0        	slli	a12, a11, 2
42038e5f:	71a8      	l32i.n	a10, a1, 28
42038e61:	99ca      	add.n	a9, a9, a12
42038e63:	72a4a0        	salt	a10, a4, a10
42038e66:	aa0b      	addi.n	a10, a10, -1
42038e68:	60a0a0        	neg	a10, a10
42038e6b:	74a0a0        	extui	a10, a10, 0, 8
42038e6e:	098d      	mov.n	a8, a9
42038e70:	4abc      	beqz.n	a10, 42038ea8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038e72:	21a8      	l32i.n	a10, a1, 8
42038e74:	72aa40        	salt	a10, a10, a4
42038e77:	aa0b      	addi.n	a10, a10, -1
42038e79:	60a0a0        	neg	a10, a10
42038e7c:	74a0a0        	extui	a10, a10, 0, 8
42038e7f:	5aac      	beqz.n	a10, 42038ea8 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x170>
42038e81:	11a8      	l32i.n	a10, a1, 4
42038e83:	21b8      	l32i.n	a11, a1, 8
42038e85:	04ed      	mov.n	a14, a4
42038e87:	43dab0        	min	a13, a10, a11
42038e8a:	742ed7        	blt	a14, a13, 42038f02 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1ca>
42038e8d:	728d40        	salt	a8, a13, a4
42038e90:	880b      	addi.n	a8, a8, -1
42038e92:	608080        	neg	a8, a8
42038e95:	748080        	extui	a8, a8, 0, 8
42038e98:	00a0a2        	movi	a10, 0
42038e9b:	002816        	beqz	a8, 42038ea1 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x169>
42038e9e:	c0ad40        	sub	a10, a13, a4
42038ea1:	828a20        	mull	a8, a10, a2
42038ea4:	4a4a      	add.n	a4, a10, a4
42038ea6:	889a      	add.n	a8, a8, a9
42038ea8:	441b      	addi.n	a4, a4, 1
42038eaa:	08fd      	mov.n	a15, a8
42038eac:	ffd346        	j	42038dfd <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xc5>
42038eaf:	a77000        	ee.stf.64.xp	f7, f10, a0, a0
42038eb2:	61f220        	xsr.ccompare2	a2
42038eb5:	0d          	.byte	0xd
42038eb6:	0c6182        	s32i	a8, a1, 48
42038eb9:	b1c9      	s32i.n	a12, a1, 44
42038ebb:	a1b9      	s32i.n	a11, a1, 40
42038ebd:	ffe3a5        	call8	42038cf8 <roo_windows::Decoration::read(short, short) const>
42038ec0:	c188      	l32i.n	a8, a1, 48
42038ec2:	a1b8      	l32i.n	a11, a1, 40
42038ec4:	08a9      	s32i.n	a10, a8, 0
42038ec6:	d1f8      	l32i.n	a15, a1, 52
42038ec8:	b1c8      	l32i.n	a12, a1, 44
42038eca:	bb1b      	addi.n	a11, a11, 1
42038ecc:	884b      	addi.n	a8, a8, 4
42038ece:	ffd106        	j	42038e16 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xde>
42038ed1:	09a9      	s32i.n	a10, a9, 0
42038ed3:	994b      	addi.n	a9, a9, 4
42038ed5:	ffd686        	j	42038e33 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xfb>
42038ed8:	20a770        	or	a10, a7, a7
42038edb:	0f61f2        	s32i	a15, a1, 60
42038ede:	0e61d2        	s32i	a13, a1, 56
42038ee1:	d189      	s32i.n	a8, a1, 52
42038ee3:	c199      	s32i.n	a9, a1, 48
42038ee5:	b1c9      	s32i.n	a12, a1, 44
42038ee7:	a1b9      	s32i.n	a11, a1, 40
42038ee9:	ffe0e5        	call8	42038cf8 <roo_windows::Decoration::read(short, short) const>
42038eec:	e1d8      	l32i.n	a13, a1, 56
42038eee:	a1b8      	l32i.n	a11, a1, 40
42038ef0:	0da9      	s32i.n	a10, a13, 0
42038ef2:	f1f8      	l32i.n	a15, a1, 60
42038ef4:	d188      	l32i.n	a8, a1, 52
42038ef6:	c198      	l32i.n	a9, a1, 48
42038ef8:	b1c8      	l32i.n	a12, a1, 44
42038efa:	bb1b      	addi.n	a11, a11, 1
42038efc:	dd4b      	addi.n	a13, a13, 4
42038efe:	ffd006        	j	42038e42 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x10a>
42038f01:	41a800        	srli	a10, a0, 8
42038f04:	201aa6        	blti	a10, 1, 42038f28 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038f07:	254226        	beqi	a2, 4, 42038f30 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f8>
42038f0a:	0fbd      	mov.n	a11, a15
42038f0c:	08ad      	mov.n	a10, a8
42038f0e:	02cd      	mov.n	a12, a2
42038f10:	d199      	s32i.n	a9, a1, 52
42038f12:	c1e9      	s32i.n	a14, a1, 48
42038f14:	b1d9      	s32i.n	a13, a1, 44
42038f16:	a1f9      	s32i.n	a15, a1, 40
42038f18:	1e7481        	l32r	a8, 420008e8 <_stext+0x8c8> (40001200 <memmove>)
42038f1b:	0008e0        	callx8	a8
42038f1e:	a1f8      	l32i.n	a15, a1, 40
42038f20:	b1d8      	l32i.n	a13, a1, 44
42038f22:	c1e8      	l32i.n	a14, a1, 48
42038f24:	d198      	l32i.n	a9, a1, 52
42038f26:	0a8d      	mov.n	a8, a10
42038f28:	882a      	add.n	a8, a8, a2
42038f2a:	ee1b      	addi.n	a14, a14, 1
42038f2c:	ffd686        	j	42038e8a <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x152>
42038f2f:	00          	.byte	00
42038f30:	0fa8      	l32i.n	a10, a15, 0
42038f32:	08a9      	s32i.n	a10, a8, 0
42038f34:	fffc06        	j	42038f28 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x1f0>
42038f37:	600000        	neg	a0, a0
42038f3a:	4020b6        	bltui	a0, 2, 42038f7e <roo_windows::internal::ShadowDistanceSqrt(unsigned long)+0x16>
42038f3d:	c4          	.byte	0xc4
42038f3e:	a77020        	ee.stf.64.xp	f7, f10, a2, a0
42038f41:	db6520        	movt.s	f6, f5, b2
42038f44:	a9661bff 	ee.fft.cmul.s16.st.xp	q4, q6, q6, a15, a11, 2, 2, 0
42038f48:	02          	.byte	0x2
42038f49:	236680        	sext	a6, a6, 15
42038f4c:	224b      	addi.n	a2, a2, 4
42038f4e:	02fd      	mov.n	a15, a2
42038f50:	e5a567        	bge	a5, a6, 42038f39 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x201>
42038f53:	441b      	addi.n	a4, a4, 1
42038f55:	234480        	sext	a4, a4, 15
42038f58:	1188      	l32i.n	a8, a1, 4
42038f5a:	02a847        	bge	a8, a4, 42038f60 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x228>
42038f5d:	ffa846        	j	42038e02 <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0xca>
42038f60:	0f2d      	mov.n	a2, a15
42038f62:	036d      	mov.n	a6, a3
42038f64:	fff986        	j	42038f4e <roo_windows::Decoration::readColorRect(short, short, short, short, roo_display::Color*) const+0x216>


/tmp/roo-shadow-review/codegen/sqrt2/firmware.elf:     file format elf32-xtensa-le


Disassembly of section .flash.text:

42039188 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)>:
42039188:	00a136        	entry	a1, 80
4203918b:	238480        	sext	a8, a4, 15
4203918e:	027d      	mov.n	a7, a2
42039190:	a189      	s32i.n	a8, a1, 40
42039192:	0f0222        	l8ui	a2, a2, 15
42039195:	238580        	sext	a8, a5, 15
42039198:	b189      	s32i.n	a8, a1, 44
4203919a:	243216        	beqz	a2, 420393e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
4203919d:	009782        	l16si	a8, a7, 0
420391a0:	0697f2        	l16si	a15, a7, 12
420391a3:	3189      	s32i.n	a8, a1, 12
420391a5:	3198      	l32i.n	a9, a1, 12
420391a7:	a188      	l32i.n	a8, a1, 40
420391a9:	c08890        	sub	a8, a8, a9
420391ac:	11c8c0        	slli	a12, a8, 4
420391af:	4189      	s32i.n	a8, a1, 16
420391b1:	019782        	l16si	a8, a7, 2
420391b4:	cc8b      	addi.n	a12, a12, 8
420391b6:	5189      	s32i.n	a8, a1, 20
420391b8:	5198      	l32i.n	a9, a1, 20
420391ba:	b188      	l32i.n	a8, a1, 44
420391bc:	c08890        	sub	a8, a8, a9
420391bf:	6189      	s32i.n	a8, a1, 24
420391c1:	069392        	l16si	a9, a3, 12
420391c4:	1188c0        	slli	a8, a8, 4
420391c7:	888b      	addi.n	a8, a8, 8
420391c9:	c0f8f0        	sub	a15, a8, a15
420391cc:	c08890        	sub	a8, a8, a9
420391cf:	9cca      	add.n	a9, a12, a12
420391d1:	7199      	s32i.n	a9, a1, 28
420391d3:	029792        	l16si	a9, a7, 4
420391d6:	d88a      	add.n	a13, a8, a8
420391d8:	8199      	s32i.n	a9, a1, 32
420391da:	1199c0        	slli	a9, a9, 4
420391dd:	2199      	s32i.n	a9, a1, 8
420391df:	039792        	l16si	a9, a7, 6
420391e2:	2168      	l32i.n	a6, a1, 8
420391e4:	11b9c0        	slli	a11, a9, 4
420391e7:	effa      	add.n	a14, a15, a15
420391e9:	c04b80        	sub	a4, a11, a8
420391ec:	9199      	s32i.n	a9, a1, 36
420391ee:	72adb0        	salt	a10, a13, a11
420391f1:	729eb0        	salt	a9, a14, a11
420391f4:	c056c0        	sub	a5, a6, a12
420391f7:	990b      	addi.n	a9, a9, -1
420391f9:	aa0b      	addi.n	a10, a10, -1
420391fb:	434480        	min	a4, a4, a8
420391fe:	7188      	l32i.n	a8, a1, 28
42039200:	609090        	neg	a9, a9
42039203:	60a0a0        	neg	a10, a10
42039206:	4355c0        	min	a5, a5, a12
42039209:	c0cbf0        	sub	a12, a11, a15
4203920c:	749090        	extui	a9, a9, 0, 8
4203920f:	74a0a0        	extui	a10, a10, 0, 8
42039212:	43ccf0        	min	a12, a12, a15
42039215:	732867        	blt	a8, a6, 4203928c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x104>
42039218:	682eb7        	blt	a14, a11, 42039284 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xfc>
4203921b:	0a0782        	l8ui	a8, a7, 10
4203921e:	7319a7        	beq	a9, a10, 42039295 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x10d>
42039221:	7f2db7        	blt	a13, a11, 420392a4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x11c>
42039224:	0a0762        	l8ui	a6, a7, 10
42039227:	23d880        	sext	a13, a8, 15
4203922a:	11ddc0        	slli	a13, a13, 4
4203922d:	02acd7        	bge	a12, a13, 42039233 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xab>
42039230:	002206        	j	420392bc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
42039233:	23f680        	sext	a15, a6, 15
42039236:	11ffc0        	slli	a15, a15, 4
42039239:	7f24f7        	blt	a4, a15, 420392bc <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x134>
4203923c:	43fc40        	min	a15, a12, a4
4203923f:	02af57        	bge	a15, a5, 42039245 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xbd>
42039242:	006f86        	j	42039404 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x27c>
42039245:	a198      	l32i.n	a9, a1, 40
42039247:	3188      	l32i.n	a8, a1, 12
42039249:	21a8      	l32i.n	a10, a1, 8
4203924b:	c08890        	sub	a8, a8, a9
4203924e:	7198      	l32i.n	a9, a1, 28
42039250:	0829a7        	blt	a9, a10, 4203925c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xd4>
42039253:	4188      	l32i.n	a8, a1, 16
42039255:	8198      	l32i.n	a9, a1, 32
42039257:	c08890        	sub	a8, a8, a9
4203925a:	881b      	addi.n	a8, a8, 1
4203925c:	096c      	movi.n	a9, -32
4203925e:	02a897        	bge	a8, a9, 42039264 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdc>
42039261:	0064c6        	j	420393f8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
42039264:	092c      	movi.n	a9, 32
42039266:	02a987        	bge	a9, a8, 4203926c <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xe4>
42039269:	006446        	j	420393fe <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
4203926c:	220b      	addi.n	a2, a2, -1
4203926e:	902220        	addx2	a2, a2, a2
42039271:	1192a0        	slli	a9, a2, 6
42039274:	229a      	add.n	a2, a2, a9
42039276:	1da091        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
42039279:	898a      	add.n	a8, a9, a8
4203927b:	882a      	add.n	a8, a8, a2
4203927d:	200822        	l8ui	a2, a8, 32
42039280:	005746        	j	420393e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
42039283:	078200        	ee.stf.64.xp	f8, f0, a0, a2
42039286:	c609      	s32i.n	a0, a6, 48
42039288:	e4          	.byte	0xe4
42039289:	b70000ff 	ee.vmulas.s16.qacc.ld.xp.qup	q6, a15, a0, q4, q0, q0, q0
4203928d:	07820c2e 	ee.vmulas.s16.accx.ld.ip.qup	q6, a2, 0x1c0, q0, q1, q0, q2
42039291:	a70b      	addi.n	a10, a7, -1
42039293:	1699      	s32i.n	a9, a6, 4
42039295:	086d      	mov.n	a6, a8
42039297:	ffe306        	j	42039227 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
4203929a:	820000        	mull	a0, a0, a0
4203929d:	c60807        	bnone	a8, a0, 42039267 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xdf>
420392a0:	fffb      	addi.n	a15, a15, 15
420392a2:	620000        	saltu	a0, a0, a0
420392a5:	060907        	bnone	a9, a0, 420392af <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x127>
420392a8:	0000ffdf 	ee.vmulas.s16.accx.ld.ip.qup	q0, a13, 240, q7, q6, q0, q0
420392ac:	052db7        	blt	a13, a11, 420392b5 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x12d>
420392af:	0b0762        	l8ui	a6, a7, 11
420392b2:	ffdc46        	j	42039227 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
420392b5:	080762        	l8ui	a6, a7, 8
420392b8:	ffdac6        	j	42039227 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x9f>
420392bb:	00          	.byte	00
420392bc:	5199a7        	bne	a9, a10, 42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
420392bf:	538680        	max	a8, a6, a8
420392c2:	238880        	sext	a8, a8, 15
420392c5:	1188c0        	slli	a8, a8, 4
420392c8:	452587        	blt	a5, a8, 42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
420392cb:	538c40        	max	a8, a12, a4
420392ce:	3f2587        	blt	a5, a8, 42039311 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x189>
420392d1:	9198      	l32i.n	a9, a1, 36
420392d3:	6188      	l32i.n	a8, a1, 24
420392d5:	c08890        	sub	a8, a8, a9
420392d8:	881b      	addi.n	a8, a8, 1
420392da:	290c      	movi.n	a9, 2
420392dc:	08aeb7        	bge	a14, a11, 420392e8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x160>
420392df:	b198      	l32i.n	a9, a1, 44
420392e1:	5188      	l32i.n	a8, a1, 20
420392e3:	c08890        	sub	a8, a8, a9
420392e6:	190c      	movi.n	a9, 1
420392e8:	0a6c      	movi.n	a10, -32
420392ea:	02a8a7        	bge	a8, a10, 420392f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x168>
420392ed:	0041c6        	j	420393f8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x270>
420392f0:	0a2c      	movi.n	a10, 32
420392f2:	02aa87        	bge	a10, a8, 420392f8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x170>
420392f5:	004146        	j	420393fe <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x276>
420392f8:	220b      	addi.n	a2, a2, -1
420392fa:	902220        	addx2	a2, a2, a2
420392fd:	229a      	add.n	a2, a2, a9
420392ff:	1d7e91        	l32r	a9, 420008f8 <_stext+0x8d8> (3c0b1050 <roo_windows::internal::(anonymous namespace)::kSideGradients>)
42039302:	11a2a0        	slli	a10, a2, 6
42039305:	aa2a      	add.n	a10, a10, a2
42039307:	898a      	add.n	a8, a9, a8
42039309:	88aa      	add.n	a8, a8, a10
4203930b:	ffdb86        	j	4203927d <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0xf5>
4203930e:	000000        	ill
42039311:	091782        	l16ui	a8, a7, 18
42039314:	4395c0        	min	a9, a5, a12
42039317:	608080        	neg	a8, a8
4203931a:	00a022        	movi	a2, 0
4203931d:	4e2987        	blt	a9, a8, 4203936f <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1e7>
42039320:	c0ad50        	sub	a10, a13, a5
42039323:	090c      	movi.n	a9, 0
42039325:	c08dc0        	sub	a8, a13, a12
42039328:	0229a7        	blt	a9, a10, 4203932e <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1a6>
4203932b:	002d46        	j	420393e4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
4203932e:	022987        	blt	a9, a8, 42039334 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1ac>
42039331:	002bc6        	j	420393e4 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x25c>
42039334:	828880        	mull	a8, a8, a8
42039337:	82aaa0        	mull	a10, a10, a10
4203933a:	21d9      	s32i.n	a13, a1, 8
4203933c:	80aa80        	add	a10, a10, a8
4203933f:	ffc2a5        	call8	42038f68 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
42039342:	0221d2        	l32i	a13, a1, 8
42039345:	0e0792        	l8ui	a9, a7, 14
42039348:	c08ad0        	sub	a8, a10, a13
4203934b:	c08890        	sub	a8, a8, a9
4203934e:	081792        	l16ui	a9, a7, 16
42039351:	828890        	mull	a8, a8, a9
42039354:	1d6b91        	l32r	a9, 42000900 <_stext+0x8e0> (80000 <UserFrameTotalSize+0x7ff00>)
42039357:	889a      	add.n	a8, a8, a9
42039359:	218c80        	srai	a8, a8, 12
4203935c:	ffa092        	movi	a9, 255
4203935f:	438890        	min	a8, a8, a9
42039362:	090c      	movi.n	a9, 0
42039364:	538890        	max	a8, a8, a9
42039367:	1d6591        	l32r	a9, 420008fc <_stext+0x8dc> (3c0b151f <roo_windows::internal::(anonymous namespace)::kKey>)
4203936a:	898a      	add.n	a8, a9, a8
4203936c:	000822        	l8ui	a2, a8, 0
4203936f:	091392        	l16ui	a9, a3, 18
42039372:	43a540        	min	a10, a5, a4
42039375:	609090        	neg	a9, a9
42039378:	00a082        	movi	a8, 0
4203937b:	492a97        	blt	a10, a9, 420393c8 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x240>
4203937e:	236680        	sext	a6, a6, 15
42039381:	1166c0        	slli	a6, a6, 4
42039384:	c09640        	sub	a9, a6, a4
42039387:	c0a650        	sub	a10, a6, a5
4203938a:	62a897        	bge	a8, a9, 420393f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
4203938d:	5fa8a7        	bge	a8, a10, 420393f0 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x268>
42039390:	829990        	mull	a9, a9, a9
42039393:	82aaa0        	mull	a10, a10, a10
42039396:	80a9a0        	add	a10, a9, a10
42039399:	ffbce5        	call8	42038f68 <roo_windows::internal::ShadowDistanceSqrt(unsigned long)>
4203939c:	0e0392        	l8ui	a9, a3, 14
4203939f:	c08a60        	sub	a8, a10, a6
420393a2:	c08890        	sub	a8, a8, a9
420393a5:	081392        	l16ui	a9, a3, 16
420393a8:	828890        	mull	a8, a8, a9
420393ab:	1d5591        	l32r	a9, 42000900 <_stext+0x8e0> (80000 <UserFrameTotalSize+0x7ff00>)
420393ae:	808890        	add	a8, a8, a9
420393b1:	218c80        	srai	a8, a8, 12
420393b4:	ffa092        	movi	a9, 255
420393b7:	438890        	min	a8, a8, a9
420393ba:	090c      	movi.n	a9, 0
420393bc:	538890        	max	a8, a8, a9
420393bf:	1d5191        	l32r	a9, 42000904 <_stext+0x8e4> (3c0b141f <roo_windows::internal::(anonymous namespace)::kAmbient>)
420393c2:	808980        	add	a8, a9, a8
420393c5:	000882        	l8ui	a8, a8, 0
420393c8:	d19280        	mul16s	a9, a2, a8
420393cb:	1d46a1        	l32r	a10, 420008e4 <_stext+0x8c4> (80808081 <_rtc_reserved_end+0x20708081>)
420393ce:	7fc992        	addi	a9, a9, 127
420393d1:	b2a9a0        	mulsh	a10, a9, a10
420393d4:	282a      	add.n	a2, a8, a2
420393d6:	99aa      	add.n	a9, a9, a10
420393d8:	219790        	srai	a9, a9, 7
420393db:	c02290        	sub	a2, a2, a9
420393de:	742020        	extui	a2, a2, 0, 8
420393e1:	f01d      	retw.n
420393e3:	00          	.byte	00
420393e4:	022c57        	blt	a12, a5, 420393ea <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x262>
420393e7:	ffd686        	j	42039345 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
420393ea:	08ad      	mov.n	a10, a8
420393ec:	ffd546        	j	42039345 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x1bd>
420393ef:	a45700        	extui	a5, a0, 7, 11
420393f2:	ada8      	l32i.n	a10, a13, 40
420393f4:	c609      	s32i.n	a0, a6, 48
420393f6:	ffe8      	l32i.n	a14, a15, 60
420393f8:	68a022        	movi	a2, 104
420393fb:	fff886        	j	420393e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
420393fe:	020c      	movi.n	a2, 0
42039400:	fff746        	j	420393e1 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x259>
42039403:	00          	.byte	00
42039404:	0219a7        	beq	a9, a10, 4203940a <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x282>
42039407:	ffc546        	j	42039320 <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x198>
4203940a:	ffac46        	j	420392bf <roo_windows::internal::ReadLookupShadow(roo_windows::internal::LookupShadowSpec const&, roo_windows::internal::LookupShadowSpec const&, short, short)+0x137>
