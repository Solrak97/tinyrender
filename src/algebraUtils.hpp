#ifndef _ALG_UTILS_
#define _ALG_UTILS_

#include "geometry.h"

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

#endif