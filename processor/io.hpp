/* Copyright 2021 Yegor Suslin
 *
 * This file is part of Kraps library.
 *
 * Kraps is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kraps is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kraps.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simd/avir_float8_avx.h"

#ifndef KRPSIO_H
#define KRPSIO_H

namespace kraps{
class Processor;
namespace io {

struct Output
{
    Output(uint32_t _id)
    {
        this->proc = nullptr;
        id = _id;
    }

    Output(Processor* proc, uint32_t _id)
    {
        this->proc = proc;
        id = _id;
    }
    float8 val = 0.0;

    int32_t id = 0;
    Processor* proc;

    inline operator double() const
    {
        return val;
    }

    inline const Output& operator=(const float8& value)
    {
        val = value;
        return *this;
    }

    inline const Output& operator+=(const float8& value)
    {
        val += value;
        return *this;
    }

    inline const Output& operator*(const float8& value)
    {
        val *= value;
        return *this;
    }
};

struct Input
{

    Input(uint32_t _id)
    {
        this->proc = nullptr;
        this->src = nullptr;
        id = _id;
    }
    Input(Processor* proc, Output* src, uint32_t _id)
    {
        this->proc = proc;
        this->src = src;
        id = _id;
    }


    inline operator float8() const
    {
        return src->val;
    }

    inline float8 operator== (const float8& value)
    {
        return src->val == value;
    }

    inline float8 operator!= (const float8& value)
    {
        return src->val != value;
    }

    inline double operator+ (const float8& value)
    {
        return src->val + value;
    }

    inline double operator*(const float8& value)
    {
        return src->val * value;
    }

    int32_t id = 0;
    Processor* proc;
    Output* src;
};

}
}
#endif