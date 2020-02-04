# Notes
## Day 1
* Introduction
* Template
* Whitted style recap
* RT-centric Game Dev

### Mon  
**None**
### Thu  
**A solid Whitted style ray tracer**  
Render time: 11s
### Fri  
Optimization -> SIMD -> **5x faster**  
### Mon  
Accel structures -> refitting -> **perhaps realtime**
### Thu  
Monte-Carlo, many polish -> path tracing -> **Cook or (DOF, area lights) Kajiya (path tracer)**  

One slide with a bit of maths. 
Σ = sigma. is a for loop, returns summation  
∏ = product. is a for loop, returns product  
∫ = integral. bottom is where we start, top is till where we go.  
R^2, R^3 is the dimension.  
ω = lowercase omega (think of the headphone). is direction.  
ϴ = theta. N*L a.k.a cosine.  
Upside down U is union???
Analytical solution is one you can explain with mathematics.

`Anti Aliasing`  
We used to point sample. We will now send multiple rays through one raster.  
Sampling patterns  
You get weird artifacts if you send consistent rays. You should randomize the sampling points. This will however make edges not look stationary, so you an use the same sampling pattern for each pixel.  
Integrating with a random pattern prevents noise.  

`Distribution Ray Tracing`
Discovered by Cook, hence Cook style. 
Soft shadows. Integrate the visibility of a light source:  
* Shadow itself is the fully visible area, umbra?
* Transition of shadow is called penumbra?
Implementation  
Same as AA. Sample multiple points. Math example
Va = light source visibility. Normalized.
Va = integrate,a(V(x)*(dx))  

Analytical solution  
Va = Light - Union(light, object)  
Hard to do with meshes

`Monte carlo. Approximating Integrals.`
We take the sum of squares. Random samples.


### Tue  
Even faster, quality of randomness -> sampling, estimation -> **Efficiency**
### Fri  
Future work -> path guiding -> **Great product**