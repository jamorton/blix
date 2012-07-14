/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLProgramStage.h"

GrGLProgramStage::GrGLProgramStage(const GrProgramStageFactory& factory)
    : fFactory(factory) {
}

GrGLProgramStage::~GrGLProgramStage() {
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgramStage::setupVariables(GrGLShaderBuilder* state, int stage) {

}

void GrGLProgramStage::initUniforms(const GrGLInterface*, int progID) {

}

void GrGLProgramStage::setData(const GrGLInterface*,
                               const GrGLTexture&,
                               const GrCustomStage&,
                               int stageNum) {
}

