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

#include "note_manager.hpp"
#include <algorithm>

namespace kraps {

NoteManager::NoteManager() : Processor (p_notemgr, 0, 4)
{
	pt = kraps::parameter::pt::ParameterTable(
		{ //new parameter::Parameter<float>("a3_tuning", 440.0, 440.0, 400, 500),
		new parameter::Parameter<bool>("is_mono", false, false, false, true),
		new parameter::Parameter<bool>("is_legato", false, false, false, true),
		new parameter::Parameter<bool>("is_always_porta", false, false, false, true),
		new parameter::Parameter<float>("porta_time", 0, 0, 0, 8000)
		});

	io_description[1] =
	{
		{ kNoteMgrFreq, "FREQ", "Current played note frequency."},
		{ kNoteMgrAmp, "VELO", "Note velocity converted to CV."},
		{ kNoteMgrGate, "GATE", "Just gate."},
		{ kNoteMgrSync, "SYNC", "DAW tempo sync. Reports bars per second in Hz."}
	};

	for (int i = 0; i < note_lookup.size(); i++)
	{
		note_lookup[i] = (i - 69) / 12.0;
	}

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
	p = { kAllNotesOff, 0, 0, timestamp };
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
	*outputs[kNoteMgrSync] = float8 (bar_size);
}
void NoteManager::process_params()
{
	bool prev_is_mono = is_mono;
	is_mono = pt.get_raw_value("is_mono");

	if (prev_is_mono != is_mono)
		voices = { Note() };

	is_legato = pt.get_raw_value("is_legato");
	is_always_porta = pt.get_raw_value("is_always_porta");
	porta_time = pt.get_raw_value("porta_time") / float8 (1000.0f);
	
}
void NoteManager::process_simd()
{

	auto it = voices.begin();
	
	if (is_mono)
	{
		kNoteEventType prev_t = voices[0].type;
		if (!queue.empty())
		{
			Note n = queue.front();
			queue.clear();
			for (auto& i : voices)
				i = n;
			top_note_num = n.note_number;
		}

		if (voices[0].type - prev_t == -3 && !is_always_porta) // "ALWAYS"
		{
			float8 dest_freq = note_lookup[top_note_num]; 
			*outputs[kNoteMgrFreq] = dest_freq;
		}
		else
		{
			float8 cur_freq = *outputs[kNoteMgrFreq];
			float8 dest_freq = note_lookup[top_note_num];

			float8 diff = dest_freq - cur_freq;
			diff = blend(diff, diff / (float8(sample_rate) * porta_time), porta_time > float8(0));

			*outputs[kNoteMgrFreq] = blend(cur_freq, cur_freq + diff, sfabs(diff) > float8(0));
		}
		
		*outputs[kNoteMgrGate] = blend(float8(1), float8(0), float8(voices[0].type) == float8(kEmpty));
		*outputs[kNoteMgrAmp] = float8(voices[0].velocity / float8(127.f));
	}
	else
	{
		while (!queue.empty())
		{
			if (it == voices.end())
			{
				voices.back() = queue.front();
				queue.clear();
			}
			else
			{
				if (it->type == kEmpty)
				{
					*it = queue.back();
					queue.pop_back();
				}
				++it;
			}
		}

		float freq[8], velo[8], types[8];
#pragma loop(hint_parallel(8))
		for (int i = 0; i < 8; i++)
		{
			types[i] = voices[i].type;
			freq[i] = note_lookup[voices[i].note_number];
			velo[i] = voices[i].velocity;
		}
		*outputs[kNoteMgrGate] = blend(float8(1), float8(0), float8::load(types) == float8(kEmpty));
		*outputs[kNoteMgrAmp] = float8::load(velo) / float8(127.f);
		*outputs[kNoteMgrFreq] = float8::load(freq);
	}

}

void NoteManager::process_callback()
{

	process_simd();

	if (notes.empty())
		return;

	for (auto it = notes.begin(); it != notes.end();)
	{
		Note& cur = *it;

		if (cur.timestamp != global_timestamp)
		{
			++it; continue;
		}

		switch (cur.type)
		{
		case kNoteOn:
			if (is_mono && !is_legato)
			{		
				voices[0].type = kEmpty;
				process_simd();
			}	
			queue.push_back(cur);
			break;
		case kNoteOff:
			if (is_mono)
				voices[0].type = kEmpty;
			else
				for (int i = 0; i < 8; i++)
				{
					if (voices[i].note_number == cur.note_number)
					{
						voices[i].type = kEmpty; break;
					}
				}

			for ( auto iter = queue.begin(); iter != queue.end(); )
				if (iter->note_number == cur.note_number)
					iter = queue.erase(iter);
				else
					++iter;

			process_simd();
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
	}
}

NoteManager::~NoteManager()
{

}

}