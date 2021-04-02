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
    float val = 0.0;

    Processor* proc;

    inline operator double() const
    {
        return this->val;
    }


    inline const Output& operator=(const double& val)
    {
        this->val = val;
        return *this;
    }

    inline const Output& operator+=(const double& val)
    {
        this->val += val;
        return *this;
    }

    inline const Output& operator*(const double& val)
    {
        this->val *= val;
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

    inline const bool operator== (const bool& val)
    {
        return src->val == val;
    }

    inline const bool operator!= (const bool& val)
    {
        return src->val != val;
    }

    inline double operator+ (const double& val)
    {
        return src->val + val;
    }

    inline const double operator*(const double& val)
    {
        return src->val * val;
    }

    Processor* proc;
    Output* src;
};

#endif