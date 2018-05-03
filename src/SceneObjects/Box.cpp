
#include <cmath>
#include <assert.h>
#include <algorithm>

#include "Box.h"

using namespace std;


bool Box::intersectLocal( const ray& r, isect& i ) const
{
  // A Box by default is centered at the origin, like the sphere
  // As such, the calculation will not take into account for axis-aligned bounding boxes
	Vec3d rayPosition = r.getPosition();
	Vec3d rayDirection = r.getDirection();

  int numberOfSlabs = 3;

  // Create a unit local bounding box, centered at the origin
  BoundingBox localBoundingBox = BoundingBox();
  localBoundingBox.max = Vec3d(0.5, 0.5, 0.5);
  localBoundingBox.min = Vec3d(-0.5, -0.5, -0.5);

  bool insideTheBox = false;

  if (localBoundingBox.min[0] <= rayPosition[0] && rayPosition[0] <= localBoundingBox.max[0] &&
      localBoundingBox.min[1] <= rayPosition[1] && rayPosition[1] <= localBoundingBox.max[1] &&
      localBoundingBox.min[2] <= rayPosition[2] && rayPosition[2] <= localBoundingBox.max[2]) {
    insideTheBox = true;
  }

  double tNear = -1e300; // Close to the smallest value for a double
  double tFar = 1e300; // Close to the largest value for a double
  double t1 = 0;
  double t2 = 0;

  // The threshold needs to be multiplied by the number of slabs, because that's
  // the potential top number of slabs that are checked
  double threshold = (RAY_EPSILON+NORMAL_EPSILON) * numberOfSlabs;

  // Go through each slab and perform the calculation for each x, y, and z plane
  for (int i = 0; i < numberOfSlabs; i++) {
    // Use the ray-plane intersection formula to calculate t1 and t2
    t1 = (localBoundingBox.min[i] - rayPosition[i]) / rayDirection[i];
    t2 = (localBoundingBox.max[i] - rayPosition[i]) / rayDirection[i];

    if (insideTheBox) {
      // There will be two intersections, and tNear will be negative
      // ("behind" the ray's origin), so we use tFar only
      double tValue;

      if (t1 < 0) {
        tValue = t2;
      } else if (t2 < 0) {
        tValue = t1;
      } else if (t1 > t2) {
        tValue = t2;
      } else {
        tValue = t1;
      }

      tFar = min(tFar, tValue);
    } else {
      // Make sure t1 is the near and t2 is the far
      if (t1 > t2) {
        swap(t1, t2);
      }

      // Check that the rays don't go to infinity for either direction
      tNear = max(tNear, t1);
      tFar = min(tFar, t2);

      if (tNear > tFar) {
        return false;
      }
    }
  }

  // The ray successfully intersects with all three slabs
  // Get the tValue based on whether or not we're inside the box
  double tValue = insideTheBox ? tFar : tNear;
  Vec3d intersectionPoint = r.at(tValue);

  // Make sure that we aren't doing self-intersection with secondary rays
  if (tValue <= threshold) {
    return false;
  }

  Vec3d intersectionPointNormal;
  int bestIndex; // used for uv coordinate calculation

  // Get the normal for this intersection on the appropriate plane
  if (abs(intersectionPoint[0] - localBoundingBox.min[0]) <= threshold) {
    intersectionPointNormal = Vec3d(-1,0,0);
    bestIndex = 0;
  } else if (abs(intersectionPoint[0] - localBoundingBox.max[0]) <= threshold) {
    intersectionPointNormal = Vec3d(1,0,0);
    bestIndex = 3;
  } else if (abs(intersectionPoint[1] - localBoundingBox.min[1]) <= threshold) {
    intersectionPointNormal = Vec3d(0,-1,0);
    bestIndex = 1;
  } else if (abs(intersectionPoint[1] - localBoundingBox.max[1]) <= threshold) {
    intersectionPointNormal = Vec3d(0,1,0);
    bestIndex = 4;
  } else if (abs(intersectionPoint[2] - localBoundingBox.min[2]) <= threshold) {
    intersectionPointNormal = Vec3d(0,0,-1);
    bestIndex = 2;
  } else if (abs(intersectionPoint[2] - localBoundingBox.max[2]) <= threshold) {
    intersectionPointNormal = Vec3d(0,0,1);
    bestIndex = 5;
  }
  
  i.setT(tValue);
	i.setN(intersectionPointNormal);
  i.setObject(this);

  // The uv coordinate calculation is taken from the skeleton code's default box.cpp
  // intersection test, adapted for use in this function by setting the bestIndex
  // in the above if-else chain that determines the normal vector
  int i1 = (bestIndex + 1) % 3;
  int i2 = (bestIndex + 2) % 3;
  Vec2d uvCoordinates;

  if (bestIndex < 3) {
    uvCoordinates = Vec2d(0.5 - intersectionPoint[ min(i1, i2) ], 0.5 + intersectionPoint[ max(i1, i2) ]);
  } else {
    uvCoordinates = Vec2d(0.5 + intersectionPoint[ min(i1, i2) ], 0.5 + intersectionPoint[ max(i1, i2) ]);
  }

  i.setUVCoordinates(uvCoordinates);

  return true;
}
