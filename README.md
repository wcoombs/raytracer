# Raytracer
University of Manitoba - Winter 2018<br>
COMP 4490 - Computer Graphics II<br>
Assignment 1

<br>

A raytracing program, built using a provided skeleton program, `trace_skel`, developed at the Univeristy of Washington in 2005.

Written in C++, this program can be built on Mac OS X using the `Makefile` and run from the command line in the `build` directory using the command `./RayTracer`. The program uses `.ray` files to raytrace. All required and numerous optional aspects of the assignment were implemented to achieve a 100% grade.

This is a relatively advanced raytracer, able to recursively raytrace box, cone, cylinder, sphere, square, and triangle intersections, phong illumination, multiple light sources, distance and shadow attenuation, transmission, refraction, reflection, basic texture mapping of almost all primitive types, supersampling, and glossy reflection. The program also implements a bounding volume hierarchy to drastically speed up this process.

<br>

## Sample Images From the `Output` Directory

![](output/BaseDataScenes/box.png?raw=true)
![](output/BaseDataScenes/easy2.png?raw=true)
![](output/BaseDataScenes/hitchcock.png?raw=true)
![](output/BaseDataScenes/reflection.png?raw=true)
![](output/BaseDataScenes/tentacles.png?raw=true)
![](output/CustomScenes/basketball.png?raw=true)
![](output/CustomScenes/custom2.png?raw=true)
![](output/CustomScenes/sphere_texture.png?raw=true)
![](output/PolymeshScenes/easy3.png?raw=true)
![](output/PolymeshScenes/easy5.png?raw=true)
![](output/PolymeshScenes/trimesh1.png?raw=true)
![](output/SimpleScenes/cylinder_refract.png?raw=true)
![](output/SimpleScenes/sphere_refract.png?raw=true)
![](output/PolymeshScenes/trimesh3.png?raw=true)

<br>

## Resources:

The earth texture map is from http://planetpixelemporium.com/earth.html and the download link is http://planetpixelemporium.com/download/download.php?earthmap1k.jpg

The cylinder texture map download link is http://texturelib.com/download/Textures/concrete/base/concrete_base_0062_01_s.jpg and from the website http://texturelib.com/texture/?path=/Textures/concrete/base/concrete_base_0062

The cone texture map link is http://texturelib.com/download/Textures/tile/tile/tile_tile_0069_01_tiled_s.jpg and from the website http://texturelib.com/texture/?path=/Textures/tile/tile/tile_tile_0069

The basketball texture map file is from http://www.robinwood.com/Catalog/FreeStuff/Textures/TextureDownloads/Balls/BasketballColor.jpg

The grass texture map is from http://texturelib.com/texture/?path=/Textures/grass/grass/grass_grass_0109 with the link http://texturelib.com/download/Textures/grass/grass/grass_grass_0109_01_s.jpg

The `triangle_texture.ray` file texture map is http://texturelib.com/download/Textures/brick/stone%20wall/brick_stone_wall_0096_01_s.jpg from http://texturelib.com/texture/?path=/Textures/brick/stone%20wall/brick_stone_wall_0096

The snow texture map for the dragon is http://texturelib.com/download/Textures/water/ice%20and%20snow/water_ice_and_snow_0031_01.jpg from the website http://texturelib.com/texture/?path=/Textures/water/ice%20and%20snow/water_ice_and_snow_0031

All other resources, such as ray files (excluding from the `custom` directory) are provided via the assignment and `trace_skel` skeleton program.