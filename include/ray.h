#ifndef __RAY_H__
#define __RAY_H__

#include "util.h"
namespace Tracer
{
struct Ray
{
    Point m_origin;
    Vector m_direction;
    float m_tMax;
    
    // Some sane defaults
    Ray()
        : m_origin(),
          m_direction(0.0f, 0.0f, 1.0f),
          m_tMax(kRayTMax)
    {
        
    }
    
    Ray(const Ray& r)
        : m_origin(r.m_origin),
          m_direction(r.m_direction),
          m_tMax(r.m_tMax)
    {
        
    }
    
    Ray(const Point& origin, const Vector& direction, float tMax = kRayTMax)
        : m_origin(origin),
          m_direction(direction),
          m_tMax(tMax)
    {
        
    }
    
    Ray& operator =(const Ray& r)
    {
        m_origin = r.m_origin;
        m_direction = r.m_direction;
        m_tMax = r.m_tMax;
        return *this;
    }
    
    Point calculate(float t) const { return m_origin + t * m_direction; }
};
}//namespace Tracer
#endif
