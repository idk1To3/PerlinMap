#pragma once
#include "olcPixelGameEngine.h"
#include <unordered_map>
#include <stdexcept>

namespace PGEws
{
	enum permission
	{
		CanClose = 1,
		CanMove = 2,
		CanResizeX = 4,
		CanResizeY = 8,
	};
}

class Window
{
public:
	Window(olc::PixelGameEngine* pge, unsigned int id, std::string name, int width, int height, int posX, int posY, int permissions = -1) : id(id), pge(pge), name(name), posX(posX), posY(posY)
	{
		changePermissions(permissions);

		sizeX = width;
		sizeY = height;
		nameMax = (sizeX - (canClose ? 18 : 2)) / 8;
		content = std::make_shared<olc::Sprite>(width, height);
	};

public:
	olc::PixelGameEngine* const pge;
	unsigned int const id;

public:
	std::shared_ptr<olc::Sprite> content;

	bool inFocus = false;
	bool hidden = false;
        bool gainedFocus = false;
        bool isResizing = false;

	bool hasBanner = true;
	int bannerHeight = 10;
	std::string name;
	int nameMax = 0;

	bool canClose = true;
	bool canMove = true;
	bool canResizeX = true;
	bool canResizeY = true;

	int posX;
	int posY;
	int sizeX;
	int sizeY;
	int scale = 1;

	bool maxFpsSet = false;
	float timeThreshold = 0.02f;
	float frameTimer = 0.0f;

	bool destruct = false;

public:

	void changePermissions(int flags)
	{
		canClose = flags & PGEws::CanClose;
		canMove = flags & PGEws::CanMove;
		canResizeX = flags & PGEws::CanResizeX;
		canResizeY = flags & PGEws::CanResizeY;
	}

public:
	olc::vi2d lGetMousePos() { return { (pge->GetMouseX() - posX)/scale, (pge->GetMouseY() - posY)/scale }; }
	int lGetMouseX() { return (pge->GetMouseX() - posX)/scale; }
	int lGetMouseY() { return (pge->GetMouseY() - posY)/scale; }

        bool lMouseInBounds()
        {
                if(pge->GetMouseX() < posX)
                        return false;
                if(pge->GetMouseY() < posY)
                        return false;
                if(lGetMouseX() > WindowWidth())
                        return false;
                if(lGetMouseY() > WindowHeight())
                        return false;
                return true;
        }

	void lClear(olc::Pixel color) { pge->FillRect(0, 0, sizeX, sizeY, color); }

	int WindowWidth() { return sizeX; }
	int WindowHeight() { return sizeY; }

private:
        bool wasInFocus = false;

private:
	void drawBanner()
	{
		if (!hasBanner)
			return;

		pge->FillRect(posX, posY - bannerHeight, sizeX*scale, bannerHeight-1, (inFocus ? olc::Pixel(190, 190, 190) : olc::Pixel(140, 140, 140)));
		pge->DrawRect(posX-1, posY - bannerHeight, sizeX*scale+1, bannerHeight-1, olc::Pixel(110, 110, 110));
		if (sizeX*scale > 10)
		{
			pge->DrawString(posX + 1, posY - bannerHeight + 2, name.substr(0, nameMax), olc::BLACK);
			if (canClose)
				pge->DrawString(posX + sizeX*scale - 9, posY - bannerHeight + 2, "X", olc::DARK_RED);
		}
	}

	void drawBorder()
	{
		olc::Pixel color(110, 110, 110);
		pge->DrawRect(posX - 1, posY - 1, sizeX*scale + 1, sizeY*scale + 1, color);
	}

public:
	void setMaxFps(bool value, float fps = 50.0f)
	{
		maxFpsSet = value;
		timeThreshold = 1.0f / fps;
		frameTimer = timeThreshold;
	}

	void setHidden(bool value)
	{
		hidden = value;
	}

	void setIfHasBanner(bool value) { hasBanner = value; };

	void setPosition(int x, int y) { posX = x; posY = y; }

