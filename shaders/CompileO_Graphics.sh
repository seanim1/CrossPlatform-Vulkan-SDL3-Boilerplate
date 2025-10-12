GLSLC="/mnt/c/VulkanSDK/1.3.296.0/Bin/glslc.exe"

# 2. Execute the compilation using the variable
for file in *.vert; do
	"$GLSLC" ${file} -o ../shaderBinary/${file}.spv -O -fshader-stage=vertex
done

for file in *.frag; do
	"$GLSLC" ${file} -o ../shaderBinary/${file}.spv -O -fshader-stage=fragment
done
