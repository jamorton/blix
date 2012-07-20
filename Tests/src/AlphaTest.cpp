
#include "Test.h"
#include "Sprite.h"
#include "Bitmap.h"
#include "TextField.h"
#include "Engine.h"

class AlphaTest : public Test
{
public:
    
    Sprite s[10];
    Bitmap bomb, bomb2, bomb3;
    TextField text;
    
    AlphaTest() :
    bomb("bomb.png"), bomb2("bomb.png"), bomb3("bomb.png"),
    text("Arial.ttf", 24)
    {
        Engine::DRAW_BOUNDS = false;
        
        // Test display tree alpha concatenation.
        DisplayObjectContainer * parent = this;
        for (int i = 0; i < 10; i++) {
            s[i].graphics.beginFill(0x0000FF);
            s[i].graphics.drawRect(0, 0, 30, 50);
            s[i].x = 30;
            s[i].alpha = 0.8;
            parent->addChild(&s[i]);
            parent = &s[i];
        }
        
        s[0].x = 10;
        s[0].y = 10;
        
        addChild(&bomb);
        addChild(&bomb2);
        addChild(&bomb3);
        
        bomb.x = 50;
        bomb2.x = 150;
        bomb3.x = 250;
        bomb.y = bomb2.y = bomb3.y = 100;
        
        bomb2.alpha = 0.5;
        bomb3.alpha = 0.25;
        
        addChild(&text);
        text.x = 10;
        text.y = 200;
        text.text("ABCDEFG 123 magic!");
        addEventListener(Event::ENTER_FRAME, methodListener(AlphaTest::frame));
    }
    
    void frame(Event * e)
    {
        text.alpha = (sin((float)Platform::getTime() * 2) + 1) / 2.0;
    }
    
};

DEFINE_TEST(AlphaTest);
