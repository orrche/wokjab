/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: PySig
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Mar 17 23:22:50 2006
//

#ifndef _PYSIG_H_
#define _PYSIG_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "Python.h"
#include "pythread.h"

using namespace Woklib;

/** 
 * Handles each signal that is created from the python script to give it a 
 * specific class to call for each script
 */
class PySig : public WLSignalInstance
{
	public:
		PySig(WLSignal *wls, PyThreadState *thread_state, std::string sig, PyObject *func, int prio);
		 ~PySig();
	
		int Exec(WokXMLTag *tag);
	protected:
		PyObject *func; /// Python function to call on execution
		PyThreadState *thread_state; /// What python interpreter to use
	
};


#endif	//_PYSIG_H_

