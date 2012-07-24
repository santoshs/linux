/*
 * gpio_led_drv.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_GPIO_LED_DRV_
#define __H_GPIO_LED_DRV_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define U1G_LED1				(u1)(0)
#define U1G_LED2				(u1)(1)
#define U1G_LED3				(u1)(2)
#define U1G_LED4				(u1)(3)
#define U1G_LED5				(u1)(4)
#define U1G_LED6				(u1)(5)
#define U1G_LED7				(u1)(6)
#define U1G_LED8				(u1)(7)
#define U1G_LED_ALL				(u1)(8)

#define U1G_OK_LED				U1G_LED1
#define U1G_NG_LED				U1G_LED2

/***************************************************************/
/* PUBLIC TYPEDEFE ENUM                                        */
/***************************************************************/

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void vog_gpio_led_test( void );
extern void vog_led_on( u1 u1t_led_num );
extern void vog_led_off( u1 u1t_led_num );

#endif /* __H_GPIO_LED_DRV_ */
