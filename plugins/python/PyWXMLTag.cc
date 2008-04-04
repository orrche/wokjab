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
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Mar 17 14:20:15 2006
//

#include "PyWXMLTag.h"

static PyMethodDef PyWXMLTag_methods[] = {
	    {"name", 					(PyCFunction)PyWXMLTag_name, 				METH_NOARGS, "Return the name" },
		{"GetAttr", 				(PyCFunction)PyWXMLTag_GetAttr, 			METH_VARARGS, "Retrieving attributes" },
		{"AddAttr", 				(PyCFunction)PyWXMLTag_AddAttr, 			METH_VARARGS, "Setting attributes" },
	    {"GetFirstTag", 			(PyCFunction)PyWXMLTag_GetFirstTag, 		METH_VARARGS, "Setting attributes" },
		{"In", 						(PyCFunction)PyWXMLTag_In, 					METH_VARARGS, "Check if the XMLTag exists in another one" },
		{"RemoveChildrenTags",		(PyCFunction)PyWXMLTag_RemoveChildrenTags,  METH_NOARGS, "Removes all children" },
		{"RemoveBody",				(PyCFunction)PyWXMLTag_RemoveBody, 			METH_NOARGS, "Removes all children" },
		{"AddText", 				(PyCFunction)PyWXMLTag_AddText, 			METH_VARARGS, "Adding text" },
		{"GetBody", 				(PyCFunction)PyWXMLTag_GetBody, 			METH_NOARGS, "Getting the body" },
		{"RemoveAttr", 				(PyCFunction)PyWXMLTag_RemoveAttr, 			METH_VARARGS, "Removing an attribute" },
		{"RemoveTag", 				(PyCFunction)PyWXMLTag_RemoveTag, 			METH_VARARGS, "Removing a specific tag" },
		{"GetChildrenStr", 			(PyCFunction)PyWXMLTag_GetChildrenStr,		METH_NOARGS, "Retrieving attributes" },
		{"AddTagName", 				(PyCFunction)PyWXMLTag_AddTagName, 			METH_VARARGS, "Adding a new tag with name" },
		{"AddTagTag", 				(PyCFunction)PyWXMLTag_AddTagTag, 			METH_VARARGS, "Adding a new tag from another tag" },
		{"Add",		 				(PyCFunction)PyWXMLTag_Add, 				METH_VARARGS, "Add data to the tag both text and other tags in text" },
		{"GetTags", 				(PyCFunction)PyWXMLTag_GetTags, 			METH_NOARGS, "Get a list of all children tags" },
		{"GetTagList",				(PyCFunction)PyWXMLTag_GetTagList,			METH_VARARGS, "Get a list of children tags with a specific name" },
		{"GetObjects",				(PyCFunction)PyWXMLTag_GetObjects,			METH_VARARGS, "Get a list of text and tag children" },
		{NULL}  /* Sentinel */
};

/* 

Missing

*/


static PyTypeObject PyWXMLTagType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Woklib.XMLTag",           /*tp_name*/
    sizeof(PyWXMLTagHolder),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyWXMLTag_dealloc, /*tp_dealloc*/
    0,												 /*tp_print*/
		0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    (cmpfunc)PyWXMLTag_compare,/*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc) PyWXMLTag_str,  /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Noddy objects",           /* tp_doc */
    0,		               			 /* tp_traverse */
    0,		               			 /* tp_clear */
    0,		               			 /* tp_richcompare */
    0,		               			 /* tp_weaklistoffset */
    0,		               			 /* tp_iter */
    0,		               			 /* tp_iternext */
    PyWXMLTag_methods,         /* tp_methods */
    0,         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyWXMLTag_init,  /* tp_init */
    0,                         /* tp_alloc */
    PyWXMLTag_new,             /* tp_new */
};

