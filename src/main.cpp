#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "interface.h"
#include "omp.h"


using namespace Tracer;

Color traceRay(Ray &ray, 
			   const ShapeSet& masterSet,
			   const std::list<Light*> lights,
			   Rng rng,
			   size_t maxBounce,
			   size_t nBounce);



// Set up a camera ray given the look-at spec, FOV, and screen position to aim at.
Ray makeCameraRay(float fieldOfViewInDegrees,
                  const Point& origin,
                  const Vector& target,
                  const Vector& targetUpDirection,
                  float xScreenPos0To1,
                  float yScreenPos0To1)
{
    Vector forward = (target - origin).normalized();
    Vector right = cross(forward, targetUpDirection).normalized();
    Vector up = cross(right, forward).normalized();
    
    // Convert to radians, as that is what the math calls expect
    float fovScale = std::tan(fieldOfViewInDegrees * M_PI / 360.0f)*2;
   	
    Ray ray;

    // Set up ray info
    ray.m_origin = origin;
    ray.m_direction = forward +
                      right * ((xScreenPos0To1 - 0.5f) * fovScale) +
                      up * ((yScreenPos0To1 - 0.5f) * fovScale);
    ray.m_direction.normalize();
    
    return ray;
}



// TODO: these should probably be read in as commandline parameters.
const size_t kWidth = 1920;
const size_t kHeight = 1080;
const size_t kNumPixelSamples = 64;
const size_t kNumLightSamples = 32;
const size_t maxBounce = 1;

int main(int argc, char **argv)
{
    // The 'scene'
    ShapeSet masterSet;
    PhongMaterial ph1(Color(0.5f,0.5f,0.5f),1,0.5f,0.8f,0.2f);
	PhongMaterial ph2(Color(0.0f,0.5f,0.0f),1,0.5f,0.8f,0.2f);
	PhongMaterial ph3(Color(0.5f,0.0f,0.0f),1,0.5f,0.8f,0.2f);
	PhongMaterial ph4(Color(0.0f,0.0f,0.5f),20,0.5f,5.0f,0.5f,0.5f);
	PhongMaterial ph5(Color(1.0f,1.0f,1.0f),1,0.5f,0.3f,0.3f);
    Plane planeBot(Point(0.0f, -2.0f, 0.0f),
                   Vector(0.0f, 1.0f, 0.0f),
				   &ph1);
    masterSet.addShape(&planeBot);
	
	Plane planeTop(Point(0.0f, 12.0f, 0.0f),
                   Vector(0.0f, -1.0f, 0.0f),
				   &ph1);
	masterSet.addShape(&planeTop);
	
	Plane planeLeft(Point(7.0f, 0.0f, 0.0f),
                   Vector(-1.0f, 0.0f, 0.0f),
                  &ph2);
	masterSet.addShape(&planeLeft);


	Plane planeRight(Point(-7.0f, 0.0f, 0.0f),
                    Vector(1.0f, 0.0f, 0.0f),
					&ph3);

	masterSet.addShape(&planeRight);
	
	Plane planeBehind(Point(0.0f,0.0f,-5.0f),
					  Vector(0.0f,0.0f,1.0f),
					  &ph1);

	masterSet.addShape(&planeBehind);

	// Add a sphere
	Sphere sphere_1(Point(2.0f,1.0f,0.0f),
					3.0f,
					&ph4);
	masterSet.addShape(&sphere_1);

	// Add an area light
    RectangleLight areaLight(Point(-2.0f, 11.99f, -2.5f),
                             Vector(4.0f, 0.0f, 0.0f),
                             Vector(0.0f, 0.0f, 4.0f),
                             &ph5,
                             1.0f);
    masterSet.addShape(&areaLight);
    
	

	// Light sources list
    std::list<Light*> lights;
	lights.push_back(&areaLight);

	// Random generator
	Rng rng;

    cv::Mat resMat(kHeight,kWidth,CV_8UC3,cv::Scalar(0,0,0));
    

    // For each row...
	omp_set_num_threads(32);
    #pragma omp parallel for
	for (size_t y = 0; y < kHeight; ++y)
    {
        bool flag = 0;
        // For each pixel across the row...
        for (size_t x = 0; x < kWidth; ++x)
        {
			unsigned int pixelValue_r = 0;
			unsigned int pixelValue_g = 0;
			unsigned int pixelValue_b = 0;

			for(size_t s_i = 0; s_i < kNumPixelSamples; ++s_i)
			{
        		float yu = 1.0f - (y + rng.nextFloat())/ float(kHeight - 1);

            	float xu = (x + rng.nextFloat()) / float(kWidth - 1);
            	
            	// Find where this pixel sample hits in the scene
            	Ray ray = makeCameraRay(60.0f,
                                        Point(0.0f, 5.0f, 15.0f),
                                        Point(0.0f, 5.0f, 0.0f),
                                        Point(0.0f, 1.0f, 0.0f),
            	                        xu,
            	                        yu);
				
				Color pixelColor = Color();
				size_t nBounce = 0;
            	
				
				pixelColor += traceRay(ray,masterSet,lights,rng,maxBounce,0);
            	
            	
            	// We're writing LDR pixel values, so clamp to 0..1 range first
            	pixelColor.clamp();
				//std::cout<<pixelColor<<std::endl;
            	// Get 24-bit pixel sample and write it out
            	pixelValue_r += (unsigned int)(pixelColor.m_r * 255.0f);
            	pixelValue_g += (unsigned int)(pixelColor.m_g * 255.0f);
            	pixelValue_b += (unsigned int)(pixelColor.m_b * 255.0f);
	    	}// for s_i
        
			pixelValue_r /= kNumPixelSamples;
			pixelValue_g /= kNumPixelSamples;
			pixelValue_b /= kNumPixelSamples;
			std::cout<<"("<<x<<","<<y<<"), "<<"("<<pixelValue_r<<","<<pixelValue_g<<","<<pixelValue_b<<")"<<std::endl;
		    // draw	
	    	resMat.at<cv::Vec3b>(y,x)[0] = pixelValue_b;
	    	resMat.at<cv::Vec3b>(y,x)[1] = pixelValue_g;
	    	resMat.at<cv::Vec3b>(y,x)[2] = pixelValue_r;
			//if(y==2:6 && x>180)
			//{
			//	flag = 1;
			//	break;
			//}

        }
		//if(flag) break;
    }
    
    imwrite("out.jpg",resMat);
    return 0;
}


