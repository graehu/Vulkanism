#!/usr/bin/env python
import os

vulkan = "../Libs/Vulkan/1.1.121.1"
glfw = "../Libs/glfw-3.3"

os.environ["LD_LIBRARY_PATH"] = vulkan+"/lib/:"+glfw+"/lib/"
os.system("echo $LD_LIBRARY_PATH")
os.system("./Vulkanism.out")