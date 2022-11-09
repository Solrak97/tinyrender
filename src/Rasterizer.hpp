#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#include "model.hpp"
#include "geometry.h"
#include "tgaimage.hpp"

namespace Rasterizer
{
    const TGAColor white = TGAColor(255, 255, 255, 255);
    const TGAColor black = TGAColor(0, 0, 0, 255);
    const TGAColor red = TGAColor(255, 0, 0, 255);
    const TGAColor green = TGAColor(0, 255, 0, 255);
    const TGAColor blue = TGAColor(0, 0, 255, 255);

    // Primitive forms
    void line(Vec2i P1, Vec2i P2, TGAColor color, TGAImage &display);
    void line(int x0, int y0, int x1, int y1, TGAColor color, TGAImage &display);

    void triangle(Vec2i *pts, TGAColor color, float *zBuffer, TGAImage &display);
    void triangle(Vec2i P1, Vec2i P2, Vec2i P3, TGAColor color, float *zBuffer, TGAImage &display);

    // Complex models
    void renderWireframe(Model model, TGAImage &display);
    void renderMesh(Model &model, TGAImage &display);
}

#endif