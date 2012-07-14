/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGradientEffects.h"
#include "gl/GrGLProgramStage.h"
#include "GrProgramStageFactory.h"

/////////////////////////////////////////////////////////////////////

class GrGLRadialGradient : public GrGLProgramStage {

public:

    GrGLRadialGradient(const GrProgramStageFactory& factory,
                       const GrCustomStage&) : INHERITED (factory) { }
    virtual ~GrGLRadialGradient() { }

    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) { return 0; }

private:

    typedef GrGLProgramStage INHERITED;

};

void GrGLRadialGradient::emitFS(GrGLShaderBuilder* state,
                                const char* outputColor,
                                const char* inputColor,
                                const char* samplerName) {
    state->fSampleCoords.printf("vec2(length(%s.xy), 0.5)",
                                state->fSampleCoords.c_str());
    state->fComplexCoord = true;

    state->emitDefaultFetch(outputColor, samplerName);
}


/////////////////////////////////////////////////////////////////////


GrRadialGradient::GrRadialGradient() {

}

GrRadialGradient::~GrRadialGradient() {

}


const GrProgramStageFactory& GrRadialGradient::getFactory() const {
    return GrTProgramStageFactory<GrRadialGradient>::getInstance();
}

bool GrRadialGradient::isEqual(const GrCustomStage& sBase) const {
    return true;
}

/////////////////////////////////////////////////////////////////////

class GrGLRadial2Gradient : public GrGLProgramStage {

public:

    GrGLRadial2Gradient(const GrProgramStageFactory& factory,
                        const GrCustomStage&);
    virtual ~GrGLRadial2Gradient() { }

    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*,
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) {
        return (static_cast<const GrRadial2Gradient&>(s).isDegenerate());
    }

protected:

    const GrGLShaderVar* fVSParamVar;
    GrGLint fVSParamLocation;
    const GrGLShaderVar* fFSParamVar;
    GrGLint fFSParamLocation;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsDegenerate;

    // @{
    /// Values last uploaded as uniforms

    GrScalar fCachedCenter;
    GrScalar fCachedRadius;
    bool     fCachedPosRoot;

    // @}

private:

    typedef GrGLProgramStage INHERITED;

};

GrGLRadial2Gradient::GrGLRadial2Gradient(
        const GrProgramStageFactory& factory,
        const GrCustomStage& baseData)
    : INHERITED(factory)
    , fVSParamVar(NULL)
    , fFSParamVar(NULL)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(GR_ScalarMax)
    , fCachedRadius(-GR_ScalarMax)
    , fCachedPosRoot(0) {

    const GrRadial2Gradient& data =
        static_cast<const GrRadial2Gradient&>(baseData);
    fIsDegenerate = data.isDegenerate();
}

void GrGLRadial2Gradient::setupVariables(GrGLShaderBuilder* state, int stage) {
    // 2 copies of uniform array, 1 for each of vertex & fragment shader,
    // to work around Xoom bug. Doesn't seem to cause performance decrease
    // in test apps, but need to keep an eye on it.
    fVSParamVar = &state->addUniform(
        GrGLShaderBuilder::kVertex_VariableLifetime,
        kFloat_GrSLType, "uRadial2VSParams", stage, 6);
    fFSParamVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_VariableLifetime,
        kFloat_GrSLType, "uRadial2FSParams", stage, 6);

    fVSParamLocation = GrGLProgramStage::kUseUniform;
    fFSParamLocation = GrGLProgramStage::kUseUniform;

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (state->fVaryingDims == state->fCoordDims) {
        state->addVarying(kFloat_GrSLType, "Radial2BCoeff", stage,
                          &fVSVaryingName, &fFSVaryingName);
    }
}

void GrGLRadial2Gradient::emitVS(GrGLShaderBuilder* state,
                                 const char* vertexCoords) {
    GrStringBuilder* code = &state->fVSCode;
    GrStringBuilder p2;
    GrStringBuilder p3;
    fVSParamVar->appendArrayAccess(2, &p2);
    fVSParamVar->appendArrayAccess(3, &p3);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (state->fVaryingDims == state->fCoordDims) {
        // r2Var = 2 * (r2Parm[2] * varCoord.x - r2Param[3])
        code->appendf("\t%s = 2.0 *(%s * %s.x - %s);\n",
                      fVSVaryingName, p2.c_str(),
                      vertexCoords, p3.c_str());
    }
}

