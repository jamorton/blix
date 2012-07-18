
#ifndef TESTS_TEST_H
#define TESTS_TEST_H

#include "Stage.h"
#include "Sprite.h"
#include "Bitmap.h"
#include "TextField.h"
#include "Engine.h"

class Test : public Stage
{
};

#define DEFINE_TEST(name) Test * make_##name () { return new name; }

#endif