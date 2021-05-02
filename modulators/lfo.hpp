#ifndef LFO_H
#define LFO_H
#include <map>
#include "../generators/generator.hpp"
namespace kraps {

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

    std::pair<std::vector<Vec2>, std::vector<double>> get_points();
    void add_point (Vec2);
    void move_point (int, Vec2);
    void set_tension (int, double);
    void remove_point (int);
    void inc_phase();


    ~LFO() { ; }
protected:
    void process_callback () override;
    void process_params() override;
private:
    inline double sigmoid (double x, double k);
    double get_interp (double x);

    bool is_env = false;
    double phase_const = 1 / (2 * M_PI);

    std::vector<Vec2> points;
    std::vector <double> tension; // == N + 2 / 2
};

}
#endif