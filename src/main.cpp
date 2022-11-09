#include <cstdlib>
#include <iostream>
#include "Rasterizer.hpp"

using namespace Rasterizer;

int main(int argc, char **argv)
{

    TGAImage image(500, 500, TGAImage::RGB);
    Model model = Model("african_head.obj");

    // drawWireframe(model, image);
    rasterize(model, image);
    image.write_tga_file("output.tga");

    return 0;
}
