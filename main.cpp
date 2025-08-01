#include <locale>
#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "PGEwindowsim.h"
#include "perlinOctave.h"
#include "TransformedViewWindow.h"
#include "HeightMap.h"

class PerlinMap : public Window
{
public:
	PerlinMap(olc::PixelGameEngine* pge, unsigned int id, std::string name, int width, int height, int posX, int posY, int permissions = -1) : Window(pge, id, name, width, height, posX, posY, permissions)
	{ 
                land_interp_meth = hm::linear;
                water_interp_meth = hm::linear;
                land_grad = hm::Gradient({-1.00f, -0.25f, 0.0f, 0.1f, 0.4f, 0.6f, 0.8f, 0.95, 1.0f},
                                {{163, 164, 147},
                                 {134, 142, 104},
                                 {78, 140, 118},
                                 {78, 140, 103},
                                 {135, 160, 127},
                                 {180, 179, 150},
                                 {168, 154, 139},
                                 {122, 103, 79},
                                 {255, 255, 254}
                                 });
                land_grad.setInterpolationMethod(land_interp_meth, -1);
                land_grad.setInterpolationMethod(hm::none, 7);

                water_grad = hm::Gradient({-2.0f, -1.0f, -0.5f, 0.0f},
                                {{0,0,0},
                                 {20,20,60},
                                 {0,0,255},
                                 {0,200,255}
                                });
                water_grad.setInterpolationMethod(water_interp_meth, -1);
        }

private:
	std::vector<float> values;

        bool heightBiggerThanWidth;
        int bigger_size;

	std::vector<perlinOctave> octaves;
        const int max_octaves = 10;

	olc::Sprite screenshot;

	int seed;

        bool changingSliceLimits = false;
        olc::vf2d startSlice = {0.0f, 0.0f};
        olc::vf2d endSlice = {1.0f, 0.0f};

        hm::interpMeth land_interp_meth;
        hm::interpMeth water_interp_meth;
public:
        TransformedViewWindow tvw;

public:
        bool wOnUserCreate() override
        {
                std::cout << "Controls: \n"
                        "Z to set the zoom back to 1.0\n"
                        "C to center the map\n"
                        "A to increase the angle offsets for each octave\n"
                        "R to increase the amplitude ratio\n"
                        "W to increase the water level\n"
                        "S to toggle changing slice limits\n"
                        "S + CTRL to get the seed of this map\n"
                        "UP to increase the number of octaves (up to " << max_octaves << ")\n"
                        "DOWN to decrease the number of octaves\n"
                        "Space to generate a new map with a new seed\n"
                        "F12 to save the current map\n"
                        "For A, R and W you can use shift for decreasing\n";

		seed = time(nullptr);
		srand(seed);

                std::cout << "\n" << "seed: " << seed << "\n";

		tvw.init(this);

                values.resize(WindowWidth() * WindowHeight());

		octaves.resize(numOctaves);
		for (int i = 0, freq = 4; i < numOctaves; i++, freq *= 2)
		{
			octaves[i].init(freq);
		}

		setValues();

                draw();

		return true;
        }

private:
	int numOctaves = 5;

	float amplRatio = 2.0f;

	void setValues()
	{
//                float valueScaling = getValueScaling();
//                scale = 1.0f;
//                if(pge->GetKey(olc::Key::D).bHeld)
//                        std::cout << "VS perlin: " << scale << "\n";

                float maximum = 1.0f;

		for (int y = 0; y < WindowHeight(); y++)
			for (int x = 0; x < WindowWidth(); x++)
			{
//				olc::vf2d worldPos = {float(x)/GetWindowWidth(), float(y)/GetWindowHeight()};
                                olc::vf2d worldPos = tvw.PixelToWorld({ x,y });

				int index = y * WindowWidth() + x;

                                values[index] = getValue(worldPos);

				if (values[index] > 1.0f)
					values[index] = 1.0f;
				if (values[index] < -1.0f)
					values[index] = -1.0f;
			}
	}

	float waterLevel = 0.0f;

	void clamp(int& comp)
	{
		if (comp < 0)
			comp = 0;
		if (comp > 255)
			comp = 255;
	}
private:
//	olc::Pixel heightMap(float value)
//	{
//		value = value * 0.5f + 0.5f;
//		int r = 255*(value <= waterLevel ? 0.1f : (2 * value - 1));
//		int g = 255*(value <= waterLevel ? 0.1f : (value <= waterLevel+0.2f ? value : (- 1.334f * value + 1.634f)));
//		int b = 255*(value <= waterLevel ? 0.25f + 1.5f*value : 0);
//		
//		clamp(r); clamp(g); clamp(b);
//
//		return olc::Pixel(r, g, b);
//	}

