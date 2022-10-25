#pragma once

#include <Femto>
#include <cstdint>

using RenderTexture = Bitmap<8, 2+32*32>;

inline RenderTexture buffers[] = {
    {32, 32},
    {32, 32},
    {32, 32},
    {32, 32},
    {32, 32},
    {32, 32},
    {32, 32},
    {32, 32},
};

constexpr inline u32 bufferCount = sizeof(buffers) / sizeof(buffers[0]);
inline u32 usedBufferCount = 0;

inline u32 bufferHash[bufferCount];

RenderTexture* drawMesh(const u8* node, f32 scale = 1, f32 yRotation = 0, u32 recolor = 0) {
    if (usedBufferCount >= bufferCount){
        usedBufferCount++;
        LOG("Ran out of textures ", usedBufferCount, "\n");
        return nullptr;
    }

    yRotation = s24q8ToF32(f32ToS24q8(yRotation) >> 5 << 5);

    u32 hash = reinterpret_cast<uintptr_t>(node) + f32ToS24q8(yRotation) * 991 + recolor * 253;
    if (bufferHash[usedBufferCount] == hash) {
        return &buffers[usedBufferCount++];
    }

    bufferHash[usedBufferCount] = hash;
    auto& bitmap = buffers[usedBufferCount++];
    bitmap.fill(0);

    u32 width = bitmap.width();
    u32 height = bitmap.height();
    u32 centerX = width / 2;
    u32 centerY = height / 2;
    u32 faceCount = (node[0] << 8) | node[1];
    u32 vertexCount = node[2];
    s8  vtxCache[node[2] * 2];
    s32 iscale = std::min(width, height) * scale;

    f32 cr = cos(yRotation);
    f32 sr = sin(yRotation);

    auto vtx = reinterpret_cast<const s8*>(node + 3 + faceCount * 4);

    for (u32 vtxId = 0; vtxId < vertexCount; ++vtxId) {
        auto ptr = vtx + vtxId * 3;
        f32 Ax = s24q8ToF32(*ptr++ * iscale);
        f32 Ay = s24q8ToF32(*ptr++ * iscale);
        f32 Az = s24q8ToF32(*ptr * iscale);

        f32 x = cr * Ax - sr * Az;
        Az = cr * Az + sr * Ax;

        vtxCache[vtxId * 2] = x;
        vtxCache[vtxId * 2 + 1] = (Ay/2) - Az;

        // vtxCache[vtxId * 3] = x;
        // vtxCache[vtxId * 3 + 1] = Ay; ///2) - Az;
        // vtxCache[vtxId * 3 + 2] = Az; // (Ay/2) - Az;
    }

    for (u32 face = 0; face < faceCount; ++face) {
        u32 index = 3 + face * 4;
        u32 color = u8(node[index++]) + (recolor << 3);
        auto indexA = vtxCache + node[index++] * 2;
        auto indexB = vtxCache + node[index++] * 2;
        auto indexC = vtxCache + node[index  ] * 2;

        s32 Ax = indexA[0];
        s32 Ay = indexA[1];
        // s32 Az = indexA[2];
        s32 Bx = indexB[0];
        s32 By = indexB[1];
        // s32 Bz = indexB[2];
        s32 Cx = indexC[0];
        s32 Cy = indexC[1];
        // s32 Cz = indexC[2];

        // s32 Nx = (Ay - Cy) * (Bz - Cz) - (Az - Cz) * (By - Cy);
        // s32 Ny = (Az - Cz) * (Bx - Cx) - (Ax - Cx) * (Bz - Cz);
        // s32 Nz = (Ax - Cx) * (By - Cy) - (Ay - Cy) * (Bx - Cx);

        // Ay = Ay/2 - Az;
        // By = By/2 - Bz;
        // Cy = Cy/2 - Cz;

        s32 Nx = (Ax - Bx)*(Ay - Cy) - (Ay - By)*(Ax - Cx);
        if (Nx <= 0)
            continue;

        u32 hue = (color >> 3) << 3;
        s32 lum = (color & 7) + (Nx >> 4) - 2;
        if (lum < 0) lum = 0;
        else if (lum > 7) lum = 7;
        color = hue + lum;

        bitmap.fillTriangle(
            centerX + Ax, centerY - Ay,
            centerX + Bx, centerY - By,
            centerX + Cx, centerY - Cy,
            color
            );
    }

    return &bitmap;
}
