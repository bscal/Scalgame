#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "GameHeaders.h"
#include "core/ResourceManager.h"

namespace TheGame
{
	template <typename T>
	class Grid
	{
	public:
		uint32_t Width, Height;
		const uint32_t TileSize;

	private:
		std::vector<T> m_GridArray;

	public:
		Grid(const uint32_t& width, const uint32_t& height, const uint32_t& tileSize)
			: Width(width), Height(height), TileSize(tileSize), m_GridArray(width * height)
		{
		}

		void DrawDebugText(const uint32_t& x, const uint32_t& y) const
		{
			float halfTileSize = TileSize / 2.0f - 8.0f;
			Vector2 worldPos = ToWorldPos(x, y);
			Rectangle rect = { worldPos.x + halfTileSize, worldPos.y + 8.0f, TileSize, TileSize };
			std::string tmp = std::to_string(x + y * Width);
			DrawTextRec(*g_ResourceManager.MainFont, tmp.c_str(), rect, 64.0f, 0.0f, false, PINK);
		}

		void DrawDebugRect(const uint32_t& x, const uint32_t& y) const
		{
			Vector2 worldPos = ToWorldPos(x, y);
			Rectangle rect = { worldPos.x, worldPos.y, TileSize, TileSize };
			DrawRectangleLinesEx(rect, 1, PINK);
		}

		Vector2 ToWorldPos(const uint32_t& x, const uint32_t& y) const
		{
			float fx = x * TileSize;
			float fy = y * TileSize;
			return Vector2{ fx, fy };
		}

		inline const std::vector<T>& GetArray() const { return m_GridArray; }
		inline T& Get(const uint32_t& x, const uint32_t& y) const { return m_GridArray[x + y * Width]; }
		inline T& GetFromIndex(const uint32_t& index) const { return m_GridArray[index]; }
		inline void Set(const uint32_t& x, const uint32_t& y, const T& value) { m_GridArray[x + y * Width] = value; }
		inline uint32_t MousePosToIndex(const Vector2& pos) const { return (pos.x / TileSize) + (pos.y / TileSize) * Width; }
	};
}



