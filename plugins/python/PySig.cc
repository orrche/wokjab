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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Mar 17 23:22:50 2006
//

#include "PySig.h"
#include "PyWXMLTag.h"

PySig::PySig(WLSignal *wls, PyThreadState *thread_state, std::string sig, PyObject *func, int prio) : WLSignalInstance(wls),
func(func),
thread_state(thread_state)
{
	EXP_SIGHOOK(sig,&PySig::Exec, prio);
}


PySig::~PySig()
{
}

int
PySig::Exec(WokXMLTag *tag)
{
	int ret = 1;
	
	PyEval_AcquireLock();
	PyEval_AcquireThread(thread_state);

	PyObject *pValue;
	PyObject *pArgs;
	pArgs = PyTuple_New(1);
	PyTuple_SET_ITEM(pArgs, 0, PyCWXMLTag_wrap(tag));
	pValue = PyObject_CallObject(func, pArgs);
	if ( pValue == NULL )
	{
		PyErr_Print();
    	fprintf(stderr,"Call failed\n");
	}
	else
	{
		if ( PyInt_Check(pValue) )
		{
			ret = PyInt_AsLong(pValue);
			if (ret == -1 && PyErr_Occurred())
			{
				ret = 1;
			}
			else
				Py_DECREF(pValue);
		}
		else
			ret = 1;
	}
	Py_DECREF(pArgs);
	
	PyEval_ReleaseThread(thread_state);
	PyEval_ReleaseLock();
	return ret;
}

