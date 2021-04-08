#ifndef KRPSIO_H
#define KRPSIO_H

class Processor;

struct Output
{
    Output()
    {
        this->proc = nullptr;
    }

    Output(Processor* proc)
    {
        this->proc = proc;
    }
    double val = 0.0;
    Processor* proc;

    inline operator double() const
    {
        return val;
    }

    inline const Output& operator=(const double& value)
    {
        val = value;
        return *this;
    }

    inline const Output& operator+=(const double& value)
    {
        val += value;
        return *this;
    }

    inline const Output& operator*(const double& value)
    {
        val *= value;
        return *this;
    }
};

struct Input
{
    Input()
    {
        this->proc = nullptr;
        this->src = nullptr;
    }
    Input(Processor* proc, Output* src)
    {
        this->proc = proc;
        this->src = src;
    }

    inline operator double() const
    {
        return src->val;
    }

    inline operator bool() const
    {
        return (bool)src->val;
    }

    inline operator float() const
    {
        return (float)src->val;
    }

    inline const bool operator== (const bool& value)
    {
        return src->val == (double)value;
    }

    inline const bool operator!= (const bool& value)
    {
        return src->val != (double)value;
    }

    inline double operator+ (const double& value)
    {
        return src->val + value;
    }

    inline const double operator*(const double& value)
    {
        return src->val * value;
    }

    Processor* proc;
    Output* src;
};

#endif