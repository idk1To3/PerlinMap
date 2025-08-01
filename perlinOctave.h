#pragma once
#include <vector>
#include <ctime>
class perlinOctave
{
public:

	void init(int frequency)
	{
		freq = frequency;

		angles.resize(freq + 1);
		gridGradientVectors.resize(freq + 1);
		for (int i = 0; i <= freq; i++)
		{
			gridGradientVectors[i].resize(freq + 1);
			angles[i].resize(freq + 1);
		}
		initAngles();

		setGradientVectors();
	}

	struct vf2
	{
		vf2() : x(0), y(0) {}
		vf2(float _x, float _y) : x(_x), y(_y) {}
		float x;
		float y;
	};

	int freq;
	std::vector<std::vector<vf2>> gridGradientVectors;
	std::vector<std::vector<float>> angles;
	float angleOffset = 0.0f;

	void initAngles()
	{
		for (int y = 0; y < freq; y++)
		{
			for (int x = 0; x < freq; x++)
			{
				angles[y][x] = float(rand() % 1000) * 0.00628318530718f;
			}
		}
	}

	void setGradientVectors()
	{
		for (int y = 0; y < freq; y++)
		{
			for (int x = 0; x < freq; x++)
			{
				float angle = angles[y][x] + angleOffset;
				gridGradientVectors[y][x] = { cos(angle),sin(angle)};
			}
			gridGradientVectors[y][freq] = gridGradientVectors[y][0];
		}
		for (int x = 0; x <= freq; x++)
		{
			gridGradientVectors[freq][x] = gridGradientVectors[0][x];
		}

		//for (int y = 0; y <= freq; y++)
		//	for (int x = 0; x <= freq; x++)
		//		std::cout << x << "," << y << "  " << gridGradientVectors[y][x].x << ", " << gridGradientVectors[y][x].y << "\n";
	}

	float dotGridGradient(float x, float y, int offx, int offy, bool debug = false)
	{
		int ix = int(x * freq) + offx;
		int iy = int(y * freq) + offy;

		vf2 gradient = gridGradientVectors[iy][ix];

		float dx = x - (float)ix/(float)freq;
		float dy = y - (float)iy/(float)freq;

		//float dist = sqrt(dx * dx + dy * dy);
		//if(debug)
		//	std::cout << offx << offy << "dx" << dx << "dy" << dy << "dist" << dist << " ";
		//dx /= dist;
		//dy /= dist;
		dx *= freq;
		dy *= freq;

		return dx * gradient.x + dy * gradient.y;
	}

	float interpolate(float a0, float a1, float w)
	{
		//return a0 + (a1 - a0) * w;
		return a0 + (a1 - a0) * (3.0f - w * 2.0f) * w * w;
	}

	float perlin(float x, float y, bool debug = false)
	{
		x = (x >= 0 ? x - floor(x) : x - (float(int(x)) - 1.0f) );
		y = (y >= 0 ? y - floor(y) : y - (float(int(y)) - 1.0f) );
		if (x == 1.0f)
			x -= 0.001f;
		if (y == 1.0f)
			y -= 0.001f;

		float cx = floor(x * freq) / freq;
		float cy = floor(y * freq) / freq;

		float sx = (x - cx)*freq;
		float sy = (y - cy)*freq;

		float ix0 = interpolate(dotGridGradient(x, y, 0, 0), dotGridGradient(x, y, 1, 0), sx);
		float ix1 = interpolate(dotGridGradient(x, y, 0, 1), dotGridGradient(x, y, 1, 1), sx);

		if (debug)
		{
			std::cout << "  c " << cx << " " << cy << "  s " << sx << " " << sy << "   dgg " << dotGridGradient(x, y, 0, 0,true) << " " << dotGridGradient(x, y, 1, 0,true) << " " << dotGridGradient(x, y, 0, 1,true) << " " << dotGridGradient(x, y, 1,1,true) << "  ix " << ix0 << " " << ix1 << "\n";
		}

		return interpolate(ix0, ix1, sy);
	}
};

