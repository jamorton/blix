
#include "TextField.h"
#include "ResourceManager.h"
#include "Platform.h"
#include "Resource.h"
#include "skia.h"

// Simple wrapper that exposes SkTypeface as a Resource
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

void TextField::setAntialias(bool aa)
{
    if (aa) {
        _paint.setAntiAlias(true);
        _paint.setLCDRenderText(true);
    } else {
        _paint.setAntiAlias(false);
        _paint.setLCDRenderText(false);
    }
}

void TextField::text(const std::string& s)
{
    _str = s;
    _remeasure();
}

void TextField::color(Color color)
{
    _paint.setColor(color);
}

void TextField::size(uint size)
{
    _paint.setTextSize(size);
    _remeasure();
}

void TextField::_remeasure()
{
    // A DisplayObject's origin should be at the top left,
    // but text is drawn relative to the baseline, so we calculate
    // an offset.

    // We could just use _paint.measureText to find the text's height
    // from the baseline, but then the text's position could shift down
    // if a higher character was added, so we measure from the highest char

    SkPaint::FontMetrics metrics;
    // returns fDescent - fAscent + fLeading
    float height = _paint.getFontMetrics(&metrics);
    _offsetX = 0;
    _offsetY = -metrics.fAscent;

    SkRect measure;
    _paint.measureText(_str.data(), _str.size(), &measure);
    _bounds.set(0, 0, measure.width() + metrics.fXMax, height);
}

void TextField::_draw(Canvas * canvas)
{
    _paint.setAlpha(canvas->curAlpha());
    canvas->drawText(_str.c_str(), _str.size(), _offsetX, _offsetY, _paint);
}