	void setSize(int w, int h)
	{
		sizeX = w; sizeY = h;
		nameMax = (sizeX*scale - (canClose ? 18 : 2)) / 8;

		std::shared_ptr<olc::Sprite> newContent = std::make_shared<olc::Sprite>(sizeX, sizeY);
		pge->SetDrawTarget(newContent.get());
		pge->DrawSprite(0, 0, content.get());
		content = newContent;
	}

	void setScale(unsigned int scale)
	{
		this->scale = scale;
		nameMax = (sizeX*scale - (canClose ? 18 : 2)) / 8;
	}

	virtual bool wOnUserCreate()
	{
		return true;
	}

	virtual bool wOnUserUpdate(float fElapsedTime)
	{
		return false;
	}

	virtual void wOnUserDestroy()
	{
	}

	void update(float fElapsedTime)
	{
		pge->SetDrawTarget(content.get());
		if (maxFpsSet)
		{
			if (frameTimer >= timeThreshold)
			{
				if (!wOnUserUpdate(frameTimer))
					destruct = true;
				frameTimer = 0.0f;
			}
			frameTimer += fElapsedTime;
		}
		else
		{
			if (!wOnUserUpdate(fElapsedTime))
				destruct = true;
		}

		pge->SetDrawTarget(nullptr);
		if (!hidden)
		{
			pge->DrawSprite(posX, posY, content.get(), scale);
			drawBanner();
			drawBorder();
		}
//                frameAfterPress0 = false;
//                frameAfterPress1 = false;
//                if(pge->GetMouse(0).bPressed)
//                        frameAfterPress0 = true;
//                if(pge->GetMouse(1).bPressed)
//                        frameAfterPress1 = true;
	}
};

class WindowList
{
public:
	WindowList(olc::PixelGameEngine* pge) : pge(pge) { }
	olc::PixelGameEngine* pge;

	std::vector<Window*> windowList;

private:
	std::list<int> orderedIndices;

	bool leftClickSelected = false;
	olc::vi2d selectionOffset;
	bool resizingWindowLeft = false;
	bool resizingWindowDown = false;
	bool resizingWindowRight = false;
	int focusIndex = -1;

public:
	int getFocusedId()
	{
		if (focusIndex == -1)
			return -1;
		return windowList[focusIndex]->id;
	}

	int getIndexOfId(unsigned int id)
	{
		for (int i = 0; i < windowList.size(); i++)
			if (windowList[i]->id == id)
				return i;
		return -1;
	}
		
public:
	void updateAll(float fElapsedTime)
	{
		resizeWindow();

		for (auto i = orderedIndices.rbegin(); i != orderedIndices.rend(); i++)
                {
			windowList[*i]->update(fElapsedTime);

                        windowList[*i]->gainedFocus = false;
                }

		moveWindows();

		for (auto i = orderedIndices.begin(); i != orderedIndices.end(); ++i)
		{
			if (windowList[*i]->destruct)
			{
				windowList[*i]->wOnUserDestroy();

				deleteWindowAndUpdate(i);

				break;
			}
		}
	}

	void addNewWindow(Window* window)
	{
		try 
		{
			addWindowAndUpdate(window);
		}
		catch (const std::invalid_argument& error) 
		{
			std::cerr << "Error: " << error.what() << "\n";

			delete window;
		}
	}

	bool deleteWindow(unsigned int id, bool callWOnUserDestroy = true)
	{
		int i = getIndexOfId(id);
		if (i == -1) return false;

		if(callWOnUserDestroy)
			windowList[i]->wOnUserDestroy();

		return true;
	}

	bool changeFocusedWindow(unsigned int id)
	{
		for (auto i = orderedIndices.begin(); i != orderedIndices.end(); i++)
		{
			if (windowList[*i]->id == id)
			{
				changeFocused(i);

				return true;
			}
		}

		return false;
	}

	bool setSize(unsigned int id, int sizeX, int sizeY)
	{
		int i = getIndexOfId(id);
		if (i == -1) return false;

		windowList[i]->setSize(sizeX, sizeY);

		return true;
	}

	bool setPosition(unsigned int id, int posX, int posY)
	{
		int i = getIndexOfId(id);
		if (i == -1) return false;

		windowList[i]->setPosition(posX, posY);

		return true;
	}

