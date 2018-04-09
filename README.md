# Raytracer
## Version
	0.1: 
		- Collision Object: Planes and Spheres
		- Lights: Colored, Directional, Point, Basic Shading
	0.2:
		- Performance Metrics in Ticks and Milliseconds
		- Optional SSAA Antialiasing with variable sample rate
		- Plane now always renders in a checker pattern
	0.3:
		- Soft Shadows
		- Specular, Diffuced Materials 
	1.0:
		- Increased Performance with Multithreading and Replacing rand()
	1.1:
		- Increased Performance by removing cos and sin by replacing them with precalculated values for random sphere points
		- added performance.md which contains a log over the optimization process

## Result
![Raytracer Result](https://github.com/Norskan/Portfolio/blob/master/RayTracer/run_tree/result.bmp?raw=true "Raytracer Result")


## Build/Run:
Relies on vcvarsall.bat to setup the cl.exe build environment.
Run build.bat to build and run.bat to execute.
