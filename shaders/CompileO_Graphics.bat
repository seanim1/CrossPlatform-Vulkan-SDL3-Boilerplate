set GLSLC=C:\VulkanSDK\1.3.296.0\Bin\glslc.exe



%GLSLC% box.vert -o ../shaderBinary/box.vert.spv -O -fshader-stage=vertex



%GLSLC% box.frag -o ../shaderBinary/box.frag.spv -O -fshader-stage=fragment



%GLSLC% box1.vert -o ../shaderBinary/box1.vert.spv -O -fshader-stage=vertex



%GLSLC% box1.frag -o ../shaderBinary/box1.frag.spv -O -fshader-stage=fragment



%GLSLC% line.vert -o ../shaderBinary/line.vert.spv -O -fshader-stage=vertex



%GLSLC% line.frag -o ../shaderBinary/line.frag.spv -O -fshader-stage=fragment



pause

