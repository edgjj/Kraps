#include "note_manager.hpp"
#include <algorithm>


NoteManager::NoteManager() : Processor (p_misc, 1, 3)
{
	params.push_back (440.0);
}

void NoteManager::note_on(int note_number, int velocity, double timestamp)
{
	notes.push_back( { note_number, velocity, -1, timestamp });
}

void NoteManager::note_off(int note_number, int velocity, double timestamp)
{
	Note t = { note_number, velocity, 0, timestamp };

	auto it = std::find_if(notes.begin(), notes.end(),
		[&cn = t](const Note& n) -> bool
		{ return n.note_number == cn.note_number; }
	);

	it->life_time = t.time_stamp - it->time_stamp;
	if (it->life_time < 0)
		it->life_time += block_size;

}

void NoteManager::all_notes_off(double timestamp)
{

	notes.push_back({ 69, 0 });
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
	if (notes.back().time_stamp == global_timestamp)
	{
		*outputs[kNoteMgrGate] = 1.0;
		*outputs[kNoteMgrFreq] = pow(2, (notes.back().note_number - 69) / 12) * params[0];
		*outputs[kNoteMgrAmp] = notes.back().velocity / 127.0;
	}

	std::remove_if(notes.begin(), notes.end(),
		[&](const Note& n)
		{
			return n.life_time == 0;
		}
	);

	std::for_each(notes.begin(), notes.end(), 
		[&](Note& n) 
		{
			if (n.life_time != -1)
				n.life_time -= 1;
		}
	);
	

}

NoteManager::~NoteManager()
{

}