#include <memory>

#pragma once
enum WrapMode
{
    // u = u % 1, u = v % 1
    WRAP = 0x0,
    // clamp
    CLAMP = 0x1,
    // decal
    DECAL = 0x3,
    // mirror
    MIRROR = 0x2,
};

class MyImage;
class MyTexture
{
public:
    MyTexture();
    ~MyTexture();
public:
    std::shared_ptr<MyImage>    m_pImage;
    // For the three axis of uv coordinates.
    WrapMode                    m_wrapMode[3];

    // TODO: Add filter mode and multiSampling mode for texture.
};

