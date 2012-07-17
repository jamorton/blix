
#ifndef ENGINE_GRAPHICS_H
#define ENGINE_GRAPHICS_H

#include "common.h"
#include "skia.h"
#include <vector>

/**
 * The graphics object is almost identical to flash's, with a few
 * small parameter/technical differences.
 *
 *
 *
 */
class SkCanvas;
static void _dummyInv() { }

class Graphics
{

public:

    typedef std::function<void ()> InvalidateCallback;

    Graphics(InvalidateCallback inv = &_dummyInv) :
        _hasFill(false), _hasStroke(false),
        _invCallback(inv)
    {
        _bounds.setEmpty();
    }
    ~Graphics() { }

    // These must be listed in the same order as in SkPaint::Cap and SkPaint::Joint
    enum CapStyle {
        CAP_STYLE_NONE,
        CAP_STYLE_ROUND,
        CAP_STYLE_SQUARE
    };

    enum JointStyle {
        JOINT_STYLE_MITER,
        JOINT_STYLE_ROUND,
        JOINT_STYLE_BEVEL
    };

    /**
     * Returns the smallest rectangle which surrounds all drawn graphics
     * in this object
     */
    const SkRect& bounds() const { return _bounds; }

    void lineStyle(
        float thickness = -1, Color color = 0, float alpha = 1,
        CapStyle cap = CAP_STYLE_ROUND,
        JointStyle joint = JOINT_STYLE_ROUND,
        float miter = 3.0f);

    void beginFill(Color color, float alpha = 1.0f);
    void endFill();

    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void curveTo(float controlX, float controlY, float anchorX, float anchorY);
    void drawRect(float x, float y, float width, float height);
    void drawCircle(float x, float y, float radius);
    void drawEllipse(float x, float y, float width, float height);
    void drawRoundRect(float x, float y, float width, float height, float ellipseWidth, float ellipseHeight = -1);

    void clear();

    void draw(SkCanvas * canvas);

private:

    void _closeFill();

    struct Cmd {
        SkPath path;
        SkPaint paint;
    };

    std::vector<Cmd> _cmds;

    uint _strokeIdx;
    uint _fillIdx;

    bool _hasFill;
    bool _hasStroke;

    SkRect _bounds;

    float _lastX;
    float _lastY;

    InvalidateCallback _invCallback;

    inline void _addBounds(const SkRect& r)
    {
        _addBounds(r.left(), r.top(), r.right(), r.bottom());
    }

    inline void _addBounds(float x1, float y1, float x2, float y2)
    {
        SkRect r, storage;
        r.set(x1, y1, x2, y2);
        r.sort();
        if (_hasStroke)
            if (_cmds[_strokeIdx].paint.canComputeFastBounds())
                r = _cmds[_strokeIdx].paint.computeFastBounds(r, &storage);
        _bounds.join(r);
        _invCallback();
    }
};

#endif
