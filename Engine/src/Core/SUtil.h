#pragma once

#include "Core.h"
#include "SHash.hpp"
#include "SMemory.h"
#include "raylib/src/rlgl.h"

#include <assert.h>
#include <stdint.h>

template<class T>
struct SAllocator
{
    typedef T value_type;

    SAllocator() = default;

    template<class U>
    constexpr SAllocator(const SAllocator <U>&) noexcept {}

    [[nodiscard]] T* allocate(size_t n)
    {
        assert(n < (SIZE_MAX / sizeof(T)));
        auto p = static_cast<T*>(Scal::MemAlloc(n * sizeof(T)));
        if (p)
        {
            report(p, n);
            return p;
        }
        else
        {
            S_LOG_ERR("SAllocator bad allocation!");
            assert(p);
            return nullptr;
        }
    }

    void deallocate(T* p, size_t n) noexcept
    {
        report(p, n, 0);
        Scal::MemFree(p);
    }
private:
    void report(T* p, size_t n, bool alloc = true) const
    {
        const char* str = (alloc) ? "Alloc" : "Dealloc";
        S_LOG_INFO("%sing %d bytes at %p", str, n, p);
    }
};

template<class T, class U>
bool operator==(const SAllocator <T>&, const SAllocator <U>&) { return true; }

template<class T, class U>
bool operator!=(const SAllocator <T>&, const SAllocator <U>&) { return false; }


inline Mesh GenMeshPlaneXY(float width, float length, int resX, int resZ)
{
    Mesh mesh = { 0 };
    int vertexCount = resX * resZ; // vertices get reused for the faces
    float hw = width / 2.0f;
    float hh = length / 2.0f;
    Vector3* vertices = (Vector3*)RL_MALLOC(vertexCount * sizeof(Vector3));
    vertices[0] = { -hw, -hh, 0.0f };
    vertices[1] = { -hw, hh, 0.0f };
    vertices[2] = { hw, hh, 0.0f };
    vertices[3] = { hw, -hh, 0.0f };

    // Normals definition
    Vector3* normals = (Vector3*)RL_MALLOC(vertexCount * sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = { 0.0f, 1.0f, 0.0f };   // Vector3.up;

    // TexCoords definition
    Vector2* texcoords = (Vector2*)RL_MALLOC(vertexCount * sizeof(Vector2));
    texcoords[0] = { 0.0f, 1.0f }; // x, h
    texcoords[1] = { 0.0f, 0.0f }; // x, w
    texcoords[2] = { 1.0f, 0.0f }; // y, w
    texcoords[3] = { 1.0f, 1.0f }; // y, h

    // Triangles definition (indices)
    //int numFaces = (resX) * (resZ);
    int* triangles = (int*)RL_MALLOC(6 * sizeof(int));
    triangles[0] = 2;
    triangles[1] = 1;
    triangles[2] = 0;
    triangles[3] = 0;
    triangles[4] = 3;
    triangles[5] = 2;

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = 2;
    mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3 * i] = vertices[i].x;
        mesh.vertices[3 * i + 1] = vertices[i].y;
        mesh.vertices[3 * i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2 * i] = texcoords[i].x;
        mesh.texcoords[2 * i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3 * i] = normals[i].x;
        mesh.normals[3 * i + 1] = normals[i].y;
        mesh.normals[3 * i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount * 3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);
    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, true);
    return mesh;
}

// Draws a texture in 3D space with pro parameters...
inline void DrawTexturePro3D(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector3 origin, float rotation, float posZ, Color tint)
{
    // Check if texture is valid
    if (texture.id > 0)
    {

        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (sourceRec.width < 0) { flipX = true; sourceRec.width *= -1; }
        if (sourceRec.height < 0) sourceRec.y -= sourceRec.height;
        rlSetTexture(texture.id);
        rlPushMatrix();
        rlTranslatef(destRec.x, destRec.y, 0.0f);
        rlRotatef(180.0f, 1.0f, 0.0f, 0.0f);
        rlTranslatef(-origin.x, -origin.y, -origin.z);

        rlBegin(RL_QUADS);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(0.0f, 1.0f, 0.0f);                          // Normal vector pointing towards viewer

            // Bottom-left corner for texture and quad
            if (flipX) rlTexCoord2f((sourceRec.x + sourceRec.width) / width, sourceRec.y / height);
            else rlTexCoord2f(sourceRec.x / width, sourceRec.y / height);
            rlVertex3f(0.0f, 0.0f, posZ);

            // Bottom-right corner for texture and quad
            if (flipX) rlTexCoord2f((sourceRec.x + sourceRec.width) / width, (sourceRec.y + sourceRec.height) / height);
            else rlTexCoord2f(sourceRec.x / width, (sourceRec.y + sourceRec.height) / height);
            rlVertex3f(0.0f, destRec.height, posZ);

            // Top-right corner for texture and quad
            if (flipX) rlTexCoord2f(sourceRec.x / width, (sourceRec.y + sourceRec.height) / height);
            else rlTexCoord2f((sourceRec.x + sourceRec.width) / width, (sourceRec.y + sourceRec.height) / height);
            rlVertex3f(destRec.width, destRec.height, posZ);

            // Top-left corner for texture and quad
            if (flipX) rlTexCoord2f(sourceRec.x / width, sourceRec.y / height);
            else rlTexCoord2f((sourceRec.x + sourceRec.width) / width, sourceRec.y / height);
            rlVertex3f(destRec.width, 0.0f, posZ);
        rlEnd();
        rlPopMatrix();
        rlSetTexture(0);
    }
}
// TODO hash test stuff remove

struct TestHashStruct
{
    int x;
    int y;
    int z;
};

inline void TestHashes()
{
    TestHashStruct s = { 5, 1, 5 };
    auto hash = SHashMerge(0, s);

    TestHashStruct ss = { 5, 2, 5 };
    auto hash2 = SHashMerge(0, ss);

    TestHashStruct sss = { 5, -1, 5 };
    auto hash3 = SHashMerge(0, sss);

    TestHashStruct s4 = { 1, 5, 1 };
    auto hash4 = SHashMerge(0, s4);

    TestHashStruct s5 = { 6, 1, 5 };
    auto hash5 = SHashMerge(0, s5);

    TestHashStruct s6 = { 4, 1, 5 };
    auto hash6 = SHashMerge(0, s6);

    TestHashStruct s7 = { 5, 1, 6 };
    auto hash7 = SHashMerge(0, s7);

    TestHashStruct s8 = { -5, 1, 5 };
    auto hash8 = SHashMerge(0, s8);

    TestHashStruct s9 = { 5, 1, 4 };
    auto hash9 = SHashMerge(0, s9);

    size_t mod = 8 * 2;
    auto hashM = hash % mod;
    auto hashM2 = hash2 % mod;
    auto hashM3 = hash3 % mod;
    auto hashM4 = hash4 % mod;
    auto hashM5 = hash5 % mod;
    auto hashM6 = hash6 % mod;
    auto hashM7 = hash7 % mod;
    auto hashM8 = hash8 % mod;
    auto hashM9 = hash9 % mod;
}
