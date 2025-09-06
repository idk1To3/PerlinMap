/*
The Pixel Game Engine Window Simulator, by 1To3

Minimal example:

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#define PGEWS_APPLICATION
#include "PGEwindowsim.h"

class MyWindow : public PGEws::Window
{
public:
        MyWindow(olc::PixelGameEngine* pge, unsigned int id, std::string name, int posX, int posY, int width, int height, int permissions = -1) : Window(pge, id, name, posX, posY, width, height, permissions)
{ }

        bool wOnUserCreate() override
        {
                return true;
        }

        bool wOnUserUpdate(float fElapsedTime) override
        {
                for (int x = 0; x < WindowWidth(); x++)
                        for (int y = 0; y < WindowHeight(); y++)
                                pge->Draw(x, y, olc::Pixel(rand() % 256, rand() % 256, rand() % 256));

                return true;
        }
};

class Application : public olc::PixelGameEngine
{
public:
        Application() : win(this)
        {
                sAppName = "Application";
        }

        PGEws::WindowList win;

        bool OnUserCreate() override
        {
                win.addNewWindow(new MyWindow(this,  0, "Name", 0, 0, 130, 130));
                win.setScale(0, 2);

                return true;
        }

        bool OnUserUpdate(float fElapsedTime) override
        {
                Clear(olc::BLACK);
                win.updateAll(fElapsedTime);

                return true;
        }

        bool OnUserDestroy() override
        {
                win.destroyAll();

                return true;
        }
};

int main()
{
        Application demo;
        if (demo.Construct(300, 300, 2, 2))
                demo.Start();

        return 0;
}

*/

#pragma once
#include "olcPixelGameEngine.h"
#include <unordered_map>
#include <stdexcept>

namespace PGEws
{
        class WindowList;

        enum permission
        {
                CanClose = 1,
                CanMove = 2,
                CanResizeX = 4,
                CanResizeY = 8,
        };

        class Window
        {
                friend class WindowList;

                public:
                Window(olc::PixelGameEngine* pge, unsigned int id, std::string name, int posX, int posY, int width, int height, int permissions = -1);

                public:
                olc::PixelGameEngine* const pge;
                WindowList* parentWindowList;

                unsigned int const id;

                public:
                bool hidden = false;

                bool hasBanner = true;
                int bannerHeight = 11;

                bool canClose = true;
                bool canMove = true;
                bool canResizeX = true;
                bool canResizeY = true;

                private:
                std::shared_ptr<olc::Sprite> content;

                bool inFocus = false;
                bool lostFocus = false;
                bool gainedFocus = false;

                int bodyDraggingMouseType = 2;
                //-1 - body dragging disabled, 0 - left click dragging, 1 - right click dragging, 2 - middle click dragging

                bool resizing = false;

                int nameMax = 0;
                std::string name;
                std::string nameTrimmed;

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
                olc::vi2d lGetMousePos();
                int lGetMouseX();
                int lGetMouseY();
                bool lMouseInBounds();

                int getScale();

                int WindowWidth();
                int WindowHeight();

                int realWindowWidth();
                int realWindowHeight();

                bool isInFocus();
                bool hasLostFocus();
                bool hasGainedFocus();

                bool isResizing();
                std::string getName();

                Window* getWindow(unsigned int id);

                private:
                void updateNameMax();
                void trimName();
                void drawBanner();

                void drawBorder();

                public:
                void lClear(olc::Pixel color);


                void changePermissions(int flags);


                void setMaxFps(bool value, float fps = 50.0f);

                void setHidden(bool value);

                void setBodyDraggingType(int value);

                void setIfHasBanner(bool value);

                void setBannerHeight(int height);

                void setPosition(int x, int y);

                void setRealPosition(int x, int y);

                void setSize(int w, int h);

                void setScale(unsigned int scale);


                virtual bool wOnUserCreate();

                virtual bool wOnUserUpdate(float fElapsedTime);

                virtual void wOnUserDestroy();

                void update(float fElapsedTime);
        };

        class WindowList
        {
                public:
                        WindowList(olc::PixelGameEngine* pge);
                        olc::PixelGameEngine* pge;

