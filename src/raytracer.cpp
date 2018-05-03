// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
Vec3d RayTracer::trace( double x, double y )
{
	// Clear out the ray cache in the scene for debugging purposes,
	scene->intersectCache.clear();
	ray r( Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY );
	
	scene->getCamera().rayThrough( x,y,r );
	int initialGlossyDepth = m_enableGlossyReflection ? 10 : 0;
	Vec3d ret = traceRay( r, Vec3d(1.0,1.0,1.0), traceUI->getDepth(), initialGlossyDepth );
	ret.clamp();
	return ret;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay( const ray& r, 
	const Vec3d& thresh, int depth, int glossyReflectionDepth )
{
	isect i;

	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material& m = i.getMaterial();
		Vec3d shading = m.shade(scene, r, i);

		if (depth <= 0) {
			return shading;
		}

		Vec3d totalReflection;
		Vec3d totalRefraction;

		// Common variables used by both reflection and refraction
		Vec3d rayIntersectionPoint = r.at(i.t);
		Vec3d theNormalVector = i.N;
		Vec3d rayDirection = r.getDirection();

		// Calculate reflection recursively
		Vec3d reflectiveProperty = m.kr(i);
		if (reflectiveProperty[0] != 0 || reflectiveProperty[1] != 0 || reflectiveProperty[2] != 0) {
			// Calculate the reflection of the viewing vector, same as for specular, but replace L with V
			// I switched around the terms slightly compared to the formula from the slides, because my
			// reflections at depth > 2 were appearing upside down			
			Vec3d reflectedViewingVector = rayDirection - 2 * (dotProduct(rayDirection, theNormalVector)) * theNormalVector;
			reflectedViewingVector.normalize();

			int glossyThreshold = 10;
			Vec3d reflectedVector;

			// If glossy reflection is enabled and the depth is > 0, proceed, otherwise
			// cast a single ray with the reflected viewing vector like usual
			if (glossyReflectionDepth > 0) {
				int conalThreshold = 128;

				for (int i = 0; i < glossyThreshold; i++) {
					// Generate the initial random real number
					double randomXValue = (valueRange(randomNumberEngine));
					double randomYValue = (valueRange(randomNumberEngine));
					double randomZValue = (valueRange(randomNumberEngine));

					// Generate a phi value for x, y, and z to use with theta for angles
					double phiValueX = 2 * M_PI * randomXValue;
					double phiValueY = 2 * M_PI * randomYValue;
					double phiValueZ = 2 * M_PI * randomZValue;

					// Take the inverse cos of 1-the random value against the respective
					// reflective property at each color value
					double thetaX = acos(pow((1 - randomXValue), reflectiveProperty[0]));
					double thetaY = acos(pow((1 - randomYValue), reflectiveProperty[1]));
					double thetaZ = acos(pow((1 - randomZValue), reflectiveProperty[2]));

					// Divide by conalThreshold to keep this in a tight conal shape
					double xOffset = sin(phiValueX) * cos(thetaX)/conalThreshold;
					double yOffset = cos(phiValueY) * sin(thetaY)/conalThreshold;
					double zOffset = sin(phiValueZ) * tan(thetaZ)/conalThreshold;

					// Create a new vector for the randomized ray direction, apply the
					// values for x, y, and z to the true reflected viewing vector
					Vec3d newRayDirection;
					newRayDirection[0] = reflectedViewingVector[0] + xOffset;
					newRayDirection[1] = reflectedViewingVector[1] + yOffset;
					newRayDirection[2] = reflectedViewingVector[2] + zOffset;
					newRayDirection.normalize();

					ray reflectionRay(rayIntersectionPoint, newRayDirection, ray::REFLECTION);
					reflectedVector += traceRay(reflectionRay, thresh, depth, glossyReflectionDepth-1);
				}
			}

			// Create and cast a single reflection ray into the scene from the "regular" reflected
			// viewing vector and get the intersection info, if it occurs. This will be executed for
			// both glossy reflection and non-glossy (so executed only once for non-glossy)
			ray reflectionRay(rayIntersectionPoint, reflectedViewingVector, ray::REFLECTION);
			reflectedVector += traceRay(reflectionRay, thresh, depth-1, glossyReflectionDepth-1);
			
			if (m_enableGlossyReflection) {
				reflectedVector = reflectedVector / glossyThreshold;
			}
			
			
			// Multiply by the material property for reflection
			totalReflection = prod(reflectedVector, reflectiveProperty);			
		}

		// Calculate refraction recursively
		Vec3d transmissiveProperty = m.kt(i);
		if (transmissiveProperty[0] != 0 || transmissiveProperty[1] != 0 || transmissiveProperty[2] != 0) {
			double indexOfRefractionForAir = 1.00029; // Air ~= 1
			double angle = -theNormalVector * rayDirection;
			double indexOfRefractionForRayOrigin, indexOfRefractionAtIntersectionPoint; // the ni and nr terms
			bool isEnteringObject = angle > 0;

			// Determine if the ray is entering or leave the object, and adjust the
			// ni (indexOfRefractionForRayOrigin), nr (indexOfRefractionAtIntersectionPoint),
			// and normalVector terms accordingly
			if (isEnteringObject) {
				indexOfRefractionForRayOrigin = indexOfRefractionForAir;
				indexOfRefractionAtIntersectionPoint = m.index(i);
			} else {
				indexOfRefractionForRayOrigin = m.index(i);
				indexOfRefractionAtIntersectionPoint = indexOfRefractionForAir;
				theNormalVector = -theNormalVector;
			}

			// Calculate the second term to find out the value of the square root first
			double viDotProductN = dotProduct(rayDirection, theNormalVector);
			double secondTermNumerator = (indexOfRefractionForRayOrigin * indexOfRefractionForRayOrigin) * (1 - (viDotProductN * viDotProductN));
			double secondTermDenominator = indexOfRefractionAtIntersectionPoint * indexOfRefractionAtIntersectionPoint;
			double secondTermDivision = secondTermNumerator / secondTermDenominator;
			double secondTermSquareRoot = sqrt(1 - secondTermDivision);

			// If the square root term is < 0, then the refracted angle is > than 90 degrees,
			// making this total internal reflection, not refraction
			if (secondTermSquareRoot > 0) {
				Vec3d secondTerm = theNormalVector * secondTermSquareRoot;

				Vec3d firstTermBracketTerm = rayDirection - (theNormalVector * viDotProductN);
				Vec3d firstTermNumerator = indexOfRefractionForRayOrigin * firstTermBracketTerm;
				Vec3d firstTerm = firstTermNumerator / indexOfRefractionAtIntersectionPoint;

				Vec3d refractedViewingVector = firstTerm - secondTerm;

				// Create and cast the refraction ray into the scene and get the intersection info, if it occurs
				ray refractionRay(rayIntersectionPoint, refractedViewingVector, ray::REFRACTION);
				Vec3d refractedVector = traceRay(refractionRay, thresh, depth-1, glossyReflectionDepth-1);

				// Multiply by the material property for refraction/transmission
				totalRefraction = prod(refractedVector, transmissiveProperty);
			}
		}

		// Update shading and return it
		shading += totalReflection + totalRefraction;
		return shading;
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return Vec3d( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
	: scene( 0 ), buffer( 0 ), buffer_width( 256 ), buffer_height( 256 ), m_bBufferReady( false )
{
}


RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn )
{
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos )
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}


	if( ! sceneLoaded() )
		return false;

	
	return true;
}

