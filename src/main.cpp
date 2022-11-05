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

void drawWireframe(Model &model, TGAImage &image)
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

Vec3f barycentric(Vec2i *tri, Vec2i P)
{
    // Remember ^ as cross product operator!
    //  u = (AC0, AB0, PA0) X (AC1, AB1, PA1)
    //  => u orthogonal to both vectors
    Vec3f u = (Vec3f{(float)(tri[2].x - tri[0].x),
                     (float)(tri[1].x - tri[0].x),
                     (float)(tri[0].x - P.x)} ^
               Vec3f{(float)(tri[2].y - tri[0].y),
                     (float)(tri[1].y - tri[0].y),
                     (float)(tri[0].y - P.y)});

    /*
    if (std::abs(u.z < 1))
        return Vec3f(-1, 1, 1);
    */

    // (x, y) -> (1 − u − v, u, v)
    return Vec3f{1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z};
}

void triangle(Vec2i *pts, TGAImage &image, TGAColor color)
{
    Vec2i bboxmin(image.width() - 1, image.height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.width() - 1, image.height() - 1);

    for (int i = 0; i < 3; i++)
    {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }

    // Lets draw the bounding box
    /*
    line(bboxmin.x, bboxmin.y, bboxmin.x, bboxmax.y, image, green);
    line(bboxmin.x, bboxmax.y, bboxmax.x, bboxmax.y, image, green);
    line(bboxmax.x, bboxmax.y, bboxmax.x, bboxmin.y, image, green);
    line(bboxmin.x, bboxmin.y, bboxmax.x, bboxmin.y, image, green);

    std::cout << "BBoxmax = <" << bboxmax.x << ", " << bboxmax.y << ">" << std::endl;
    std::cout << "BBoxmin = <" << bboxmin.x << ", " << bboxmin.y << ">" << std::endl;
    */

    // This works pretty good using bounding boxes
    // drawing the box was an interesting way to visualize
    // how it reduces the drawing workload

    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            image.set(P.x, P.y, color);
        }
    }
}

void drawMesh(Model &model, TGAImage &image)
{
    int width = image.width();
    int height = image.height();

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
            triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
}

void meshTest()
{
    TGAImage image(500, 500, TGAImage::RGB);
    Model model = Model("african_head.obj");

    // drawWireframe(model, image);
    drawMesh(model, image);
    image.write_tga_file("output.tga");
}

void sceneTest()
{ // just dumping the 2d scene (yay we have enough dimensions!)

    int width, height;
    width = height = 1000;
    TGAImage scene(width, height, TGAImage::RGB);

    // scene "2d mesh"
    line(20, 34, 744, 400, scene, red);
    line(120, 434, 444, 400, scene, green);
    line(330, 463, 594, 200, scene, blue);

    // screen line
    line(10, 10, 790, 10, scene, white);

    scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    scene.write_tga_file("scene.tga");
}

int main(int argc, char **argv)
{
    // meshTest();
    sceneTest();
    return 0;
}
