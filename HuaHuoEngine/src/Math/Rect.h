#pragma once

#include "Math/Vector2f.h"
#include "Math/Vector3f.h"
#include "Math/Matrix4x4.h"
#include "Modules/ExportModules.h"

/// A rectangle.
template<typename T>
class EXPORT_COREMODULE RectT
{
public:
    typedef RectT<T> RectType;
    typedef float BaseType;

    T x; ///< Rectangle x coordinate.
    T y; ///< Rectangle y coordinate.
    T width; ///< Rectangle width.
    T height; ///< Rectangle height.

    //DECLARE_SERIALIZE_OPTIMIZE_TRANSFER (Rectf)
    inline static const char* GetTypeString();
    inline static bool MightContainPPtr() { return false; }
    inline static bool AllowTransferOptimization() { return true; }
    template<class TransferFunction>
    void Transfer(TransferFunction& transfer)
    {
        TRANSFER(x);
        TRANSFER(y);
        TRANSFER(width);
        TRANSFER(height);
    }

    /// Create a empty rectangle.
    RectT()
    {
        Reset();
    }

    /// Create a new rectangle.
    RectT(T inX, T inY, T iWidth, T iHeight)
    {
        x = inX; width = iWidth;
        y = inY; height = iHeight;
    }

    // Top/Bottom assume (0,0) is top left like in Direct3D.
    // In OpenGL the y coordinates are inverted, so Top/Bottom is flipped.
    T GetRight() const { return x + width; }
    T GetBottom() const { return y + height; }
    void SetLeft(T l) { T oldXMax = GetXMax(); x = l; width = oldXMax - x; }
    void SetTop(T t) { T oldYMax = GetYMax(); y = t; height = oldYMax - y; }
    void SetRight(T r) { width = r - x; }
    void SetBottom(T b) { height = b - y; }


    T GetXMax() const { return x + width; }
    T GetYMax() const { return y + height; }

    /// Return true if rectangle is empty.
    inline bool IsEmpty() const { return width <= 0 || height <= 0; }

    inline void     SetPosition(const Vector2f& position) { x = position.x; y = position.y; }
    inline Vector2f GetPosition() const { return Vector2f(x, y); }

    inline void     SetSize(const Vector2f& size) { width = size.x; height = size.y; }
    inline Vector2f GetSize() const { return Vector2f(width, height); }
    /// Resets the rectangle
    inline void Reset() { x = y = width = height = 0; }

    /// Sets the rectangle
    inline void Set(T inX, T inY, T iWidth, T iHeight)
    {
        x = inX; width = iWidth;
        y = inY; height = iHeight;
    }

    inline void Scale(T dx, T dy)      { x *= dx; width *= dx; y *= dy; height *= dy; }

    /// Set Center position of rectangle (size stays the same)
    void SetCenterPos(T cx, T cy)      { x = cx - width / 2; y = cy - height / 2; }
    Vector2f GetCenterPos() const       { return Vector2f(x + (BaseType)width / 2, y + (BaseType)height / 2); }

    /// Ensure this is inside the rect r.
    void Clamp(const RectType &r)
    {
        T x2 = x + width;
        T y2 = y + height;
        T rx2 = r.x + r.width;
        T ry2 = r.y + r.height;

        if (x < r.x) x = r.x;
        if (x2 > rx2) x2 = rx2;
        if (y < r.y) y = r.y;
        if (y2 > ry2) y2 = ry2;

        width = x2 - x;
        if (width < 0) width = 0;

        height = y2 - y;
        if (height < 0) height = 0;
    }

    /// Move rectangle by deltaX, deltaY.
    inline void Move(T dX, T dY)       { x += dX; y += dY; }

    /// Return the width of rectangle.
    inline T Width() const                 { return width; }

    /// Return the height of rectangle.
    inline T Height() const                    { return height; }

    /// Return true if a point lies within rectangle bounds.
    inline bool Contains(T px, T py) const     { return (px >= x) && (px < x + width) && (py >= y) && (py < y + height); }
    inline bool Contains(const Vector2f& p) const      { return Contains(p.x, p.y); }
    /// Return true if a relative point lies within rectangle bounds.
    inline bool ContainsRel(T x, T y) const
    { return (x >= 0) && (x < Width()) && (y >= 0) && (y < Height()); }

    inline bool Intersects(const RectType& r) const
    {
        // Rects are disjoint if there's at least one separating axis
        bool disjoint = x + width < r.x;
        disjoint |= r.x + r.width < x;
        disjoint |= y + height < r.y;
        disjoint |= r.y + r.height < y;
        return !disjoint;
    }