void GrGLRadial2Gradient::emitFS(GrGLShaderBuilder* state,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const char* samplerName) {
    GrStringBuilder* code = &state->fFSCode;
    GrStringBuilder cName("c");
    GrStringBuilder ac4Name("ac4");
    GrStringBuilder rootName("root");
    GrStringBuilder p0;
    GrStringBuilder p1;
    GrStringBuilder p2;
    GrStringBuilder p3;
    GrStringBuilder p4;
    GrStringBuilder p5;
    fFSParamVar->appendArrayAccess(0, &p0);
    fFSParamVar->appendArrayAccess(1, &p1);
    fFSParamVar->appendArrayAccess(2, &p2);
    fFSParamVar->appendArrayAccess(3, &p3);
    fFSParamVar->appendArrayAccess(4, &p4);
    fFSParamVar->appendArrayAccess(5, &p5);

    // If we we're able to interpolate the linear component,
    // bVar is the varying; otherwise compute it
    GrStringBuilder bVar;
    if (state->fCoordDims == state->fVaryingDims) {
        bVar = fFSVaryingName;
        GrAssert(2 == state->fVaryingDims);
    } else {
        GrAssert(3 == state->fVaryingDims);
        bVar = "b";
        //bVar.appendS32(stageNum);
        code->appendf("\tfloat %s = 2.0 * (%s * %s.x - %s);\n",
                      bVar.c_str(), p2.c_str(),
                      state->fSampleCoords.c_str(), p3.c_str());
    }

    // c = (x^2)+(y^2) - params[4]
    code->appendf("\tfloat %s = dot(%s, %s) - %s;\n",
                  cName.c_str(), state->fSampleCoords.c_str(),
                  state->fSampleCoords.c_str(),
                  p4.c_str());

    // If we aren't degenerate, emit some extra code, and accept a slightly
    // more complex coord.
    if (!fIsDegenerate) {

        // ac4 = 4.0 * params[0] * c
        code->appendf("\tfloat %s = %s * 4.0 * %s;\n",
                      ac4Name.c_str(), p0.c_str(),
                      cName.c_str());

        // root = sqrt(b^2-4ac)
        // (abs to avoid exception due to fp precision)
        code->appendf("\tfloat %s = sqrt(abs(%s*%s - %s));\n",
                      rootName.c_str(), bVar.c_str(), bVar.c_str(),
                      ac4Name.c_str());

        // x coord is: (-b + params[5] * sqrt(b^2-4ac)) * params[1]
        // y coord is 0.5 (texture is effectively 1D)
        state->fSampleCoords.printf("vec2((-%s + %s * %s) * %s, 0.5)",
                            bVar.c_str(), p5.c_str(),
                            rootName.c_str(), p1.c_str());
    } else {
        // x coord is: -c/b
        // y coord is 0.5 (texture is effectively 1D)
        state->fSampleCoords.printf("vec2((-%s / %s), 0.5)",
                            cName.c_str(), bVar.c_str());
    }
    state->fComplexCoord = true;

    state->emitDefaultFetch(outputColor, samplerName);
}

void GrGLRadial2Gradient::initUniforms(const GrGLInterface* gl, int programID) {
    GR_GL_CALL_RET(gl, fVSParamLocation,
        GetUniformLocation(programID, fVSParamVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fFSParamLocation,
        GetUniformLocation(programID, fFSParamVar->getName().c_str()));
}

void GrGLRadial2Gradient::setData(const GrGLInterface* gl,
                                  const GrGLTexture& texture,
                                  const GrCustomStage& baseData,
                                  int stageNum) {
    const GrRadial2Gradient& data =
        static_cast<const GrRadial2Gradient&>(baseData);
    GrAssert(data.isDegenerate() == fIsDegenerate);
    GrScalar centerX1 = data.center();
    GrScalar radius0 = data.radius();
    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedPosRoot != data.isPosRoot()) {

        GrScalar a = GrMul(centerX1, centerX1) - GR_Scalar1;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            GrScalarToFloat(a),
            1 / (2.f * GrScalarToFloat(a)),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(GrMul(radius0, radius0)),
            data.isPosRoot() ? 1.f : -1.f
        };

        GR_GL_CALL(gl, Uniform1fv(fVSParamLocation, 6, values));
        GR_GL_CALL(gl, Uniform1fv(fFSParamLocation, 6, values));
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedPosRoot = data.isPosRoot();
    }
}


/////////////////////////////////////////////////////////////////////

