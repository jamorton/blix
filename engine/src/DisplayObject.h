
#ifndef ENGINE_DISPLAY_OBJECT_H
#define ENGINE_DISPLAY_OBJECT_H

#include "common.h"
#include "EventDispatcher.h"
#include "skia.h"

class Canvas;
class DisplayObjectContainer;

class DisplayObject : public EventDispatcher
{

private:

    // special properties implementation lets us detect changes in DisplayObject
    // values, without using getter and setter functions.
    // So we can have:
    //     foo.x += bar.x++;
    // Instead of:
    //     foo.x(foo.x() + bar.x()); bar.x(bar.x() + 1);
    class Property
    {
    public:
        Property(float val, DisplayObject * obj) : _val(val), _obj(obj) { }

#define INV _obj->_invalidateBounds()
#define DECL template <typename T> inline
        DECL float operator=(T v)       { INV; return _val = v; }
        DECL float operator+(T v) const { return _val + v; }
        DECL float operator-(T v) const { return _val - v; }
        DECL float operator*(T v) const { return _val * v; }
        DECL float operator/(T v) const { return _val / v; }
        DECL float operator+() const { return +_val; }
        DECL float operator-() const { return -_val; }
        DECL float operator++()          { INV; return ++_val; }
        DECL float operator++(int dummy) { INV; return _val++; }
        DECL float operator--()          { INV; return --_val; }
        DECL float operator--(int dummy) { INV; return _val--; }
        DECL bool operator==(T other) const { return _val == other; }
        DECL bool operator!=(T other) const { return _val != other; }
        DECL bool operator>(T other) const  { return _val >  other; }
        DECL bool operator<(T other) const  { return _val <  other; }
        DECL bool operator>=(T other) const { return _val >= other; }
        DECL bool operator<=(T other) const { return _val <= other; }
        DECL float operator+=(T v) { INV; return _val += v; }
        DECL float operator-=(T v) { INV; return _val -= v; }
        DECL float operator*=(T v) { INV; return _val *= v; }
        DECL float operator/=(T v) { INV; return _val /= v; }
        inline operator float()        { return _val; }
#undef DECL
#undef INV

    private:
        float _val;
        DisplayObject * _obj;
    };
    friend class Property;

public:

    DisplayObject() :
        x(0, this),
        y(0, this),
        rotation(0, this),
        scaleX(1, this),
        scaleY(1, this),
        regX(0, this),
        regY(0, this),
        alpha(1),
        visible(true),
        _parent(NULL), _boundsInvalid(true)
    {
        _bounds.set(0, 0, 0, 0);
    }

    virtual ~DisplayObject() { }

    virtual void update(Canvas * canvas);

    Property x, y, rotation, scaleX, scaleY, regX, regY;
    float alpha;
    bool visible;

    inline float width() { _recalcBounds(); return _bounds.width() * scaleX; }
    inline float height() { _recalcBounds(); return _bounds.height() * scaleX; }

    inline void width(float w) { scaleX = w / _bounds.width(); }
    inline void height(float h) { scaleY = h / _bounds.height(); }

    inline DisplayObjectContainer * parent() const { return _parent; }

protected:

    virtual void _draw(Canvas * canvas) { }

    const SkRect& bounds() { _recalcBounds(); return _bounds; }

    SkRect _bounds;
    bool   _boundsInvalid;

    void _invalidateBounds();
    virtual void _recalcBounds();
    void _drawBounds(Canvas * canvas);

    SkMatrix _transform;

    void _inverseTransform(SkMatrix& m);

private:

    friend class DisplayObjectContainer;
    DisplayObjectContainer * _parent;
    size_t _containerIndex;

    friend class Engine;
    virtual void _handleTouch(TouchEvent * event, SkMatrix * m);

};

#endif