                        std::vector<Window*> windowList;

                private:
                        std::list<int> orderedIndices;

                        bool leftClickSelected = false;
                        olc::vi2d selectionOffset;
                        int selectingMouseType = 0;

                        bool resizing = false;
                        bool resizingWindowLeft = false;
                        bool resizingWindowDown = false;
                        bool resizingWindowRight = false;
                        bool resizingWindowUp = false;

                        int focusIndex = -1;

                public:
                        int getFocusedId();

                        int getIndexOfId(unsigned int id);

                public:
                        void updateAll(float fElapsedTime);

                        void addNewWindow(Window* window);

                        bool deleteWindow(unsigned int id, bool callWOnUserDestroy = true);

                        bool changeFocusedWindow(unsigned int id);

                        bool setSize(unsigned int id, int sizeX, int sizeY);

                        bool setPosition(unsigned int id, int posX, int posY);

                        bool setRealPosition(unsigned int id, int posX, int posY);

                        bool setScale(unsigned int id, int scale);

                        bool setMaxFPS(unsigned int id, float fps);

                        bool setHidden(unsigned int id, bool value);

                        bool toggleHidden(unsigned int id);

                        bool setBodyDraggingType(unsigned int id, int value);

                        bool setIfHasBanner(unsigned int id, bool value);

                        bool setBannerHeight(unsigned int id, int value);

                        void destroyAll();

                private:
                        void changeFocused(std::list<int>::iterator indexIt);

                        void addWindowAndUpdate(Window* window);

                        void deleteWindowAndUpdate(std::list<int>::iterator indexIt);

                private:
                        bool rectContainsPoint(int x, int y, int rx0, int ry0, int rx1, int ry1);

                        void resizeWindow();

                        void updateOnClick(std::list<int>::iterator index, int mousePosX, int mousePosY, bool startMoving, bool closingClick);

                        void moveWindows();
        };

#ifdef PGEWS_APPLICATION
#undef PGEWS_APPLICATION

        //Window implementation

        Window::Window(olc::PixelGameEngine* pge, unsigned int id, std::string name, int posX, int posY, int width, int height, int permissions) : id(id), pge(pge), name(name), posX(posX+1), posY(posY+bannerHeight)
        {
                changePermissions(permissions);

                sizeX = width;
                sizeY = height;
                updateNameMax();
                trimName();
                content = std::make_shared<olc::Sprite>(width, height);
        };


        olc::vi2d Window::lGetMousePos() { return { (pge->GetMouseX() - posX)/scale, (pge->GetMouseY() - posY)/scale }; }
        int Window::lGetMouseX() { return (pge->GetMouseX() - posX)/scale; }
        int Window::lGetMouseY() { return (pge->GetMouseY() - posY)/scale; }

        bool Window::lMouseInBounds()
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

        int Window::getScale() { return scale; }

        int Window::WindowWidth() { return sizeX; }
        int Window::WindowHeight() { return sizeY; }

        int Window::realWindowWidth() { return sizeX + 2; }
        int Window::realWindowHeight() { return sizeY + 1 + (hasBanner ? bannerHeight : 1); }

        bool Window::isInFocus() { return inFocus; }
        bool Window::hasLostFocus() { return lostFocus; }
        bool Window::hasGainedFocus() { return gainedFocus; }

        bool Window::isResizing() { return resizing; }
        std::string Window::getName() { return name; }

        Window* Window::getWindow(unsigned int id)
        {
                int index = parentWindowList->getIndexOfId(id);

                if(index == -1)
                        return nullptr;
                else 
                        return parentWindowList->windowList[index];
        }


        inline void Window::updateNameMax()
        {
                nameMax = (sizeX*scale - (canClose ? 14 : 2)) / 8;
        }

        void Window::trimName()
        {
                nameTrimmed.clear();

                int length = 0;

                for(const auto& c : name)
                {
                        if(c == '\n')
                        {
                                length = 0;
                                nameTrimmed += "\n";
                        }
                        else if(length < nameMax)
                        {
                                nameTrimmed += c;
                                length++;
                        }
                }
        }

