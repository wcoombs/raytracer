#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/ray.h"
#include <random>

class Scene;

class RayTracer
{
public:
    RayTracer();
    ~RayTracer();

    Vec3d trace( double x, double y );
	Vec3d traceRay( const ray& r, const Vec3d& thresh, int depth, int glossyReflectionDepth );

	double dotProduct(const Vec3d v1, const Vec3d v2) const {
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	Vec3d crossProduct(const Vec3d &v1, const Vec3d &v2) const {
		return Vec3d(v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2], v1[0] * v2[1] - v1[1] * v2[0]);
	}

	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();

	bool createBVH();

	void traceSetup( int w, int h, bool enableBVH, bool enableAntialiasing, bool enableGlossyReflection );
	void tracePixel( int i, int j );

	bool loadScene( char* fn );

	bool sceneLoaded() { return scene != 0; }

    void setReady( bool ready )
      { m_bBufferReady = ready; }
    bool isReady() const
      { return m_bBufferReady; }
	
	bool enableBVHEnabled() const { return m_enableBVH; }

	const Scene& getScene() { return *scene; }

private:
	unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	Scene* scene;

    bool m_bBufferReady;
	
	bool m_enableBVH;

	// For antialiasing
	// These are from the <random> library and are supposedly a huge
	// improvement over rand(), and offers built-in support for
	// floats/doubles by way of the uniform_real_distribution
	bool m_enableAntialiasing;
	std::uniform_real_distribution<double> valueRange;
	std::default_random_engine randomNumberEngine;

	bool m_enableGlossyReflection;
};

#endif // __RAYTRACER_H__