	bool setScale(unsigned int id, int scale)
	{
		int i = getIndexOfId(id);
		if (i == -1) return false;

		windowList[i]->setScale(scale);

		return true;
	}

	bool setMaxFPS(unsigned int id, float fps)
	{
		int i = getIndexOfId(id);
		if (i == -1) return false;

		if (fps <= 0.0f)
		{
			windowList[i]->setMaxFps(false);
		}
		else
		{
			windowList[i]->setMaxFps(true, fps);
		}

		return true;
	}

	bool setHidden(unsigned int id, bool value)
	{
		for (auto i = orderedIndices.begin(); i != orderedIndices.end(); i++)
		{
			if (windowList[*i]->id == id)
			{
				windowList[*i]->setHidden(value);

				if (value == false)
				{
					changeFocused(i);
				}
				
				return true;
			}
		}
		return false;
	}

	void destroyAll()
	{
		for (int i = 0; i < windowList.size(); i++)
			delete windowList[i];
	}

private:
	void changeFocused(std::list<int>::iterator indexIt)
	{
		windowList[focusIndex]->inFocus = false;
		focusIndex = *indexIt;
		windowList[focusIndex]->inFocus = true;

		int value = *indexIt;
		orderedIndices.erase(indexIt);
		orderedIndices.push_front(value);
	}

	void addWindowAndUpdate(Window* window)
	{
		int idGiven = window->id;
		for (const auto& w : windowList)
		{
			if (w->id == idGiven)
			{
				throw std::invalid_argument("non-unique id " + std::to_string(idGiven) + " given to window \"" + window->name + "\".");
			}
		}

		windowList.push_back(window);
		orderedIndices.push_back(windowList.size() - 1);

		if (focusIndex == -1)
		{
			focusIndex = 0;
			windowList[0]->inFocus = true;
		}

		pge->SetDrawTarget(windowList.back()->content.get());
		if (!windowList.back()->wOnUserCreate())
			windowList.back()->destruct = true;
                pge->SetDrawTarget(nullptr);
	}

	void deleteWindowAndUpdate(std::list<int>::iterator indexIt)
	{
		int value = *indexIt;
		bool wasInFocus = windowList[value]->inFocus;
		delete windowList[value];
		windowList.erase(windowList.begin() + value);
		orderedIndices.erase(indexIt);

		for (auto& oi : orderedIndices)
			if (oi > value)
				oi--;
		if (wasInFocus)
		{
			windowList[orderedIndices.front()]->inFocus = true;
			focusIndex = orderedIndices.front();
		}
		if (focusIndex > value)
			focusIndex--;
	}
private:

	bool rectContainsPoint(int x, int y, int rx0, int ry0, int rx1, int ry1)
	{
		return (x >= rx0 && x <= rx1 && y >= ry0 && y <= ry1);
	}

