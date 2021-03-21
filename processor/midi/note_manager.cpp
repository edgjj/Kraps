#include "note_manager.hpp"

NoteManager::NoteManager() : Processor (p_misc, 0, 2)
{
	params.push_back (440.0);
}

void NoteManager::NoteOn(uint8_t note_number, uint8_t velocity)
{
	notes.push_back({ note_number, velocity });
}

void NoteManager::NoteOff(uint8_t note_number, uint8_t velocity)
{
	for (auto it = notes.begin(); it != notes.end(); it++)
	{
		if (it->note_number == note_number)
		{
			notes.erase(it);
			return;
		}
	}
}

void NoteManager::AllNotesOff()
{
	notes.clear();
}

void NoteManager::process_callback()
{
	*outputs[kNoteMgrFreq] = pow(2, (notes.back().note_number - 69) / 12) * params[0];
	*outputs[kNoteMgrGate] = notes.empty();
	*outputs[kNoteMgrAmp] = notes.back().velocity / 127.0;
}

NoteManager::~NoteManager()
{

}