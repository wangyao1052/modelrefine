# ModelRefine

Removes redundant faces and edges from OpenCASCADE (OCCT) shapes by uniting adjacent
same-domain faces. This is a standalone, dynamically linked library extracted from the
[FreeCAD](https://github.com/FreeCAD/FreeCAD) `Part` workbench so that host applications do
not have to compile LGPL code into their own binaries.

## Provenance

- `src/modelRefine.h/.cpp` are ported from FreeCAD 1.1.3
  `src/Mod/Part/App/modelRefine.h/.cpp`
  (Copyright (c) 2011 Thomas Anderson <blobfish[at]gmx.com>, LGPL-2.1-or-later).
  Only namespaces, includes and language-level details were adjusted, except for the
  history completion described below (marked with `YI3D:` comments) so that upstream changes
  can still be merged.
- `src/refine.cpp` wraps the algorithm with the closed-state/orientation handling used by
  FreeCAD's `TopoShape::makeElementRefine()` and builds a `BRepTools_History` that host
  applications can merge into their own operation histories (e.g. for topological naming).

## History guarantees

`FaceUniter::process()` rebuilds the modification relations from the original shell after
sewing and edge fusing. The incremental relation list produced by FreeCAD is incomplete:
faces rebuilt by sewing but not united are keyed by post-sew shapes (which
`BRepTools_History` drops), and united faces other than the first keep pre-sew values. The
rebuild guarantees that:

- every relation value is a sub-shape of the returned shell, and
- every changed face or edge with a topological successor is keyed by the original
  sub-shape.

The relations are strictly topological: the library never guesses a match from geometry.
Sub-shapes without a successor (absorbed by a union, or seam segments rebuilt into one
longer seam) are reported as removed by `refine()`; the host is expected to name the new
shapes by its own rules. This is the only intentional algorithmic divergence from FreeCAD.

## License

LGPL-2.1-or-later. See [LICENSE](LICENSE). Distributed as a shared library so that host
applications can link it dynamically, as allowed by the LGPL.

## Requirements

- CMake 3.20 or newer, C++17 compiler
- OpenCASCADE 7.7 or newer, built as shared libraries (tested with OCCT 7.7.0, MSVC 2019)

The public API passes OCCT types across the DLL boundary, so the library and the host
application must be built with the same compiler toolset and against the same OCCT build.

## Build and install

```bat
cmake -S . -B build-Release -G Ninja -DCMAKE_BUILD_TYPE=Release ^
      -DOpenCASCADE_DIR=<path-to>/occt/cmake
cmake --build build-Release
ctest --test-dir build-Release --output-on-failure
cmake --install build-Release --prefix <install-prefix>
```

Repeat with `-DCMAKE_BUILD_TYPE=Debug` into a separate build directory and install into the
same prefix. Debug artifacts carry the `d` postfix (`ModelRefined.dll` /
`ModelRefined.lib`). CMake merges both configurations into the installed package.

`build.bat` is a convenience script for a local Windows setup and installs into
`yi3d/3rdParty/bundles/ModelRefine`; adjust the paths at the top for other setups.

Installed layout:

```text
<prefix>/
  bin/ModelRefine.dll, bin/ModelRefined.dll
  lib/ModelRefine.lib, lib/ModelRefined.lib
  inc/modelrefine/refine.h
  lib/cmake/ModelRefine/ModelRefineConfig.cmake
```

Host applications use it with:

```cmake
find_package(ModelRefine CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE ModelRefine::ModelRefine)
```

## API

```cpp
#include <modelrefine/refine.h>

TopoDS_Shape refinedShape;
Handle(BRepTools_History) history;
if (modelrefine::refine(inputShape, refinedShape, history))
{
    // refinedShape is the refined result; merge history into your operation history.
}
else
{
    // Nothing changed, or the refinement failed: keep inputShape.
}
```

`refine()` only merges planar, cylindrical and BSpline faces, the same limitation as
FreeCAD's implementation. It handles `SOLID`, `SHELL` and `COMPOUND` shapes. On failure it
returns `false` and leaves the output arguments untouched.
