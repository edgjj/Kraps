#ifndef KRPS_PARAMETER
#define KRPS_PARAMETER
#include "../../serialize/nlohmann/json.hpp"
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

	virtual float get_value() = 0;
	virtual void set_value(const float& v) = 0;

	using raw_pair = struct { float first;  float second; };

	virtual raw_pair get_range () = 0;
	virtual std::string get_name() = 0;


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
	

	Parameter(std::string _name) : param_name(_name)
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
		min_value = p.min_value;
		max_value = p.max_value;
	}


	float get_value() override
	{
		float v = value.load();
		return v;
	}
	void set_value(const float& v) override
	{
		value = v;
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
		min_value = p.min_value;
		max_value = p.max_value;

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
	}
	void internal_from_json(const nlohmann::json& j) override
	{
		T val;
		j.at ("value").get_to(val);
		value.store(val);
	}

	std::atomic<T> value = 0.0f;
	std::atomic<T> default_value = 0.0f;

	using range_pair = struct { T first;  T second; };
	std::atomic<range_pair> range = 0.0f;


	std::string param_name;

};



}

}

#endif