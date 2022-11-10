#include <iostream>
#include "rasterizer.hpp"

using namespace Rasterizer;

int main(int argc, char **argv)
{

    TGAImage display(500, 500, TGAImage::RGB);
    TGAImage texture = TGAImage();
    texture.read_tga_file("frican_head_diffuse.tga");
    Model model = Model("african_head.obj");

    //  renderWireframe(model, display);
    renderMesh(model, texture, display);
    display.write_tga_file("output.tga");

    return 0;
}