GrRadial2Gradient::GrRadial2Gradient(GrScalar center,
                                     GrScalar radius,
                                     bool posRoot)
    : fCenterX1 (center)
    , fRadius0 (radius)
    , fPosRoot (posRoot) {

}

GrRadial2Gradient::~GrRadial2Gradient() {

}


const GrProgramStageFactory& GrRadial2Gradient::getFactory() const {
    return GrTProgramStageFactory<GrRadial2Gradient>::getInstance();
}

bool GrRadial2Gradient::isEqual(const GrCustomStage& sBase) const {
    const GrRadial2Gradient& s = static_cast<const GrRadial2Gradient&>(sBase);
    return (this->isDegenerate() == s.isDegenerate());
}

/////////////////////////////////////////////////////////////////////

class GrGLConical2Gradient : public GrGLProgramStage {

public:

    GrGLConical2Gradient(const GrProgramStageFactory& factory,
                         const GrCustomStage&);
    virtual ~GrGLConical2Gradient() { }

    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;
    virtual void initUniforms(const GrGLInterface*, int programID) SK_OVERRIDE;
    virtual void setData(const GrGLInterface*,
                         const GrGLTexture&,
                         const GrCustomStage&,
                         int stageNum) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) {
        return (static_cast<const GrConical2Gradient&>(s).isDegenerate());
    }

protected:

    const GrGLShaderVar* fVSParamVar;
    GrGLint fVSParamLocation;
    const GrGLShaderVar* fFSParamVar;
    GrGLint fFSParamLocation;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsDegenerate;

    // @{
    /// Values last uploaded as uniforms

    GrScalar fCachedCenter;
    GrScalar fCachedRadius;
    GrScalar fCachedDiffRadius;

    // @}

private:

    typedef GrGLProgramStage INHERITED;

};

GrGLConical2Gradient::GrGLConical2Gradient(
        const GrProgramStageFactory& factory,
        const GrCustomStage& baseData)
    : INHERITED(factory)
    , fVSParamVar(NULL)
    , fFSParamVar(NULL)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(GR_ScalarMax)
    , fCachedRadius(-GR_ScalarMax)
    , fCachedDiffRadius(-GR_ScalarMax) {

    const GrConical2Gradient& data =
        static_cast<const GrConical2Gradient&>(baseData);
    fIsDegenerate = data.isDegenerate();
}

void GrGLConical2Gradient::setupVariables(GrGLShaderBuilder* state, int stage) {
    // 2 copies of uniform array, 1 for each of vertex & fragment shader,
    // to work around Xoom bug. Doesn't seem to cause performance decrease
    // in test apps, but need to keep an eye on it.
    fVSParamVar = &state->addUniform(
        GrGLShaderBuilder::kVertex_VariableLifetime,
        kFloat_GrSLType, "uConical2VSParams", stage, 6);
    fFSParamVar = &state->addUniform(
        GrGLShaderBuilder::kFragment_VariableLifetime,
        kFloat_GrSLType, "uConical2FSParams", stage, 6);

    fVSParamLocation = GrGLProgramStage::kUseUniform;
    fFSParamLocation = GrGLProgramStage::kUseUniform;

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (state->fVaryingDims == state->fCoordDims) {
        state->addVarying(kFloat_GrSLType, "Conical2BCoeff", stage,
                          &fVSVaryingName, &fFSVaryingName);
    }
}

void GrGLConical2Gradient::emitVS(GrGLShaderBuilder* state,
                                  const char* vertexCoords) {
    GrStringBuilder* code = &state->fVSCode;
    GrStringBuilder p2; // distance between centers
    GrStringBuilder p3; // start radius
    GrStringBuilder p5; // difference in radii (r1 - r0)
    fVSParamVar->appendArrayAccess(2, &p2);
    fVSParamVar->appendArrayAccess(3, &p3);
    fVSParamVar->appendArrayAccess(5, &p5);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (state->fVaryingDims == state->fCoordDims) {
        // r2Var = -2 * (r2Parm[2] * varCoord.x - r2Param[3] * r2Param[5])
        code->appendf("\t%s = -2.0 * (%s * %s.x + %s * %s);\n",
                      fVSVaryingName, p2.c_str(),
                      vertexCoords, p3.c_str(), p5.c_str());
    }
}

