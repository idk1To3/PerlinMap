#pragma once
#include "olcPixelGameEngine.h"
#include "PGEwindowsim.h"

class TransformedViewWindow
{
private:
	Window* win;
	float W, H;

	float scale;
	olc::vf2d offset;
	olc::vf2d ZoomWorldMousePos;
	olc::vi2d PanPixelMousePos;
	olc::vf2d PanInitialOffset;


public:
	bool zoomed = false;
	bool panned = false;
	void init(Window* win, float beginningScale = 1.0f, olc::vf2d beginningOffset = { 0.0f, 0.0f })
	{
		this->win = win;
		scale = beginningScale;
		W = (float)win->WindowWidth();
		H = (float)win->WindowHeight();
                beginningOffset.y -= 0.5f;
		offset = olc::vf2d{ 0.0f, H / (2.0f * W) } / scale + beginningOffset;

	}

	olc::vf2d PixelToWorld(olc::vi2d PixelPos)
	{
		return (olc::vf2d(PixelPos) / (W * scale) + offset)/* * olc::vf2d(1.0f, -1.0f)*/;
	}

	olc::vi2d WorldToPixel(olc::vf2d WorldPos)
	{
		return olc::vi2d(W * (WorldPos /* * olc::vf2d(1.0f, -1.0f) */ - offset) * scale);
	}

	bool handlePanning()
	{
		if (win->pge->GetMouse(0).bHeld)
		{
                        bool oob = win->lGetMouseX() < 0 || win->lGetMouseX() > win->WindowWidth() || win->lGetMouseY() < 0 || win->lGetMouseY() > win->WindowHeight();
                        if(oob || !win->inFocus)
                        {
                                panned = false;
                                return false;
                        }

			if (win->pge->GetMouse(0).bPressed || win->gainedFocus)
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
				scale *= 0.95f;
				offset = offset - (olc::vf2d(win->lGetMousePos()) / (W * scale)) * 0.05f;
			}
			else
			{
				scale *= 1.05f;
				offset = offset + (olc::vf2d(win->lGetMousePos()) / (W * scale)) * 0.05f;
			}

			zoomed = true;
			return true;
		}

		zoomed = false;
		return false;
	}

        void setScale(float newScale, olc::vi2d zoomInPlace = {0,0})
        {
                float factor = newScale / scale;

                scale = newScale;
                offset = offset + (olc::vf2d(zoomInPlace) / (W * scale)) * (factor - 1.0f);
        }

        float getScale() { return scale; }

        void setOffset(olc::vf2d newOffset) { offset = newOffset; }

	bool handlePanZoom()
	{
		return handlePanning() || handleZooming();
	}
};