        void Window::drawBanner()
        {
                if (!hasBanner)
                        return;

                pge->FillRect(posX, posY - bannerHeight, sizeX*scale, bannerHeight-1, (inFocus ? olc::Pixel(190, 190, 190) : olc::Pixel(140, 140, 140)));

                pge->DrawLine(posX-1, posY - bannerHeight, posX+sizeX*scale, posY - bannerHeight, (inFocus ? olc::Pixel(255, 255, 255) : olc::Pixel(193, 193, 193)));
                pge->DrawLine(posX-1, posY - bannerHeight + 1, posX-1, posY - 1, (inFocus ? olc::Pixel(255, 255, 255) : olc::Pixel(193, 193, 193)));

                pge->DrawLine(posX+sizeX*scale, posY - bannerHeight + 1, posX+sizeX*scale, posY - 1, olc::Pixel(110,110,110));

                if (sizeX*scale >= 10)
                {
                        pge->DrawString(posX + 1, posY - bannerHeight + 2, nameTrimmed, olc::BLACK);
                        if (canClose)
                                pge->DrawString(posX + sizeX*scale - 9, posY - bannerHeight/2 - 4, "X", olc::DARK_RED);
                }
        }

        void Window::drawBorder()
        {
                pge->DrawRect(posX - 1, posY - 1, sizeX*scale + 1, sizeY*scale + 1, (inFocus ? olc::Pixel(110,110,110) : olc::Pixel(50, 50, 50)));
        }


        void Window::lClear(olc::Pixel color) { pge->FillRect(0, 0, sizeX, sizeY, color); }

        void Window::changePermissions(int flags)
        {
                canClose = flags & PGEws::CanClose;
                canMove = flags & PGEws::CanMove;
                canResizeX = flags & PGEws::CanResizeX;
                canResizeY = flags & PGEws::CanResizeY;
        }
        void Window::setMaxFps(bool value, float fps)
        {
                maxFpsSet = value;
                timeThreshold = 1.0f / fps;
                frameTimer = timeThreshold;
        }

        void Window::setBodyDraggingType(int value)
        {
                bodyDraggingMouseType = value;
        }

        void Window::setHidden(bool value)
        {
                hidden = value;
        }

        void Window::setIfHasBanner(bool value) { hasBanner = value; };

        void Window::setPosition(int x, int y) { posX = x; posY = y; }

        void Window::setRealPosition(int x, int y) { posX = x+1; posY = (hasBanner ? y + bannerHeight : y + 1); }

        void Window::setSize(int w, int h)
        {
                sizeX = w; sizeY = h;
                updateNameMax();
                trimName();

                std::shared_ptr<olc::Sprite> newContent = std::make_shared<olc::Sprite>(sizeX, sizeY);
                pge->SetDrawTarget(newContent.get());
                pge->DrawSprite(0, 0, content.get());
                content = newContent;
        }

        void Window::setScale(unsigned int scale)
        {
                this->scale = scale;
                updateNameMax();
                trimName();
        }


        bool Window::wOnUserCreate()
        {
                return true;
        }

        bool Window::wOnUserUpdate(float fElapsedTime)
        {
                return false;
        }

        void Window::wOnUserDestroy()
        {
        }

        void Window::update(float fElapsedTime)
        {
                pge->SetDrawTarget(content.get());
                if (maxFpsSet)
                {
                        if (frameTimer >= timeThreshold)
                        {
                                if (!wOnUserUpdate(frameTimer))
                                        destruct = true;

                                lostFocus = false;
                                gainedFocus = false;

                                frameTimer = 0.0f;
                        }
                        frameTimer += fElapsedTime;
                }
                else
                {
                        if (!wOnUserUpdate(fElapsedTime))
                                destruct = true;

                        lostFocus = false;
                        gainedFocus = false;
                }

                pge->SetDrawTarget(nullptr);
                if (!hidden)
                {
                        pge->DrawSprite(posX, posY, content.get(), scale);
                        drawBanner();
                        drawBorder();
                }
        }

        //WindowList implementation

