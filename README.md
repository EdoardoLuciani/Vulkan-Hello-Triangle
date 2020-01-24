# Vulkan-Hello-Triangle

Simple Hello Triangle program written using the Vulkan API. It draws a rotating triangle using per-vertex colors with a uniform buffer update per frame. The code illustrates the basics of rendering with Vulkan, but can be served as a base for other purposes. The only Windows specific function used is vkCreateWin32Surface().

## Dependencies
The external libraries used are
- volk: to dynamically load entrypoints - https://github.com/zeux/volk
- GLFW: for window creation and related operations (glfwPollEvents()) - https://github.com/glfw/glfw
- OpenGL-Mathematics (GLM): for constructing the glm::rotate matrix - https://github.com/g-truc/glm

## Files
- Vulkan Tutorial.cpp: file containing the VulkanTriangle class and program entrypoint
- vulkan_helper.cpp: functions used to abstract some logic boilerplate code
- Shaders: source code for shaders, need to be compiled to SPIR-V with glslLangValidator.exe before execution

## License
Do whatever you want with it!


