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
// Class: PyScript
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Mar 13 15:39:31 2006
//

#ifndef _PYSCRIPT_H_
#define _PYSCRIPT_H_


#include "Python.h"

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "pythread.h"
#include "PySig.h"

#include <list>

class PyScript;

typedef struct {
    PyObject_HEAD
    PyScript *c;
} PluginObject;

/**
 * Handles data for each script stores all the signal classes and the python 
 * interpreter to use
 */
class PyScript : public WLSignalInstance
{
	public:
		PyScript(WLSignal *wls, const std::string &filename);
		PyScript();
		 ~PyScript();
	
		static PyObject *HookSignal(PluginObject *self, PyObject *args);
		static PyObject *SendSignal(PluginObject *self, PyObject *args);
	protected:
		PyObject* module;																											/// Python version of the class
		PyThreadState *thread_state;
		std::list <PySig *> signal; 																					/// List of signals that the script uses
		FILE *fp;
};


#endif	//_PYSCRIPT_H_