        WindowList::WindowList(olc::PixelGameEngine* pge) : pge(pge) { }

        int WindowList::getFocusedId()
        {
                if (focusIndex == -1)
                        return -1;
                return windowList[focusIndex]->id;
        }

        int WindowList::getIndexOfId(unsigned int id)
        {
                for (int i = 0; i < windowList.size(); i++)
                        if (windowList[i]->id == id)
                                return i;

                return -1;
        }

        void WindowList::updateAll(float fElapsedTime)
        {
                resizeWindow();

                moveWindows();

                for (auto i = orderedIndices.rbegin(); i != orderedIndices.rend(); i++)
                        windowList[*i]->update(fElapsedTime);

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

        void WindowList::addNewWindow(Window* window)
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

        bool WindowList::deleteWindow(unsigned int id, bool callWOnUserDestroy)
        {
                int i = getIndexOfId(id);
                if (i == -1) return false;

                if(callWOnUserDestroy)
                        windowList[i]->wOnUserDestroy();

                return true;
        }

        bool WindowList::changeFocusedWindow(unsigned int id)
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

        bool WindowList::setSize(unsigned int id, int sizeX, int sizeY)
        {
                int i = getIndexOfId(id);
                if (i == -1) return false;

                windowList[i]->setSize(sizeX, sizeY);

                return true;
        }

        bool WindowList::setPosition(unsigned int id, int posX, int posY)
        {
                int i = getIndexOfId(id);
                if (i == -1) return false;

                windowList[i]->setPosition(posX, posY);

                return true;
        }

        bool WindowList::setRealPosition(unsigned int id, int posX, int posY)
        {
                int i = getIndexOfId(id);
                if (i == -1) return false;

                windowList[i]->setRealPosition(posX, posY);

                return true;
        }

        bool WindowList::setScale(unsigned int id, int scale)
        {
                int i = getIndexOfId(id);
                if (i == -1) return false;

                windowList[i]->setScale(scale);

                return true;
        }

        bool WindowList::setMaxFPS(unsigned int id, float fps)
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

        bool WindowList::setBodyDraggingType(unsigned int id, int value)
        {
                int i = getIndexOfId(id);
                if (i == -1) return false;

                windowList[i]->setBodyDraggingType(value);

                return true;
        }

        bool WindowList::setHidden(unsigned int id, bool value)
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

        bool WindowList::toggleHidden(unsigned int id)
        {
                for (auto i = orderedIndices.begin(); i != orderedIndices.end(); i++)
                {
                        if (windowList[*i]->id == id)
                        {
                                bool value = !windowList[*i]->hidden;
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

        bool WindowList::setIfHasBanner(unsigned int id, bool value)
        {
                int i = getIndexOfId(id);
                if(i == -1) return false;

                windowList[i]->setIfHasBanner(value);

                return true;
        }

        bool WindowList::setBannerHeight(unsigned int id, int value)
        {
                int i = getIndexOfId(id);
                if(i == -1) return false;

                int realPosY = windowList[i]->posY - windowList[i]->bannerHeight;

                windowList[i]->bannerHeight = value;

                windowList[i]->posY = realPosY + value;

                return true;
        }

        void WindowList::destroyAll()
        {
                for (int i = 0; i < windowList.size(); i++)
                        delete windowList[i];
        }

        void WindowList::changeFocused(std::list<int>::iterator indexIt)
        {
                if(focusIndex != *indexIt)
                {
                        windowList[focusIndex]->lostFocus = true;
                        windowList[*indexIt]->gainedFocus = true;
                }

                windowList[focusIndex]->inFocus = false;
                focusIndex = *indexIt;
                windowList[focusIndex]->inFocus = true;

                int value = *indexIt;
                orderedIndices.erase(indexIt);
                orderedIndices.push_front(value);
        }

        void WindowList::addWindowAndUpdate(Window* window)
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

                window->parentWindowList = this;

                pge->SetDrawTarget(windowList.back()->content.get());
                if (!windowList.back()->wOnUserCreate())
                        windowList.back()->destruct = true;
                pge->SetDrawTarget(nullptr);
        }

        void WindowList::deleteWindowAndUpdate(std::list<int>::iterator indexIt)
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

        inline bool WindowList::rectContainsPoint(int x, int y, int rx0, int ry0, int rx1, int ry1)
        {
                return (x >= rx0 && x <= rx1 && y >= ry0 && y <= ry1);
        }

        void WindowList::resizeWindow()
        {
                if (!windowList[focusIndex]->canResizeX && !windowList[focusIndex]->canResizeY)
                        return;

                if (!windowList[focusIndex]->resizing)
                {
                        if (pge->GetMouse(0).bPressed)
                        {
                                int mouseX = pge->GetMouseX();
                                int mouseY = pge->GetMouseY();
                                Window& win = *windowList[focusIndex];
                                if(win.hidden)
                                        return;
                                int scaledSizeX = win.sizeX * win.scale;
                                int scaledSizeY = win.sizeY * win.scale;
                                int bh = (win.hasBanner ? win.bannerHeight : 0);
                                if (rectContainsPoint(mouseX, mouseY, win.posX + scaledSizeX, win.posY - bh, win.posX + scaledSizeX + 8, win.posY + scaledSizeY))
                                {
                                        resizingWindowRight = true;
                                        resizing = true;
                                        if (mouseY > win.posY + scaledSizeY - 12)
                                                resizingWindowDown = true;
                                }
                                else if (rectContainsPoint(mouseX, mouseY, win.posX - 9, win.posY - bh, win.posX-1, win.posY + scaledSizeY))
                                {
                                        resizingWindowLeft = true;
                                        resizing = true;
                                        if (mouseY > win.posY + scaledSizeY - 12)
                                                resizingWindowDown = true;
                                }
                                else if(rectContainsPoint(mouseX, mouseY, win.posX - 9, win.posY + scaledSizeY, win.posX + scaledSizeX + 8, win.posY + scaledSizeY + 8))
                                {
                                        resizingWindowDown = true;
                                        resizing = true;
                                        if (mouseX < win.posX + 12)
                                                resizingWindowLeft = true;
                                        else if (mouseX > win.posX + scaledSizeX - 12)
                                                resizingWindowRight = true;
                                }
                                else if(rectContainsPoint(mouseX, mouseY, win.posX - 9, win.posY - bh - 8, win.posX + scaledSizeX + 8, win.posY - bh))
                                {
                                        resizingWindowUp = true;
                                        resizing = true;
                                        if(mouseX < win.posX + 12)
                                                resizingWindowLeft = true;
                                        else if(mouseX > win.posX + scaledSizeX - 12)
                                                resizingWindowRight = true;
                                }

                                windowList[focusIndex]->resizing = resizing;
                        }
                }
                else
                {
                        int scale = windowList[focusIndex]->scale;
                        int bannerHeight = (windowList[focusIndex]->hasBanner ? windowList[focusIndex]->bannerHeight : 0);
                        if (pge->GetMouse(0).bReleased)
                        {
                                resizingWindowLeft = false;
                                resizingWindowDown = false;
                                resizingWindowRight = false;
                                resizingWindowUp = false;
                                resizing = false;
                                windowList[focusIndex]->resizing = false;
                        }
                        int newSizeX;
                        int newSizeY;

                        if (resizingWindowDown && windowList[focusIndex]->canResizeY)
                        {
                                newSizeY = pge->GetMouseY() - windowList[focusIndex]->posY;
                        }
                        else if(resizingWindowUp && windowList[focusIndex]->canResizeY)
                        {
                                newSizeY = windowList[focusIndex]->sizeY * scale + windowList[focusIndex]->posY - ((pge->GetMouseY()+bannerHeight)/scale + 1)*scale + 1;
                                windowList[focusIndex]->posY = ((pge->GetMouseY()+bannerHeight)/scale + 1)*scale - 1;
                        }
                        else
                        {
                                newSizeY = windowList[focusIndex]->sizeY * scale;
                        }

                        if (resizingWindowRight && windowList[focusIndex]->canResizeX)
                        {
                                newSizeX = pge->GetMouseX() - windowList[focusIndex]->posX;
                        }
                        else if(resizingWindowLeft && windowList[focusIndex]->canResizeX)
                        {
                                newSizeX = windowList[focusIndex]->sizeX * scale + windowList[focusIndex]->posX - (pge->GetMouseX()/scale + 1)*scale;
                                windowList[focusIndex]->posX = (pge->GetMouseX()/scale + 1)*scale;
                        }
                        else
                        {
                                newSizeX = windowList[focusIndex]->sizeX * scale;
                        }

                        bool Xclamped = false; //Ensures the window is always wide enough for the X button
                        if (newSizeX < 11)
                        {
                                if(windowList[focusIndex]->canClose && windowList[focusIndex]->hasBanner)
                                {
                                        Xclamped = true;
                                        newSizeX = (10/scale + 1)*scale;
                                }
                        }
                        if (newSizeX <= scale && !Xclamped)
                        {
                                newSizeX = scale;
                        }
                        if (newSizeY <= scale)
                                newSizeY = scale;
                        windowList[focusIndex]->setSize(newSizeX/scale, newSizeY/scale);
                }
        }

        void WindowList::updateOnClick(std::list<int>::iterator index, int mousePosX, int mousePosY, bool startMoving, bool closingClick)
        {
                int scale = windowList[*index]->scale;

                int oldFocusIndex = focusIndex;
                windowList[focusIndex]->inFocus = false;
                focusIndex = *index;
                windowList[*index]->inFocus = true;

                if(focusIndex != oldFocusIndex)
                {
                        windowList[oldFocusIndex]->lostFocus = true;
                        windowList[focusIndex]->gainedFocus = true;
                }

                int value = *index;

                orderedIndices.erase(index);
                orderedIndices.push_front(value);

                //                if (mousePosY < windowList[value]->posY && windowList[value]->hasBanner)
                if(startMoving)
                {
                        if (closingClick && windowList[value]->canClose && mousePosX >= windowList[value]->posX + windowList[value]->sizeX * scale - 9)
                        {
                                windowList[value]->destruct = true;
                                focusIndex = oldFocusIndex;
                                windowList[focusIndex]->inFocus = true;
                        }
                        else
                        {
                                leftClickSelected = true;
                                //make *i the focusIndex and make the previous focused window unfocused
                                selectionOffset = { mousePosX - windowList[value]->posX, mousePosY - windowList[value]->posY };
                        }
                }
        }

        void WindowList::moveWindows()
        {
                if (!leftClickSelected)
                {
                        if ((pge->GetMouse(0).bPressed || pge->GetMouse(1).bPressed || pge->GetMouse(2).bPressed) && !resizing)
                        {
                                int mousePosX = pge->GetMouseX();
                                int mousePosY = pge->GetMouseY();

                                for (auto i = orderedIndices.begin(); i != orderedIndices.end(); i++)
                                {
                                        if (windowList[*i]->hidden)
                                                continue;
                                        Window* w = windowList[*i];

                                        bool dragging = false;
                                        bool closingClick = false;

                                        if(!rectContainsPoint(mousePosX, mousePosY, 
                                                                w->posX, w->posY - (w->hasBanner ? w->bannerHeight : 0) + 1, 
                                                                w->posX + w->sizeX * w->scale, w->posY + w->sizeY * w->scale))
                                                continue;

                                        if(windowList[*i]->bodyDraggingMouseType >= 0 && pge->GetMouse(windowList[*i]->bodyDraggingMouseType).bPressed) //body dragging
                                        {
                                                selectingMouseType = windowList[*i]->bodyDraggingMouseType;
                                                dragging = true;
                                        }
                                        else if(pge->GetMouse(0).bPressed && mousePosY < w->posY) //banner dragging
                                        {
                                                selectingMouseType = 0;
                                                dragging = true;
                                                closingClick = true;
                                        }

                                        updateOnClick(i, mousePosX, mousePosY, dragging, closingClick);

                                        break;
                                }
                        }
                }
                else
                {
                        if (pge->GetMouse(selectingMouseType).bReleased)
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

#endif
};
