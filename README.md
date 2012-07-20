
## Blix

blix is a cross-platform 2d game engine for iOS and Android, built in C++.

Its API is built around a port of Flash's AS3 API, including an
implementation of its display list and event system.

## What does it look like?

```C++
#include "Stage.h"
#include "Sprite.h"
#include "Bitmap.h"
#include "TextField.h"

class Game : public Stage
{

public:

    TextField text;
    Bitmap bomb;
    Sprite s;

    Game() :
        bomb("bomb.png"),
        text("Arial.ttf", 32)
    {

        addChild(&bomb);
        addChild(&text);

        text.text("Explosions!");

        text.x = 50;
        text.y = 50;

        // put the bomb to the right of the text
        bomb.x = text.x + text.width() + 50;
        bomb.y = text.y + text.height() / 2.0;

        addChild(&s);

        // draw a box with a line through it.
        s.graphics.beginFill(0xABCDEF);
        s.graphics.drawRect(100, 200, 50, 50);
        s.graphics.lineStyle(3, 0);
        s.graphics.moveTo(100, 200);
        s.graphics.lineTo(150, 250);
    }
};

// The engine will call this at startup
Stage * createStage()
{
    return new Game;
}
```
