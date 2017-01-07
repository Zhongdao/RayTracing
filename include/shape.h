#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "util.h"
#include "ray.h"
#include "material.h"

namespace Tracer
{
class Shape;

struct Intersection
{
    Ray m_ray;
    float m_t;
    Shape *m_pShape;
    const Material *m_pMaterial;
    Color m_emitted;
    Vector m_normal;
    
    
    Intersection()
        : m_ray(),
          m_t(kRayTMax),
          m_pShape(NULL),
          m_pMaterial(NULL),
          m_emitted(),
          m_normal()
    {
        
    }
    
    Intersection(const Intersection& i)
        : m_ray(i.m_ray),
          m_t(i.m_t),
          m_pShape(i.m_pShape),
          m_pMaterial(i.m_pMaterial),
          m_emitted(i.m_emitted),
          m_normal(i.m_normal)
    {
        
    }
    
    Intersection(const Ray& ray)
         : m_ray(ray),
           m_t(ray.m_tMax),
           m_pShape(NULL),
           m_pMaterial(NULL),
           m_emitted(),
           m_normal()
    {
        
    }
    
    Intersection& operator =(const Intersection& i)
    {
        m_ray = i.m_ray;
        m_t = i.m_t;
        m_pShape = i.m_pShape;
        m_pMaterial = i.m_pMaterial;
        m_emitted = i.m_emitted;
        m_normal = i.m_normal;
        return *this;
    }
    
    bool intersected() const { return (m_pShape == NULL) ? false : true; }
    
    Point position() const { return m_ray.calculate(m_t); }
};


class Shape
{
public:
    virtual ~Shape() { }
    
    // Subclasses must implement this; this is the meat of ray tracing
    virtual bool intersect(Intersection& intersection) = 0;
	std::string getShapeType(){return m_shapeType;}
protected:
	std::string m_shapeType;
};


// List of shapes, so you can aggregate a pile of them
class ShapeSet : public Shape
{
public:
    virtual ~ShapeSet() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        bool intersectedAny = false;
        for (std::list<Shape*>::iterator iter = m_shapes.begin();
             iter != m_shapes.end();
             ++iter)
        {
            Shape *pShape = *iter;
            bool intersected = pShape->intersect(intersection);
            if (intersected)
            {
                intersectedAny = true;
            }
        }
        return intersectedAny;
    }
    
    void addShape(Shape *pShape) { m_shapes.push_back(pShape); }
    
    void clearShapes() { m_shapes.clear(); }
    
protected:
    std::list<Shape*> m_shapes;
};


class Plane : public Shape
{
public:
    Plane(const Point& position, const Vector& normal,  const Material* pMaterial)
        : m_position(position),
          m_normal(normal.normalized()),
		  m_pMaterial(pMaterial)
    {
		m_shapeType="Plane";
        
    }
    
    virtual ~Plane() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        // Plane eqn: ax+by+cz+d=0; another way of writing it is: dot(n, p-p0)=0
        // where n=normal=(a,b,c), and p=(x,y,z), and p0 is position.  Now, p is
        // the ray equation (the intersection point is along the ray): p=origin+t*direction
        // So the plane-ray intersection eqn is dot(n, origin+t*direction-p0)=0.
        // Distribute, and you get:
        //     dot(n, origin) + t*dot(n, direction) - dot(n, p0) = 0
        // Solve for t, and you get:
        //    t = (dot(n, p0) - dot(n, origin)) / dot(n, direction)
        
        // Check if it's even possible to intersect
        float nDotD = dot(m_normal, intersection.m_ray.m_direction);
        if (nDotD >= 0.0f)
        {
            return false;
        }
        
        float t = (dot(m_position, m_normal) - dot(intersection.m_ray.m_origin, m_normal)) /
                  dot(intersection.m_ray.m_direction, m_normal);
        
        // Make sure t is not behind the ray, and is closer than the current
        // closest intersection.
        if (t >= intersection.m_t || t < kRayTMin)
        {
            return false;
        }
        
        // This intersection is closer, so record it.
        intersection.m_t = t;
        intersection.m_pShape = this;
        intersection.m_pMaterial = m_pMaterial;
        intersection.m_normal = m_normal;
       

	 
        return true;
    }

protected:
    Point m_position;
    Vector m_normal;
    Color m_color;
	const Material* m_pMaterial;
};

