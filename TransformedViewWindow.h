#pragma once
#include "olcPixelGameEngine.h"
#include "PGEWindowSim.h"

class TransformedViewWindow
{
private:
        PGEws::Window* win;
	float W, H;
	float scale;

	olc::vf2d offset;
	olc::vf2d ZoomWorldMousePos;
	olc::vi2d PanPixelMousePos;
	olc::vf2d PanInitialOffset;

        float zoom_factor;
        float zoom_out_factor;
        float zoom_in_factor;

public:
	bool zoomed = false;
	bool panned = false;
	
	void init(PGEws::Window* win, float zoom_factor = 0.05f, float beginningScale = 1.0f, olc::vf2d beginningOffset = { 0.0f, 0.0f })
	{
		this->win = win;
		scale = beginningScale;
		W = (float)win->WindowWidth();
		H = (float)win->WindowHeight();
                this->zoom_factor = zoom_factor;
                zoom_out_factor = 1.0f - zoom_factor;
                zoom_in_factor = 1.0f + zoom_factor;
		offset = {-beginningOffset.x, H/W * -beginningOffset.y};
	}

        olc::vi2d DrawSprite(olc::vi2d pixel_XY, olc::Sprite* spr)
	{
		return DrawSprite(pixel_XY.x, pixel_XY.y, spr);
	}

        olc::vi2d DrawSprite(int pixel_X, int pixel_Y, olc::Sprite* spr)
	{
		//if(pge->GetKey(olc::Key::S).bPressed)
		//	std::cout << "scale: " << scale << "\n";
		//if(pge->GetKey(olc::X).bPressed)
		//	std::cout << pixel_X << "\n";
		//if(pge->GetKey(olc::Y).bPressed)
		//	std::cout << pixel_Y << "\n";

		if(scale < 1.6f) //this used to be for scale < 1.0f, but strangely i've found it also works quite well for values between 1.0 and 1.6
//                if(pge->GetKey(olc::Key::SPACE).bHeld)
		{
			int width = int((float)spr->Size().x * scale);
			int height = int((float)spr->Size().y * scale);

                        int x = (pixel_X < 0 ? -pixel_X : 0);
                        int y = (pixel_Y < 0 ? -pixel_Y : 0);
			for(; y < height; y++)
			{
				if(pixel_Y + y > win->WindowHeight())
                                {
                                        return {pixel_X + x - 1, pixel_Y + y - 1};
                                }

				for(x = (pixel_X < 0 ? -pixel_X : 0); x < width; x++)
				{
					if(pixel_X + x > win->WindowWidth())
						break;

					win->pge->Draw(pixel_X + x, pixel_Y + y, spr->GetPixel(x/scale, y/scale));
				}
			}

                        return {pixel_X + x - 1, pixel_Y + y - 1};
		}
		else // optimization for when zoomed in (a lot)
		{
			int width = spr->Size().x;
			int height = spr->Size().y;

			// in this branch, on the right and upper borders there is black, fix

                        const float min_y = (pixel_Y < 0 ? (float)pixel_Y - int(pixel_Y/scale) * scale : pixel_Y);
			const float min_x = (pixel_X < 0 ? (float)pixel_X - int(pixel_X/scale) * scale : pixel_X);
			float x = min_x;
			float y = min_y;

			const int min_u = (pixel_X < 0 ? float(-pixel_X)/scale : 0);
			const int min_v = (pixel_Y < 0 ? float(-pixel_Y)/scale : 0);

                        int scaledWidth;
                        int scaledHeight;

			for(int v = min_v; v < height; v++)
			{
				if(y >= win->WindowHeight())
                                {
//                                        int upperLeft_x = int(min_x);
//                                        int upperLeft_y = int(min_y);

                                        x -= scale; y -= scale;

                                        int bottomRight_x = (int)x + scaledWidth;
                                        int bottomRight_y = (int)y + scaledHeight;

                                        return {bottomRight_x - 1, bottomRight_y - 1};
                                }

				x = min_x;

				scaledHeight = int(y + scale) - int(y);

				for(int u = min_u; u < width; u++)
				{
					if(x >= win->WindowWidth())
						break;

					scaledWidth = int(x + scale) - int(x);

					win->pge->FillRect((int)x, (int)y, scaledWidth, scaledHeight, spr->GetPixel(u, v));

					x += scale;
				}

				y += scale;
			}

                        x -= scale; y -= scale;

                        int bottomRight_x = (int)x + scaledWidth;
                        int bottomRight_y = (int)y + scaledHeight;

                        return {bottomRight_x - 1, bottomRight_y - 1};
		}

                return {0,0};
	}

	olc::vf2d PixelToWorld(olc::vi2d PixelPos)
	{
		return (olc::vf2d(PixelPos) / (W * scale) + offset);
	}

	olc::vi2d WorldToPixel(olc::vf2d WorldPos)
	{
		return olc::vi2d(W * (WorldPos - offset) * scale);
	}
	olc::vi2d WorldToPixel(int x, int y)
	{
		return WorldToPixel(olc::vf2d{(float)x, (float)y});
	}



	bool handlePanning()
	{
		if (win->pge->GetMouse(0).bHeld)
		{
			if (win->pge->GetMouse(0).bPressed)
			{
				PanPixelMousePos = win->lGetMousePos();
				PanInitialOffset = offset;
			}

			offset = PanInitialOffset + olc::vf2d(PanPixelMousePos - win->lGetMousePos()) / (scale * W);

			panned = true;
			return true;
		}
		panned = false;
		return false;
	}

	bool handleZooming()
	{
		int mouseWheel = win->pge->GetMouseWheel();

		if (mouseWheel != 0)
		{
			if (mouseWheel < 0)
			{
				scale *= zoom_out_factor;
				offset = offset - (olc::vf2d(win->lGetMousePos()) / (W * scale)) * zoom_factor;
			}
			else
			{
				scale *= zoom_in_factor;
				offset = offset + (olc::vf2d(win->lGetMousePos()) / (W * scale)) * zoom_factor;
			}

			zoomed = true;
			return true;
		}

		zoomed = false;
		return false;
	}

	bool handlePanZoom()
	{
		return handlePanning() || handleZooming();
	}

        void setScale(float newScale, olc::vi2d zoomInPlace = {0,0})
        {
                float factor = newScale / scale;

                scale = newScale;
                offset = offset + (olc::vf2d(zoomInPlace) / (W * scale)) * (factor - 1.0f);
        }

        float getScale() { return scale; }

        void setOffset(olc::vf2d newOffset) { offset = newOffset; }
};

