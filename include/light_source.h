#ifndef __LIGHT_SOURCE_H__
#define __LIGHT_SOURCE_H__

#include "util.h"
#include "shape.h"

namespace Tracer
{
class Light : public virtual Shape
{
public:
	Light(const Color& c,float power) : m_color(c), m_power(power) 
	{
	m_shapeType="Light";
	}	
	virtual ~Light() {}
	virtual Color emitted() const {return m_color * m_power; }
	virtual bool samplePoint(Rng rng,
							 Point& position,
							 Point& lightPostion,
							 Vector& lightNormal) {}
protected:
	Color m_color;
	float m_power;
};

class RectangleLight : public Light, public Rectangle
{
public:
	RectangleLight(const Point& pos,
                   const Vector& side1,
                   const Vector& side2,
                   const Material* pMaterial,
                   float power)
        : Light(pMaterial->m_color, power),Rectangle(pos,side1,side2,Color(0,0,0)),m_pMaterial(pMaterial)	
		{
			m_shapeType="RectangleLight";
		}   
    virtual ~RectangleLight() {}
	virtual bool intersect(Intersection& intersection)
	{
		if(!Rectangle::intersect(intersection)) 
			return false;
		intersection.m_emitted = emitted();
		//intersection.m_color = Color();	
		intersection.m_pMaterial = m_pMaterial;
		if (dot(intersection.m_normal, intersection.m_ray.m_direction) > 0.0f)
        {
            intersection.m_normal *= -1.0f;
        }
		return true;
	}
	virtual  bool samplePoint(Rng rng,
							  Point& position,
							  Point& lightPosition,
							  Vector& lightNormal)	
	{
		float u1 = rng.nextFloat();
		float u2 = rng.nextFloat();
		lightNormal = cross(m_side1, m_side2).normalized();
        lightPosition = m_position + m_side1 * u1 + m_side2 * u2;
        
		if (dot(lightNormal, lightPosition - position) > 0.0f)
        {
            lightNormal *= -1.0f;
        }
        return true;
	}
protected:
	const Material* m_pMaterial;
};





}// namespace Tracer

#endif
