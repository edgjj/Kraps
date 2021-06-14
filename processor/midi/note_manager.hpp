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

#ifndef KRPSNOTEMGR_H
#define KRPSNOTEMGR_H
#include <vector>
#include <deque>
#include "../processor.hpp"

// m = 12*log2(fm/440 Hz) + 69 and fm =  2(m−69)/12(440 Hz).

namespace kraps {

enum kNoteMgrOutputs
{
	kNoteMgrFreq,
	kNoteMgrAmp,
	kNoteMgrGate,
	kNoteMgrSync
};

class NoteManager : public Processor
{
public:
	NoteManager();
	~NoteManager();

	void note_on (int note_number, int velocity, double timestamp);
	void note_off (int note_number, int velocity, double timestamp);
	void all_notes_off (double timestamp);
	void upd_timestamp(int timestamp);
	void reset();
	void upd_tempo(int time_sig_numerator, int time_sig_denominator, double tempo);
protected:
	void process_callback();
	void process_bypass();
	void process_params() override;
	void recalculate_sr() { ; }
private:
	void process_simd();



	enum kNoteEventType
	{
		kNoteOn,
		kNoteOff,
		kAllNotesOff,
		kEmpty
	};
	struct Note
	{
		kNoteEventType type = kEmpty;
		int note_number = 0;
		int velocity = 0;
		double timestamp = -1;
	};


	int global_timestamp = 0;
	int block_size = 0;

	double bar_size = 0;

	float8 a3_tune;
	float8 porta_time;


	

	bool is_mono = false, is_legato = false, is_always_porta =false;


	std::vector<Note> notes;
	std::array<Note, 8> voices = { Note() };

	int top_note_num = 0;


	std::array<float, 127> note_lookup;

	std::deque<Note> queue;
};

}

#endif