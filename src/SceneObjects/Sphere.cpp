#include <cmath>

#include "Sphere.h"

using namespace std;


bool Sphere::intersectLocal( const ray& r, isect& i ) const {
	Vec3d rayPosition = r.getPosition();
	Vec3d rayDirection = r.getDirection();

    // Default sphere is a unit sphere (radius = 1) centered at the origin
	double radius = 1.0;
	double threshold = RAY_EPSILON+NORMAL_EPSILON;

	// I couldn't get this to work using the provided formula.
	// After researching online I found a number of articles that
	// provide a formula using a, b, and c terms, where b is the
	// discriminant, so I substituted and converted it into a C++
	// friendly equivalent and it works well
	double a = dotProduct(rayDirection, rayDirection);
	double b = 2 * dotProduct(rayPosition, rayDirection);
	double c = dotProduct(rayPosition, rayPosition) - radius;

	double discriminant = (b*b) - (4*a*c);

    if (discriminant < 0) {
		// The ray completely misses the sphere
        return false;
    }

	// Take the squre root of the discriminant to be used in the full formula to calculate the t-values
    double squareRootOfTheDiscriminant = sqrt(discriminant);
	double denominator = 2*a;
	double t1 = (-b - squareRootOfTheDiscriminant) / denominator;
	double t2 = (-b + squareRootOfTheDiscriminant) / denominator;

	// Take the smaller of the two t's
	double tValue = t1 < t2 ? t1 : t2;
	Vec3d intersectionPointNormal = r.at(tValue);
	
	// Make sure that we aren't doing self-intersection with secondary rays
	double distanceFromRayOriginToIntersectionPoint = (rayPosition - intersectionPointNormal).length();
	if (distanceFromRayOriginToIntersectionPoint <= threshold || tValue <= threshold) {
		return false;
	}

	intersectionPointNormal.normalize();

	// If the ray is inside the sphere, flip the normal
	if (dotProduct(intersectionPointNormal, rayDirection) > 0) {
		intersectionPointNormal = -intersectionPointNormal;
	}

	i.setT(tValue);
	i.setN(intersectionPointNormal);
    i.setObject(this);

	// Calculate uv coordinates for texture mapping
	double uCoordinate = atan2(intersectionPointNormal[0], intersectionPointNormal[2]) / (2*M_PI) + 0.5;
	double vCoordinate = intersectionPointNormal[1] * 0.5 + 0.5;

	Vec2d uvCoordinates = Vec2d(uCoordinate, vCoordinate);
	i.setUVCoordinates(uvCoordinates);

	return true;
}