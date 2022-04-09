#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi-headers/windows-h.hpp>
#include <ole2.h>   // Defines `small`, so probably drags in RPC headers. :(
#ifdef small
#   undef small     // Extreme macro pollution from the Microsoft coders.
#endif

/*
OleCreate
Creates an embedded object identified by a CLSID. You use it typically to implement the menu item that allows the end user to insert a new object.

OleCreateFromFile
Creates an embedded object from the contents of a named file.

OleDraw
Enables drawing objects more easily. You can use it instead of calling IViewObject::Draw directly.

OleLoadFromStream
Loads an object from the stream.

OleSaveToStream
Saves an object with the IPersistStream interface on it to the specified stream.
*/
