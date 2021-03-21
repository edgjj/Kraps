#pragma once
#include <vector>
#include "../processor.hpp"

struct Note
{
	uint8_t note_number;
	uint8_t velocity;
};

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
	void NoteOn(uint8_t note_number, uint8_t velocity);
	void NoteOff(uint8_t note_number, uint8_t velocity);
	void AllNotesOff();
protected:
	void process_callback();
	void process_params() { ; }
	void recalculate_sr() { ; }
private:
	std::vector<Note> notes;
	std::vector<Output*> outputs;
};