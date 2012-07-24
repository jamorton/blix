
#ifndef ENGINE_STAGE_H
#define ENGINE_STAGE_H

#include "DisplayObjectContainer.h"

class Stage : public DisplayObjectContainer
{

public:

    Stage() : _bgColor(0xFFFFFF) { }
    virtual ~Stage() { }

    void bgColor(uint c) { _bgColor = c; }
    Color bgColor() { return _bgColor; }

private:
    Color _bgColor;

};

#endif