        hm::Gradient water_grad;
        hm::Gradient land_grad;
	olc::Pixel heightMap(float value)
        {
                if(value <= waterLevel)
//                        return water_grad.getColor((waterLevel >= 0.0f ? value : value - waterLevel));
                        return water_grad.getColor(value - waterLevel);
                else
                        return land_grad.getColor(value);
        }

	void draw()
	{
		for(int y = 0; y < WindowHeight(); y++)
			for (int x = 0; x < WindowWidth(); x++)
			{
				float value = getFromValuesArray(x,y);

				//Draw({ x,y }, olc::PixelF(value, value, value));
				pge->Draw({ x,y }, heightMap(value));
			}
	}

        void drawSlice()
        {
                olc::vi2d startScreen = tvw.WorldToPixel(startSlice);
                olc::vi2d endScreen = tvw.WorldToPixel(endSlice);

                pge->DrawLine(startScreen, endScreen, olc::WHITE, 0x99999999);
                pge->DrawCircle(startScreen, 2, olc::Pixel(255,0,0));
                pge->DrawCircle(endScreen, 2, olc::Pixel(0,0,255));
        }

private:
        bool redrawSignal = false;
public:
        void needToRedraw() { redrawSignal = true; }

        void recalculateAndDraw()
        {
                setValues();
                draw();
        }

        float getFromValuesArray(int x, int y)
        {
                return values[y * WindowWidth() + x];
        }

        int getSeed() { return seed; }

        int getNumberOfOctaves() { return numOctaves; }

public:
        olc::vf2d getSliceStart() { return startSlice; }
        olc::vf2d getSliceEnd() { return endSlice; }

        float getValue(olc::vf2d worldPos)
        {
                float value = 0.0f;
                float ampl = 1.0f;
                for (int i = 0; i < numOctaves; i++, ampl /= amplRatio)
                {
                        value += ampl * octaves[i].perlin(worldPos.x, worldPos.y);
                }

                return 1.4f * value;
        }

        float getWaterLevel() { return waterLevel; }
        void setWaterLevel(float newLevel) { waterLevel = newLevel; }

        bool isInSliceMode() { return changingSliceLimits; }

private:
        bool sliceInput()
        {
                bool recalculate = false;
                olc::vi2d mousePos = lGetMousePos();
                if(mousePos.x < 0)
                        return false;
                if(mousePos.y < 0)
                        return false;
                if(mousePos.x >= WindowWidth())
                        return false;
                if(mousePos.y >= WindowHeight())
                        return false;

                if(pge->GetMouse(0).bHeld)
                {
                        startSlice = tvw.PixelToWorld(mousePos);
                        recalculate = true;
                }
                if(pge->GetMouse(1).bHeld)
                {
                        endSlice = tvw.PixelToWorld(mousePos);
                        recalculate = true;
                }

                return recalculate;
        }

