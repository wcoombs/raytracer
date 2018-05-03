#include <cmath>
#include <float.h>
#include "trimesh.h"

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt )
        return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    faces.push_back( newFace );
    scene->add(newFace);
    return true;
}

char *
Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

// Calculates and returns the normal of the triangle too.
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const
{
    const Vec3d& a = parent->vertices[ids[0]];
    const Vec3d& b = parent->vertices[ids[1]];
    const Vec3d& c = parent->vertices[ids[2]];

    // Solve the ray-plane intersection first
	Vec3d rayPosition = r.getPosition();
	Vec3d rayDirection = r.getDirection();
    Vec3d normalVector = crossProduct(b-a, c-a); // The intersection point

    if (dotProduct(normalVector, rayDirection) == 0) {
        // The ray is parallel to the plane, no intersection
        return false;
    }

    double numerator = dotProduct(normalVector, a - rayPosition);
    double denominator = dotProduct(normalVector, rayDirection);
    
    double tValue = numerator / denominator;
	double threshold = RAY_EPSILON+NORMAL_EPSILON;
    
    // Make sure that we aren't doing self-intersection with secondary rays
	double distanceFromRayOriginToIntersectionPoint = (rayPosition - normalVector).length();
	if (distanceFromRayOriginToIntersectionPoint <= threshold || tValue <= threshold) {
		return false;
	}

    // Then calculate the exact point to use in the point-in-triangle test
    Vec3d x = r.at(tValue);
    double v1 = dotProduct(crossProduct(b-a, x-a), normalVector);
    double v2 = dotProduct(crossProduct(c-b, x-b), normalVector);
    double v3 = dotProduct(crossProduct(a-c, x-c), normalVector);

    // If all three values are of the same sign, this point is inside the triangle
    if ((v1 > 0 && v2 > 0 && v3 > 0) || (v1 < 0 && v2 < 0 && v3 < 0)) {
        i.setT(tValue);

        // Note: We're already getting the normalized vector/intersection point in the ray-plane
        // calculation from the parent triangle vertices. So we can just set isect normal to
        // it and there's no need to normalize it
        i.setN(normalVector);
        i.setObject(this);

        // Calculate uv coordinates for texture mapping using barycentric coordinates

        // The denominator for calculating the u and v coordinates is the cross product of
        // b-a and c-a, then taking that and the normal vector and calculating the dot product.
        // But the b-a and c-a are already calculated during the intersection test and stored as
        // the normal vector, so it's just the dot product of the normal vector with itself
        // (i.e. it's magnitude)
        double denominator = dotProduct(normalVector, normalVector);

        // Then calculate the numerator for the barycentric coordinates, then divide by
        // the denominator to get u and v
        double barycentricUNumerator = dotProduct(crossProduct(c-b, x-b), normalVector);
        double barycentricVNumerator = dotProduct(crossProduct(a-c, x-c), normalVector);
        double uCoordinate = barycentricUNumerator / denominator;
        double vCoordinate = barycentricVNumerator / denominator;

        Vec2d uvCoordinates = Vec2d(uCoordinate, vCoordinate);
        i.setUVCoordinates(uvCoordinates);

        return true;
    }

    return false;
}


void
Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
        Vec3d a = vertices[(**fi)[0]];
        Vec3d b = vertices[(**fi)[1]];
        Vec3d c = vertices[(**fi)[2]];
        
        Vec3d faceNormal = ((b-a) ^ (c-a));
		faceNormal.normalize();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
}

