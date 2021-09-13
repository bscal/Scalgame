#pragma once

#include "Tile.h"
#include "GameHeaders.h"

namespace TheGame
{
	Color LerpColor(const Color& src, const Color& dest, float weight);

	class HeatMapTile : Tile
	{
	public:
		const Color ColdColor;
		const Color HotColor;
		const Rectangle TileRect;
		const float Min, Max;

	private:
		float Value;
		Color CurrentColor;
	public:
		HeatMapTile(float min, float max, float startValue, Color cold, Color hot, Rectangle tileRect);
		// Inherited via Tile
		virtual void Render(const GameClient& client) override;
		virtual void Update() override;

		void SetValue(float value);
		inline float GetValue() const { return Value; }
		inline Color GetColor() const { return CurrentColor; }

	private:
		void SetColorFromValue();
	};
}