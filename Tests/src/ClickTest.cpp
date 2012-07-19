
#include "Test.h"
#include "Sprite.h"
#include "Bitmap.h"
#include "TextField.h"

class ClickTest : public Test
{
    
public:
    
    TextField text;
    Bitmap bomb;
    Sprite s;
    
    ClickTest() :
        bomb("bomb.png"),
        text("Arial.ttf", 32)
    {
        
        addChild(&s);
        s.addChild(&bomb);
        s.addChild(&text);
        s.x = 125;
        s.y = 75;
        s.rotation = 45;
        text.text("Foo");
        text.x = text.y = 100;
        text.scaleX = 2;
        addEventListener(TouchEvent::DOWN, methodListener(ClickTest::click));
        bomb.addEventListener(TouchEvent::DOWN, methodListener(ClickTest::click));
        text.addEventListener(TouchEvent::DOWN, methodListener(ClickTest::click));
    }
    
    void click(Event * e)
    {
        TouchEvent * touch = e->get<TouchEvent>();
        if (touch->target() == &bomb)
            DBG_LOG("BOMB %f %f", touch->localX(), touch->localX());
        else if (touch->target() == &text)
            DBG_LOG("TEXT %f %f", touch->localX(), touch->localX());
        else
            DBG_LOG("OTHER %f %f", touch->localX(), touch->localX());
    }
    
};

DEFINE_TEST(ClickTest);
