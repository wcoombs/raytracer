//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

// who the hell cares if my identifiers are longer than 255 characters:
#pragma warning(disable : 4786)

#include <math.h>
#include "../vecmath/vec.h"
#include "../vecmath/mat.h"

#include <string>

using std::string;

class RayTracer;

class TraceUI {
public:
	TraceUI()
		: m_nDepth(2), m_nSize(512),
		m_displayDebuggingInfo( false ),
		m_enableBVH( true ),
		m_enableAntialiasing( false ),
		m_enableGlossyReflection( false ),
		m_nAntialiasingSamples(16),
		raytracer( 0 )
	{ }

	virtual int		run() = 0;

	// Send an alert to the user in some manner
	virtual void		alert(const string& msg) = 0;

	// setters
	virtual void		setRayTracer( RayTracer* r )
		{ raytracer = r; }

	// accessors:
	int		getSize() const { return m_nSize; }
	int		getDepth() const { return m_nDepth; }
	bool	enableBVHEnabled() const { return m_enableBVH; }
	bool	enableAntialiasingEnabled() const { return m_enableAntialiasing; }
	bool	enableGlossyReflectionEnabled() const { return m_enableGlossyReflection; }
	int		getAntialiasingSamples() const { return m_nAntialiasingSamples; }

protected:
	RayTracer*	raytracer;

	int			m_nSize;				// Size of the traced image
	int			m_nDepth;				// Max depth of recursion
	bool		m_enableBVH;		// Flag to create and enable a BVH to render trimeshes faster
	bool		m_enableAntialiasing;		// Flag to enable supersampling antialiasing
	bool		m_enableGlossyReflection;		// Flag to enable glossy reflection for distribution raytracing
	int			m_nAntialiasingSamples;				// Max samples for supersampling antialiasing

	// Determines whether or not to show debugging information
	// for individual rays.  Disabled by default for efficiency
	// reasons.
	bool		m_displayDebuggingInfo;



};

#endif
