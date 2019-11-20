#!/bin/python
import os

vulkan="../Libs/Vulkan/1.1.121.1/x86_64"
glfw="../Libs/glfw-3.3"
includes = "-I"+vulkan+"/include/ -I"+glfw+"/include/ -I../Libs/glm-0.9.9.6/"
libs = "-L"+vulkan+"/lib/ -L"+glfw+"/lib/"
links = "-lvulkan -lglfw -lstdc++"
flags = "-std=c++17"

# with open(".clang-complete", "r") as clang_complete:
#     line = clang_complete.readline()
#     while(line != null):
#         if(line.startswith("-I")):
#             includes.append(line)
#         line = clang_complete.readline()

# files = os.listdir(os.curdir)
# for in_file in files:
#     if in_file.startswith("."):
#         continue
#     if in_file.endswith(".cpp"):
#         source.append(in_file)
#     elif in_file.endswith(".h"):
#         headers.append(in_file)

# source_string = ""
# for s_s in source:
#     source_string += s_s+" "

command = "clang -Wall "+includes+" "+libs+" "+links+" "+flags+" -g main.cpp -o Vulkanism.out"
print(command)
if os.system(command) == 0:
    print("success!\n")
    print("============\n")
    os.environ["LD_LIBRARY_PATH"] = vulkan+"/lib/:"+glfw+"/lib/"
    os.system("echo $LD_LIBRARY_PATH")
    os.system("./Vulkanism.out")

