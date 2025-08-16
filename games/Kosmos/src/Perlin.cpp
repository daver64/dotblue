#include "KosmosBase.h"
#include <algorithm>
#include <cmath>
#include <numeric>
PerlinNoise::PerlinNoise(uint32_t seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.end(), engine);
    p.insert(p.end(), p.begin(), p.end());
}

float PerlinNoise::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x) {
    return (hash & 1) ? x : -x;
}

float PerlinNoise::grad(int hash, float x, float y) {
    return ((hash & 1) ? x : -x) + ((hash & 2) ? y : -y);
}

float PerlinNoise::grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float PerlinNoise::grad(int hash, float x, float y, float z, float w) {
    // Simple 4D gradient
    int h = hash & 31;
    float a = h < 24 ? x : y;
    float b = h < 16 ? y : z;
    float c = h < 8 ? z : w;
    return ((h & 1) ? -a : a) + ((h & 2) ? -b : b) + ((h & 4) ? -c : c);
}

float PerlinNoise::noise(float x) const {
    int X = (int)std::floor(x) & 255;
    x -= std::floor(x);
    float u = fade(x);
    int a = p[X], b = p[X + 1];
    return lerp(grad(a, x), grad(b, x - 1), u);
}

float PerlinNoise::noise(float x, float y) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    float u = fade(x), v = fade(y);
    int aa = p[p[X] + Y], ab = p[p[X] + Y + 1];
    int ba = p[p[X + 1] + Y], bb = p[p[X + 1] + Y + 1];
    return lerp(
        lerp(grad(aa, x, y), grad(ba, x - 1, y), u),
        lerp(grad(ab, x, y - 1), grad(bb, x - 1, y - 1), u),
        v);
}

float PerlinNoise::noise(float x, float y, float z) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    int Z = (int)std::floor(z) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    float u = fade(x), v = fade(y), w = fade(z);
    int aaa = p[p[p[X] + Y] + Z];
    int aba = p[p[p[X] + Y + 1] + Z];
    int aab = p[p[p[X] + Y] + Z + 1];
    int abb = p[p[p[X] + Y + 1] + Z + 1];
    int baa = p[p[p[X + 1] + Y] + Z];
    int bba = p[p[p[X + 1] + Y + 1] + Z];
    int bab = p[p[p[X + 1] + Y] + Z + 1];
    int bbb = p[p[p[X + 1] + Y + 1] + Z + 1];
    return lerp(
        lerp(
            lerp(grad(aaa, x, y, z), grad(baa, x - 1, y, z), u),
            lerp(grad(aba, x, y - 1, z), grad(bba, x - 1, y - 1, z), u),
            v),
        lerp(
            lerp(grad(aab, x, y, z - 1), grad(bab, x - 1, y, z - 1), u),
            lerp(grad(abb, x, y - 1, z - 1), grad(bbb, x - 1, y - 1, z - 1), u),
            v),
        w);
}

float PerlinNoise::noise(float x, float y, float z, float w) const {
    // 4D Perlin noise (simple extension)
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    int Z = (int)std::floor(z) & 255;
    int W = (int)std::floor(w) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    w -= std::floor(w);
    float u = fade(x), v = fade(y), t = fade(z), s = fade(w);
    int a = p[X] + Y, aa = p[a] + Z, ab = p[a + 1] + Z;
    int b = p[X + 1] + Y, ba = p[b] + Z, bb = p[b + 1] + Z;
    // 16 corners in 4D
    float n0000 = grad(p[p[aa] + W], x, y, z, w);
    float n1000 = grad(p[p[ba] + W], x - 1, y, z, w);
    float n0100 = grad(p[p[ab] + W], x, y - 1, z, w);
    float n1100 = grad(p[p[bb] + W], x - 1, y - 1, z, w);
    float n0010 = grad(p[p[aa + 1] + W], x, y, z - 1, w);
    float n1010 = grad(p[p[ba + 1] + W], x - 1, y, z - 1, w);
    float n0110 = grad(p[p[ab + 1] + W], x, y - 1, z - 1, w);
    float n1110 = grad(p[p[bb + 1] + W], x - 1, y - 1, z - 1, w);
    float n0001 = grad(p[p[aa] + W + 1], x, y, z, w - 1);
    float n1001 = grad(p[p[ba] + W + 1], x - 1, y, z, w - 1);
    float n0101 = grad(p[p[ab] + W + 1], x, y - 1, z, w - 1);
    float n1101 = grad(p[p[bb] + W + 1], x - 1, y - 1, z, w - 1);
    float n0011 = grad(p[p[aa + 1] + W + 1], x, y, z - 1, w - 1);
    float n1011 = grad(p[p[ba + 1] + W + 1], x - 1, y, z - 1, w - 1);
    float n0111 = grad(p[p[ab + 1] + W + 1], x, y - 1, z - 1, w - 1);
    float n1111 = grad(p[p[bb + 1] + W + 1], x - 1, y - 1, z - 1, w - 1);
    // Interpolate
    float nx000 = lerp(n0000, n1000, u);
    float nx100 = lerp(n0100, n1100, u);
    float nx010 = lerp(n0010, n1010, u);
    float nx110 = lerp(n0110, n1110, u);
    float nx001 = lerp(n0001, n1001, u);
    float nx101 = lerp(n0101, n1101, u);
    float nx011 = lerp(n0011, n1011, u);
    float nx111 = lerp(n0111, n1111, u);
    float nxy00 = lerp(nx000, nx100, v);
    float nxy10 = lerp(nx010, nx110, v);
    float nxy01 = lerp(nx001, nx101, v);
    float nxy11 = lerp(nx011, nx111, v);
    float nxyz0 = lerp(nxy00, nxy10, t);
    float nxyz1 = lerp(nxy01, nxy11, t);
    return lerp(nxyz0, nxyz1, s);
}
