#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "util.h"
#include "ray.h"

namespace Tracer
{


class Material
{
public:
	virtual ~Material() { }
    
    
    virtual Color getColor(const Point& position,
                           const Vector& normal,
                           const Vector& incomingRayDirection,
                           const Vector& lightDirection,
						   const Color& incomingPower) const = 0; 
    Color m_color;	
	float m_kAmbient;
	float m_rReflect;
	float m_rRefract;
};


class PhongMaterial : public Material
{
public:
	PhongMaterial(const Color& color, float exponent,float k1, float k2, float k3,float rReflect = 0.0f, float rRefract = 0.0f) : 
					m_exponent(exponent), 
					m_kReflect(k2),
					m_kDiffuse(k1)
					
					{
						m_color = color;
						m_kAmbient = k3;
						m_rReflect = rReflect;
						m_rRefract = rRefract;
					}
    
    virtual ~PhongMaterial() { }
    
    virtual Color getColor(const Point& position,
                           const Vector& normal,
                           const Vector& incomingRayDirection,
                           const Vector& lightDirectionNorm,
						   const Color& incomingPower) const 
    {
        //Vector reflect = 2*dot(lightDirectionNorm,normal)*normal + lightDirectionNorm;
		//reflect.normalize();
		//std::cout<<"3";

		Vector H =  (lightDirectionNorm - incomingRayDirection ).normalized();
		normal.normalize();
        Color res = 
			   //(m_kReflect * std::pow(std::max(0.0f, -dot(reflect, incomingRayDirection)), m_exponent)* incomingPower + 
				m_kReflect * std::pow(std::max(0.0f,dot(H,normal)),m_exponent)* incomingPower * m_color +
			    m_kDiffuse * std::max(0.0f,dot(lightDirectionNorm, normal))* incomingPower * m_color;
		//std::cout<<std::pow(std::max(0.0f, -dot(reflect, incomingRayDirection)),m_exponent)<<reflect<<incomingRayDirection<<std::endl;
		//std::cout<<"4";
		return res;
	}
    
protected:
    //Color m_color;
	float m_exponent;
	float m_kDiffuse;
	float m_kReflect;
	
};


}//namespace Tracer



#endif
