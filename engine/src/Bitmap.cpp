
#include "Bitmap.h"
#include "BitmapData.h"
#include "ResourceManager.h"

#include "SkCanvas.h"
#include "SkImageDecoder.h"

static SkPaint _bitmapPaint;

Bitmap::Bitmap(const std::string& path)
{
    _data = ResourceManager::load<BitmapData>(path);
    _bounds.set(0, 0, _data->bitmap().width(), _data->bitmap().height());
    _bitmapPaint.setFilterBitmap(true);
}

Bitmap::~Bitmap()
{
    _data->release();
}

void Bitmap::_draw(Canvas * canvas)
{
    _bitmapPaint.setAlpha(canvas->curAlpha());
    canvas->drawBitmap(
        _data->bitmap(),
        x - round(x), y - round(y), // snap to nearest pixel
        &_bitmapPaint);
}
