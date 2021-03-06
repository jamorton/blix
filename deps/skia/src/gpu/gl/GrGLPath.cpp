
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPath.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(this->getGpu())

#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(GPUGL->glInterface(), R, X)

namespace {
inline GrGLubyte verb_to_gl_path_cmd(const SkPath::Verb verb) {
    static const GrGLubyte gTable[] = {
        GR_GL_MOVE_TO,
        GR_GL_LINE_TO,
        GR_GL_QUADRATIC_CURVE_TO,
        GR_GL_CUBIC_CURVE_TO,
        GR_GL_CLOSE_PATH,
    };
    GR_STATIC_ASSERT(0 == SkPath::kMove_Verb);
    GR_STATIC_ASSERT(1 == SkPath::kLine_Verb);
    GR_STATIC_ASSERT(2 == SkPath::kQuad_Verb);
    GR_STATIC_ASSERT(3 == SkPath::kCubic_Verb);
    GR_STATIC_ASSERT(4 == SkPath::kClose_Verb);

    GrAssert(verb >= 0 && verb < GR_ARRAY_COUNT(gTable));
    return gTable[verb];
}

inline int num_pts(const SkPath::Verb verb) {
    static const int gTable[] = {
        1, // move
        1, // line
        2, // quad
        3, // cubic
        0, // close
    };
    GR_STATIC_ASSERT(0 == SkPath::kMove_Verb);
    GR_STATIC_ASSERT(1 == SkPath::kLine_Verb);
    GR_STATIC_ASSERT(2 == SkPath::kQuad_Verb);
    GR_STATIC_ASSERT(3 == SkPath::kCubic_Verb);
    GR_STATIC_ASSERT(4 == SkPath::kClose_Verb);

    GrAssert(verb >= 0 && verb < GR_ARRAY_COUNT(gTable));
    return gTable[verb];
}
}

GrGLPath::GrGLPath(GrGpuGL* gpu, const SkPath& path) : INHERITED(gpu) {
    GL_CALL_RET(fPathID, GenPaths(1));
    //GrPrintf("\tGenPaths ID: %d\n", fPathID);
    SkPath::Iter iter(path, true);

    SkSTArray<16, GrGLubyte, true> pathCommands;
#ifndef SK_SCALAR_IS_FLOAT
    GrCrash("Assumes scalar is float.");
#endif
    SkSTArray<16, SkPoint, true> pathPoints;

    int verbCnt = path.countVerbs();
    int pointCnt = path.countPoints();
    pathCommands.resize_back(verbCnt);
    pathPoints.resize_back(pointCnt);

    // TODO: Direct access to path points since we could pass them on directly.
    path.getPoints(&pathPoints[0], pointCnt);
    path.getVerbs(&pathCommands[0], verbCnt);
    
    GR_DEBUGCODE(int numPts = 0);
    for (int i = 0; i < verbCnt; ++i) {
        SkPath::Verb v = static_cast<SkPath::Verb>(pathCommands[i]);
        pathCommands[i] = verb_to_gl_path_cmd(v);
        GR_DEBUGCODE(numPts += num_pts(v));
    }
    GrAssert(pathPoints.count() == numPts);
    
    GL_CALL(PathCommands(fPathID,
                         verbCnt, &pathCommands[0],
                         2 * pointCnt, GR_GL_FLOAT, &pathPoints[0]));
    //GrPrintf("\tPathCommands ID: %d\n", fPathID);
    fBounds = path.getBounds();
}

GrGLPath::~GrGLPath() {
    this->release();
}

void GrGLPath::onRelease() {
    if (0 != fPathID) {
    // FIXME: When we draw a clipped path we may get a call sequence that looks
    // like this:
    // GenPaths(1, &fPathID); // fPathID = 1, the path to draw
    // PathCommands(1, ...);
    // GenPaths(1, &fPathID); // fPathID = 2, the clip path
    // PathCommands(2, ...);
    // PathStencilFunc(...);
    // StencilFillPath(2, ...); // draw the clip
    // DeletePath(1, &fPathID); // fPathID == 2
    // PathStencilFunc(...);
    // StencilFillPath(2, ...); // draw the path
    // DeletePath(1, &fPathID); // fPathID == 1
    //
    // Deleting the clip path causes the second StencilFillPath to fail with
    // INVALID_OPERATION.
#if 0
        GL_CALL(DeletePaths(1, fPathID));
        //GrPrintf("\tDeletePaths ID: %d\n", fPathID);
#else
        //GrPrintf("\tLeak Path ID: %d\n", fPathID);
#endif
        fPathID = 0;
    }
}

void GrGLPath::onAbandon() {
    fPathID = 0;
}

