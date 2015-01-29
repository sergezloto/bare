/*
 *  vt100.cpp
 *  Embedded
 *
 *  Created by Serge on 11/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "vt100.h"

const char* vt100::CRLF = "\n\r";
const char* vt100::UP = "\033[A";
const char* vt100::DOWN = "\033[B";
const char* vt100::RIGHT = "\033[C";
const char* vt100::LEFT = "\033[D";
const char* vt100::BOS = "\033[0;0H";
const char* vt100::BOL = "\r";

const char* vt100::CURON = "\033[?25h";
const char* vt100::CUROFF = "\033[?25l";


const char* vt100::CLREOL = "\033[K";
const char* vt100::CLRBOL = "\033[1K";
const char* vt100::CLRLIN = "\033[2K";

const char* vt100::CLREOS = "\033[J";
const char* vt100::CLRBOS = "\033[1J";
const char* vt100::CLRSCR = "\033[2J";

const char* vt100::BOLD = "\033[1m";
const char* vt100::UNDERSCORE = "\033[4m";
const char* vt100::BLINK = "\033[5m";
const char* vt100::REVERSE = "\033[7m";
const char* vt100::OFF = "\033[m";
