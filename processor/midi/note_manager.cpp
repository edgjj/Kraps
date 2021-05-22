#include "note_manager.hpp"
#include <algorithm>

namespace kraps {

NoteManager::NoteManager() : Processor (p_notemgr, 0, 4)
{
	params.push_back (440.0);
	params_constrainments.push_back(std::make_pair<double, double>(400, 500));

	io_description[1] =
	{
		{ kNoteMgrFreq, "FREQ", "Current played note frequency."},
		{ kNoteMgrAmp, "VELO", "Note velocity converted to CV."},
		{ kNoteMgrGate, "GATE", "Just gate."},
		{ kNoteMgrSync, "SYNC", "DAW tempo sync. Reports bars per second in Hz."}
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

void NoteManager::reset()
{
	notes.clear();
	queue.clear();
}

void NoteManager::upd_timestamp(int timestamp)
{
	global_timestamp = timestamp;
}

void NoteManager::upd_tempo(int time_sig_numerator, int time_sig_denominator, double tempo)
{
	int basic_dem = 4;
	double ratio = (double)basic_dem / time_sig_denominator;
	bar_size = tempo / (time_sig_numerator * ratio * 60);
	*outputs[kNoteMgrSync] = bar_size;
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

	for (auto it = notes.begin(); it != notes.end();)
	{
		Note cur = *it;
		
		if (cur.timestamp != global_timestamp)
		{
			++it;
			continue;
		}

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
		it = notes.erase(it);
	}

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