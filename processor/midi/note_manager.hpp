﻿#ifndef KRPSNOTEMGR_H
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
	void process_params() { ; }
	void recalculate_sr() { ; }
private:
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
		int note_number = -1;
		int velocity = -1;
		double timestamp = -1;
	};


	int global_timestamp = 0;
	int block_size = 0;

	double bar_size = 0;

	std::vector<Note> notes;
	std::deque<Note> queue;
	Note cur_played_note;
};

}

#endif