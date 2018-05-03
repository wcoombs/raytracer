Assignment 1 Parts A and B - RayTracer
William Coombs
6852347
University of Manitoba
Winter 2018

This program was built on Mac OS X using Visual Studio Code for Mac, using the provided "trace_skel" zip folder containing skeleton code, developed at the University of Washington. It can be built with the Makefile and run from the build directory (the executable is called RayTracer, run with the ./RayTracer command).

I was not able to get the Commandline UI to work with this (I get seg faults with commands like "./RayTracer -r 1 -w 150 ../data/box.ray output.png" and other similar variations. As such, the program does not accept commandline arguments, but I assume this is fine because the program itself lets you change window sizes and load scenes from within it using the GUI.

In order to get the skeleton code to work on Mac OS X, I replaced the provided Makefile from trace_skel with the one from the comp4490 zip, as it contains Mac-specific building/linking properties. I added the X11 flags that link to my machine's local copy of X11 (or Xquartz) and added all subdirectories to source and targets. When I ran it again, the linker threw errors in the graphical debugger. That code is in a .cxx file, not .cpp, .c, or .C, so I added that in to the sources and it works like a charm. After this, the code is able to be built on my machine and execute normally.

I changed the max window size to 800 and set the default screen size to be 512, as the skeleton code's default of 150 is too tiny and was annoying to constantly change during testing.

One small shortcut I took (if you can call it that) was adding a dotProduct() and crossProduct() method to the Geometry class, so that all scene objects can use it.

For the Sphere intersection test, I was not able to get it to work with the provided formula from the lecture notes. I did a lot of research online to see what other methods and mathematical formulas there are and found that almost all of the material out there uses a formula with a, b, and c terms. More information is in a comment block in the code. So I looked into it and converted it to C++-friendly code and it seems to work nicely.

For the Box intersection test, the Kay-Kajiya method I wrote works perfectly when there is no reflection or refraction involved. If there is reflection or refraction, I get something similar to shadow self-intersections on the interior of the box, and have been unsuccessful in my attempts to correct this (too many hours spent). I will use the skeleton code's provided box intersection test for my project, as a result (an example of this anomoly can be found running the trans.ray file at a depth >= 1). This issue also appears when using the built-in bounding box test, so I know the problem isn't with my intersection test, per se, but rather the faces/normals.

Important note
--------------
The shell.ray and sier.ray files do not display anything when rendered. After speaking with the instructor, this is because the color is interpolated from the vertex normals. This is an optional part of the assignment and I did not implement it, so those two scenes will render a black screen. However, I wanted to demonstrate that this is not related to my intersection test for trimeshes. In material.cpp:90, there is a commented out line that will return a Vec3d(1,1,1) for pure white to see that these two files are intersected properly by the rays. This was enabled for all sier and shell output files, then I disabled it and set it back to return the computed color value. All other scenes are rendered using this proper format. Some scenes from a1scenes render a little odd, and I have brought this up to the instructor, who informed me that they will be ignored for marking purposes for those that developed with trace_skel, and instead their equivalents from trace_skel will be used.

Almost all output files were rendered at size = 512, depth = 4, BVH enabled, and antialiasing enabled at 24 samples. For some scenes that contain reflections, there are two rendered outputs - one with and one without glossy reflection (both include the aforementioned settings, but the glossy one's depth was reduced to 2 in order to speed up rendering). Almost every ray file we have access to has been rendered, including from trace_skel and the a1scenes, as well as the custom scenes I created, and are all found in the output directory (organized by sub-directories). Trimesh2 and 3 were rendered at reduced size and quality due to the time it takes to render them at higher quality.

Custom scenes are located in data/custom. There are 8 in total. 2 are more complex, 6 are more basic to demonstrate texture mapping.

Custom1.ray is based on the scene from reflection.ray with the inclusion of the dragon from dragon.ray. It keeps the spheres but scales them down and moves them such that one is in front of the dragon and casts a shadow onto it (on the back of it's middle section), and the other is behind such that the dragon casts a shadow on it. The dragon model needed to be scaled up to 20x to be dominant in the scene and it's centered so all lights can hit it and create different colors and effects. The shadows on the ground also overlap one another, and I believe these changes overall makes for an interesting scene.

Custom2.ray is based on the texture mapping checkerboard scene, with the inclusion of the spheres from reflection. It is set up such that the ground, sides, and back walls are mapped with the checkerboard (in Part B), a point light casts a bright light throughout the scene, and two directional lights point towards the scene - from up top and right of the camera. I believe this is an interesting scene as there are many opportunities for reflective rays to make for interesting effects with the texture mappings.


Part A Required Aspects (Completed/Not Completed):
--------------------------------------------------
Completed - Sphere intersection
Completed - Triangle intersection (for meshes)
Completed - Box intersection (Kay-Kajiya)
Completed - Phong illumination model (non-recursive)
Completed - Multiple light sources, including both directional and point
Completed - Distance attenuation
Completed - Shadow attenuation
Completed - Two additional scene files

Part B Required Aspects (Completed/Not Completed):
--------------------------------------------------
Completed - Transmission and refraction
Completed - Recursive ray tracing (reflection and refraction)
Completed - One acceleration technique
Completed - Additional features (see below)

Part B Completed Optional Aspects
---------------------------------
Completed - Jittered or adaptive supersampling for antialiasing (From 2 marks section)
Completed - Texture mapping of flat surfaces (From 2 marks section)
Completed - Texture mapping of curved surfaces (From 2 marks section)
Completed - Texture mapping of triangle meshes (From 2 marks section)
Completed - Glossy reflection (distribution ray tracing) (From 2 marks section)


I added a lot of comments in my code for this project, and used descriptive variable names rather than mathematical symbols, as I found it very helpful to use plain English to think through what steps I'm working on during the various calculations.


Acceleration Technique:
-----------------------
I created a BVH and it can be toggled on or off in the GUI (it's on by default). I've tested it against all scenes from trace_skel and all geometries are rendered (no random holes from missing the closest, appropriate intersection) and there is a very noticeable speedup. I'm sure the intersection testing can be optimized further, but it does the job. I use a stack and a while loop and add a node to the stack if the ray intersects its bounding box. At the start of the while loop I pop the stack to grab the next BVHNode object, and continue until the stack is empty. There are numerous comments in the scene.h file where the BVH code exists to help explain what I'm doing at each step. Some examples of the speed ups are as follows:

=================================================
Executing a new render with the following options
=================================================
Filename:                   dragon.ray
Screen size:                64
Recursive raytracing depth: 2
Glossy Reflection:          Disabled
Antialiasing:               Disabled
Bounding Volume Hierarchy:  Enabled

BVH creation in progress... Complete
Rendering in progress...    Complete
Total render time:          10.856 seconds


Versus:

=================================================
Executing a new render with the following options
=================================================
Filename:                   dragon.ray
Screen size:                64
Recursive raytracing depth: 2
Glossy Reflection:          Disabled
Antialiasing:               Disabled
Bounding Volume Hierarchy:  Disabled

Rendering in progress...    Complete
Total render time:          65.1878 seconds


And another comparison:

=================================================
Executing a new render with the following options
=================================================
Filename:                   trimesh1.ray
Screen size:                156
Recursive raytracing depth: 2
Glossy Reflection:          Disabled
Antialiasing:               Disabled
Bounding Volume Hierarchy:  Enabled

BVH creation in progress... Complete
Rendering in progress...    Complete
Total render time:          110.058 seconds


Versus:

=================================================
Executing a new render with the following options
=================================================
Filename:                   trimesh1.ray
Screen size:                156
Recursive raytracing depth: 2
Glossy Reflection:          Disabled
Antialiasing:               Disabled
Bounding Volume Hierarchy:  Disabled

Rendering in progress...    Complete
Total render time:          312.876 seconds


Optional Aspects
----------------

Jittered Supersampling for Antialiasing
---------------------------------------
For the supersampling for antialiasing, I chose to use the jittered technique, which essentially randomizes the sample points within a pixel. I used some built-in functions from the <random> library to generate a random real number from 0..1, then multiplied this by 2 then subtracted 1, so that I could get a random double between -1 and 1. I did this for both x and y then added it to the x and y value for the trace() function, for every sample specified in a pixel. I added a rough speed up to this process, where if antialiasing is enabled, then for each pixel, I cast 5 rays into the scene (4 at each corner and one at the center of the pixel) and evaluate the combined color returned. If anything other than black is returned, I continue with the supersampling (which could be anywhere from 8-64 samples per pixel, based on user input), otherwise I return the black color and proceed to the next pixel. This helps a lot with scenes that have a lot of empty space, as there is no unneeded supersampling of empty space (this is especially noticeable on tentacles.ray). Below is an example (note the render times):

Without the speed up technique:

=================================================
Executing a new render with the following options
=================================================
Filename:                   tentacles.ray
Screen size:                512
Recursive raytracing depth: 2
Glossy Reflection:          Disabled
Antialiasing:               Enabled
Antialiasing sample size:   16
Bounding Volume Hierarchy:  Enabled

BVH creation in progress... Complete
Rendering in progress...    Complete
Total render time:          209.929 seconds


With the speedup technique:

=================================================
Executing a new render with the following options
=================================================
Filename:                   tentacles.ray
Screen size:                512
Recursive raytracing depth: 2
Glossy Reflection:          Disabled
Antialiasing:               Enabled
Antialiasing sample size:   16
Bounding Volume Hierarchy:  Enabled

BVH creation in progress... Complete
Rendering in progress...    Complete
Total render time:          99.8212 seconds


Texture mapping of flat surfaces
--------------------------------
I took flat surfaces to mean both squares and boxes. I enabled texture mapping via the getMappedValue() function in material.cpp and have a lengthy write-up in the comments section on how I derived the formula to translate from uv coordinates to texels. The Square primitive already sets the uv coordinates upon intersection, and I found right away when I finally was able to enable texture mapping that the texture_box.ray was showing up correctly. To add texture mapping for boxes, I needed to set the uv coordinates for an intersection point too. Essentially, I used a combination of this program's skeleton code box intersection to find the "bestIndex" value, set an number for it when the normal vector is decided, then checked what it was at the end of the function. Using the same code from the trace_skel, if that index is < 3 then I set the uv coordinates a certain way, and if it's > 3 then I set it another way, using variables i1 and i2 that use the bestIndex. The texture mapped box.ray shows up as such, as well as various planes (like the checkerboard).


Texture mapping of curved surfaces
--------------------------------------------
Sphere: I found a plethora of useful tips and formulas to calculate the u and v coordinates for a sphere and implemented it in the sphere intersection code. The sphere is demonstrated with the sphere_texture.ray file in the custom directory (it's the sphere from scene.ray but replaces the diffuse color with the texture map) and can also be seen on the basketball in the basketball.ray file (more information regarding this file is in the Texture mapping of triangle meshes section). The texture map itself is from http://planetpixelemporium.com/earth.html and the download link is http://planetpixelemporium.com/download/download.php?earthmap1k.jpg

Cylinder: The ray file is cylinder_texture.ray in the custom directory. It uses the cylinder from cylinder.ray, but translated and slightly rotated to be closer to the middle of the screen. The calculations I found difficult, and they are not perfect, but it's sort of decent. I used the sphere calculation for the cylinder caps, and the body a mixture of the y-value and variation of the sphere. The download link for the texture is http://texturelib.com/download/Textures/concrete/base/concrete_base_0062_01_s.jpg and from the website http://texturelib.com/texture/?path=/Textures/concrete/base/concrete_base_0062

Cone: The cone calculation is similar to the cylinder, but it uses theRoot. It, like the cylinder, is nowhere near perfect, but it looks decent. The sample ray file is cone_texture.ray in the custom directory. I used the cone from cone.ray but just added the texture map to the diffuse material. The texture material link is http://texturelib.com/download/Textures/tile/tile/tile_tile_0069_01_tiled_s.jpg and from the website http://texturelib.com/texture/?path=/Textures/tile/tile/tile_tile_0069


Texture mapping of triangle meshes
----------------------------------
I implemented a very basic uv coordinate calculation using barycentric coordinates for trimeshes. In my research on how to do this, I came across various pseudo-code formulas that all use barycentric coordinates to some degree to interpolate across the triangle. But one simple method I discovered was using the cross product and dot product of the normal vector with the vertices of the triangle and the intersection point (explained further in comments in the code). I created two scenes to demonstrate basic triangle mesh texture mapping: basketball.ray (the easy1.ray file texture mapped) and triangle_texture.ray (the cube.ray file texture mapped). The basketball sphere texture map file is from http://www.robinwood.com/Catalog/FreeStuff/Textures/TextureDownloads/Balls/BasketballColor.jpg and the grass is from http://texturelib.com/texture/?path=/Textures/grass/grass/grass_grass_0109 with the link http://texturelib.com/download/Textures/grass/grass/grass_grass_0109_01_s.jpg and for the triangle_texture.ray file the texture is http://texturelib.com/download/Textures/brick/stone%20wall/brick_stone_wall_0096_01_s.jpg from http://texturelib.com/texture/?path=/Textures/brick/stone%20wall/brick_stone_wall_0096

I also added a scene for the dragon, but texture mapped, just to see how it would turn out. If I implemented interpolation across vertex normals and materials for trimeshes, I'm sure it would look nicer. The texture is of snow, and the link is http://texturelib.com/download/Textures/water/ice%20and%20snow/water_ice_and_snow_0031_01.jpg from the website http://texturelib.com/texture/?path=/Textures/water/ice%20and%20snow/water_ice_and_snow_0031


Glossy reflection
-----------------
If glossy reflection is enabled on the GUI, be warned, render times are drastically increased. A decent example of this implementation is seen on the reflection.ray and reflection1.ray scenes. It's not perfect, but it does a decent job. The hardest part was figuring out how to keep the reflected rays contained within a conal shape. Once I was able to do this though it became a lot simpler. The spheres in reflection.ray and reflection1.ray have a subtle glossy look to them, while the reflections in the mirror plane (for reflection.ray) are glossed over (same with the floor), so it does not mimic a perfect mirror. Some comparisons in render times:

=================================================
Executing a new render with the following options
=================================================
Filename:                   reflection.ray
Screen size:                512
Recursive raytracing depth: 2
Glossy Reflection:          Enabled
Antialiasing:               Disabled
Bounding Volume Hierarchy:  Enabled

BVH creation in progress... Complete
Rendering in progress...    Complete
Total render time:          42.876 seconds


Versus (note the depth even at 10):

=================================================
Executing a new render with the following options
=================================================
Filename:                   reflection.ray
Screen size:                512
Recursive raytracing depth: 10
Glossy Reflection:          Disabled
Antialiasing:               Disabled
Bounding Volume Hierarchy:  Enabled

BVH creation in progress... Complete
Rendering in progress...    Complete
Total render time:          6.8033 seconds