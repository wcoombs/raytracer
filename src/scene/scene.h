//
// scene.h
//
// The Scene class and the geometric types that it can contain.
//

#pragma warning (disable: 4786)


#ifndef __SCENE_H__
#define __SCENE_H__

#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <memory>

#include "ray.h"
#include "material.h"
#include "camera.h"
#include "../vecmath/vec.h"
#include "../vecmath/mat.h"

#include <stack>

class Light;
class Scene;


class SceneElement
{
public:
	virtual ~SceneElement() {}

	Scene *getScene() const { return scene; }

	// For debugging purposes, draws using OpenGL
	virtual void glDraw(int quality, bool actualMaterials, bool actualTextures) const  { }

protected:
	SceneElement( Scene *s )
		: scene( s ) {}

    Scene *scene;
};

class BoundingBox
{
public:
	Vec3d min;
	Vec3d max;

	void operator=(const BoundingBox& target);

	// Does this bounding box intersect the target?
	bool intersects(const BoundingBox &target) const;
	
	// does the box contain this point?
	bool intersects(const Vec3d& point) const;

	// if the ray hits the box, put the "t" value of the intersection
	// closest to the origin in tMin and the "t" value of the far intersection
	// in tMax and return true, else return false.
	bool intersect(const ray& r, double& tMin, double& tMax) const;
};

class TransformNode
{
protected:

    // information about this node's transformation
    Mat4d    xform;
	Mat4d    inverse;
	Mat3d    normi;

    // information about parent & children
    TransformNode *parent;
    std::vector<TransformNode*> children;
    
public:
   	typedef std::vector<TransformNode*>::iterator          child_iter;
	typedef std::vector<TransformNode*>::const_iterator    child_citer;

    ~TransformNode()
    {
        for(child_iter c = children.begin(); c != children.end(); ++c )
            delete (*c);
    }

    TransformNode *createChild(const Mat4d& xform)
    {
        TransformNode *child = new TransformNode(this, xform);
        children.push_back(child);
        return child;
    }
    
    // Coordinate-Space transformation
    Vec3d globalToLocalCoords(const Vec3d &v)
    {
        return inverse * v;
    }

    Vec3d localToGlobalCoords(const Vec3d &v)
    {
        return xform * v;
    }

    Vec4d localToGlobalCoords(const Vec4d &v)
    {
        return xform * v;
    }

    Vec3d localToGlobalCoordsNormal(const Vec3d &v)
    {
        Vec3d ret = normi * v;
		ret.normalize();
		return ret;
    }

	const Mat4d& transform() const		{ return xform; }

protected:
    // protected so that users can't directly construct one of these...
    // force them to use the createChild() method.  Note that they CAN
    // directly create a TransformRoot object.
    TransformNode(TransformNode *parent, const Mat4d& xform )
        : children()
    {
        this->parent = parent;
        if (parent == NULL)
            this->xform = xform;
        else
            this->xform = parent->xform * xform;
        
        inverse = this->xform.inverse();
        normi = this->xform.upper33().inverse().transpose();
    }
};

class TransformRoot : public TransformNode
{
public:
    TransformRoot()
        : TransformNode(NULL, Mat4d()) {}
};

