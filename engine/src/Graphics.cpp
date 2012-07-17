
#include "Graphics.h"
#include "SkCanvas.h"
#include "SkPath.h"

/**
 * Calculate the exact bounds of a quadratic bezier.
 * From the LEMON Graph Library
 * (http://lemon.cs.elte.hu/trac/lemon)
 */
static inline SkRect _bezierBounds(float x1, float y1,
    float x2, float y2, float x3, float y3)
{
    float minx, miny;
    float maxx, maxy;
    if (x1 < x3) {
        minx = x1; maxx = x3;
    } else {
        minx = x3; maxx = x1;
    }
    if (y1 < y3) {
        miny = y1; maxy = y3;
    } else {
        miny = y3; maxy = y1;
    }
    if (x1 + x3 - 2 * x2 != 0.0) {
        float t = (x1 - x2) / (x1 + x3 - 2 * x2);
        if (t >= 0.0 && t <= 1.0) {
            float x = ((1-t)*(1-t))*x1+(2*(1-t)*t)*x2+(t*t)*x3;
            if (x < minx) minx = x;
            if (x > maxx) maxx = x;
        }
    }
    if (y1 + y3 - 2 * y2 != 0.0) {
        float t = (y1 - y2) / (y1 + y3 - 2 * y2);
        if (t >= 0.0 && t <= 1.0) {
            float y = ((1-t)*(1-t))*y1+(2*(1-t)*t)*y2+(t*t)*y3;
            if (y < miny) miny = y;
            if (y > maxy) maxy = y;
        }
    }
    SkRect r;
    r.set(minx, miny, maxx, maxy);
    return r;
}

//-----------------------------------------------------------
// Draw Commands
//-----------------------------------------------------------

void Graphics::_closeFill()
{
    Cmd cmd;
    // preserve last fill's paint
    cmd.paint = _cmds[_fillIdx].paint;
    cmd.path.moveTo(_lastX, _lastY);

    _cmds[_fillIdx].path.close();
    _fillIdx = _cmds.size();
    _cmds.push_back(cmd);

    // Strokes need to appear after fills so that they are drawn above.
    // Duplicate the current stroke style to a new cmd at the front of
    // the list
    if (_hasStroke) {
        Cmd cmd;
        cmd.paint = _cmds[_strokeIdx].paint;
        _strokeIdx = _cmds.size();
        _cmds.push_back(cmd);
    }
}

void Graphics::lineStyle(float thickness, Color color, float alpha,
    CapStyle cap, JointStyle joint, float miter)
{
    if (thickness < 0)
        _hasStroke = false;
    else {
        Cmd cmd;
        cmd.paint.setAntiAlias(true);
        cmd.paint.setStyle(SkPaint::kStroke_Style);
        cmd.paint.setStrokeWidth(thickness);
        cmd.paint.setARGB(
            floatToU8(alpha),
            (color >> 16) & 0xFF,
            (color >> 8)  & 0xFF,
            color & 0xFF);
        cmd.paint.setStrokeCap((SkPaint::Cap)cap);
        cmd.paint.setStrokeJoin((SkPaint::Join)joint);
        cmd.paint.setStrokeMiter(miter);

        cmd.path.moveTo(_lastX, _lastY);

        _strokeIdx = _cmds.size();
        _cmds.push_back(cmd);
        _hasStroke = true;
    }
}

void Graphics::beginFill(Color color, float alpha)
{
    if (_hasFill)
        _cmds[_fillIdx].path.close();

    Cmd cmd;
    cmd.paint.setColor(color);
    cmd.paint.setAlpha(floatToU8(alpha));
    cmd.paint.setStyle(SkPaint::kFill_Style);

    cmd.path.moveTo(_lastX, _lastY);

    _fillIdx = _cmds.size();
    _cmds.push_back(cmd);
    _hasFill = true;

    // Strokes need to appear after fills so that they are drawn above.
    // Duplicate the current stroke style to a new cmd at the front of
    // the list
    if (_hasStroke) {
        Cmd cmd;
        cmd.paint = _cmds[_strokeIdx].paint;
        _strokeIdx = _cmds.size();
        _cmds.push_back(cmd);
    }
}

void Graphics::endFill()
{
    if (_hasFill)
        _closeFill();
    _hasFill = false;
}

void Graphics::moveTo(float x, float y)
{
    _lastX = x;
    _lastY = y;

    if (_hasFill) {
        _closeFill();
        _cmds[_fillIdx].path.moveTo(x, y);
    }
    if (_hasStroke)
        _cmds[_strokeIdx].path.moveTo(x, y);
    /*
    _ensureValidFill();
    DrawCmd cmd(DrawCmd::TYPE_MOVE_TO);
    cmd.floats.f1 = x;
    cmd.floats.f2 = y;
    _cmds.push_back(cmd);
    */
}

void Graphics::lineTo(float x, float y)
{
    _addBounds(_lastX, _lastY, x, y);
    _lastX = x;
    _lastY = y;
    if (_hasFill)
        _cmds[_fillIdx].path.lineTo(x, y);
    if (_hasStroke)
        _cmds[_strokeIdx].path.lineTo(x, y);
}

void Graphics::curveTo(float controlX, float controlY, float anchorX, float anchorY)
{
    _addBounds(_bezierBounds(_lastX, _lastY,
            controlX, controlY, anchorX, anchorY));
    _lastX = anchorX;
    _lastY = anchorY;
    if (_hasFill)
        _cmds[_fillIdx].path.quadTo(controlX, controlY, anchorX, anchorY);
    if (_hasStroke)
        _cmds[_strokeIdx].path.quadTo(controlX, controlY, anchorX, anchorY);
    _invCallback();
}

void Graphics::drawRect(float x, float y, float width, float height)
{
    _addBounds(x, y, x + width, y + height);
    _lastX = x;
    _lastY = y;

    if (_hasFill) {
        _closeFill();
        _cmds[_fillIdx].path.addRect(x, y, x + width, y + height);
    }
    if (_hasStroke)
        _cmds[_strokeIdx].path.addRect(x, y, x + width, y + height);
}

void Graphics::drawCircle(float x, float y, float radius)
{
    _addBounds(x - radius, y - radius, x + radius, y + radius);
    _lastX = x + radius;
    _lastY = y;
    if (_hasFill) {
        _closeFill();
        _cmds[_fillIdx].path.addCircle(x, y, radius);
    }
    if (_hasStroke)
        _cmds[_strokeIdx].path.addCircle(x, y, radius);
}

void Graphics::drawEllipse(float x, float y, float width, float height)
{
}

void Graphics::drawRoundRect(float x, float y, float width, float height,
    float ellipseWidth, float ellipseHeight)
{
}

void Graphics::clear()
{
    _cmds.clear();
    _hasStroke = false;
    _hasFill = false;
    _bounds.setEmpty();
    _lastX = 0;
    _lastY = 0;
    _invCallback();
}

void Graphics::draw(SkCanvas * canvas)
{
    for (int i = 0; i < _cmds.size(); i++)
        canvas->drawPath(_cmds[i].path, _cmds[i].paint);
}
