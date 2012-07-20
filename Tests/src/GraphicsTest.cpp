
#include "Test.h"
#include "Sprite.h"
#include "Bitmap.h"
#include "TextField.h"

class GraphicsTest : public Test
{
public:
    
    Sprite s;
    
    GraphicsTest()
    {
        addChild(&s);
        
        s.graphics.lineStyle(4, 0);
        
        s.graphics.drawRoundRect(50, 50, 100, 100, 20, 20);
        s.graphics.lineTo(0, 0);
        
        s.graphics.drawCircle(200, 200, 50);
        s.graphics.lineTo(300, 300);
    }
};

DEFINE_TEST(GraphicsTest);