        bool userInput()
        {
                if(!pge->AnyKeyPressed())
                        return false;

                bool recalculate;

                if(pge->GetKey(olc::Key::M).bPressed)
                {
                        int lim = int(land_interp_meth);
                        lim++;
                        if(lim == int(hm::NR_METHODS))
                                lim = 0;
                        std::cout << lim << "\n";
                        land_interp_meth = hm::interpMeth(lim);
                        land_grad.setInterpolationMethod(land_interp_meth);
                        redrawSignal = true;
                }

                if (pge->GetKey(olc::Key::A).bPressed)
		{
			std::cout << "Angle offsets: ";
			for (int i = 0; i < numOctaves; i++)
			{
				if(pge->GetKey(olc::Key::SHIFT).bHeld)
					octaves[i].angleOffset -= 0.3926990817f;
				else
					octaves[i].angleOffset += 0.39269908167f;

                                if(octaves[i].angleOffset > 6.2831853f)
                                        octaves[i].angleOffset -= 6.2831853f;
                                else if(octaves[i].angleOffset < 0)
                                        octaves[i].angleOffset += 6.2831853f;

				std::cout << octaves[i].angleOffset << " ";

				octaves[i].setGradientVectors();
			}
			std::cout << "\n";
			
			recalculate = true;
		}

		if (pge->GetKey(olc::Key::UP).bPressed)
		{
			if (numOctaves < max_octaves)
			{
				numOctaves++;

				if (numOctaves > octaves.size())
				{
					int freq = octaves.back().freq;
					octaves.resize(octaves.size() + 1);
					octaves.back().init(2 * freq);
				}

				recalculate = true;
			}
		}

		if (pge->GetKey(olc::Key::DOWN).bPressed)
		{
			if (numOctaves > 1)
			{
				numOctaves--;
				recalculate = true;
			}
		}

		if (pge->GetKey(olc::Key::SPACE).bPressed)
		{
                        if(pge->GetKey(olc::Key::SHIFT).bHeld)
                                seed--;
                        else
                                seed++;
			srand(seed);
                        std::cout << "seed: " << seed << "\n";
			for (int i = 0; i < numOctaves; i++)
			{
				octaves[i].initAngles();
				octaves[i].setGradientVectors();
			}
			recalculate = true;
		}

		if (pge->GetKey(olc::Key::R).bPressed)
		{
			if (pge->GetKey(olc::Key::SHIFT).bHeld)
				amplRatio -= 0.1f;
			else
				amplRatio += 0.1f;

			std::cout << "amplitude ratio: " << amplRatio << "\n";

			recalculate = true;
		}

		if (pge->GetKey(olc::Key::W).bPressed)
		{
			if (pge->GetKey(olc::Key::SHIFT).bHeld)
				waterLevel -= 0.02f;
			else
				waterLevel += 0.02f;

			std::cout << "water level: " << waterLevel << "\n";

			recalculate = true;
		}

		if (pge->GetKey(olc::Key::F12).bPressed)
		{
                        olc::Sprite* screenSpritePtr = pge->GetDrawTarget();
                        std::string sFileName = "worldmap" + std::to_string(seed) + "_" + std::to_string(time(0) - seed) + ".png";
                        screenSpritePtr->SaveToFile("saved_maps\\" + sFileName);

                        std::cout << "Saved map: " << sFileName << "\n";
		}

		if (pge->GetKey(olc::Key::S).bPressed)
		{
                        if(pge->GetKey(olc::Key::CTRL).bHeld)
                                std::cout << "seed: " << seed << "\n";
                        else
                        {
                                changingSliceLimits = !changingSliceLimits;
                                recalculate = true;
                        }
		}

                if(pge->GetKey(olc::Key::C).bPressed)
                {
                        tvw.setOffset({0.0f,0.0f});

                        recalculate = true;
                }

                if(pge->GetKey(olc::Key::Z).bPressed)
                {
                        tvw.setScale(1.0f, lGetMousePos());

                        recalculate = true;
                }

                return recalculate;
        }

public:

        bool wOnUserUpdate(float fElapsedTime) override
        {
                if(isResizing)
                        values.resize(WindowWidth()*WindowHeight());

                if(!inFocus && !redrawSignal)
                {
                        return true;
                }

                bool recalculate = isResizing;

                if(changingSliceLimits && !pge->GetKey(olc::Key::SHIFT).bHeld)
                        recalculate |= sliceInput();
                else
                        recalculate |= tvw.handlePanning();

                recalculate |= tvw.handleZooming();

                recalculate |= userInput();

                recalculate |= redrawSignal;

                if(recalculate)
                {
                        recalculateAndDraw();
                        if(changingSliceLimits)
                                drawSlice();
                }

                redrawSignal = false;

                return true;
        }
};

class Info : public Window
{
public:
	Info(olc::PixelGameEngine* pge, unsigned int id, std::string name, int width, int height, int posX, int posY, int permissions = -1) : Window(pge, id, name, width, height, posX, posY, permissions)
        { }

        void setPerlinMapPtr(PerlinMap* perlin_map)
        {
                this->perlin_map = perlin_map;
        }

private:

        PerlinMap* perlin_map;

        int scale;

public:
        bool wOnUserCreate() override
        {
                return true;
        }

        bool wOnUserUpdate(float fElapsedTime) override
        {
                pge->Clear(olc::BLACK);

                int x = perlin_map->lGetMouseX();
                int y = perlin_map->lGetMouseY();

                pge->DrawString(0,0,"Zoom: " + std::to_string(perlin_map->tvw.getScale()));

                pge->DrawString(0, 30, "Number of octaves: " + std::to_string(perlin_map->getNumberOfOctaves()));

                pge->DrawString(0, 40, "Water level: " + std::to_string(perlin_map->getWaterLevel()));

                if(!perlin_map->inFocus || !perlin_map->lMouseInBounds())
                        return true;

                pge->DrawString(0,10,"Height: " + std::to_string(perlin_map->getFromValuesArray(x, y)));

                olc::vf2d wpos = perlin_map->tvw.PixelToWorld({x,y});
                pge->DrawString(0,20,"Position: " + wpos.str());

                return true;
        }

};

