/*
 *  vt100.h
 *  Embedded
 *
 *  Created by Serge on 11/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

struct vt100 {
  static const char* CRLF;
  static const char* UP;
  static const char* DOWN;
  static const char* RIGHT;
  static const char* LEFT;
  static const char* BOS;
  static const char* BOL;

  static const char* CURON;
  static const char* CUROFF;

  static const char* CLREOL;
  static const char* CLRBOL;
  static const char* CLRLIN;
  
  static const char* CLREOS;
  static const char* CLRBOS;
  static const char* CLRSCR;
  
  static const char* BOLD;
  static const char* UNDERSCORE;
  static const char* BLINK;
  static const char* REVERSE;
  static const char* OFF;
  
private:
  vt100();
  vt100(const vt100&);
};