extern "C" {

void PyWXMLTagInit(PyObject* module )
{
	if (PyType_Ready(&PyWXMLTagType) < 0)
        return;
	Py_INCREF(&PyWXMLTagType);
	PyTypeObject *tt;
	tt = &PyWXMLTagType;
	PyModule_AddObject(module, "XMLTag", (PyObject *)tt);
}
 
int
PyWXMLTag_compare( PyWXMLTagHolder *o1, PyWXMLTagHolder *o2 )
{
	std::cout << "Ggggggg" << std::endl;
	
	if( *o1->c == *o2->c )
	{
		std::cout << "True" << std::endl;
		return 0;
	}
	std::cout << "False" << std::endl;
	return 1;
}

void
PyWXMLTag_dealloc(PyWXMLTagHolder* self)
{
	if( self->pyowned )
		delete self->c;
	self->ob_type->tp_free((PyObject*)self);
}

PyObject *
PyWXMLTag_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyWXMLTagHolder *self;
	char *name=NULL;
	
	static char *kwlist[] = {"name", NULL};
	if (! PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
                                  &name))
		return NULL;
    self = (PyWXMLTagHolder *)type->tp_alloc(type, 0);
    if (self != NULL) 
	{
        self->c = new WokXMLTag(NULL,name);
				self->pyowned = true;
    }

    return (PyObject *)self;
}

int
PyWXMLTag_init(PyWXMLTagHolder *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

PyObject *
PyWXMLTag_name(PyWXMLTagHolder* self)
{
	PyObject *ret;
	ret = PyString_FromString(self->c->GetName().c_str());
	
	return ret;
}

PyObject *
PyWXMLTag_GetAttr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *attrname = NULL;
	
	if (! PyArg_ParseTuple(args, "s", &attrname))
	{
		return NULL;
	}
	
	PyObject *ret;
	ret = PyString_FromString(self->c->GetAttr(attrname).c_str());
	
	return ret;
}

PyObject *
PyWXMLTag_RemoveTag(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	PyWXMLTagHolder* other = NULL;
	
	if (! PyArg_ParseTuple(args, "O", &other))
	{
		return NULL;
	}
	
	self->c->RemoveTag(other->c);
#warning should delete the python object somehow
	
	return PyInt_FromLong(0);
}

PyObject *
PyWXMLTag_AddTagName(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *name = NULL;
	
	if (! PyArg_ParseTuple(args, "s", &name))
	{
		return NULL;
	}
	PyObject * ret = PyCWXMLTag_wrap(&self->c->AddTag(name));
	return ret;
}

PyObject *
PyWXMLTag_AddTagTag(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	PyWXMLTagHolder* other = NULL;
	
	if (! PyArg_ParseTuple(args, "O", &other))
	{
		return NULL;
	}
	
	return PyCWXMLTag_wrap(&self->c->AddTag(other->c));
}

PyObject *
PyWXMLTag_Add(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char* data = NULL;
	
	if (! PyArg_ParseTuple(args, "s", &data))
	{
		return NULL;
	}
	self->c->Add(data);
	
	return PyInt_FromLong(0);
}


PyObject *
PyWXMLTag_GetBody(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	PyObject *ret;
	ret = PyString_FromString(self->c->GetBody().c_str());
	
	return ret;

}

PyObject *
PyWXMLTag_GetTags(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	std::list<WokXMLTag *> &list = self->c->GetTags();
	std::list<WokXMLTag *>::iterator iter;
	int i = 0;
	
	PyObject *ret;
	ret = PyList_New(list.size());
	
	for(iter = list.begin() ; iter != list.end() ; iter++ )
	{
		PyList_SetItem(	ret, i++, PyCWXMLTag_wrap(*iter));
	}
	return ret;
}

