
#ifndef TESTS_TEST_H
#define TESTS_TEST_H

#include "Stage.h"

class Test : public Stage
{
};

#define DEFINE_TEST(name) Test * make_##name () { return new name; }

#endif
