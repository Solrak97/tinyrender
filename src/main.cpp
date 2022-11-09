#include <iostream>
#include "rasterizer.hpp"

using namespace Rasterizer;

int main(int argc, char **argv)
{

    TGAImage image(500, 500, TGAImage::RGB);
    Model model = Model("african_head.obj");

    renderWireframe(model, image);
    //renderMesh(model, image);
    image.write_tga_file("output.tga");

    return 0;
}