class Slice : public Window
{

public:
	Slice(olc::PixelGameEngine* pge, unsigned int id, std::string name, int width, int height, int posX, int posY, int permissions = -1) : Window(pge, id, name, width, height, posX, posY, permissions)
	{ }

        void setPerlinMapPtr(PerlinMap* perlin_map)
        {
                this->perlin_map = perlin_map;
        }

private:
        PerlinMap* perlin_map;

        int inputY = 0;

        int infoX = -1;

        void Draw()
        {
                pge->Clear(olc::WHITE);

                float fWL = perlin_map->getWaterLevel();
                int WL = (1.0f-(fWL+1.0f)/2.0f)*WindowHeight();

//                float valueScaling = perlin_map->getValueScaling();
//                valueScaling = 1.0f;
//                if(pge->GetKey(olc::Key::D).bHeld)
//                        std::cout << "VS slice: " << valueScaling << "\n";

                for(int wx = 0; wx < WindowWidth(); wx++)
                {
                        float t = float(wx) / WindowWidth();

                        olc::vf2d start = perlin_map->getSliceStart();
                        olc::vf2d end = perlin_map->getSliceEnd();

//                        olc::vi2d pos = perlin_map->tvw.WorldToPixel(start + (end - start) * t);
                        olc::vf2d pos = (start + (end - start) * t);

                        float v = perlin_map->getValue(pos);

                        int WH = WindowHeight();

                        int H = WH - int(((v + 1.0f) / 2.0f) * WindowHeight());

                        pge->DrawLine(wx, H, wx, WH, olc::BLACK);

                        if(H > WL)
                        {
                                pge->DrawLine(wx, WL, wx, H-1, olc::BLUE);
                        }

                        if(wx == infoX)
                                std::cout << pos << ". " << v << "\n";
                }

                if(!perlin_map->isInSliceMode())
                        return;

                pge->DrawCircle(0, WindowHeight()/2, 2, olc::Pixel(255,0,0));
                pge->DrawCircle(WindowWidth()-1, WindowHeight()/2, 2, olc::Pixel(0,0,255));
        }

public:
        bool wOnUserCreate() override
        {
                return true;
        }

        bool wOnUserUpdate(float fElapsedTime) override
        {
                Draw();

                if(!inFocus)
                        return true;

                if(!lMouseInBounds())
                        return true;

                if(pge->GetMouse(0).bHeld)
                {
                        inputY = lGetMouseY();
                        perlin_map->setWaterLevel(-(((float)inputY/WindowHeight()) * 2.0f - 1.0f));
                        perlin_map->needToRedraw();
                }

                infoX = -1;
                if(pge->GetMouse(1).bPressed)
                {
                        infoX = lGetMouseX();
                }

                return true;
        }
};

class Application : public olc::PixelGameEngine
{
public:
	Application() : win(this)
	{
		sAppName = "Perlin";
	}

        WindowList win;

        enum win_ids
        {
                perlin_window,
                slice_window,
                info_window
        };

public:
	bool OnUserCreate() override
	{
                win.addNewWindow(new PerlinMap(this, perlin_window, "Perlin map", 150, 150, 15, 10, ~(PGEws::CanClose)));
                win.addNewWindow(new Slice(this, slice_window, "Slice of terrain", 400, 150, 180, 10, ~(PGEws::CanClose)));

                win.addNewWindow(new Info(this, info_window, "Map info", 500, 60, 15, 180, ~(PGEws::CanClose)));
                
                dynamic_cast<Info*>(win.windowList[win.getIndexOfId(info_window)])->setPerlinMapPtr(dynamic_cast<PerlinMap*>(win.windowList[win.getIndexOfId(perlin_window)]));
                dynamic_cast<Slice*>(win.windowList[win.getIndexOfId(slice_window)])->setPerlinMapPtr(dynamic_cast<PerlinMap*>(win.windowList[win.getIndexOfId(perlin_window)]));

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
                Clear(olc::BLACK);

                win.updateAll(fElapsedTime);

		return true;
	}
};


int main()
{
	Application demo;
	if (demo.Construct(600, 250, 2, 2))
		demo.Start();

	return 0;
}
