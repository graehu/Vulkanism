#!/bin/python
import os

# Setup environment
print("Setup Environment: ")
vulkan="../Libs/Vulkan/1.1.121.1/x86_64"
glfw="../Libs/glfw-3.3"


os.environ["PATH"] = os.environ["PATH"]+":"+vulkan+"/bin/"

print(os.environ["PATH"])
print("")

# Compile the shaders.

print("Compiling Shaders: ")
command = "glslangValidator -V triangle.vert"

print(command)
os.system(command)
print("")

command = "glslangValidator -V triangle.frag"

print(command)
os.system(command)
print("")

# Compile the program
print("\nCompiling Vulkanism: ")


includes = "-I"+vulkan+"/include/ -I"+glfw+"/include/ -I../Libs/glm-0.9.9.6/"
libs = "-L"+vulkan+"/lib/ -L"+glfw+"/lib/"
links = "-lvulkan -lglfw -lstdc++"
flags = "-std=c++17"

command = "clang -Wall "+includes+" "+libs+" "+links+" "+flags+" -g main.cpp -o Vulkanism.out"

print(command)

if os.system(command) == 0:
    print("success!\n")
    print("============\n")
    os.environ["LD_LIBRARY_PATH"] = vulkan+"/lib/:"+glfw+"/lib/"
    os.system("echo $LD_LIBRARY_PATH")
    os.system("./Vulkanism.out")
