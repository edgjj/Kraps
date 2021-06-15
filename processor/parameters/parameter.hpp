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

#ifndef KRPS_PARAMETER
#define KRPS_PARAMETER
#include "../../serialize/nlohmann/json.hpp"
#include "../../simd/avir_float8_avx.h"

#include <utility>

namespace kraps 
{
namespace parameter
{
class ParameterInterface
{
public:
	ParameterInterface()
	{

	}		

	friend void to_json(nlohmann::json& j, const ParameterInterface& p) 
	{
		p.internal_to_json(j);
	}

	friend void from_json(const nlohmann::json& j, ParameterInterface& p)
	{
		p.internal_from_json(j);
	}


	virtual void set_value(const float& v) = 0;
	virtual void set_value() = 0;
	virtual void set_distribution(const float& v, const int& t) = 0;

	using distr_params = struct { float amt; float type; };
	virtual distr_params get_distribution_params() = 0;

	using raw_pair = struct { float first;  float second; };

	virtual raw_pair get_range () = 0;
	virtual std::string get_name() = 0;

	virtual float8 get_value() = 0;
	virtual float get_default_value() = 0;

	virtual ~ParameterInterface()
	{

	}
private:
	virtual void internal_to_json(nlohmann::json& j) const = 0;
	virtual void internal_from_json(const nlohmann::json& j) = 0;
};


template <typename T>
class Parameter : public ParameterInterface
{
public:
	

	Parameter(std::string _name) : param_name(_name) // individual voice-stacking per parameter
	{ 
	}

	Parameter(std::string _name, T _v, T _def_v, T _min_v, T _max_v) :
		param_name(_name), value(_v), default_value(_def_v), range ({ _min_v, _max_v })
	{ 
	}

	Parameter(const Parameter& p)
	{
		value = p.value;
		default_value = p.default_value;
		range = p.range;
		distribution_amt = p.distribution_amt;
		distribution_type = p.distribution_type;
	}


	void set_value(const float& v) override
	{
		value = v;
	}

	void set_value() override
	{
		value = default_value.load();
	}

	void set_distribution(const float& v, const int& t) override
	{
		distribution_amt = v;
		distribution_type = t;
	}

	distr_params get_distribution_params() override
	{
		distr_params p = { distribution_amt.load(), distribution_type.load() };
		return p;
	}

	float8 get_value() override
	{
		if (distribution_amt > 0.0f)
		{
			
			range_pair cur_range = range.load();
			T step = ( (cur_range.second - cur_range.first) / 8.0 ) * distribution_amt; // just linear atm

			float val = value.load();

			float data[8];

			for (int i = 1; i <= 8; i++) // in future there should be switch
			{
				data[i-1] = fmax (fmin(val + step * i, cur_range.second), cur_range.first);
			}
			return float8::load(data);

		}
		else 
			return value.load();
	}

	float get_default_value() override
	{
		float v = default_value.load();
		return v;
	}

	raw_pair get_range() override
	{
		range_pair rp = range.load();
		raw_pair p{
			rp.first, rp.second };
		return p;
	}

	std::string get_name() override
	{
		return param_name;
	}


	template <typename V>
	Parameter<T>& operator= (const Parameter <V>& p)
	{
		value = p.value;
		default_value = p.default_value;
		range = p.range;
		distribution_range = p.distribution_range;
		distribution_type = p.distribution_type;

		return *this;
	}

	template <typename V>
	Parameter& operator= (const V& v)
	{
		value = v;
		return *this;
	}

	operator float() const 
	{
		return value; 
	}

	~Parameter() { ; }
private:

	void internal_to_json(nlohmann::json& j) const override
	{
		j["value"] = value.load();
		j["param_name"] = param_name;
		j["distribution_amt"] = distribution_amt.load();
		j["distribution_type"] = distribution_type.load();
	}
	void internal_from_json(const nlohmann::json& j) override
	{
		try
		{
			T val;
			j.at("value").get_to(val);
			value.store(val);
			j.at("distribution_amt").get_to(val);
			distribution_amt.store(val);
			j.at("distribution_type").get_to(val);
			distribution_type.store(val);
		}
		catch (std::exception& e)
		{
			return;
		}
	}

	std::atomic<T> value = 0.0f;
	std::atomic<T> default_value = 0.0f;

	using range_pair = struct { T first;  T second; };
	std::atomic<range_pair> range = 0.0f;

	std::atomic<float> distribution_amt = 0.0f;
	std::atomic<int> distribution_type = 0; // 0 - linear; 1 - exp; 2 - log; 3 - etc; 4 - random;

	std::string param_name;

};



}

}

#endif