#!/usr/bin/env python
import os

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_error(error_string):
    print(bcolors.FAIL+error_string+bcolors.ENDC)

def print_warning(warning_string):
    print(bcolors.WARNING+warning_string+bcolors.ENDC)

def print_success(success_string):
    print(bcolors.OKGREEN+success_string+bcolors.ENDC)

# Setup environment
print("Setup Environment: ")
#TODO: Fix this annoying pathing I had to change here.
vulkan = "../Libs/Vulkan/1.1.121.1"
glfw = "../Libs/glfw-3.3"
source = "main.cpp"
command = ""

vulkan_exists = os.path.exists(vulkan)
print("\tVulkan: "+vulkan+" : "+str(vulkan_exists))
if not vulkan_exists:
    print_error("\nBuild failed.")
    print("\n============\n")
    quit()

glfw_exists = os.path.exists(glfw)
print("\tGLFW: "+glfw+" : "+str(os.path.exists(glfw)))
if not glfw_exists:
    print_error("\nBuild failed.")
    print("\n============\n")
    quit()

print("\n============\n")

if os.name == "nt":
    print("Building for windows:\n")
    msvc = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.23.28105/bin/Hostx64/x64"
    vs_tools = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/Tools/"

    libs = "/LIBPATH:"+vulkan+"/lib32/"+" /LIBPATH:"+glfw+"/lib32/"
    includes = "/I"+vulkan+"/include/"+" /I"+glfw+"/include/"+" /I"+os.path.abspath("../Libs/glm-0.9.9.6/")
    links = "/link vulkan-1.lib VkLayer_utils.lib shaderc_combined.lib glfw3.lib glfw3dll.lib msvcrt.lib msvcmrt.lib User32.lib Gdi32.lib Shell32.lib"
    flags = "/W4 /EHsc /Zi /std:c++17"
    output = "/Fe:Vulkanism.exe"
    compiler = "cl.exe"

    #setup vs environment
    #%VSCMD_DEBUG%
    #os.environ["VSCMD_DEBUG"] = "3"
    os.environ["PATH"] = os.environ["PATH"]+";"+vulkan+"/bin/"+";"+msvc+";"+vs_tools    
    print("")

else:
    print("Building for linux:\n")
    includes = "-I"+vulkan+"/x86_64/include/ -I"+glfw+"/include/ -I../Libs/glm-0.9.9.6/"
    # set vulkan library path
    # set glfw library path
    libs = "-L"+vulkan+"/x86_64/lib/ -L"+glfw+"/lib/"


    # link vulkan shared library
    # link glfw shared library
    # link standard c++ shared library
    links = "-lvulkan -lglfw -lstdc++"


    # -std=c++17 : Set C++17 compliancy
    # -Wall : All warnings
    # -g : Debug on
    flags = "-std=c++17 -Wall -g"
    compiler = "clang"
    output = "-o Vulkanism.out"
    os.environ["PATH"] = os.environ["PATH"]+":"+vulkan+"/x86_64/bin/"

# Add vulkan to the path, allows glslangValidator to be called.

print("PATH: "+os.environ["PATH"])
print("")

# Compile the shaders.

print("Adding Shader commands: \n")
command += "glslangValidator -V triangle.vert.glsl && "

command += "glslangValidator -V triangle.frag.glsl"

print(command)
os.system(command)
command = ""

# input("")

# Compile the program
print("\nCompiling Vulkanism:")
print("\tCompiler: "+compiler)
print("\tIncludes: "+includes)
print("\tLibs: "+libs)
print("\tLinks: "+links)
print("\tFlags: "+flags)
print("\tSource: "+source)
print("\tOutput: "+output)

if os.name == "nt":
    command += "call vsdevcmd.bat && "
command += compiler+" "+includes+" "+flags+" "+source+" "+output+" "+links+" "+libs


print("\nFinal command: \n\n"+command)
print("\n==================")

if os.system(command) == 0:
     print_success("\nBuild success!\n")
#     os.environ["LD_LIBRARY_PATH"] = vulkan+"/lib/:"+glfw+"/lib/"
#     os.system("echo $LD_LIBRARY_PATH")
#     os.system("./Vulkanism.out")
else:
    print_error("\nBuild failed.")
print("\n============\n")
