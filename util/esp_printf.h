/*
 *  esp_printf.h
 *  Embedded
 *
 *  Created by Serge on 05/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif
	
	typedef char* charptr;
	typedef void (*func_ptr)(char c);
	void esp_printf( const func_ptr f_ptr,  const charptr ctrlI, ...);
	
#if defined(__cplusplus)
}
#endif

