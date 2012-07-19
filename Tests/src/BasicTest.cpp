
#include "Test.h"
#include "Sprite.h"
#include "Bitmap.h"
#include "TextField.h"

class BasicTest : public Test
{
public:
    
    TextField text;
    Bitmap bomb;
    Sprite s;
    
    BasicTest() :
        bomb("bomb.png"),
        text("Arial.ttf", 32)
    {
        addChild(&bomb);
        addChild(&text);
        text.text("Foo");
        text.x = text.y = 100;
    }
    
    
};

DEFINE_TEST(BasicTest);
