/**
 * @file Bool.h
 * @brief A bool type for old compilers that lack one (IASLIB_DEFINE_BOOL__).
 *
 * Boolean
 *
 *	On some old compilers (DEC springs to mind) the new "bool" value
 * is not defined. This file defines a bool type as an integer and
 * sets up some true and false values.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/20/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BOOL_H__
#define IASLIB_BOOL_H__

#ifdef IASLIB_DEFINE_BOOL__

/// bool as an integer, for compilers without the built-in type.
typedef int bool;

#define true 1      ///< The true value.
#define false 0     ///< The false value.
#define TRUE 1      ///< The true value, upper case.
#define FALSE 0     ///< The false value, upper case.

#endif

#endif
