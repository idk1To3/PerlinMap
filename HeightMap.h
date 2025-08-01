#pragma once
#include "olcPixelGameEngine.h"
#include <vector>

namespace hm
{
        enum interpMeth
        {
                none,
                abrupt,
                linear,
                squared,
                cubed,
                smooth,
                NR_METHODS
        };
        class Gradient
        {
        public:
                Gradient(){}
                Gradient(std::vector<float>&& endpoints, std::vector<olc::Pixel>&& colors) : endpoints(endpoints), colors(colors)
                { 
                        interpMeths = std::vector<interpMeth>(endpoints.size(), linear);
                }

                void setInterpolationMethod(interpMeth method, int index = -1)
                {
                        if(index == -1)
                                for(int i = 0; i < interpMeths.size(); i++)
                                        interpMeths[i] = method;
                        else if(index >= 0 && index < interpMeths.size())
                                interpMeths[index] = method;
                        else
                                std::cout << "Invalid index\n";
                }

                void setAllInterpolationMethod(const std::vector<interpMeth>& methods)
                {
                        for(int i = 0; i < interpMeths.size() && i < methods.size(); i++)
                        {
                                interpMeths[i] = methods[i];
                        }
                }

        private:
                std::vector<float> endpoints;

                std::vector<olc::Pixel> colors;

                std::vector<interpMeth> interpMeths;

        public:
                olc::Pixel getColor(float value)
                {
                        if(value < endpoints[0])
                                return colors[0];
                        if(value >= endpoints.back())
                                return colors.back();

                        int index = 0;

                        while(endpoints[index] <= value)
                                index++;

                        index--;

                        float t;
                        if(interpMeths[index] == none)
                                t = 0.0f;
                        else
                                t = (value - endpoints[index]) / (endpoints[index+1] - endpoints[index]);

                        switch(interpMeths[index])
                        {
                                case none:
                                case linear:
                                        break;
                                case abrupt:
                                        t = (t < 0.5f ? 0.0f : 1.0f);
                                        break;
                                case squared:
                                        t = t*t;
                                        break;
                                case cubed:
                                        t = t*t*t;
                                        break;
                                case smooth:
                                        {
                                                float a = 2.0f*t-1.0f;
                                                t = a / (a * a + 1) + 0.5f;
                                        }
                                        break;
                        }

                        int r = colors[index+1].r - colors[index].r;
                        int g = colors[index+1].g - colors[index].g;
                        int b = colors[index+1].b - colors[index].b;

                        r *= t;
                        g *= t;
                        b *= t;

                        r += colors[index].r;
                        g += colors[index].g;
                        b += colors[index].b;

                        return olc::Pixel(r, g, b);
                }

        };
}
