//
// Created by Mehmet Fatih BAKIR on 07/12/2016.
//

#pragma once

#include "quad.h"
namespace e2e
{
    class drawable_base
    {
    public:
        virtual void draw() = 0;
        virtual ~drawable_base() = 0;
    };

    inline drawable_base::~drawable_base() {}

    template <class DrawT>
    class drawable : public drawable_base
    {
        DrawT& obj;

    public:

        explicit drawable(DrawT& to_draw) : obj{to_draw}
        {}

        void draw() override
        {
            obj.draw();
        }

        ~drawable() override = default;
    };

    template <class T>
    auto make_drawable(T& obj)
    {
        return drawable<T> {obj};
    }
}
