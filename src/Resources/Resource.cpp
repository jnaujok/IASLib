/*
 * Resource Class
 *
 *      This class defines an abstract base class for resources. Resources
 * are an abstraction of storage that allows for access to many different
 * types of data. A typical use, for example, would be to store graphics and
 * sound files in a game, or required data for a neural network.
 *      The abstracted data can be in any form that can return a stream to
 * read, including files, memory, generated data, or remote data.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 04/11/2021
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2021, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Resource.h"
namespace IASLib {
    CResource::CResource() {}

    CResource::~CResource(void) {
    }

    IMPLEMENT_OBJECT(CResource, CObject);

}; // namespace IASLib

