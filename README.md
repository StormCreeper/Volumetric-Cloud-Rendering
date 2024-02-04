# IGR202 Course Project : Real-time volumetric cloud rendering
*By Telo PHILIPPE*  
This is my project for my IGR course in Telecom Paris. My goal is to render real time volumetric clouds, using volume raymarching in a deferred pipeline.  
I re-used some boilerplate code from the first practical project of this class (Interactive solar system), and adapted it to allow deferred rendering.

## How to run with CMake
- Clone the project with `git clone https://github.com/StormCreeper/Volumetric-Cloud-Rendering.git`
- Update the dependencies if it is not done automatically: `git submodule update --init --recursive`
- In the root folder,  run `cmake -C build`
- `cd build`, then compile with `make` (it may take some time building the libraries)
- Finally, run the executable: `./IGR_Clouds`

## Implemented
- Traditionnal mesh rendering with rasterization
- Deferred rendering pipeline
- Basic volume raymarching and lighting, with multiple lights
- GUI to configure the lights and volume parameters
- Simple density function in the shader
## Todo
- Volume traversing in a pre-computed texture instead of mathematical function
- Compute the texture in a compute shader
- More accurated cloud volume generation with different kinds of noise
## Renders
Here are some renders with a basic density function and colored lights  
![Render 1](images/Render1.png)  
![Render 2](images/Render2.png)  
First success with a 3D texture for the density map.  
![Render 3](images/computed.png)  
Changed the sky and volume shape to make it look like clouds:  
![Render 4](images/clouds.png)
Added a time offset and recomputing every 5 frames:  
![Render 5](images/clouds.gif)