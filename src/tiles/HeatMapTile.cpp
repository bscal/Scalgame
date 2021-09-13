#include "HeatMapTile.h"

#include <cmath>
#include "core/GameClient.h"

namespace TheGame
{
	HeatMapTile::HeatMapTile(float min, float max, float startValue, Color cold, Color hot, Rectangle tileRect)
		: Min(min), Max(max), Value(startValue), ColdColor(cold), HotColor(hot), TileRect(tileRect), CurrentColor(cold)
	{
	}

	void HeatMapTile::Render(const GameClient& client)
	{
		DrawRectangleRec(TileRect, CurrentColor);
	}

	void HeatMapTile::Update()
	{
	}

	void HeatMapTile::SetValue(float value)
	{
		if (value < Min) Value = Min;
		else if (value > Max) Value = Max;
		else Value = value;
	}

	void HeatMapTile::SetColorFromValue()
	{
		float weight = Value - Min / Max - Min;
		CurrentColor = LerpColor(ColdColor, HotColor, weight);
	}

	Color LerpColor(const Color& src, const Color& dest, float weight)
	{
		unsigned int i;

		i = weight * src.r + weight * dest.r;
		unsigned char r = i > 255 ? 255 : i;

		i = weight * src.g + weight * dest.g;
		unsigned char g = i > 255 ? 255 : i;

		i = weight * src.b + weight * dest.b;
		unsigned char b = i > 255 ? 255 : i;

		i = weight * src.a + weight * dest.a;
		unsigned char a = i > 255 ? 255 : i;

		return Color{ r, g, b, a };
	}
}