
#ifndef ENGINE_BITMAP_H
#define ENGINE_BITMAP_H

#include "common.h"
#include "DisplayObject.h"

class BitmapData;

class Bitmap : public DisplayObject
{
public:

    Bitmap(const std::string& path);
    ~Bitmap();

protected:

    void _draw(Canvas * canvas);

private:

    BitmapData * _data;

};

#endif
