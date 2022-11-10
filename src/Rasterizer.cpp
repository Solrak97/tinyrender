#include <cstdlib>

#include "rasterizer.hpp"
#include "algebraUtils.hpp"

void Rasterizer::line(int x0, int y0, int x1, int y1, TGAColor color, TGAImage &display)
{
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
            display.set(y, x, color);
        }
        else
        {
            display.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void Rasterizer::line(Vec2i P1, Vec2i P2, TGAColor color, TGAImage &display)
{
    Rasterizer::line(P1.x, P1.y, P2.x, P2.y, color, display);
}

void Rasterizer::triangle(Vec2i P1, Vec2i P2, Vec2i P3, TGAColor color, float *zBuffer, TGAImage &display) {}
void Rasterizer::triangle(Vec2i *pts, TGAColor color, float *zBuffer, TGAImage &display)

{
    int height = display.height();
    int width = display.width();

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
                display.set(P.x, P.y, color);
            }
        }
    }
}

// Complex models
void Rasterizer::renderWireframe(Model model, TGAImage &display)
{
    int width = display.width();
    int height = display.height();

    for (int i = 0; i < model.nfaces(); i++)
    {
        std::vector<int> face = model.face(i);

        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model.vert(face[j]);
            Vec3f v1 = model.vert(face[(j + 1) % 3]);

            int x0 = (v0.x + 1) * width / 2;
            int x1 = (v1.x + 1) * width / 2;

            int y0 = (v0.y + 1) * height / 2;
            int y1 = (v1.y + 1) * height / 2;

            Rasterizer::line(x0, y0, x1, y1, white, display);
        }
    }
}

void Rasterizer::renderMesh(Model &model, TGAImage &display)
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
            Rasterizer::triangle(screen_coords, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255), zBuffer, display);
        }
    }

    delete zBuffer;
}

void Renderizer::renderMesh(Model &model, TGAImage &texture, TGAImage &display)
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
            Rasterizer::triangle(screen_coords, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255), zBuffer, display);
        }
    }

    delete zBuffer;
}