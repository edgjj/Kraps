#include "note_manager.hpp"
#include <algorithm>


NoteManager::NoteManager() : Processor (p_misc, 1, 3)
{
	params.push_back (440.0);
	params_constrainments.push_back(std::make_pair<double, double>(400, 500));
}

void NoteManager::note_on(int note_number, int velocity, double timestamp)
{
	Note t = { note_number, velocity, -1, timestamp };

	auto it = std::find_if(notes.begin(), notes.end(),
		[&cn = t](const Note& n) -> bool
		{ return n.note_number == cn.note_number; }
	);

	if (it != notes.end())
		return;

	notes.push_back( { note_number, velocity, -1, timestamp });
}

void NoteManager::note_off(int note_number, int velocity, double timestamp)
{
	if (notes.empty()) 
		return;

	Note t = { note_number, velocity, 0, timestamp };

	int note_num = 0;
	for (int i = 0; i < notes.size(); i++)
		if (notes[i].note_number == t.note_number)
		{
			note_num = i; break;
		}

	notes[note_num].life_time = t.time_stamp - notes[note_num].time_stamp;

	if (notes[note_num].life_time < 0)
		notes[note_num].life_time += block_size;

}

void NoteManager::all_notes_off(double timestamp)
{
	for (int i = 0; i < notes.size(); i++)
		notes[i].life_time = timestamp - notes[i].time_stamp;

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
	if (notes.empty())
		return;

	if (notes.back().time_stamp == global_timestamp)
	{
		*outputs[kNoteMgrGate] = 1.0;
		*outputs[kNoteMgrFreq] = pow(2, (notes.back().note_number - 69) / 12.0) * params[0];
		*outputs[kNoteMgrAmp] = notes.back().velocity / 127.0;
	}

	
	notes.erase(std::remove_if(notes.begin(), notes.end(),
		[&](Note& n)
		{
			return n.life_time == 0;
		}
	),notes.end());

	std::for_each(notes.begin(), notes.end(), 
		[&](Note& n) 
		{
			if (n.life_time > 0)
				n.life_time -= 1;
		}
	);

	if (notes.empty())
	{
		*outputs[kNoteMgrGate] = 0.0;
	}

}

NoteManager::~NoteManager()
{

}