    /// Normalize a rectangle such that xmin <= xmax and ymin <= ymax.
    inline void Normalize()
    {
        width = std::max<T>(width, 0);
        height = std::max<T>(height, 0);
    }

    bool operator==(const RectType& r) const       { return x == r.x && y == r.y && width == r.width && height == r.height; }
    bool operator!=(const RectType& r) const       { return x != r.x || y != r.y || width != r.width || height != r.height; }

    // not meaningful; only for use with associative containers
    bool operator<(const RectType& r) const
    {
        if (x != r.x)
            return x < r.x;
        else if (y != r.y)
            return y < r.y;
        else if (width != r.width)
            return width < r.width;
        else
            return height < r.height;
    }
};

typedef RectT<float> Rectf;
// BIND_MANAGED_TYPE_NAME(Rectf, UnityEngine_Rect);
typedef RectT<int> RectInt;
// BIND_MANAGED_TYPE_NAME(RectInt, UnityEngine_RectInt);

template<> inline const char* Rectf::GetTypeString() { return "Rectf"; }
template<> inline const char* RectInt::GetTypeString() { return "RectInt"; }

inline bool CompareApproximately(const Rectf& lhs, const Rectf& rhs)
{
    return CompareApproximately(lhs.x, rhs.x) && CompareApproximately(lhs.y, rhs.y) &&
        CompareApproximately(lhs.width, rhs.width) && CompareApproximately(lhs.height, rhs.height);
}

/// Make a rect with width & height
template<typename T>
inline RectT<T> MinMaxRect(T minx, T miny, T maxx, T maxy) { return RectT<T>(minx, miny, maxx - minx, maxy - miny); }

/// Compute the intersection between two rects
template<typename T>
inline RectT<T> IntersectRects(const RectT<T>& lhs, const RectT<T>& rhs)
{
    if (lhs.IsEmpty() || rhs.IsEmpty())
        return RectT<T>();

    T minx = std::max(lhs.x, rhs.x);
    T miny = std::max(lhs.y, rhs.y);
    T maxx = std::min(lhs.x + lhs.width, rhs.x + rhs.width);
    T maxy = std::min(lhs.y + lhs.height, rhs.y + rhs.height);
    return MinMaxRect(minx, miny, maxx, maxy);
}

template<typename T>
inline RectT<T> UnionRects(const RectT<T>& lhs, const RectT<T>& rhs)
{
    if (lhs.IsEmpty())
        return rhs;

    if (rhs.IsEmpty())
        return lhs;

    T minx = std::min(lhs.x, rhs.x);
    T miny = std::min(lhs.y, rhs.y);
    T maxx = std::max(lhs.x + lhs.width, rhs.x + rhs.width);
    T maxy = std::max(lhs.y + lhs.height, rhs.y + rhs.height);
    return MinMaxRect(minx, miny, maxx, maxy);
}

/// Compute the rect "AABB" from the given a transform
inline Rectf TransformRectAABB(const Rectf& r, const Matrix4x4f& mat)
{
    Vector3f v0 = mat.MultiplyPoint3(Vector3f(r.x, r.y, 0.0f));
    Vector3f v1 = mat.MultiplyPoint3(Vector3f(r.x + r.width, r.y, 0.0f));
    Vector3f v2 = mat.MultiplyPoint3(Vector3f(r.x, r.y + r.height, 0.0f));
    Vector3f v3 = mat.MultiplyPoint3(Vector3f(r.x + r.width, r.y + r.height, 0.0f));
    return MinMaxRect(
        std::min(v0.x, std::min(v1.x, std::min(v2.x, v3.x))),
        std::min(v0.y, std::min(v1.y, std::min(v2.y, v3.y))),
        std::max(v0.x, std::max(v1.x, std::max(v2.x, v3.x))),
        std::max(v0.y, std::max(v1.y, std::max(v2.y, v3.y))));
}

// RectT<float> specialization
template<>
inline bool Rectf::IsEmpty() const { return width <= 0.00001F || height <= 0.00001F; }

template<>
template<class TransferFunction> inline
void Rectf::Transfer(TransferFunction& transfer)
{
    transfer.SetVersion(2);

    TRANSFER(x);
    TRANSFER(y);
    TRANSFER(width);
    TRANSFER(height);

    #if UNITY_EDITOR
    if (transfer.IsOldVersion(1))
    {
        float xmax = 0.0F, ymax = 0.0F, ymin = 0.0F, xmin = 0.0F;
        TRANSFER(xmin);
        TRANSFER(ymin);
        TRANSFER(xmax);
        TRANSFER(ymax);

        *this = MinMaxRect(xmin, ymin, xmax, ymax);
    }
    #endif
}

// BIND_MANAGED_TYPE_NAME(Rectf, UnityEngine_Rect)
