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

void NoteManager::process_simd()
{
	while (!queue.empty())
	{
		auto it = std::find_if(voices.begin(), voices.end(), [](const Note& n) { return n.type == kEmpty; });
		if (it != voices.end())			
			*it = queue.back();
		else
			voices[7] = queue.back();

		queue.pop_back();
	}
	
	float freq[8], velo[8], gate[8];

	

#pragma loop(hint_parallel(8))
	for (int i = 0; i < 8; i++)
	{
		if (voices[i].type == kEmpty)
			gate[i] = 0;
		else
			gate[i] = 1;
		
		freq[i] = pow(2, (voices[i].note_number - 69) / 12.0) * params[0];
		velo[i] = voices[i].velocity / 127.f;
	}

	using f8 = float8;
	f8* gatep = &outputs[kNoteMgrGate]->val;
	f8* velop = &outputs[kNoteMgrAmp]->val;
	f8* freqp = &outputs[kNoteMgrFreq]->val;

	*gatep = gatep->load(gate);
	*velop = velop->load(velo);
	*freqp = freqp->load(freq);
}

void NoteManager::process_callback()
{

	
	//if (!queue.empty() && cur_played_note.type == kEmpty)
	//{
	//	cur_played_note = queue.back();
	//	queue.pop_back();
	//	*outputs[kNoteMgrGate] = 1.0;
	//	*outputs[kNoteMgrFreq] = pow(2, (cur_played_note.note_number - 69) / 12.0) * params[0]; // possible to do RMS smoothing to make "slides"
	//	*outputs[kNoteMgrAmp] = cur_played_note.velocity / 127.0;
	//}

	process_simd();

	auto find_erase_note = [](std::deque<Note>& v, Note n)
	{
		for (auto iter = v.begin(); iter != v.end(); )
		{
			if (iter->note_number == n.note_number)
			{
				iter = v.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	};

	if (notes.empty())
		return;

	for (auto it = notes.begin(); it != notes.end();)
	{
		Note cur = *it;
		
		if (cur.timestamp != global_timestamp)
		{
			++it; continue;
		}

		switch (cur.type)
		{
		case kNoteOn:
			queue.push_back(cur);
			break;
		case kNoteOff:
			for (int i = 0; i < 8; i++)
			{
				if (voices[i].note_number == cur.note_number)
				{
					voices[i] = Note();
					voices[i].note_number = cur.note_number;
				}
			}
			find_erase_note(queue, cur);
			process_simd();
			break;
		/*case kAllNotesOff:
			*outputs[kNoteMgrGate] = 0.0;
			cur_played_note = Note();
			queue.clear();
			break;*/
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