class Rectangle : public virtual Shape
{
public:
    Rectangle(const Point& pos,
              const Vector& side1,
              const Vector& side2,
			  const Material* pMaterial
			  )
        :  m_position(pos), m_side1(side1), m_side2(side2), m_pMaterial(pMaterial) 
    {
        m_shapeType="Rectangle";
    }
    Rectangle(const Point& pos,
              const Vector& side1,
              const Vector& side2,
			  const Color& color
			  )
        :  m_position(pos), m_side1(side1), m_side2(side2), m_color(color) 
    {
        m_shapeType="Rectangle";
    }
    virtual bool intersect(Intersection& intersection)
    {
        
        Vector normal = cross(m_side1, m_side2).normalized();
        float nDotD = dot(normal, intersection.m_ray.m_direction);
        if (nDotD < EPSL && -nDotD <EPSL)
        {
            return false;
        }
        
        float t = (dot(m_position, normal) - dot(intersection.m_ray.m_origin, normal)) /
                  dot(intersection.m_ray.m_direction, normal);
        
        if (t >= intersection.m_t || t < kRayTMin)
        {
            return false;
        }
        
        Vector side1Norm = m_side1;
        Vector side2Norm = m_side2;
        float side1Length = side1Norm.normalize();
        float side2Length = side2Norm.normalize();
        
        Point worldPoint = intersection.m_ray.calculate(t);
        Point worldRelativePoint = worldPoint - m_position;
        Point localPoint = Point(dot(worldRelativePoint, side1Norm),
                                 dot(worldRelativePoint, side2Norm),
                                 0.0f);
        
        if (localPoint.m_x < -EPSL || localPoint.m_x > side1Length ||
            localPoint.m_y < -EPSL || localPoint.m_y > side2Length)
        {
            return false;
        }
        
        intersection.m_t = t;
        intersection.m_pShape = this;
        intersection.m_emitted = Color();
        intersection.m_normal = normal;
        intersection.m_pMaterial = m_pMaterial;
        return true;
    }
    
    
protected:
    Point m_position;
    Vector m_side1, m_side2; 
	const Material* m_pMaterial;
	const Color& m_color = Color();
};

class Sphere : public Shape
{
public:
    Sphere(const Point& position, float radius, const Material* pMaterial)
        : m_position(position),
          m_radius(radius),
          m_pMaterial(pMaterial)
    {
        
    }
    
    virtual ~Sphere() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        Ray localRay = intersection.m_ray;
        localRay.m_origin -= m_position;
        
        // Ray-sphere intersection can result in either zero, one or two points
        // of intersection.  It turns into a quadratic equation, so we just find
        // the solution using the quadratic formula.  Note that there is a
        // slightly more stable form of it when computing it on a computer, and
        // we use that method to keep everything accurate.
        
        // Calculate quadratic coeffs
        float a = localRay.m_direction.length2();
        float b = 2.0f * dot(localRay.m_direction, localRay.m_origin);
        float c = localRay.m_origin.length2() - m_radius * m_radius;
        
        float t0, t1, discriminant;
        discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
        {
            // Discriminant less than zero?  No solution => no intersection.
            return false;
        }
        discriminant = std::sqrt(discriminant);
        
        // Compute a more stable form of our param t (t0 = q/a, t1 = c/q)
        // q = -0.5 * (b - sqrt(b * b - 4.0 * a * c)) if b < 0, or
        // q = -0.5 * (b + sqrt(b * b - 4.0 * a * c)) if b >= 0
        float q;
        if (b < 0.0f)
        {
            q = -0.5f * (b - discriminant);
        }
        else
        {
            q = -0.5f * (b + discriminant);
        }
        
        // Get our final parametric values
        t0 = q / a;
        if (q > EPSL || q <-EPSL )
        {
            t1 = c / q;
        }
        else
        {
            t1 = intersection.m_t;
        }
        
        // Swap them so they are ordered right
        if (t0 > t1)
        {
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }
        
        // Check our intersection for validity against this ray's extents
        if (t0 >= intersection.m_t || t1 < kRayTMin)
        {
            return false;
        }
        
        if (t0 >= kRayTMin)
        {
            intersection.m_t = t0;
        }
        else if (t1 < intersection.m_t)
        {
            intersection.m_t = t1;
        }
        else
        {
            return false;
        }
        
        // Create our intersection data
        Point localPos = localRay.calculate(intersection.m_t);
        Vector worldNorm = localPos.normalized();

		//-----------------------------------
        // Final Norm : wordNorm + m_position
		//-----------------------------------

        intersection.m_pShape = this;
        intersection.m_pMaterial = m_pMaterial;
		//std::cout<<"--------"<<m_color<<"---------"<<std::endl;
		intersection.m_normal = worldNorm;
        //intersection.m_colorModifier = Color(1.0f, 1.0f, 1.0f);
        
        return true;
    }
    

protected:
    Point m_position;
    float m_radius;
    const Material *m_pMaterial;
    
};

}//namespace Tracer
#endif

