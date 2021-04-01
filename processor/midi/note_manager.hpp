#pragma once
#include <vector>
#include "../processor.hpp"



enum kNoteMgrOutputs
{
	kNoteMgrFreq,
	kNoteMgrAmp,
	kNoteMgrGate,
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
	void set_block_size(int samples_per_block);
protected:
	void process_callback();
	void process_params() { ; }
	void recalculate_sr() { ; }
private:
	enum kNoteEventType
	{
		kNoteOn,
		kNoteOff,
		kAllNotesOff
	};

	struct Note
	{
		int note_number;
		int velocity;
		int life_time;
		double time_stamp;
	};
	int global_timestamp = 0;
	int block_size = 0;
	std::vector<Note> notes;

};