// This is called from the callback function when the Render button
// is clicked. So one of the first things done is we check if the
// Enable BVH checkbox is selected, and if so, call this function,
// which in turn calls the actual create function in the scene.
// This ensures that the BVH is created right before rendering
// (I originally put this in scene and called it after the file
// was loaded, but sometimes it would not load in time for the code
// to execute and reach the creation for the BVH - in short,
// it was unreliable. Here, it's reliable). Returns created or not
bool RayTracer::createBVH() {
	return scene->createBVH();
}

void RayTracer::traceSetup( int w, int h, bool enableBVH, bool enableAntialiasing, bool enableGlossyReflection )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];

	}
	memset( buffer, 0, w*h*3 );
	m_bBufferReady = true;

	// Custom options
	m_enableBVH = enableBVH;
	scene->enableBVHEnabled(enableBVH);
	m_enableAntialiasing = enableAntialiasing;
	m_enableGlossyReflection = enableGlossyReflection;
}

void RayTracer::tracePixel( int i, int j )
{	
	Vec3d col;

	if( ! sceneLoaded() )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	if (m_enableAntialiasing) {
		int totalSamples = traceUI->getAntialiasingSamples();
		
		// Sample each of the four corners of the pixel for cases
		// 0-4, and the center for case 5 (so do nothing for case 5).
		// This helps to speed up the antialiasing process a bit, as
		// if this pixel will "likely" just output a black color, then
		// there's no need to check all given sample levels (8 to
		// potentially 64). Helps a ton for scenes with a lot of empty space
		for (int k = 0; k < 5; k++) {
			double preSupersampleXValue = x;
			double preSupersampleYValue = y;

			switch (k) {
				case 0:
					preSupersampleXValue += -1;
					preSupersampleYValue += -1;
					break;
				case 1:
					preSupersampleXValue += -1;
					preSupersampleYValue += 1;
					break;
				case 2:
					preSupersampleXValue += 1;
					preSupersampleYValue += -1;
					break;
				case 3:
					preSupersampleXValue += 1;
					preSupersampleYValue += 1;
					break;
				case 4:
					break;
			}
			
			col += trace(preSupersampleXValue, preSupersampleYValue);
		}

		col = col / 5;

		// Check if this pixel is anything other than pure black
		// If it is, execute the supersampling, otherwise continue
		if (col[0] != 0 || col[1] != 0 || col[2] != 0) {
			col = Vec3d(0,0,0); // Reinitialize to remove the color value from pre-supersampling

			for (int k = 0; k < totalSamples; k++) {
				// Generate a random number from -1 to 1 for both X and Y. I found
				// that the engine needs to be initialized only once, otherwise
				// it will generate a series of repeating "random" numbers. Make
				// sure that this value is clamped within the width and height buffers
				double randomXValue = (valueRange(randomNumberEngine) * 2 - 1) / buffer_width;
				double randomYValue = (valueRange(randomNumberEngine) * 2 - 1) / buffer_height;

				// Generate a sample value for this x and y coordinate
				double randomXSampleValue = x + randomXValue;
				double randomYSampleValue = y + randomYValue;

				// Call the trace() function with these sample values and add the
				// resulting color to the overall color
				col += trace(randomXSampleValue, randomYSampleValue);
			}

			// Divide by the total number of samples to average out the overall color
			col = col / totalSamples;
		}
	} else {
		col = trace( x,y );
	}

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}

