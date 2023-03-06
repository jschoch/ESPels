/* 
 *				YASM - Yet Another State Machine library
 * 
 * Lightweight library for creating function pointer based state machines
 * 
 * Copyright 2016/2019 bricofoy bricofoy@free.fr
 * 
 * 
 * Library provide basic functions for managing state machine execution
 * and state running timings and counts, and it is intended to be very 
 * easy to use.
 * 
 * Please see examples and README for documentation.
 * 
 * 
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include "yasm.h"

YASM::YASM()
{
  //
  _stateInfos.isStopped = true;
  //set the state pointer to NULL just in case resume() is called before next()
  _pState=NULL;
}

void YASM::next(void (*pnextstate)(), bool reset/*=false*/)
{
  //first check if we need to do something
  if ((!reset) && (!_stateInfos.isStopped) && (pnextstate == _pState))
    return; //we do nothing more if we stay in the same running state
  
  //now we are asked for reset or restart or state change, so technically 
  //we act like changing the state in both cases, so we set the associated
  //flags as needed
  _stateInfos.needReset = true; 
  _stateInfos.isStopped = false;
  //and enventually change exec pointer to the next state
  _pState = pnextstate; 
}

bool YASM::run()
{
  //first check if the machine is currently stopped
  if(_stateInfos.isStopped) 
    return false; //and just return false if this is the case
  
  //now machine is running
  
  //if state change flag is true, we are in the first pass in the current state (either 
  //it is a real state change or a restart of the current state), so we set the
  //associated values as needed
  if(_stateInfos.needReset) {
    _stateInfos.needReset=false;
    _stateInfos.isFirstRun=true;
    _lastTime = _enterTime = millis();
    _runCount = 0;
  }
  
  //and run the current state
  _pState(); 
  
  //then update execution informations for next time
  _stateInfos.isFirstRun = false; 
  _runCount++; 
  
  //and eventually report machine was running this time
  return true; 
}

void YASM::resume(bool reset/*=false*/)
{
  //if pointer==NULL then machine is not initialised so we do nothing more
  if(_pState==NULL) return;
  
  //or just remove the stop flag
  _stateInfos.isStopped=false;
  //and if asked, reset the current state
  if(reset)
    _stateInfos.needReset=true;
}

bool YASM::elapsed(unsigned long durationms)
{
  if (YASM::timeOnState()>durationms) 
    return true;
  return false;
}

bool YASM::periodic(unsigned long durationms, bool first/*=true*/)
{
  unsigned long time = (millis()-_lastTime);
  if( time >= durationms ) {
    _lastTime += time;
    return true;
  }
  if(first) 
    return _stateInfos.isFirstRun;
  return false;
}




