
#include "Graphics.h"
#include "SkCanvas.h"
#include "SkPath.h"

//-----------------------------------------------------------
// Draw Commands
//-----------------------------------------------------------

void Graphics::_closeFill()
{
    Cmd cmd;
    // preserve last fill's paint
    cmd.paint = _cmds[_fillIdx].paint;

    SkPoint pt;
    _cmds[_fillIdx].path.getLastPt(&pt);
    cmd.path.moveTo(pt);

    _cmds[_fillIdx].path.close();
    _fillIdx = _cmds.size();
    _cmds.push_back(cmd);

    // Strokes need to appear after fills, so they are drawn above,
    // so we duplicate the current stroke style to a new cmd
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

        if (_hasStroke) {
            SkPoint pt;
            _cmds[_strokeIdx].path.getLastPt(&pt);
            cmd.path.moveTo(pt);
        }

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

    if (_hasFill) {
        SkPoint pt;
        _cmds[_strokeIdx].path.getLastPt(&pt);
        cmd.path.moveTo(pt);
    }

    _fillIdx = _cmds.size();
    _cmds.push_back(cmd);
    _hasFill = true;
}

void Graphics::endFill()
{
    _hasFill = false;
}

void Graphics::moveTo(float x, float y)
{
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
    if (_hasFill)
        _cmds[_fillIdx].path.lineTo(x, y);
    if (_hasStroke)
        _cmds[_strokeIdx].path.lineTo(x, y);
}

void Graphics::curveTo(float controlX, float controlY, float anchorX, float anchorY)
{

}

void Graphics::drawRect(float x, float y, float width, float height)
{
    if (_hasFill) {
        _closeFill();
        _cmds[_fillIdx].path.addRect(x, y, x + width, y + height);
    }

    if (_hasStroke)
        _cmds[_strokeIdx].path.addRect(x, y, x + width, y + height);
}

void Graphics::drawCircle(float x, float y, float radius)
{

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
}

void Graphics::draw(SkCanvas * canvas)
{
    for (int i = 0; i < _cmds.size(); i++)
        canvas->drawPath(_cmds[i].path, _cmds[i].paint);
}
