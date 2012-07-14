
#include "TextField.h"
#include "ResourceManager.h"
#include "Platform.h"
#include "Resource.h"
#include "skia.h"

class _Typeface : public Resource
{
public:
    REGISTER_RESOURCE(_Typeface);

    void load()
    {
        // TODO: SkMemoryStream should access file buffer directly
        // (mmap will work and no need for copy)
        std::string file;
        Platform::readFile(path(), &file);
        SkStream * stream = new SkMemoryStream(file.data(), file.size(), true);
        _tf = SkTypeface::CreateFromStream(stream);
        stream->unref();
    }

    void unload()
    {
        _tf->unref();
    }

    void set(SkPaint& paint)
    {
        paint.setTypeface(_tf);
    }

private:

    SkTypeface * _tf;

};

TextField::TextField(const std::string& face, uint size)
{
    _typeface = ResourceManager::load<_Typeface>(face);
    _typeface->set(_paint);
    _paint.setAntiAlias(true);
    _paint.setLCDRenderText(true);
    _paint.setTextSize(size);
    _paint.setTextAlign(SkPaint::kLeft_Align);
    _paint.setARGB(255, 0, 0, 0);
    text("");
}

TextField::~TextField()
{
    _typeface->release();
}

void TextField::text(const std::string& s)
{
    _str = s;
    // Skia sets (0, 0) as somewhere on the text baseline. We want
    // it to be the top left of the text
    _paint.measureText(_str.c_str(), _str.size(), &_bounds);
    _offsetX = -_bounds.left();
    _offsetY = -_bounds.top();
    _bounds.offset(_offsetX, _offsetY);
}

void TextField::color(Color color, float alpha)
{
    _paint.setARGB(
        floatToU8(alpha),
        (color >> 16) & 0xFF,
        (color >> 8)  & 0xFF,
        color & 0xFF);
}

void TextField::_draw(SkCanvas * canvas)
{
    canvas->drawText(_str.c_str(), _str.size(), _offsetX, _offsetY, _paint);
}
