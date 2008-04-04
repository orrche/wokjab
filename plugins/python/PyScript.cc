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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Mar 13 15:39:31 2006
//

#include "PyScript.h"
#include "PyWXMLTag.h"

static PyTypeObject Plugin_Type = {
	PyObject_HEAD_INIT(NULL)
	0,			/*ob_size*/
	"Woklib.Plugin",		/*tp_name*/
	sizeof(PluginObject),	/*tp_basicsize*/
	0,			/*tp_itemsize*/
	0, /*tp_dealloc*/
	0,			/*tp_print*/
	0,			/*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	0,			/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
	0,                      /*tp_call*/
	0,                      /*tp_str*/
	PyObject_GenericGetAttr,/*tp_getattro*/
	PyObject_GenericSetAttr,/*tp_setattro*/
	0,                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,     /*tp_flags*/
	0,                      /*tp_doc*/
	0,                      /*tp_traverse*/
	0,                      /*tp_clear*/
	0,                      /*tp_richcompare*/
	0,                      /*tp_weaklistoffset*/
	0,                      /*tp_iter*/
	0,                      /*tp_iternext*/
	0,                      /*tp_methods*/
	0,                      /*tp_members*/
	0,                      /*tp_getset*/
	0,                      /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	0,                      /*tp_init*/
	PyType_GenericAlloc,    /*tp_alloc*/
	PyType_GenericNew,      /*tp_new*/
	_PyObject_Del,          /*tp_free*/
	0,                      /*tp_is_gc*/
};

PyObject *
PyScript::SendSignal(PluginObject *self, PyObject *args)
{
	PyObject *plugin;
	plugin = PySys_GetObject("__plugin__");
	PyScript *c = static_cast <PyScript *> (((PluginObject *)(plugin))->c);
	
	char *sig;
	PyWXMLTagHolder *xml;
	PyArg_ParseTuple(args, "sO", &sig, &xml);
	
	PyEval_ReleaseThread(c->thread_state);
	PyEval_ReleaseLock();
	int ret = c->wls->SendSignal(sig, xml->c);
	PyEval_AcquireLock();
	PyEval_AcquireThread(c->thread_state);
	
	return PyInt_FromLong(ret);
}

PyObject *
PyScript::HookSignal(PluginObject *self, PyObject *args)
{
	PyObject *plugin;
	plugin = PySys_GetObject("__plugin__");
	PyScript *c = static_cast <PyScript *> (((PluginObject *)(plugin))->c);
	
	char *sig;
	int prio;
	PyObject *func;
	PyArg_ParseTuple(args, "sOi", &sig, &func, &prio);
	
	
	if ( sig != NULL )
	{
		c->signal.push_back(new PySig(c->wls, c->thread_state, sig, func, prio));
	}
	
	return PyInt_FromLong(0);
}

PyMethodDef
python_methods[] = {
	{"HookSignal", (PyCFunction)PyScript::HookSignal, METH_VARARGS, "To hook a signal to a function"},
	{"SendSignal", (PyCFunction)PyScript::SendSignal, METH_VARARGS, "To send a signal to a function"},
	
	{NULL}
};

PyScript::PyScript(WLSignal *wls, const std::string &filename) : WLSignalInstance(wls)
{
	PyEval_AcquireLock();
	thread_state = Py_NewInterpreter();
	
	fp = fopen(filename.c_str(), "r");
	
	if ( fp == NULL )
	{
		std::cout << "Python file " << filename << " not found" << std::endl;
		return;
	}
	
	
	PluginObject *plugin = NULL;
	plugin = PyObject_New(PluginObject, &Plugin_Type);
	((PluginObject *)(plugin))->c = this;


	module = Py_InitModule("Woklib", python_methods);
	Py_INCREF(module);
	
	PyWXMLTagInit(module);
	
	
	PySys_SetObject("__plugin__", (PyObject *) plugin);
	
	PyRun_SimpleFile(fp, filename.c_str());
	
	PyEval_ReleaseThread(thread_state);
	PyEval_ReleaseLock();
}

PyScript::~PyScript()
{
	std::list <PySig *>::iterator iter;
	
	for ( iter = signal.begin() ; iter != signal.end() ; iter++)
	{
		delete *iter;
	}
	PyEval_AcquireLock();
	PyEval_AcquireThread(thread_state);

	Py_EndInterpreter(thread_state);
	PyEval_ReleaseLock();
	fclose(fp);
	
}

extern "C" {
PyMethodDef Foo_methods[] = {
{ NULL }
};

void initWoklib()
{
	PyObject* module = Py_InitModule("Woklib", Foo_methods);	
	Py_INCREF(module);
	
	PyWXMLTagInit(module);
}
}


