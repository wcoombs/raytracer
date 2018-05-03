#include <cmath>

#include "light.h"


using namespace std;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
	// Create a ray of type shadow and cast it from the point towards this light
	Vec3d vectorToTheLight = getDirection(P);
	ray rayFromIntersectionPointToLight(P, vectorToTheLight, ray::SHADOW);
	isect i;

	// If there is an intersection, return a non-pure black color, otherwise return the default light color
	if (scene->intersect(rayFromIntersectionPointToLight, i)) {
		return Vec3d(0.2, 0.2, 0.2);
	}

	// No intersection, return the default light color
  return color;
}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
	// Color doesn't depend on P 
	return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{
	double distance = (P - position).length();

	// Use the constant, linear, and quadratic terms from light.h
	// These are the equivalent of a0, a1, and a2 from the lecture notes
	double denominator = constantTerm + (linearTerm * distance) + (quadraticTerm * (distance*distance));
	distance = 1.0/denominator;

	// "Normalize" the attenuation - looks nicer and keeps everything in terms of unit vector "sizes"
	if (distance > 1.0) {
		distance = 1.0;
	}

	return distance;
}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
	// Color doesn't depend on P 
	return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
	Vec3d ret = position - P;
	ret.normalize();
	return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
	Vec3d originalIntersectionPoint = P; // Renaming to help clarify for the calculations

	// Point lights are different from directional in that they radiate outwards
	// in all directions from a specific point - so we need to make use of the
	// light's position in the calculation

	// Create a ray of type shadow and cast it from the point towards this light
	// The getDirection() function normalizes it for us
	Vec3d vectorToTheLight = getDirection(originalIntersectionPoint);
	ray rayFromIntersectionPointToLight(originalIntersectionPoint, vectorToTheLight, ray::SHADOW);
	isect i;

	if (scene->intersect(rayFromIntersectionPointToLight, i)) {
		// It isn't enough to just check that an intersection occurs
		// Since this light has a position, we need to check that the
		// intersection occurs before "hitting" the light
		Vec3d someIntersectionPoint = rayFromIntersectionPointToLight.at(i.t);
		double distanceBetweenIntersectionPoints = (originalIntersectionPoint - someIntersectionPoint).length();
		double distanceBetweenOriginalIntersectionPointAndLightPosition = (position - originalIntersectionPoint).length();

		// If there is an intersection and it's before reaching the light, return a non-pure black color
		// Otherwise we drop out of the if-statements and return the default light color
		if (distanceBetweenIntersectionPoints < distanceBetweenOriginalIntersectionPointAndLightPosition) {
			return Vec3d(0.2, 0.2, 0.2);
		}
	}

	// No intersection, return the default light color
  return color;
}
