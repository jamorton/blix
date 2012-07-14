
#ifndef ENGINE_TEXT_H
#define ENGINE_TEXT_H

#include "common.h"
#include "DisplayObject.h"
#include "SkPaint.h"

class ShaderProgram;
class SkCanvas;

class _Typeface;

/**
 * Although it shares a name with flash.display.TextField, our TextField
 * does not share much of the same interface or functionality.
 *
 * Flash text rendering is extremely complex, full of quirks from old flash
 * versions, and has a bulky API. For now, we have a simple, lightweight
 * interface, although it is lacking in advanced functionality.
 */
class TextField : public DisplayObject
{
public:

    /**
     * @param face the name of a truetype text file in assets to use.
     *        Files are cached and not reloaded in each new TextField instance.
     *
     * @param size Text point size.
     */
    TextField(const std::string& face, uint size);
    ~TextField();

    inline const std::string& text() const { return _str; }
    void text(const std::string& s);

    void color(uint color = 0, float alpha = 1.0);

protected:

    void _draw(SkCanvas * canvas);

private:

    _Typeface * _typeface;
    std::string _str;
    SkPaint _paint;
    float _offsetX;
    float _offsetY;
};

#endif
