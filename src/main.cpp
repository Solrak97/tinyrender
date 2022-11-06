#include <cstdlib>
#include <iostream>

#include "model.hpp"
#include "tgaimage.hpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

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

void wireframe(Model &model, TGAImage &image)
{
    int width = image.width();
    int height = image.height();

    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);
        // this'll get the 2 vertices to draw a line
        // but why does it need the +1?

        // On the .obj file, space is defined between -1 and 1
        // As we deal only on positive integers the +1 centers the data
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

Vec3f barycentric(Vec2i *tri, Vec3f P)
{
    Vec3f s[2];
    s[0].x = tri[2].x - tri[0].x;
    s[0].y = tri[1].x - tri[0].x;
    s[0].z = tri[0].x - P.x;
    s[1].x = tri[2].y - tri[0].y;
    s[1].y = tri[1].y - tri[0].y;
    s[1].z = tri[0].y - P.y;

    Vec3f u = s[0] ^ s[1];
    if (std::abs(u.z) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

void triangle(Vec2i *pts, TGAImage &image, float *zBuffer, TGAColor color)
{
    int height = image.height();
    int width = image.width();

    Vec2f bboxmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec2f bboxmax{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
    Vec2f clamp(width - 1, height - 1);

    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, (float)pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, (float)pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, (float)pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, (float)pts[i].y));
    }

    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            P.z = 0;
            P.z = 0;

            //... i need a way to [] into vectors!!!
            // so ill just map it to an array atm

            float bc_screen_map[3] = {bc_screen.x, bc_screen.y, bc_screen.z};
            for (int i = 0; i < 3; i++)
                P.z += pts[i].y * bc_screen_map[i];
            if (zBuffer[int(P.x + P.y * width)] < P.z)
            {
                zBuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

void rasterize(Model &model, TGAImage &display)
{
    int width = display.width();
    int height = display.height();
    int zBufferSize = width * height;
    float *zBuffer = new float[zBufferSize];
    std::fill_n(zBuffer, zBufferSize, std::numeric_limits<int>::min());
    Vec3f light_dir{0, 0, -1};

    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];

        for (int j = 0; j < 3; j++)
        {
            Vec3f v = model.vert(face[j]);
            screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2);
            world_coords[j] = v;
        }

        // N = AB ^ AC
        Vec3f n{world_coords[2] - world_coords[0] ^ world_coords[1] - world_coords[0]};
        n.normalize();

        float intensity = n * light_dir;
        if (intensity > 0)
        {
            triangle(screen_coords, display, zBuffer, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }

    // deleting the buffer after rasterizing
    // we dont want memory leaks, dont we?
    delete zBuffer;
}

void meshTest()
{
    TGAImage image(500, 500, TGAImage::RGB);
    Model model = Model("african_head.obj");

    // drawWireframe(model, image);
    rasterize(model, image);
    image.write_tga_file("output.tga");
}

int main(int argc, char **argv)
{
    meshTest();
    return 0;
}
