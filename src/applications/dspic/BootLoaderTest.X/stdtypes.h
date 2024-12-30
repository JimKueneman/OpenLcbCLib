/************************************************************************/
/*									*/
/*  stdtypes.h	--  Digilent Standard Type Declarations			*/
/*									*/
/************************************************************************/
/*  Author: Gene Apperson						*/
/*  Copyright 2005, Digilent Inc.					*/
/************************************************************************/
/*  File Description:							*/
/*									*/
/* This header file contains declarations for standard Digilent data	*/
/* types and constants for use with avr-gcc.				*/
/*									*/
/************************************************************************/
/*  Revision History:							*/
/*									*/
/*  01/04/2005(GeneA): created						*/
/*  11/03/2015(MichaelA): modified to support PIC16F1618 8-bit          */
/*      processor line                                                  */
/*  11/13/2015(MichaelA): added conditional definition of NULL          */
/*									*/
/************************************************************************/

#if !defined(_STDTYPES_INC)
#define _STDTYPES_INC

#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------ */
/*		General Type Declarations			*/
/* ------------------------------------------------------------ */

#if defined(DEAD)
#define	fFalse	0x00
#define	fTrue	0xFF

#define	BOOL	uint8_t
#endif

#define fFalse  false
#define fTrue   true
#define BOOL    bool

#define	BYTE	uint8_t
#define WORD	uint16_t
#define	DWORD	uint32_t
#define QWORD	uint64_t

#ifndef NULL
#define NULL    0
#endif

/* ------------------------------------------------------------ */
/*		Object Class Declarations			*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*		Variable Declarations				*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */

#endif

/************************************************************************/
