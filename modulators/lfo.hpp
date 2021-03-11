#ifndef LFO_H
#define LFO_H
#include <map>
#include "../generators/generator.hpp"

struct Vec2 
{
    double x = 0.0;
    double y = 0.0;
};


enum kLFOInputs 
{
    kLFOFreqIn,
    kLFOPhaseIn,
    kLFOGate
};

enum kLFOOutputs 
{
    kLFOAudioOut,
    kLFOPhaseOut
};

class LFO : public Generator
{
public:
    LFO();
    
    void preprocess ();
    void add_point (Vec2);
    void move_point (int, Vec2);
    void set_tension (int, double);
    void remove_point (int);

    ~LFO() { ; }
protected:
    

    void process_callback () override;
    void process_params () override { ; }
private:
    double sigmoid (double x, double k);
    double phase_const = 0.0;

    std::array<double, 2048> lookup = { 0.0 };

    std::vector<Vec2> points;
    std::vector <double> tension; // == N + 2 / 2
};

#endif