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
// Class: PyWXMLTag
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Mar 17 14:20:15 2006
//

#ifndef _PYWXMLTAG_H_
#define _PYWXMLTAG_H_

#include "Python.h"
#include "pythread.h"

#include <Woklib/WokXMLTag.h>

using namespace Woklib;

typedef struct {
    PyObject_HEAD
    WokXMLTag *c;
		bool pyowned;
} PyWXMLTagHolder;

extern "C" void PyWXMLTagInit(PyObject* module );
extern "C" void PyWXMLTag_dealloc(PyWXMLTagHolder* self);
extern "C" PyObject * PyWXMLTag_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
extern "C" int PyWXMLTag_init(PyWXMLTagHolder *self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_name(PyWXMLTagHolder* self);
extern "C" PyObject * PyWXMLTag_GetAttr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_AddAttr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_GetFirstTag(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_In(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_RemoveChildrenTags(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_RemoveBody(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_AddText(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_GetBody(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_RemoveAttr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_RemoveTag(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_GetChildrenStr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_AddTagName(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_AddTagTag(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_Add(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_GetTags(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_GetTagList(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);
extern "C" PyObject * PyWXMLTag_GetObjects(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds);

extern "C" PyObject * PyWXMLTag_str(PyWXMLTagHolder* self);
extern "C" PyObject * PyCWXMLTag_new(std::string name);
extern "C" PyObject * PyCWXMLTag_wrap(WokXMLTag *tag);
extern "C" int PyWXMLTag_compare( PyWXMLTagHolder *o1, PyWXMLTagHolder *o2 );

#endif	//_PYWXMLTAG_H_