// A Geometry object is anything that has extent in three dimensions.
// It may not be an actual visible scene object.  For example, hierarchical
// spatial subdivision could be expressed in terms of Geometry instances.
class Geometry
	: public SceneElement
{
public:
    // intersections performed in the global coordinate space.
    bool intersect(const ray&r, isect&i) const;
    
protected:
    // intersections performed in the object's local coordinate space
    // do not call directly - this should only be called by intersect()
	virtual bool intersectLocal( const ray& r, isect& i ) const = 0;

public:
	virtual double dotProduct(const Vec3d v1, const Vec3d v2) const {
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	virtual Vec3d crossProduct(const Vec3d &v1, const Vec3d &v2) const {
		return Vec3d(v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2], v1[0] * v2[1] - v1[1] * v2[0]);
    }

	virtual void createBVH() {}

	virtual bool hasBoundingBoxCapability() const;
	const BoundingBox& getBoundingBox() const { return bounds; }
	virtual void ComputeBoundingBox()
    {
        // take the object's local bounding box, transform all 8 points on it,
        // and use those to find a new bounding box.

        BoundingBox localBounds = ComputeLocalBoundingBox();
        
        Vec3d min = localBounds.min;
		Vec3d max = localBounds.max;

		Vec4d v, newMax, newMin;

		v = transform->localToGlobalCoords( Vec4d(min[0], min[1], min[2], 1) );
		newMax = v;
		newMin = v;
		v = transform->localToGlobalCoords( Vec4d(max[0], min[1], min[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( Vec4d(min[0], max[1], min[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( Vec4d(max[0], max[1], min[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( Vec4d(min[0], min[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( Vec4d(max[0], min[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( Vec4d(min[0], max[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		v = transform->localToGlobalCoords( Vec4d(max[0], max[1], max[2], 1) );
		newMax = maximum(newMax, v);
		newMin = minimum(newMin, v);
		
		bounds.max = Vec3d(newMax);
		bounds.min = Vec3d(newMin);
    }

    // default method for ComputeLocalBoundingBox returns a bogus bounding box;
    // this should be overridden if hasBoundingBoxCapability() is true.
    virtual BoundingBox ComputeLocalBoundingBox() { return BoundingBox(); }

    void setTransform(TransformNode *transform) { this->transform = transform; };
    
	Geometry( Scene *scene ) 
		: SceneElement( scene ) {}

	// For debugging purposes, draws using OpenGL
	void glDraw(int quality, bool actualMaterials, bool actualTextures) const;

	// The defult does nothing; this is here because it is not required
	// that you implement this function if you create your own scene objects.
	virtual void glDrawLocal(int quality, bool actualMaterials, bool actualTextures) const { }

protected:
	BoundingBox bounds;
    TransformNode *transform;
};

// A SceneObject is a real actual thing that we want to model in the 
// world.  It has extent (its Geometry heritage) and surface properties
// (its material binding).  The decision of how to store that material
// is left up to the subclass.
class SceneObject
	: public Geometry
{
public:
	virtual const Material& getMaterial() const = 0;
	virtual void setMaterial( Material *m ) = 0;

	void glDraw(int quality, bool actualMaterials, bool actualTextures) const;

protected:
	SceneObject( Scene *scene )
		: Geometry( scene ) {}
};

// A simple extension of SceneObject that adds an instance of Material
// for simple material bindings.
class MaterialSceneObject
	: public SceneObject
{
public:
	virtual ~MaterialSceneObject() { delete material; }

	virtual const Material& getMaterial() const { return *material; }
	virtual void setMaterial( Material* m )	{ delete material; material = m; }

protected:
	MaterialSceneObject( Scene *scene, Material *mat ) 
		: SceneObject( scene ), material( mat ) {}

	Material* material;
};

// Using this as a template class so that the BVH can work with
// generic data types, and I found this very helpful to deal with both
// the Geometry and Trimesh/TrimeshFace classes. Though the latter is a
// sub-sub-sub class of Geometry, I found this more easy to use.
// Setup took a bit of time, but I found working with the BVH to be
// much simpler as a result
class BVH {
public:
	virtual bool intersect(const ray& r, isect& i) = 0;
	virtual ~BVH() {}
};

template <typename T>
class BVHNode : public BVH {
public:
	BoundingBox boundingBox;
	Geometry *nodeObject;
	BVHNode *leftNode;
	BVHNode *rightNode;
	bool leafNode;

	BVHNode(std::vector<T*> &givenGeometryObjects) {
		// Create an initial bounding box for this new object that's centered at the
		// origin and has a size of 0, and assume this node to be a child node
		boundingBox = BoundingBox();
		boundingBox.min = Vec3d(0,0,0);
		boundingBox.max = Vec3d(0,0,0);
		leafNode = true;

		// Go through the given objects and merge their individual bounding box
		// dimensions together until we end up with our one bounding box that
		// encompasses all the objects that are given
		for (int i = 0; i < givenGeometryObjects.size(); i++) {
			BoundingBox currentBoundingBox = givenGeometryObjects[i]->getBoundingBox();

			for (int i = 0; i < 3; i++) {
				if (currentBoundingBox.max[i] > boundingBox.max[i]) {
					boundingBox.max[i] = currentBoundingBox.max[i];
				}
				if (currentBoundingBox.min[i] < boundingBox.min[i]) {
					boundingBox.min[i] = currentBoundingBox.min[i];
				}
			}
		}

		// If we end up with 0 or 1 given objects, this is a leaf node. Store
		// the object (if there is one) and we're done
		if (givenGeometryObjects.size() <= 1) {
			nodeObject = givenGeometryObjects.size() == 0 ? nullptr : givenGeometryObjects[0];
		} else {
			leafNode = false;

			std::vector<T*> leftNodeObjects;
			std::vector<T*> rightNodeObjects;

			// Split along the longest axis for this bounding box
			double distanceX = boundingBox.max[0] - boundingBox.min[0];
			double distanceY = boundingBox.max[1] - boundingBox.min[1];
			double distanceZ = boundingBox.max[2] - boundingBox.min[2];
			int axisToSplit;

			if (distanceX >= distanceY && distanceX >= distanceZ) {
				axisToSplit = 0;
			} else if (distanceY >= distanceX && distanceY >= distanceZ) {
				axisToSplit = 1;
			} else {
				axisToSplit = 2;
			}

			// Sort the list according to the axisToSplit, using custom operator() functions
			if (axisToSplit == 0) {
				std::sort(givenGeometryObjects.begin(), givenGeometryObjects.end(), SortByXAxis());
			} else if (axisToSplit == 1) {
				std::sort(givenGeometryObjects.begin(), givenGeometryObjects.end(), SortByYAxis());
			} else {
				std::sort(givenGeometryObjects.begin(), givenGeometryObjects.end(), SortByZAxis());
			}

			if (givenGeometryObjects.size() == 2) {
				leftNodeObjects.push_back(givenGeometryObjects[0]);
				rightNodeObjects.push_back(givenGeometryObjects[1]);
			} else {
				int medianIndex = givenGeometryObjects.size() / 2;

				// Go through each of the given geometry objects and add them to the left and
				// right node objects list, depending on which side of the median they're on
				for (int i = 0; i < givenGeometryObjects.size(); i++) {
					if (i < medianIndex) {
						leftNodeObjects.push_back(givenGeometryObjects[i]);
					} else {
						rightNodeObjects.push_back(givenGeometryObjects[i]);
					}
				}
			}

			// Recursively create the left and right nodes for the given objects
			leftNode = new BVHNode<T>(leftNodeObjects);
			rightNode = new BVHNode<T>(rightNodeObjects);
		}
	}

	struct SortByXAxis {
		bool operator() (Geometry* const &left, Geometry* const &right) {
			BoundingBox leftBB = left->getBoundingBox();
			BoundingBox rightBB = right->getBoundingBox();

			double middleLeft = (leftBB.min[0] + leftBB.max[0]) / 2.0;
			double middleRight = (rightBB.min[0] + rightBB.max[0]) / 2.0;

			return middleLeft < middleRight;
		}
	};

	struct SortByYAxis {
		bool operator() (Geometry* const &left, Geometry* const &right) {
			BoundingBox leftBB = left->getBoundingBox();
			BoundingBox rightBB = right->getBoundingBox();

			double middleLeft = (leftBB.min[1] + leftBB.max[1]) / 2.0;
			double middleRight = (rightBB.min[1] + rightBB.max[1]) / 2.0;

			return middleLeft < middleRight;
		}
	};

	struct SortByZAxis {
		bool operator() (Geometry* const &left, Geometry* const &right) {
			BoundingBox leftBB = left->getBoundingBox();
			BoundingBox rightBB = right->getBoundingBox();

			double middleLeft = (leftBB.min[2] + leftBB.max[2]) / 2.0;
			double middleRight = (rightBB.min[2] + rightBB.max[2]) / 2.0;

			return middleLeft < middleRight;
		}
	};

	bool intersect(const ray& r, isect& i) {
		// Initialize the t value to an enormous double
		i.t = 1e300;
		double tMin, tMax;
		std::stack<BVHNode*> stack;

		// initialize the i object to null, we'll check for this when the while loop ends
		i.obj = nullptr;

		if (!boundingBox.intersect(r, tMin, tMax)) {
			// No intersection with this bounding box, return false immediately
			return false;
		}

		BVHNode *currNode = this;
		stack.push(currNode);
		
		while (!stack.empty()) {
			// Get the next node off the stack and perform the checks with it
			currNode = stack.top();
			stack.pop();

			if (currNode->leafNode) {
				// Check for an intersection with this leaf node's object
				isect newIntersectionPoint;

				if (currNode->nodeObject->intersect(r, newIntersectionPoint)) {
					// Update i if and only if the newIntersectionPoint's t value is less than
					// what is currently stored as the closest in i.t
					if (newIntersectionPoint.t < i.t) {
						i = newIntersectionPoint;
					}
				}
			} else {
				// Check for intersection with the sibling nodes' bounding box
				// and add to the stack if there is an intersection
				bool hitLeftNode = currNode->leftNode->boundingBox.intersect(r, tMin, tMax);;
				bool hitRightNode = currNode->rightNode->boundingBox.intersect(r, tMin, tMax);

				if (hitRightNode) {
					stack.push(currNode->rightNode);
				}
				if (hitLeftNode) {
					stack.push(currNode->leftNode);
				}
			}
		}

		// If obj is not null, that means we hit a leaf node (i.e. an actual object),
		// then we return true, otherwise we haven't and return false
		return i.obj != nullptr;
	}

	~BVHNode() {}
};

class Scene
{
public:
	typedef std::vector<Light*>::iterator 			liter;
	typedef std::vector<Light*>::const_iterator 	cliter;

	typedef std::vector<Geometry*>::iterator 		giter;
	typedef std::vector<Geometry*>::const_iterator cgiter;

    TransformRoot transformRoot;

public:
	Scene() 
		: transformRoot(), objects(), lights()
		{}
	virtual ~Scene();

	// BVH specifics
	bool createBVH();
	void enableBVHEnabled(bool value) { enableBVH = value; }

	void add( Geometry* obj )
	{
		obj->ComputeBoundingBox();
		objects.push_back( obj );
	}
	void add( Light* light )
	{ lights.push_back( light ); }

	bool intersect( const ray& r, isect& i ) const;


	std::vector<Light*>::const_iterator beginLights() const { return lights.begin(); }
	std::vector<Light*>::const_iterator endLights() const { return lights.end(); }
        
	const Camera& getCamera() const		{ return camera; }
	Camera& getCamera()					{ return camera; }

	// For efficiency reasons, we'll store texture maps in a cache
	// in the Scene.  This makes sure they get deleted when the scene
	// is destroyed.
	TextureMap* getTexture( string name );

	// These two functions are for handling ambient light; in the Phong model,
	// the "ambient" light is considered a property of the _scene_ as a whole
	// and hence should be set here.
	Vec3d ambient() const	{ return ambientIntensity; }
	void addAmbient( const Vec3d& ambient ) { ambientIntensity += ambient; }

	void glDraw(int quality, bool actualMaterials, bool actualTextures) const;

	const BoundingBox& bounds() const		{ return sceneBounds; }

private:
    std::vector<Geometry*> objects;
	std::vector<Geometry*> nonboundedobjects;
	std::vector<Geometry*> boundedobjects;
    std::vector<Light*> lights;
    Camera camera;

	// BVH specifics
	BVH* bvh;
	bool enableBVH;

	// This is the total amount of ambient light in the scene
	// (used as the I_a in the Phong shading model)
	Vec3d ambientIntensity;

	typedef std::map< std::string, TextureMap* > tmap;
	tmap textureCache;
	
	// Each object in the scene, provided that it has hasBoundingBoxCapability(),
	// must fall within this bounding box.  Objects that don't have hasBoundingBoxCapability()
	// are exempt from this requirement.
	BoundingBox sceneBounds;


public:
	// This is used for debugging purposes only.
	mutable std::vector< std::pair<ray, isect> > intersectCache;
};

#endif // __SCENE_H__
