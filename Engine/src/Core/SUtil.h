#pragma once

#include "Core.h"
#include "SHash.hpp"
#include "SMemory.h"

#include <assert.h>
#include <stdint.h>

inline float ClampF(float min, float max, float value)
{
    return fmaxf(min, fminf(max, value));
}

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


//inline Mesh GenMeshPlaneXY(float width, float length, int resX, int resZ)
//{
//    Mesh mesh = { 0 };
//    int vertexCount = resX * resZ; // vertices get reused for the faces
//    float hw = width / 2.0f;
//    float hh = length / 2.0f;
//    Vector3* vertices = (Vector3*)RL_MALLOC(vertexCount * sizeof(Vector3));
//    vertices[0] = { -hw, -hh, 0.0f };
//    vertices[1] = { -hw, hh, 0.0f };
//    vertices[2] = { hw, hh, 0.0f };
//    vertices[3] = { hw, -hh, 0.0f };
//
//    // Normals definition
//    Vector3* normals = (Vector3*)RL_MALLOC(vertexCount * sizeof(Vector3));
//    for (int n = 0; n < vertexCount; n++)
//        normals[n] = { 0.0f, 1.0f, 0.0f };   // Vector3.up;
//
//    // TexCoords definition
//    Vector2* texcoords = (Vector2*)RL_MALLOC(vertexCount * sizeof(Vector2));
//    texcoords[0] = { 0.0f, 1.0f }; // x, h
//    texcoords[1] = { 0.0f, 0.0f }; // x, w
//    texcoords[2] = { 1.0f, 0.0f }; // y, w
//    texcoords[3] = { 1.0f, 1.0f }; // y, h
//
//    // Triangles definition (indices)
//    //int numFaces = (resX) * (resZ);
//    int* triangles = (int*)RL_MALLOC(6 * sizeof(int));
//    triangles[0] = 2;
//    triangles[1] = 1;
//    triangles[2] = 0;
//    triangles[3] = 0;
//    triangles[4] = 3;
//    triangles[5] = 2;
//
//    mesh.vertexCount = vertexCount;
//    mesh.triangleCount = 2;
//    mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
//    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
//    mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
//    mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));
//
//    // Mesh vertices position array
//    for (int i = 0; i < mesh.vertexCount; i++)
//    {
//        mesh.vertices[3 * i] = vertices[i].x;
//        mesh.vertices[3 * i + 1] = vertices[i].y;
//        mesh.vertices[3 * i + 2] = vertices[i].z;
//    }
//
//    // Mesh texcoords array
//    for (int i = 0; i < mesh.vertexCount; i++)
//    {
//        mesh.texcoords[2 * i] = texcoords[i].x;
//        mesh.texcoords[2 * i + 1] = texcoords[i].y;
//    }
//
//    // Mesh normals array
//    for (int i = 0; i < mesh.vertexCount; i++)
//    {
//        mesh.normals[3 * i] = normals[i].x;
//        mesh.normals[3 * i + 1] = normals[i].y;
//        mesh.normals[3 * i + 2] = normals[i].z;
//    }
//
//    // Mesh indices array initialization
//    for (int i = 0; i < mesh.triangleCount * 3; i++)
//        mesh.indices[i] = triangles[i];
//
//    RL_FREE(vertices);
//    RL_FREE(normals);
//    RL_FREE(texcoords);
//    RL_FREE(triangles);
//    // Upload vertex data to GPU (static mesh)
//    UploadMesh(&mesh, true);
//    return mesh;
//}
