
#ifndef ENGINE_BITMAP_DATA_H
#define ENGINE_BITMAP_DATA_H

#include "common.h"
#include "Resource.h"

#include "SkBitmap.h"

class BitmapData : public Resource
{

public:

    REGISTER_RESOURCE(BitmapData);

    void load();
    void unload();

    inline const SkBitmap& bitmap() const { return _bitmap; }

private:

    SkBitmap _bitmap;

};


#endif
