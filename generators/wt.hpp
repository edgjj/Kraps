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

#ifndef WT_H
#define WT_H
#include <array>
#include "generator.hpp"
#include "../fft/kissfft.hpp"

#define NUM_OCTAVES 14

namespace kraps {

enum kWtInputs 
{
    kWtShiftIn = kGenGate + 1
};


class Wavetable : public Generator{
public:

    Wavetable(uint16_t);

    void fill_table_from_buffer (float*, uint32_t);
    void fill_table_from_buffer (double*, uint32_t);
    void fill_table_from_fcn (double (*fcn) (double phase));

    const nlohmann::json get_serialize_obj() override;
    void set_serialize(const nlohmann::json&) override;

    double* get_table_view () const;
    uint32_t get_shift();
    uint16_t get_wform_size();
    uint32_t get_table_size();
    ~Wavetable ();

protected:
    void process_callback () override;
    void process_params() override;
private:
    float8 pack_voices(const float8& oct, const float8& pos);

    void fill_mipmap ();

    uint32_t shift = 0;
    double phase_cst = 0.0;

    uint16_t waveform_size = 0;
    uint32_t table_size = 0;

    std::unique_ptr<double[]> table;
    std::array <std::unique_ptr<double[]>, NUM_OCTAVES> tables;

};

}

#endif 