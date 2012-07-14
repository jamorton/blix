#include "GrGpuGL.h"

#include "effects/GrConvolutionEffect.h"
#include "effects/GrGradientEffects.h"
#include "effects/GrMorphologyEffect.h"
#include "GrProgramStageFactory.h"
#include "GrRandom.h"

namespace {

// GrRandoms nextU() values have patterns in the low bits
// So using nextU() % array_count might never take some values.
int random_int(GrRandom* r, int count) {
    return (int)(r->nextF() * count);
}

// min is inclusive, max is exclusive
int random_int(GrRandom* r, int min, int max) {
    return (int)(r->nextF() * (max-min)) + min;
}

bool random_bool(GrRandom* r) {
    return r->nextF() > .5f;
}

typedef GrGLProgram::StageDesc StageDesc;
// TODO: Effects should be able to register themselves for inclusion in the
// randomly generated shaders. They should be able to configure themselves
// randomly.
GrCustomStage* create_random_effect(StageDesc* stageDesc,
                                    GrRandom* random) {
    enum EffectType {
        kConvolution_EffectType,
        kErode_EffectType,
        kDilate_EffectType,
        kRadialGradient_EffectType,
        kRadial2Gradient_EffectType,
        kSweepGradient_EffectType,

        kEffectCount
    };

    // TODO: Remove this when generator doesn't apply this non-custom-stage
    // notion to custom stages automatically.
    static const uint32_t kMulByAlphaMask =
        StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag |
        StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag;

    static const Gr1DKernelEffect::Direction gKernelDirections[] = {
        Gr1DKernelEffect::kX_Direction,
        Gr1DKernelEffect::kY_Direction
    };

    // TODO: When matrices are property of the custom-stage then remove the
    // no-persp flag code below.
    int effect = random_int(random, kEffectCount);
    switch (effect) {
        case kConvolution_EffectType: {
            int direction = random_int(random, 2);
            int kernelRadius = random_int(random, 1, 4);
            float kernel[GrConvolutionEffect::kMaxKernelWidth];
            for (int i = 0; i < GrConvolutionEffect::kMaxKernelWidth; i++) {
                kernel[i] = random->nextF();
            }
            // does not work with perspective or mul-by-alpha-mask
            stageDesc->fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            stageDesc->fInConfigFlags &= ~kMulByAlphaMask;
            return new GrConvolutionEffect(gKernelDirections[direction],
                                           kernelRadius,
                                           kernel);
            }
        case kErode_EffectType: {
            int direction = random_int(random, 2);
            int kernelRadius = random_int(random, 1, 4);
            // does not work with perspective or mul-by-alpha-mask
            stageDesc->fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            stageDesc->fInConfigFlags &= ~kMulByAlphaMask;
            return new GrMorphologyEffect(gKernelDirections[direction],
                                          kernelRadius,
                                          GrContext::kErode_MorphologyType);
            }
        case kDilate_EffectType: {
            int direction = random_int(random, 2);
            int kernelRadius = random_int(random, 1, 4);
            // does not work with perspective or mul-by-alpha-mask
            stageDesc->fOptFlags |= StageDesc::kNoPerspective_OptFlagBit;
            stageDesc->fInConfigFlags &= ~kMulByAlphaMask;
            return new GrMorphologyEffect(gKernelDirections[direction],
                                          kernelRadius,
                                          GrContext::kDilate_MorphologyType);
            }
        case kRadialGradient_EffectType: {
            return new GrRadialGradient();
            }
        case kRadial2Gradient_EffectType: {
            float center;
            do {
                center = random->nextF();
            } while (GR_Scalar1 == center);
            float radius = random->nextF();
            bool root = random_bool(random);
            return new GrRadial2Gradient(center, radius, root);
            }
        case kSweepGradient_EffectType: {
            return new GrSweepGradient();
            }
        default:
            GrCrash("Unexpected custom effect type");
    }
    return NULL;
}
}

