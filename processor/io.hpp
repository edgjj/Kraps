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
    double val = 0.0;

    int32_t id = 0;
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

    int32_t id = 0;
    Processor* proc;
    Output* src;
};

}
}
#endif