/*
 * The template collections
 *
 *      TArray          a growable array
 *      TSortedArray    an array kept in order, with binary search
 *      TSparseArray    an array where only some indexes have values
 *      TList           a doubly linked list
 *      TStack          last in, first out
 *      TQueue          first in, first out (a ring buffer)
 *      THash           a hash table, iterated in the order entries were added
 *      TCache          a least-recently-used cache, with optional expiry
 *
 *      They hold values of any copyable or movable type, grow as described
 * in TCommon.h, and work with range-based for. They are not thread-safe.
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TCOLLECTIONS_H__
#define IASLIB_TCOLLECTIONS_H__

#include "TArray.h"
#include "TSortedArray.h"
#include "TSparseArray.h"
#include "TList.h"
#include "TStack.h"
#include "TQueue.h"
#include "THash.h"
#include "TCache.h"

#endif // IASLIB_TCOLLECTIONS_H__