	void resizeWindow()
	{
		if (!windowList[focusIndex]->canResizeX && !windowList[focusIndex]->canResizeY)
			return;
		
		if (!(resizingWindowLeft || resizingWindowDown || resizingWindowRight))
		{
			if (pge->GetMouse(0).bPressed)
			{
				int mouseX = pge->GetMouseX();
				int mouseY = pge->GetMouseY();
				Window& win = *windowList[focusIndex];
				int scaledSizeX = win.sizeX * win.scale;
				int scaledSizeY = win.sizeY * win.scale;
				if (rectContainsPoint(mouseX, mouseY, win.posX + scaledSizeX, win.posY - win.bannerHeight, win.posX + scaledSizeX + 7, win.posY + scaledSizeY))
				{
					resizingWindowRight = true;
                                        windowList[focusIndex]->isResizing = true;
					if (mouseY > win.posY + scaledSizeY - 12)
						resizingWindowDown = true;
				}
				else if (rectContainsPoint(mouseX, mouseY, win.posX - 8, win.posY - win.bannerHeight, win.posX-1, win.posY + scaledSizeY))
				{
					resizingWindowLeft = true;
                                        windowList[focusIndex]->isResizing = true;
					if (mouseY > win.posY + scaledSizeY - 12)
						resizingWindowDown = true;
				}
				else if(rectContainsPoint(mouseX, mouseY, win.posX - 8, win.posY + scaledSizeY, win.posX + scaledSizeX + 7, win.posY + scaledSizeY + 8))
				{
					resizingWindowDown = true;
                                        windowList[focusIndex]->isResizing = true;
					if (mouseX < win.posX + 12)
						resizingWindowLeft = true;
					else if (mouseX > win.posX + scaledSizeX - 12)
						resizingWindowRight = true;
				}
			}
		}
		else
		{
			int scale = windowList[focusIndex]->scale;
			if (pge->GetMouse(0).bReleased)
			{
				resizingWindowLeft = false;
				resizingWindowDown = false;
				resizingWindowRight = false;
                                windowList[focusIndex]->isResizing = false;
			}
			int newSizeX;
			int newSizeY;

			if (resizingWindowDown && windowList[focusIndex]->canResizeY)
				newSizeY = pge->GetMouseY() - windowList[focusIndex]->posY;
			else
				newSizeY = windowList[focusIndex]->sizeY * scale;

			if (resizingWindowRight && windowList[focusIndex]->canResizeX)
			{
				newSizeX = pge->GetMouseX() - windowList[focusIndex]->posX;
			}
			else if(resizingWindowLeft && windowList[focusIndex]->canResizeX)
			{
				newSizeX = windowList[focusIndex]->sizeX * scale + windowList[focusIndex]->posX - pge->GetMouseX() - 1;
				windowList[focusIndex]->posX = pge->GetMouseX() + 1;
			}
			else
			{
				newSizeX = windowList[focusIndex]->sizeX * scale;
			}

			if (newSizeX < 1)
				newSizeX = scale;
			if (newSizeY < 1)
				newSizeY = scale;
			windowList[focusIndex]->setSize(newSizeX/scale, newSizeY/scale);
		}
	}

	void moveWindows()
	{
		if (!leftClickSelected)
		{
			if (pge->GetMouse(0).bPressed && !resizingWindowLeft && !resizingWindowDown && !resizingWindowRight)
			{
				int mousePosX = pge->GetMouseX();
				int mousePosY = pge->GetMouseY();

				for (auto i = orderedIndices.begin(); i != orderedIndices.end(); i++)
				{
					if (windowList[*i]->hidden)
						continue;
					int scale = windowList[*i]->scale;

					if (mousePosY >= windowList[*i]->posY - windowList[*i]->bannerHeight && mousePosY < windowList[*i]->posY + windowList[*i]->sizeY * scale)
						if (mousePosX >= windowList[*i]->posX && mousePosX < windowList[*i]->posX + windowList[*i]->sizeX * scale)
						{
							int oldFocusIndex = focusIndex;
							windowList[focusIndex]->inFocus = false;
							focusIndex = *i;
							windowList[*i]->inFocus = true;
                                                        windowList[*i]->gainedFocus = true;

							int value = *i;
							orderedIndices.erase(i);
							orderedIndices.push_front(value);

							if (mousePosY < windowList[value]->posY)
							{
								if (!windowList[value]->canClose || mousePosX < windowList[value]->posX + windowList[value]->sizeX * scale - 9)
								{
									leftClickSelected = true;
									//make *i the focusIndex and make the previous focused window unfocused
									selectionOffset = { mousePosX - windowList[value]->posX, mousePosY - windowList[value]->posY };
								}
								else
								{
									windowList[value]->destruct = true;
									focusIndex = oldFocusIndex;
									windowList[focusIndex]->inFocus = true;
								}
							}

							break;
						}
				}
			}
		}
		else
		{
			if (pge->GetMouse(0).bReleased)
			{
				leftClickSelected = false;
			}
			else if (windowList[focusIndex]->canMove)
			{
				windowList[focusIndex]->posX = pge->GetMouseX() - selectionOffset.x;
				windowList[focusIndex]->posY = pge->GetMouseY() - selectionOffset.y;
			}
		}
	}
};
