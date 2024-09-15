# kraf

My first attempt at a *Minecraft* clone using C++ and OpenGL.

The [article on this project](https://aedorian.github.io/blug/_posts/2024-08-30-kraf.html) is available on my blog. Check it out for some explanations.

<p style="float: left">
<img src="https://aedorian.github.io/blug/assets/img/kraf/grass.PNG" style="width:550px; display: inline-block">
<img src="https://aedorian.github.io/blug/assets/img/kraf/dzer.PNG" style="width:550px; display: inline-block">
</p>

## Features:
- infinite world and chunk system
- structure generation (trees, towers...)
- block breaking/placing using raycasts
- ambient occlusion
- day-night cycle

## Credits:
- the `shader.h` and `camera.h` classes from [learnopengl.com](https://learnopengl.com/) (shader compiling and camera)
- [FastNoise2](https://github.com/Auburn/FastNoise2) (noise generation)
- [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) (image loading)

---

### More features to add (probably never):
- transparent geometry (water, glass...)
- swaying vegetation shader effect
- clouds
- distance fog (fragment shader)
- caves and ore generation
- other biomes
