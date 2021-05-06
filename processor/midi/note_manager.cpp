#include "note_manager.hpp"
#include <algorithm>

namespace kraps {

NoteManager::NoteManager() : Processor (p_notemgr, 0, 3)
{
	params.push_back (440.0);
	params_constrainments.push_back(std::make_pair<double, double>(400, 500));

	io_description[1] =
	{
		{ kNoteMgrFreq, "FREQ", "Current played note frequency."},
		{ kNoteMgrAmp, "VELO", "Note velocity converted to CV."},
		{ kNoteMgrGate, "GATE", "Just gate."}
	};
}

void NoteManager::note_on(int note_number, int velocity, double timestamp)
{
	Note p;
	p = { kNoteOn, note_number, velocity, timestamp };
	notes.push_back (p);
}

void NoteManager::note_off(int note_number, int velocity, double timestamp)
{
	Note p;
	p = { kNoteOff, note_number, velocity, timestamp };
	notes.push_back(p);
}

void NoteManager::all_notes_off(double timestamp)
{
	Note p;
	p = { kAllNotesOff, -1, -1, timestamp };
	notes.push_back(p);
}

void NoteManager::set_block_size(int samples_per_block)
{
	block_size = samples_per_block;
	notes.clear();
}

void NoteManager::upd_timestamp(int timestamp)
{
	global_timestamp = timestamp;
}

void NoteManager::process_callback()
{
	if (!queue.empty() && cur_played_note.type == kEmpty)
	{
		cur_played_note = queue.back();
		queue.pop_back();
		*outputs[kNoteMgrGate] = 1.0;
		*outputs[kNoteMgrFreq] = pow(2, (cur_played_note.note_number - 69) / 12.0) * params[0]; // possible to do RMS smoothing to make "slides"
		*outputs[kNoteMgrAmp] = cur_played_note.velocity / 127.0;
	}

	if (notes.empty())
		return;

	for (int i = 0; i < notes.size(); i++)
	{
		Note cur = notes[i];
		if (cur.timestamp != global_timestamp)
			continue;

		switch (cur.type)
		{
		case kNoteOn:
			if (cur_played_note.type != kEmpty)
			{
				queue.push_back(cur_played_note);
				cur_played_note = Note();
				queue.push_back(cur);
				*outputs[kNoteMgrGate] = 0.0;
				break;
			}
			else
			{
				queue.push_back(cur);
			}
			break;
		case kNoteOff:
			if (cur_played_note.note_number == cur.note_number)
			{
				cur_played_note = Note();
				*outputs[kNoteMgrGate] = 0.0;
				*outputs[kNoteMgrAmp] = cur.velocity / 127.0;
			}
			else
			{
				for (auto iter = queue.begin(); iter != queue.end(); )
				{
					if (iter->note_number == cur.note_number)
					{
						iter = queue.erase(iter);
					}
					else
					{
						++iter;
					}
				}
			}
			break;
		case kAllNotesOff:
			*outputs[kNoteMgrGate] = 0.0;
			cur_played_note = Note();
			queue.clear();
			break;
		}
	}

	if (global_timestamp == block_size - 1)
		notes.clear();
}

void NoteManager::process_bypass()
{
	if (!queue.empty() || !notes.empty())
	{
		queue.clear();
		notes.clear();
		cur_played_note = Note();
	}
}

NoteManager::~NoteManager()
{

}

}