Color traceRay(Ray &ray, 
			   const ShapeSet& masterSet,
			   const std::list<Light*> lights,
			   Rng rng,
			   size_t maxBounce,
			   size_t nBounce)
{
	Intersection intersection(ray);
    bool intersected = masterSet.intersect(intersection);
    Color pixelColor(0.0f, 0.0f, 0.0f);
    if (intersected)
    {		
	    // Add ambient	
		pixelColor = intersection.m_pMaterial->m_kAmbient *
					 intersection.m_pMaterial->m_color * kNumLightSamples;
		Point position = intersection.position();
	
		for(size_t s_l=0;s_l<kNumLightSamples;++s_l)
		{
        	for (std::list<Light*>::const_iterator iter = lights.begin();
            	 iter != lights.end();
             	++iter)
        	{
           		 // Ask the light for a random position/normal we can use
           		 // for lighting
           		 Point lightPoint;
           		 Vector lightNormal;
           		 Light *pLightShape =*iter;
           		 pLightShape->samplePoint(rng,
           		  	                      position,
           		       	                  lightPoint,
           		           	              lightNormal);
    
           		 // Fire a shadow ray to make sure we can actually see
           		 // that light position
           		 Vector toLight = lightPoint - position;
           		 float lightDistance = toLight.normalize();
           		 Ray shadowRay(position, toLight, lightDistance);
           		 Intersection shadowIntersection(shadowRay);
           		 bool intersected = masterSet.intersect(shadowIntersection);
           		 
           		 if (!intersected || shadowIntersection.m_pShape == pLightShape)
           		 {
					Color emit = pLightShape->emitted();

					pixelColor +=
           		    intersection.m_pMaterial->getColor(position,
														intersection.m_normal,
														ray.m_direction,
														toLight, 
														emit); 
					
				                       		 }
			} //for light
        } //for s_l
	    pixelColor /= kNumLightSamples;
		if((intersection.m_pShape)->getShapeType().find("Light")!=std::string::npos)
        {
			pixelColor += intersection.m_emitted;
		}
    }
	if(nBounce>=maxBounce)
	{
		return pixelColor;
	}
	else
	{
		
		Ray reflectRay(intersection.position(), 
					   -2*(dot(intersection.m_normal,ray.m_direction) * intersection.m_normal) + ray.m_direction);
		pixelColor += intersection.m_pMaterial->m_rReflect * traceRay(reflectRay,masterSet,lights,rng,maxBounce,nBounce+1);
		//pixelColor += intersection.m_pMaterial->mrReflect * traceRay(reflectRay,masterSet,lights,rng,nBounce+1);
	}
	return pixelColor;
}
