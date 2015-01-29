/*
 *  fourcc.h
 *  Embedded
 *
 *  Created by Serge on 17/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"

typedef uint32 fourcc;

inline fourcc FOURCC(const char* str) {
	return (fourcc) *((fourcc*)str);
}