bool GrGpuGL::programUnitTest() {

    // GrGLSLGeneration glslGeneration = 
            GrGetGLSLGeneration(this->glBinding(), this->glInterface());
    static const int STAGE_OPTS[] = {
        0,
        StageDesc::kNoPerspective_OptFlagBit,
    };
    static const int IN_CONFIG_FLAGS[] = {
        StageDesc::kNone_InConfigFlag,
        StageDesc::kSwapRAndB_InConfigFlag,
        StageDesc::kSwapRAndB_InConfigFlag |
        StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag,
        StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag,
        StageDesc::kSmearAlpha_InConfigFlag,
        StageDesc::kSmearRed_InConfigFlag,
    };
    GrGLProgram program;
    ProgramDesc& pdesc = program.fProgramDesc;

    static const int NUM_TESTS = 512;

    GrRandom random;
    for (int t = 0; t < NUM_TESTS; ++t) {

#if 0
        GrPrintf("\nTest Program %d\n-------------\n", t);
        static const int stop = -1;
        if (t == stop) {
            int breakpointhere = 9;
        }
#endif

        pdesc.fVertexLayout = 0;
        pdesc.fEmitsPointSize = random.nextF() > .5f;
        pdesc.fColorInput = random_int(&random, ProgramDesc::kColorInputCnt);
        pdesc.fCoverageInput = random_int(&random, ProgramDesc::kColorInputCnt);

        pdesc.fColorFilterXfermode = random_int(&random, SkXfermode::kCoeffModesCnt);

        pdesc.fFirstCoverageStage = random_int(&random, GrDrawState::kNumStages);

        pdesc.fVertexLayout |= random_bool(&random) ?
                                    GrDrawTarget::kCoverage_VertexLayoutBit :
                                    0;

#if GR_GL_EXPERIMENTAL_GS
        pdesc.fExperimentalGS = this->getCaps().fGeometryShaderSupport &&
                                random_bool(&random);
#endif
        pdesc.fOutputConfig =  random_int(&random, ProgramDesc::kOutputConfigCnt);

        bool edgeAA = random_bool(&random);
        if (edgeAA) {
            pdesc.fVertexLayout |= GrDrawTarget::kEdge_VertexLayoutBit;
            if (this->getCaps().fShaderDerivativeSupport) {
                pdesc.fVertexEdgeType = (GrDrawState::VertexEdgeType) random_int(&random, GrDrawState::kVertexEdgeTypeCnt);
            } else {
                pdesc.fVertexEdgeType = GrDrawState::kHairLine_EdgeType;
            }
        } else {
        }

        pdesc.fColorMatrixEnabled = random_bool(&random);

        if (this->getCaps().fDualSourceBlendingSupport) {
            pdesc.fDualSrcOutput = random_int(&random, ProgramDesc::kDualSrcOutputCnt);
        } else {
            pdesc.fDualSrcOutput = ProgramDesc::kNone_DualSrcOutput;
        }

        SkAutoTUnref<GrCustomStage> customStages[GrDrawState::kNumStages];

        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            StageDesc& stage = pdesc.fStages[s];
            // enable the stage?
            if (random_bool(&random)) {
                // use separate tex coords?
                if (random_bool(&random)) {
                    int t = random_int(&random, GrDrawState::kMaxTexCoords);
                    pdesc.fVertexLayout |= StageTexCoordVertexLayoutBit(s, t);
                }
                stage.setEnabled(true);
            }
            // use text-formatted verts?
            if (random_bool(&random)) {
                pdesc.fVertexLayout |= kTextFormat_VertexLayoutBit;
            }

            stage.fCustomStageKey = 0;

            stage.fOptFlags = STAGE_OPTS[random_int(&random, GR_ARRAY_COUNT(STAGE_OPTS))];
            stage.fInConfigFlags = IN_CONFIG_FLAGS[random_int(&random, GR_ARRAY_COUNT(IN_CONFIG_FLAGS))];

            bool useCustomEffect = random_bool(&random);
            if (useCustomEffect) {
                customStages[s].reset(create_random_effect(&stage, &random));
                if (NULL != customStages[s]) {
                    stage.fCustomStageKey =
                        customStages[s]->getFactory().glStageKey(*customStages[s]);
                }
            }
        }
        CachedData cachedData;
        GR_STATIC_ASSERT(sizeof(customStages) ==
                         GrDrawState::kNumStages * sizeof(GrCustomStage*));
        GrCustomStage** stages = reinterpret_cast<GrCustomStage**>(&customStages);
        if (!program.genProgram(this->glContextInfo(), stages, &cachedData)) {
            return false;
        }
        DeleteProgram(this->glInterface(), &cachedData);
    }
    return true;
}
