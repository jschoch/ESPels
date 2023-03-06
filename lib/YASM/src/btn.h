/* 
				YASM - Yet Another State Machine library

Lightweight library for creating function pointer based state machines

Copyright 2016/2017 bricofoy bricofoy@free.fr


Library provide basic functions for managing state machine execution
and state running timings and counts, and it is intended to be very 
easy to use.

Please see examples and README for documentation.



This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef BTN_h
#define BTN_h
#include <Arduino.h>
#include "yasm.h"

//button possible states
#define BTN_OFF			0
//#define BTN_ON			1
#define BTN_CLICK		2
#define BTN_LONGCLICK	4
#define BTN_DOUBLECLICK	8

//delays (ms)
#define BTN_D_DEBOUNCE		5
#define BTN_D_LONGCLICK		700
#define BTN_D_DOUBLECLICK	100

class BTN{
	public:
		BTN(){_SM.next(BTN::btn_wait);};
		void update(bool);
		bool state(byte);
		byte state(void);
	private:
		YASM _SM;
		bool _input;
		byte _state=BTN_OFF;
        //static(global) pointer on BTN object, updated by the update() method wich is not 
        //static to the current used BTN object when calling it. 
        //So through it static member functions can still acces non-static variables 
        //corresponding to the right BTN object if more than one are in use.
		static BTN* _this;
		static void btn_wait();
		static void btn_debounce();
		static void btn_check();
		static void btn_checkdouble();
		static void btn_longclick();
		static void btn_doubleclick();
};
#endif
