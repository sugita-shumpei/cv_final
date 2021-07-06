#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Camera.h>
#include <GLLib.h>
#include <Utils.h>
#include <Config.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <tuple>
#include <filesystem>
#include <cstdio>
#include <string>
#include <string_view>
#define CV_TEST1_GLSL_SHADER_CODE(VERSION,STR) "#version "#VERSION" core;\n" #STR
int main(){
    std::string_view str = CV_TEST1_GLSL_SHADER_CODE(450,
    unform sampler2D tex;\n
    void main() {\n
    });
    std::cout << str << "\n";
}