void GrGLConical2Gradient::emitFS(GrGLShaderBuilder* state,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const char* samplerName) {
    GrStringBuilder* code = &state->fFSCode;

    GrStringBuilder cName("c");
    GrStringBuilder ac4Name("ac4");
    GrStringBuilder dName("d");
    GrStringBuilder qName("q");
    GrStringBuilder r0Name("r0");
    GrStringBuilder r1Name("r1");
    GrStringBuilder tName("t");
    GrStringBuilder p0; // 4a
    GrStringBuilder p1; // 1/(2a)
    GrStringBuilder p2; // distance between centers
    GrStringBuilder p3; // start radius
    GrStringBuilder p4; // start radius squared
    GrStringBuilder p5; // difference in radii (r1 - r0)
    fFSParamVar->appendArrayAccess(0, &p0);
    fFSParamVar->appendArrayAccess(1, &p1);
    fFSParamVar->appendArrayAccess(2, &p2);
    fFSParamVar->appendArrayAccess(3, &p3);
    fFSParamVar->appendArrayAccess(4, &p4);
    fFSParamVar->appendArrayAccess(5, &p5);

    // If we we're able to interpolate the linear component,
    // bVar is the varying; otherwise compute it
    GrStringBuilder bVar;
    if (state->fCoordDims == state->fVaryingDims) {
        bVar = fFSVaryingName;
        GrAssert(2 == state->fVaryingDims);
    } else {
        GrAssert(3 == state->fVaryingDims);
        bVar = "b";
        code->appendf("\tfloat %s = -2.0 * (%s * %s.x + %s * %s);\n", 
                      bVar.c_str(), p2.c_str(), state->fSampleCoords.c_str(), 
                      p3.c_str(), p5.c_str());
    }

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    code->appendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", outputColor);

    // c = (x^2)+(y^2) - params[4]
    code->appendf("\tfloat %s = dot(%s, %s) - %s;\n", cName.c_str(), 
                  state->fSampleCoords.c_str(), state->fSampleCoords.c_str(),
                  p4.c_str());

    // Non-degenerate case (quadratic)
    if (!fIsDegenerate) {

        // ac4 = params[0] * c
        code->appendf("\tfloat %s = %s * %s;\n", ac4Name.c_str(), p0.c_str(),
                      cName.c_str());
        
        // d = b^2 - ac4
        code->appendf("\tfloat %s = %s * %s - %s;\n", dName.c_str(), 
                      bVar.c_str(), bVar.c_str(), ac4Name.c_str());

        // only proceed if discriminant is >= 0
        code->appendf("\tif (%s >= 0.0) {\n", dName.c_str());

        // intermediate value we'll use to compute the roots
        // q = -0.5 * (b +/- sqrt(d))
        code->appendf("\t\tfloat %s = -0.5 * (%s + (%s < 0.0 ? -1.0 : 1.0)"
                      " * sqrt(%s));\n", qName.c_str(), bVar.c_str(), 
                      bVar.c_str(), dName.c_str());

        // compute both roots
        // r0 = q * params[1]
        code->appendf("\t\tfloat %s = %s * %s;\n", r0Name.c_str(), 
                      qName.c_str(), p1.c_str());
        // r1 = c / q
        code->appendf("\t\tfloat %s = %s / %s;\n", r1Name.c_str(), 
                      cName.c_str(), qName.c_str());

        // Note: If there are two roots that both generate radius(t) > 0, the 
        // Canvas spec says to choose the larger t.

        // so we'll look at the larger one first:
        code->appendf("\t\tfloat %s = max(%s, %s);\n", tName.c_str(), 
                      r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0, then we're done; t will be our x coordinate
        code->appendf("\t\tif (%s * %s + %s > 0.0) {\n", tName.c_str(), 
                      p5.c_str(), p3.c_str());

        // y coord is 0.5 (texture is effectively 1D)
        code->appendf("\t\t");
        state->fSampleCoords.printf("vec2(%s, 0.5)", tName.c_str());
        state->emitDefaultFetch(outputColor, samplerName);

        // otherwise, if r(t) for the larger root was <= 0, try the other root
        code->appendf("\t\t} else {\n");
        code->appendf("\t\t\t%s = min(%s, %s);\n", tName.c_str(), 
                      r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0 for the smaller root, then t will be our x coordinate
        code->appendf("\t\t\tif (%s * %s + %s > 0.0) {\n",
                      tName.c_str(), p5.c_str(), p3.c_str());

        // y coord is 0.5 (texture is effectively 1D)
        code->appendf("\t\t\t");
        state->fSampleCoords.printf("vec2(%s, 0.5)", tName.c_str());
        state->emitDefaultFetch(outputColor, samplerName);

        // end if (r(t) > 0) for smaller root 
        code->appendf("\t\t\t}\n");
        // end if (r(t) > 0), else, for larger root
        code->appendf("\t\t}\n");
        // end if (discriminant >= 0)
        code->appendf("\t}\n");
    } else {

        // linear case: t = -c/b
        code->appendf("\tfloat %s = -(%s / %s);\n", tName.c_str(), 
                      cName.c_str(), bVar.c_str());

        // if r(t) > 0, then t will be the x coordinate
        code->appendf("\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                      p5.c_str(), p3.c_str());
        code->appendf("\t");
        state->fSampleCoords.printf("vec2(%s, 0.5)", tName.c_str());
        state->emitDefaultFetch(outputColor, samplerName);
        code->appendf("\t}\n");
    }
    state->fComplexCoord = true;
}

void GrGLConical2Gradient::initUniforms(const GrGLInterface* gl, int programID) {
    GR_GL_CALL_RET(gl, fVSParamLocation,
        GetUniformLocation(programID, fVSParamVar->getName().c_str()));
    GR_GL_CALL_RET(gl, fFSParamLocation,
        GetUniformLocation(programID, fFSParamVar->getName().c_str()));
}

void GrGLConical2Gradient::setData(const GrGLInterface* gl,
                                   const GrGLTexture& texture,
                                   const GrCustomStage& baseData,
                                   int stageNum) {
    const GrConical2Gradient& data =
        static_cast<const GrConical2Gradient&>(baseData);
    GrAssert(data.isDegenerate() == fIsDegenerate);
    GrScalar centerX1 = data.center();
    GrScalar radius0 = data.radius();
    GrScalar diffRadius = data.diffRadius();

    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedDiffRadius != diffRadius) {

        GrScalar a = GrMul(centerX1, centerX1) - diffRadius * diffRadius;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            GrScalarToFloat(a * 4),
            1.f / (GrScalarToFloat(a)),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(SkScalarMul(radius0, radius0)),
            GrScalarToFloat(diffRadius)
        };

        GR_GL_CALL(gl, Uniform1fv(fVSParamLocation, 6, values));
        GR_GL_CALL(gl, Uniform1fv(fFSParamLocation, 6, values));
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedDiffRadius = diffRadius;
    }
}


/////////////////////////////////////////////////////////////////////

GrConical2Gradient::GrConical2Gradient(GrScalar center,
                                       GrScalar radius,
                                       GrScalar diffRadius)
    : fCenterX1 (center)
    , fRadius0 (radius)
    , fDiffRadius (diffRadius) {

}

GrConical2Gradient::~GrConical2Gradient() {

}


const GrProgramStageFactory& GrConical2Gradient::getFactory() const {
    return GrTProgramStageFactory<GrConical2Gradient>::getInstance();
}

bool GrConical2Gradient::isEqual(const GrCustomStage& sBase) const {
    const GrConical2Gradient& s = static_cast<const GrConical2Gradient&>(sBase);
    return (this->isDegenerate() == s.isDegenerate());
}

/////////////////////////////////////////////////////////////////////


class GrGLSweepGradient : public GrGLProgramStage {

public:

    GrGLSweepGradient(const GrProgramStageFactory& factory,
                      const GrCustomStage&) : INHERITED (factory) { }
    virtual ~GrGLSweepGradient() { }

    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) { return 0; }

private:

    typedef GrGLProgramStage INHERITED;

};

void GrGLSweepGradient::emitFS(GrGLShaderBuilder* state,
                              const char* outputColor,
                              const char* inputColor,
                              const char* samplerName) {
    state->fSampleCoords.printf(
        "vec2(atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5, 0.5)",
        state->fSampleCoords.c_str(), state->fSampleCoords.c_str());
    state->fComplexCoord = true;

    state->emitDefaultFetch(outputColor, samplerName);
}

/////////////////////////////////////////////////////////////////////

GrSweepGradient::GrSweepGradient() {

}

GrSweepGradient::~GrSweepGradient() {

}

const GrProgramStageFactory& GrSweepGradient::getFactory() const {
    return GrTProgramStageFactory<GrSweepGradient>::getInstance();
}

bool GrSweepGradient::isEqual(const GrCustomStage& sBase) const {
    return true;
}

