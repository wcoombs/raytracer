#include "ray.h"
#include "material.h"
#include "light.h"

#include "../fileio/imageio.h"

using namespace std;
extern bool debugMode;


// Apply the Phong model to this point on the surface of the object, returning
// the color of that point.
Vec3d Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	if( debugMode )
		std::cout << "Debugging the Phong code (or lack thereof...)" << std::endl;

    /*
        The tracePixel call that starts the rayTracing process gets back a 3D vector
        and stores the value in a variable called col.
        My assumption is that this is the total color output, and the pixels themselves
        are calculated as such:
            pixel[0] = (int)( 255.0 * col[0]);
            pixel[1] = (int)( 255.0 * col[1]);
            pixel[2] = (int)( 255.0 * col[2]);
        So I need to return Vec3d as well (as this function as skeleton code already
        does) and store the red, green, and blue as Vec3d(red, green, blue)
    */

    // Get the ambient
    // Calculated outside of the for-loop so that it is applied only once
    // Vec3d * Vec3d does not return the multiplication of two vectors
    // So we're using the built-in prod() function to get the product
    // Note: adding in emissive light because some of the .ray files contain
    // it, and /data/simple/cyl_emissive.ray is -only- emissive light
    // It should also only be calculated once, outside of the loop
    Vec3d emissiveLightColor = ke(i);
    Vec3d ambientLightColor = prod(ka(i), scene->ambient());
    Vec3d pixelColor = emissiveLightColor + ambientLightColor;

    Vec3d theNormalVector = i.N;
    Vec3d rayIntersectionPoint = r.at(i.t);

    for (vector<Light*>::const_iterator litr = scene->beginLights(); litr != scene->endLights(); litr++) {
        Light* currentLight = *litr;
        Vec3d vectorToTheLight = currentLight->getDirection(rayIntersectionPoint);

        // Calculate the shadow color and distance attenuation at this pixel
        Vec3d shadowColor = currentLight->shadowAttenuation(rayIntersectionPoint);
        double distAttenuation = currentLight->distanceAttenuation(rayIntersectionPoint);

        // Calculate the Phong components

        // Calculate the diffuse
        double nDotProductLTerm = dotProduct(theNormalVector, vectorToTheLight);

        // Get rid of the negatives so it doesn't mess up the calculation
        // Adding it to pixelColor will then do nothing for this pixel
        if (nDotProductLTerm < 0) {
            nDotProductLTerm = 0;
        }
        Vec3d diffuseLightColor = kd(i) * nDotProductLTerm;

        // Calculate the specular
        Vec3d vectorToViewer = scene->getCamera().getEye() - rayIntersectionPoint;
        Vec3d perfectReflection = (2 * (dotProduct(theNormalVector, vectorToTheLight)) * theNormalVector) - vectorToTheLight;

        // Normalize the two new vectors, otherwise specular highlights
        // will show up as large white circles on objects
        vectorToViewer.normalize();
        perfectReflection.normalize();

        double rDotProductVTerm = dotProduct(perfectReflection, vectorToViewer);
        
        // Get rid of the negatives so it doesn't mess up the calculation
        // If a negative is found, it'll be converted to 0, and 0 to some power is still 0
        // Adding it to pixelColor will then do nothing for this pixel
        if (rDotProductVTerm < 0) {
            rDotProductVTerm = 0;
        }
        Vec3d specularLightColor = ks(i) * (pow(rDotProductVTerm, shininess(i)));

        // Add the diffuse and specular to the overall color
        // Multiply the components together to get the pixel color for this light
        Vec3d phongComponents = diffuseLightColor + specularLightColor;
        Vec3d lightComponents = prod(shadowColor, phongComponents);
        pixelColor += distAttenuation * lightComponents;
    }

    // return Vec3d(1,1,1); // Enable this to see the shell and sier output.
	return pixelColor;
}


TextureMap::TextureMap( string filename )
{
    data = load( filename.c_str(), width, height );
    if( 0 == data )
    {
        width = 0;
        height = 0;
        string error( "Unable to load texture map '" );
        error.append( filename );
        error.append( "'." );
        throw TextureMapException( error );
    }
}

Vec3d TextureMap::getMappedValue( const Vec2d& coord ) const
{
	// YOUR CODE HERE

    // In order to add texture mapping support to the 
    // raytracer, you need to implement this function.
    // What this function should do is convert from
    // parametric space which is the unit square
    // [0, 1] x [0, 1] in 2-space to Image coordinates,
    // and use these to perform bilinear interpolation
    // of the values.

    // If the data (aka the TextureMap) is empty, return the default
    // of Vec3d(1.0, 1.0, 1.0), which will have no effect on the material.
    // This check is identical to the check in the above constructor
    // for data (TextureMap::TextureMap)
	if (0 == data) {
        return Vec3d(1.0, 1.0, 1.0);
    } else {
        // Convert the uv coordinates to x and y integers for
        // the screen
        int x = width * coord[0];
        int y = height * coord[1];

        // Clamp the x and y coordinate values to within the screen size
        x = min(x, width-1);
        y = min(y, height-1);

        // The TextureMap constructor uses a load() function that exists in
        // imageio.cpp. I checked it out and saw some multiplications with the
        // width and height and 3. Essentially, I'm just trying to reverse-
        // engineer it. In doing some research on this, it looks to be like the
        // conversion from uv coordinates to texels requires byte-shifting to
        // some degree (or at least some variation of it), so my guess is
        // that this mimics something like that. Specifically, the line
        // data[(height-1-y)*3*width + 3*x + rgb] = image.data[rgb*width*height + y*width + x];
        // uses rgb stored in an array. I'm mimicking this by calculating the
        // texel start position in the array, using that index for the first
        // of rgb (the red), then moving to the next (+1) for green, and again (+1) for the blue

        // The multiplication with 3 formula also occurs in raytracer.cpp's tracePixel()
        // function (unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;), where i
        // and j are the raw x and y coordinates in integer form and buffer width is,
        // essentially, the screen width, which enhances my confidence that this is what needs
        // to be done to enable texture mapping for materials with uv coordinates
        int texelStartPositionInData = (x + y * width) * 3;
        double redTexel = data[texelStartPositionInData];
        double greenTexel = data[texelStartPositionInData+1];
        double blueTexel = data[texelStartPositionInData+2];

        Vec3d textureColor = Vec3d(redTexel, greenTexel, blueTexel);
        textureColor = textureColor / 255.0; // Maintain that it's within the color range [0...255]

        return textureColor;
    }
}


Vec3d TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return Vec3d(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return Vec3d( double(data[pos]) / 255.0, 
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0 );
}

Vec3d MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        Vec3d value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}

