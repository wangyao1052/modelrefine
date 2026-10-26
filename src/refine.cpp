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

#include <modelrefine/refine.h>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepLib.hxx>
#include <BRepTools_History.hxx>
#include <Standard_Failure.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Solid.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include "modelRefine.h"

namespace
{

// Ported from FreeCAD TopoShape::isClosed().
bool isClosed(const TopoDS_Shape& shape)
{
    if (shape.IsNull())
    {
        return false;
    }

    bool closed = false;
    switch (shape.ShapeType())
    {
    case TopAbs_SHELL:
    case TopAbs_WIRE:
    case TopAbs_EDGE:
        closed = BRep_Tool::IsClosed(shape);
        break;
    case TopAbs_COMPSOLID:
    case TopAbs_SOLID:
    {
        closed = true;
        for (TopExp_Explorer it(shape, TopAbs_SHELL); it.More(); it.Next())
        {
            closed = closed && BRep_Tool::IsClosed(it.Current());
        }
        break;
    }
    case TopAbs_COMPOUND:
    {
        closed = true;
        for (TopExp_Explorer it(shape, TopAbs_SHELL); it.More(); it.Next())
        {
            closed = closed && BRep_Tool::IsClosed(it.Current());
        }
        for (TopExp_Explorer it(shape, TopAbs_FACE, TopAbs_SHELL); it.More(); it.Next())
        {
            closed = closed && BRep_Tool::IsClosed(it.Current());
        }
        for (TopExp_Explorer it(shape, TopAbs_WIRE, TopAbs_FACE); it.More(); it.Next())
        {
            closed = closed && BRep_Tool::IsClosed(it.Current());
        }
        for (TopExp_Explorer it(shape, TopAbs_EDGE, TopAbs_WIRE); it.More(); it.Next())
        {
            closed = closed && BRep_Tool::IsClosed(it.Current());
        }
        for (TopExp_Explorer it(shape, TopAbs_VERTEX, TopAbs_EDGE); it.More(); it.Next())
        {
            closed = closed && BRep_Tool::IsClosed(it.Current());
        }
        break;
    }
    default:
        break;
    }

    return closed;
}

// Ported from FreeCAD TopoShape::fixSolidOrientation().
bool fixSolidOrientation(TopoDS_Shape& shape)
{
    if (shape.IsNull())
    {
        return false;
    }

    if (shape.ShapeType() == TopAbs_SOLID)
    {
        TopoDS_Solid solid = TopoDS::Solid(shape);
        BRepLib::OrientClosedSolid(solid);
        if (solid.IsEqual(shape))
        {
            return false;
        }
        shape = solid;
        return true;
    }

    if (shape.ShapeType() == TopAbs_COMPOUND || shape.ShapeType() == TopAbs_COMPSOLID)
    {
        std::vector<TopoDS_Shape> children;
        bool touched = false;
        for (TopoDS_Iterator it(shape); it.More(); it.Next())
        {
            TopoDS_Shape child = it.Value();
            touched = fixSolidOrientation(child) || touched;
            children.push_back(child);
        }
        if (!touched)
        {
            return false;
        }

        BRep_Builder builder;
        if (shape.ShapeType() == TopAbs_COMPOUND)
        {
            TopoDS_Compound comp;
            builder.MakeCompound(comp);
            for (const TopoDS_Shape& child : children)
            {
                if (!child.IsNull())
                {
                    builder.Add(comp, child);
                }
            }
            shape = comp;
        }
        else
        {
            TopoDS_CompSolid comp;
            builder.MakeCompSolid(comp);
            for (const TopoDS_Shape& child : children)
            {
                if (!child.IsNull())
                {
                    builder.Add(comp, child);
                }
            }
            shape = comp;
        }
        return true;
    }

    return false;
}

} // namespace

bool modelrefine::refine(const TopoDS_Shape& shape,
                         TopoDS_Shape& refinedShape,
                         Handle(BRepTools_History)& history)
{
    if (shape.IsNull())
    {
        return false;
    }

    try
    {
        BRepBuilderAPI_RefineModel refineMaker(shape);
        TopoDS_Shape result = refineMaker.Shape();
        if (result.IsNull() || result.IsSame(shape))
        {
            return false;
        }

        // Discard the refinement if it changed the closed/open state, as FreeCAD does in
        // TopoShape::makeElementRefine().
        const bool wasClosed = isClosed(shape);
        fixSolidOrientation(result);
        if (isClosed(result) != wasClosed)
        {
            return false;
        }

        TopTools_ListOfShape arguments;
        arguments.Append(shape);
        Handle(BRepTools_History) refineHistory = new BRepTools_History(arguments, refineMaker);

        // ModelRefine only reports united faces and fused edges. Sub-shapes that vanish
        // during refinement (e.g. the seam edges of united faces) have no relation at all,
        // so a merged history would keep mapping them to sub-shapes of the unrefined input.
        // Report them as removed to keep the history consistent with the refined shape.
        TopTools_IndexedMapOfShape inputSubShapes;
        TopExp::MapShapes(shape, inputSubShapes);
        TopTools_IndexedMapOfShape resultSubShapes;
        TopExp::MapShapes(result, resultSubShapes);
        for (int i = 1; i <= inputSubShapes.Extent(); ++i)
        {
            const TopoDS_Shape& subShape = inputSubShapes(i);
            if (!BRepTools_History::IsSupportedType(subShape) ||
                resultSubShapes.Contains(subShape) ||
                !refineHistory->Modified(subShape).IsEmpty() ||
                !refineHistory->Generated(subShape).IsEmpty())
            {
                continue;
            }
            refineHistory->Remove(subShape);
        }

        refinedShape = result;
        history = refineHistory;
        return true;
    }
    catch (const Standard_Failure&)
    {
    }

    return false;
}
