#ifndef VRLE_H
#define VRLE_H

#include <vector>
#include "vcowptr.h"
#include "vglobal.h"
#include "vpoint.h"
#include "vrect.h"

V_BEGIN_NAMESPACE

class VRle {
public:
    struct Span {
        short  x;
        short  y;
        ushort len;
        uchar  coverage;
    };
    typedef void (*VRleSpanCb)(int count, const VRle::Span *spans,
                               void *userData);
    bool  isEmpty() const;
    VRect boundingRect() const;
    void  addSpan(const VRle::Span *span, int count);

    void reset();
    void translate(const VPoint &p);
    void invert();

    void operator*=(int alpha);

    void intersect(const VRect &r, VRleSpanCb cb, void *userData) const;

    VRle operator&(const VRle &o) const;
    VRle operator-(const VRle &o) const;
    VRle operator+(const VRle &o) const;

    static VRle toRle(const VRect &rect);

private:
    struct VRleData {
        bool  isEmpty() const { return mSpans.empty(); }
        void  addSpan(const VRle::Span *span, int count);
        void  updateBbox() const;
        VRect bbox() const;
        void  reset();
        void  translate(const VPoint &p);
        void  operator*=(int alpha);
        void  invert();
        void  opIntersect(const VRect &, VRle::VRleSpanCb, void *) const;
        void  opAdd(const VRle::VRleData &, const VRle::VRleData &);
        void  opIntersect(const VRle::VRleData &, const VRle::VRleData &);
        void  addRect(const VRect &rect);
        std::vector<VRle::Span> mSpans;
        VPoint                  mOffset;
        mutable VRect           mBbox;
        mutable bool            mBboxDirty = true;
    };

    vcow_ptr<VRleData> d;
};

inline bool VRle::isEmpty() const
{
    return d->isEmpty();
}

inline void VRle::addSpan(const VRle::Span *span, int count)
{
    d.write().addSpan(span, count);
}

inline VRect VRle::boundingRect() const
{
    return d->bbox();
}

inline void VRle::invert()
{
    d.write().invert();
}

inline void VRle::operator*=(int alpha)
{
    d.write() *= alpha;
}

inline VRle VRle::operator&(const VRle &o) const
{
    if (isEmpty() || o.isEmpty()) return VRle();

    VRle result;
    result.d.write().opIntersect(d.read(), o.d.read());

    return result;
}

inline VRle VRle::operator+(const VRle &o) const
{
    if (isEmpty()) return o;
    if (o.isEmpty()) return *this;

    VRle result;
    if (d->bbox().top() < o.d->bbox().top())
        result.d.write().opAdd(d.read(), o.d.read());
    else
        result.d.write().opAdd(o.d.read(), d.read());

    return result;
}

inline VRle VRle::operator-(const VRle &o) const
{
    if (o.isEmpty()) return *this;

    VRle result = o;
    result.invert();

    if (isEmpty())
        return result;
    else
        return *this + result;
}

inline void VRle::reset()
{
    d.write().reset();
}

inline void VRle::translate(const VPoint &p)
{
    d.write().translate(p);
}

inline void VRle::intersect(const VRect &r, VRleSpanCb cb, void *userData) const
{
    d->opIntersect(r, cb, userData);
}

V_END_NAMESPACE

#endif  // VRLE_H
