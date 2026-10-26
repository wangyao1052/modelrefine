// SPDX-License-Identifier: LGPL-2.1-or-later

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2024-2026 Wang Yao <wangyao1052@163.com>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
// General Public License for more details.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MODELREFINE_REFINE_H
#define MODELREFINE_REFINE_H

#include <BRepTools_History.hxx>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>

#if defined(_WIN32)
#  if defined(MODELREFINE_LIBRARY)
#    define MODELREFINE_EXPORT __declspec(dllexport)
#  else
#    define MODELREFINE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODELREFINE_EXPORT __attribute__((visibility("default")))
#endif

namespace modelrefine
{

// Removes redundant faces and edges from a shape by uniting adjacent
// same-domain faces. This is a port of FreeCAD's ModelRefine module.
//
// On success, refinedShape holds the refined result and history maps the
// sub-shapes of the input to the refined sub-shapes. Sub-shapes that vanish
// during refinement are reported as removed, so the history can be merged
// into another operation's history for topological naming.
//
// Returns false when the shape cannot be refined: null input, no change,
// refinement failure, or a closed-state change. The output arguments are
// left untouched in that case.
MODELREFINE_EXPORT bool refine(const TopoDS_Shape& shape,
                               TopoDS_Shape& refinedShape,
                               Handle(BRepTools_History)& history);

} // namespace modelrefine

#endif // MODELREFINE_REFINE_H
