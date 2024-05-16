#!/usr/bin/env python3
import os


vulkan = "../Libs/Vulkan/1.3.280.1"
glfw = "../Libs/glfw-3.4/src"


if os.name == "nt":
    pass
elif os.name == "posix":
    os.environ["LD_LIBRARY_PATH"] = vulkan+"/lib/:"+glfw
    os.system("echo $LD_LIBRARY_PATH")
    os.system("./Vulkanism.out")
