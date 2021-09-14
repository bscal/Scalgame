#include "HeatMapTile.h"

#include <cmath>
#include "core/GameClient.h"

namespace TheGame
{
	HeatMapTile::HeatMapTile(float min, float max, float startValue, Color cold, Color hot)
		: Min(min), Max(max), Value(startValue), ColdColor(cold), HotColor(hot), CurrentColor(cold)
	{
	}

	void HeatMapTile::Render(const Rectangle& destination, const GameClient& client)
	{
		DrawRectangleRec(destination, CurrentColor);
	}

	void HeatMapTile::Update()
	{
	}

	void HeatMapTile::SetValue(float value)
	{
		if (value < Min) Value = Min;
		else if (value > Max) Value = Max;
		else Value = value;
		SetColorFromValue();
	}

	void HeatMapTile::SetColorFromValue()
	{
		float weight = (Value - Min) / (Max - Min);
		CurrentColor = LerpColor(ColdColor, HotColor, weight);
	}

	Color LerpColor(const Color& src, const Color& dest, float weight)
	{
		uint32_t rr = src.r * (1 - weight) + dest.r * weight;
		unsigned char r = rr > 255 ? 255 : rr;

		uint32_t gg = src.g * (1 - weight) + dest.g * weight;
		unsigned char g = gg > 255 ? 255 : gg;

		uint32_t bb = src.b * (1 - weight) + dest.b * weight;
		unsigned char b = bb > 255 ? 255 : bb;

		return Color{ r, g, b, 255 };
	}
}