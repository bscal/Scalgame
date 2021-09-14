#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <cmath>

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
		Grid(uint32_t width, uint32_t height, uint32_t tileSize)
			: Width(width), Height(height), TileSize(tileSize), m_GridArray(width * height)
		{
		}

		void DrawDebugText(uint32_t x, uint32_t y) const
		{
			std::string tmp = std::to_string(x + y * Width);
			const char* text = tmp.c_str();
			float halfTileSize = TileSize / 2.0f;
			Vector2 worldPos = ToWorldPos(x, y);
			Vector2 textSize = MeasureTextEx(*g_ResourceManager.MainFont, text, 64.0f, 0.0f);
			Rectangle rect = { worldPos.x + halfTileSize - (textSize.x / 2.0f), worldPos.y + (textSize.y / 8.0f), TileSize, TileSize };
			DrawTextRec(*g_ResourceManager.MainFont, text, rect, 64.0f, 0.0f, false, DARKGREEN);
		}

		void DrawDebugRect(uint32_t x, uint32_t y) const
		{
			Vector2 worldPos = ToWorldPos(x, y);
			Rectangle rect = { worldPos.x, worldPos.y, TileSize, TileSize };
			DrawRectangleLinesEx(rect, 1, PINK);
		}

		Vector2 ToWorldPos(uint32_t x, uint32_t y) const
		{
			float fx = x * TileSize;
			float fy = y * TileSize;
			return Vector2{ fx, fy };
		}

		inline uint32_t MousePosToIndex(const Vector2& pos) const
		{
			if (pos.x < 0.0f || pos.y < 0.0f || pos.x > m_GridArray.size() || pos.y >= m_GridArray.size())
			{
				TraceLog(LOG_ERROR, "MousePosToIndex: Position was out of bounds! x=%f.0, y=%f.0", pos.x, pos.y);
				return 0;
			}
			return floorf(pos.x / TileSize) + floorf(pos.y / TileSize) * Width;
		}

		inline std::vector<T>& GetArray() { return m_GridArray; }
		inline const T& Get(uint32_t x,uint32_t y) const { return m_GridArray[x + y * Width]; }
		inline const T& GetFromIndex(uint32_t index) const { return m_GridArray[index]; }
		/// Will set the index to value. This will not work for some types like unique_ptrs. Use Move instead
		inline void Set(uint32_t x, uint32_t y, const T& value) { m_GridArray[x + y * Width] = value; }
		/// Will preform a std::move with value into the position
		inline void Move(uint32_t x, uint32_t y, T& value) { m_GridArray[x + y * Width] = std::move(value); }
	};
}



