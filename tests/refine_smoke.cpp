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
///////////////////////////////////////////////////////////////////////////////

#include <modelrefine/refine.h>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <gp_Pnt.hxx>

#include <cstdio>

int main()
{
    // Null input must not be refined.
    {
        TopoDS_Shape refinedShape;
        Handle(BRepTools_History) history;
        if (modelrefine::refine(TopoDS_Shape(), refinedShape, history))
        {
            std::printf("FAIL: null shape was refined\n");
            return 1;
        }
    }

    // Two stacked 100x100x100 boxes fuse into a single 100x100x200 box. The raw fuse keeps
    // the split at the common plane (10 faces); refinement must unite them into 6 faces.
    BRepPrimAPI_MakeBox box1(100.0, 100.0, 100.0);
    BRepPrimAPI_MakeBox box2(gp_Pnt(0.0, 0.0, 100.0), 100.0, 100.0, 100.0);
    BRepAlgoAPI_Fuse fuse(box1.Shape(), box2.Shape());
    fuse.Build();
    if (!fuse.IsDone())
    {
        std::printf("FAIL: fuse failed\n");
        return 1;
    }

    TopTools_IndexedMapOfShape rawFaces;
    TopExp::MapShapes(fuse.Shape(), TopAbs_FACE, rawFaces);
    if (rawFaces.Extent() != 10)
    {
        std::printf("FAIL: raw fuse has %d faces, expected 10\n", rawFaces.Extent());
        return 1;
    }

    TopoDS_Shape refinedShape;
    Handle(BRepTools_History) history;
    if (!modelrefine::refine(fuse.Shape(), refinedShape, history))
    {
        std::printf("FAIL: refine returned false\n");
        return 1;
    }
    if (refinedShape.IsNull())
    {
        std::printf("FAIL: refined shape is null\n");
        return 1;
    }
    if (history.IsNull())
    {
        std::printf("FAIL: refine history is null\n");
        return 1;
    }

    TopTools_IndexedMapOfShape refinedFaces;
    TopExp::MapShapes(refinedShape, TopAbs_FACE, refinedFaces);
    if (refinedFaces.Extent() != 6)
    {
        std::printf("FAIL: refined shape has %d faces, expected 6\n", refinedFaces.Extent());
        return 1;
    }

    std::printf("OK: raw faces = %d, refined faces = %d\n",
        rawFaces.Extent(), refinedFaces.Extent());
    return 0;
}
