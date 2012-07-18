

#include "../SET_ACTIVE_TEST.h"
#include "Engine.h"
#include "test.h"

#ifndef SHOW_BOUNDS
#define SHOW_BOUNDS false
#endif

#ifndef ACTIVE_TEST
#define ACTIVE_TEST BasicTest
#endif

#define GET_TEST_FUNC2(name) make_##name ()
#define GET_TEST_FUNC(name) GET_TEST_FUNC2(name)

extern Test * GET_TEST_FUNC(ACTIVE_TEST);

Stage * createStage()
{
    Engine::DRAW_BOUNDS = SHOW_BOUNDS;
    return GET_TEST_FUNC(ACTIVE_TEST);
}
