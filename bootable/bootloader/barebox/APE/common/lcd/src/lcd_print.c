/*	lcd_print.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "com_type.h"
#include "string.h"
#include "lcd_common.h"
#include "lcd_api.h"
#include "timer_drv.h"
#include "lcd.h"
#include "lcd_font_en.h"
#include "lcd_font_num.h"
#include "lcd_font_sign.h"




// =============================================
//	(space)	!	"	#	$	%	&	'
//	(		)	*	+	,	-	.	/
//	0		1	2	3	4	5	6	7
//	8		9	:	;	<	=	>	?
//	@		A	B	C	D	E	F	G
//	H		I	J	K	L	M	N	O
//	P		Q	R	S	T	U	V	W
//	X		Y	Z	[	\	]	^	_
//	`		a	b	c	d	e	f	g
//	h		i	j	k	l	m	n	o
//	p		q	r	s	t	u	v	w
//	x		y	z	{	|	}	~	(other)
// =============================================


static const uchar* LCD_FONT[LCD_FONT_MAX] = {
  en_space,              sign_exclamation,  sign_w_quotation,  sign_hash,       sign_dollars,   sign_percent,    sign_ampersand,  sign_s_quotation,
  sign_paren_L,          sign_paren_R,      sign_asterisk,     sign_plus,       sign_comma,     sign_minus,      sign_dot,        sign_slash,
  number_0,              number_1,          number_2,          number_3,        number_4,       number_5,        number_6,        number_7,
  number_8,              number_9,          sign_colon,        sign_semicolon,  sign_less,      sign_equal,      sign_grater,     sign_question,
  sign_at,               en_A,              en_B,              en_C,            en_D,           en_E,            en_F,            en_G,
  en_H,                  en_I,              en_J,              en_K,            en_L,           en_M,            en_N,            en_O,
  en_P,                  en_Q,              en_R,              en_S,            en_T,           en_U,            en_V,            en_W,
  en_X,                  en_Y,              en_Z,              sign_bracket_L,  sign_yen,       sign_bracket_R,  sign_caret,      sign_lowline,
  sign_back_apostrophe,  en_a,              en_b,              en_c,            en_d,           en_e,            en_f,            en_g,
  en_h,                  en_i,              en_j,              en_k,            en_l,           en_m,            en_n,            en_o,
  en_p,                  en_q,              en_r,              en_s,            en_t,           en_u,            en_v,            en_w,
  en_x,                  en_y,              en_z,              sign_brace_L,    sign_vertical,  sign_brace_R,    sign_tilde,      font_other,
};


/**
 * LCD_Print - LCD print processing
 * @return   - LCD_SUCCESS           : Success
 *             LCD_ERR_NOT_INIT      : Not Initialized
 *             LCD_ERR_PARAM         : Parameter Error
 *             LCD_ERR_STRING_LENGTH : String Length error
 */
RC LCD_Print(ulong set_x, ulong set_y, const char* pStr)
{
	RC ret = LCD_SUCCESS;
	uchar idx = 0;
	ulong i = 0;
	ulong len = 0;
	ulong point_x = 0;
	
	
	/* Check Initialize */
	if(gLcdInit != TRUE)
	{
		return LCD_ERR_NOT_INIT;
	}
	
	/* Get String Length */
	len = strlen(pStr);
	
	/* Check String Length */
	if(len > LCD_STR_LEN_MAX)
	{
		return LCD_ERR_STRING_LENGTH;
	}
	
	/* Print String */
	for(i = 0; i < len; i++)
	{
		/* Calculates a Font index */
		idx = (uchar)pStr[i] - LCD_FONT_OFFSET;
		
		/* Check a Font index */
		if( idx > LCD_FONT_INDEX_MAX )
		{
			/* NG Font */
			idx = LCD_FONT_OTHER;
		}
		
		/* Calculates a Font point X */
		point_x = (LCD_FONT_WIDTH * i) + set_x;
		
		/* Print Font */
		ret = LCD_PrintFont(point_x, set_y, LCD_FONT_WIDTH, LCD_FONT_HIGH, LCD_FONT[idx]);
		
		/* Check a LCD_PrintDot return value */
		if(ret != LCD_SUCCESS)
		{
			break;
		}
	}
	
	
	return ret;
}



