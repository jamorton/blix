
#ifndef ENGINE_GRAPHICS_H
#define ENGINE_GRAPHICS_H

#include "common.h"
#include "SkRect.h"
#include "SkPicture.h"
#include "SkPaint.h"
#include "SkPath.h"
#include <vector>

/**
 * The graphics object is almost identical to flash's, with a few
 * small parameter/technical differences.
 *
 *
 *
 */
class SkCanvas;

class Graphics
{

public:

    Graphics() : _hasFill(false), _hasStroke(false)
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
};

#endif
