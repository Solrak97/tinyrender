#include <cstdlib>
#include <iostream>

#include "model.hpp"
#include "tgaimage.hpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    // Maybe linear interpolation would work on this
    // At least is a shorter code for a line
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++)
    {
        if (steep)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void mesh(const char *filename, TGAImage &image)
{
    int width = image.width();
    int height = image.height();

    Model model = Model(filename);
    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);
        // this'll get the 2 vertices to draw a line
        // but why does it need the +1?

        //On the .obj file, space is defined between -1 and 1
        //As we deal only on positive integers the +1 centers the data
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model.vert(face[j]);
            Vec3f v1 = model.vert(face[(j + 1) % 3]);

            int x0 = (v0.x + 1) * width / 2;
            int x1 = (v1.x + 1) * width / 2;

            int y0 = (v0.y + 1) * height / 2;
            int y1 = (v1.y + 1) * height / 2;

            line(x0, y0, x1, y1, image, white);
        }
    }
}

Vec3f barycentric(Vec2i *tri, Vec2i P)
{
    //Remember ^ as cross product operator!
    // u = (AC0, AB0, PA0) X (AC1, AB1, PA1)
    // => u orthogonal to both vectors
    Vec3f u = Vec3f{tri[2].x - tri[0].x, tri[1].x - tri[0].x, tri[0].x - P.x} ^
              Vec3f { tri[2].x - tri[0].x, tri[1].x - tri[0].x, tri[0].x - P.x };
    

}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t0.y == t1.y && t0.y == t2.y)
        return; // I dont care about degenerate triangles
    // that explains kinda a lot

    if (t0.y > t1.y)
        std::swap(t0, t1);
    if (t0.y > t2.y)
        std::swap(t0, t2);
    if (t1.y > t2.y)
        std::swap(t1, t2);
    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++)
    {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

        // Axis swap, always draw left to right
        if (A.x > B.x)
            std::swap(A, B);
        for (int j = A.x; j <= B.x; j++)
        {
            image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y
        }
    }
}

void filledMesh(const char *filename, TGAImage &image)
{
    int width = image.width();
    int height = image.height();

    Model model = Model(filename);
    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);

        Vec2i pts[3];

        for (int j = 0; j < 3; j++)
        {
            pts[j] = Vec2i{
                (int)((model.vert(face[j]).x + 1) * width / 2),
                (int)((model.vert(face[j]).y + 1) * height / 2)};
        }

        triangle(pts[0], pts[1], pts[2], image, white);
    }
}

int main(int argc, char **argv)
{
    TGAImage image(500, 500, TGAImage::RGB);
    // mesh("african_head.obj", image);
    filledMesh("african_head.obj", image);
    image.write_tga_file("output.tga");
    return 0;
}