PyObject *
PyWXMLTag_GetObjects(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	std::list<WokXMLObject *> &list = self->c->GetItemList();
	std::list<WokXMLObject *>::iterator iter;
	int i = 0;
	
	PyObject *ret;
	ret = PyList_New(list.size());
	
	for(iter = list.begin() ; iter != list.end() ; iter++ )
	{
		switch ( (*iter)->GetType() )
		{
			case 1:
				PyList_SetItem(	ret, i++, PyCWXMLTag_wrap((WokXMLTag *)*iter));
				break;
			case 2:
				PyList_SetItem(	ret, i++, PyString_FromString((*iter)->GetStr().c_str()));

				break;
		}

	}
	return ret;
}

PyObject *
PyWXMLTag_GetTagList(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *text = NULL;
	
	if (! PyArg_ParseTuple(args, "s", &text))
	{
		return NULL;
	}
	
	std::list<WokXMLTag *> &list = self->c->GetTagList(text);
	std::list<WokXMLTag *>::iterator iter;
	int i = 0;
	
	PyObject *ret;
	ret = PyList_New(list.size());
	
	for(iter = list.begin() ; iter != list.end() ; iter++ )
	{
		PyList_SetItem(	ret, i++, PyCWXMLTag_wrap(*iter));
	}
	return ret;
}

PyObject *
PyWXMLTag_RemoveChildrenTags(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	self->c->RemoveChildrenTags();
	return PyInt_FromLong(0);
}

PyObject *
PyWXMLTag_GetChildrenStr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	PyObject *ret;
	ret = PyString_FromString(self->c->GetChildrenStr().c_str());
	
	return ret;

}

PyObject *
PyWXMLTag_RemoveBody(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	self->c->RemoveBody();
	return PyInt_FromLong(0);
}

PyObject * 
PyWXMLTag_AddText(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *text = NULL;
	
	if (! PyArg_ParseTuple(args, "s", &text))
	{
		return NULL;
	}
	
	self->c->AddText(text);
	
	return PyInt_FromLong(0);
}

PyObject * 
PyWXMLTag_RemoveAttr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *name = NULL;
	
	if (! PyArg_ParseTuple(args, "s", &name))
	{
		return NULL;
	}
	
	self->c->RemoveAttr(name);
	
	return PyInt_FromLong(0);
}

PyObject *
PyWXMLTag_In(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	PyWXMLTagHolder *obj;
	
	if( !PyArg_ParseTuple(args, "O", &obj))
	{
		return NULL;
	}
	
	if( self->c->In ( *obj->c ) )
	{
		return PyInt_FromLong(1);
	}
	return PyInt_FromLong(0);
}

PyObject *
PyWXMLTag_AddAttr(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *name = NULL;
	char *value = NULL;
	
	if( !PyArg_ParseTuple(args, "ss", &name, &value))
	{
		return NULL;
	}
	
	self->c->AddAttr(name, value);
	
	return PyInt_FromLong(0);
}

PyObject *
PyWXMLTag_GetFirstTag(PyWXMLTagHolder* self, PyObject *args, PyObject *kwds)
{
	char *name = NULL;
	
	if( !PyArg_ParseTuple(args, "s", &name))
	{
		return NULL;
	}
	
	return PyCWXMLTag_wrap(&self->c->GetFirstTag(name));
}

PyObject *
PyWXMLTag_str(PyWXMLTagHolder* self)
{
	return PyString_FromString(self->c->GetStr().c_str());
}

PyObject *
PyCWXMLTag_new(std::string name)
{
	PyWXMLTagHolder *self = NULL;
	self = (PyWXMLTagHolder*) _PyObject_New(&PyWXMLTagType);
	
	if (self != NULL) {
		self->c = new WokXMLTag(NULL,name);
		self->pyowned = true;
  }
		
	return (PyObject *) self;
}

PyObject *
PyCWXMLTag_wrap(WokXMLTag *tag)
{
	PyWXMLTagHolder *self = NULL;
	self = (PyWXMLTagHolder*) _PyObject_New(&PyWXMLTagType);
	
	if (self != NULL) {
		self->c = tag;
		self->pyowned = false;
  }
		
	return (PyObject *) self;
}

}
