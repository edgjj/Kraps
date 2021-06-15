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

#ifndef KRPS_PARAMTABLE
#define KRPS_PARAMTABLE

#include "parameter.hpp"

namespace kraps
{
namespace parameter
{
namespace pt
{


class ParameterTable
{
public:

	ParameterTable()
	{
	}

	template <typename... Items>
	ParameterTable(Items... items) // ALSO we should try to load older presets
	{ 
		std::unique_ptr<ParameterInterface> itemArr[] = { std::unique_ptr<ParameterInterface>(items)... };
		parameters = std::vector<std::unique_ptr<ParameterInterface>>{ std::make_move_iterator(std::begin(itemArr)), std::make_move_iterator(std::end(itemArr)) };
	}

	ParameterTable(ParameterTable&& p) noexcept // we can also make undomanager, and call its callback when needed
	{
		parameters = std::move(p.parameters);
	}
	
	template <typename... Items>
	void add_parameter(Items... items)
	{
		std::unique_ptr<ParameterInterface> itemArr[] = { std::unique_ptr<ParameterInterface>(items)... };
		parameters.insert ( parameters.end (), std::make_move_iterator(std::begin(itemArr)), std::make_move_iterator(std::end(itemArr)));
	}

	void set_new_range(const std::string& name, const float& l, const float& r)
	{
		for (auto& i : parameters)
			if (i->get_name() == name)
				return i->set_new_range(l, r);
	}


	void set_value(const std::string& name)
	{
		for (auto& i : parameters)
			if (i->get_name() == name)
				return i->set_value();
	}

	void set_value(const std::string& name, const float& val)
	{
		for (auto& i : parameters)
			if (i->get_name() == name)
				return i->set_value(val);
	}

	float8 get_raw_value(const std::string& name)
	{
		for (auto& i : parameters)
			if (i->get_name() == name)
				return i->get_value();
	}

	ParameterInterface::raw_pair get_raw_range(const std::string& name)
	{
		for (auto& i : parameters)
			if (i->get_name() == name)
				return i->get_range();
	}

	size_t get_param_count()
	{
		return parameters.size();
	}

	friend void to_json(nlohmann::json& j, const ParameterTable& p)
	{
		for (auto& i : p.parameters)
			j.push_back (*i);
	}

	friend void from_json(const nlohmann::json& j, ParameterTable& p)
	{
		std::map <std::string, ParameterInterface*> m;

		if (j.is_array() && j.size() > 0 && !j.at(0).is_object())
		{
			std::vector<float> params_old;
			j.get_to(params_old);

			for (int i = 0; i < params_old.size(); i++)
				p.parameters[i]->set_value(params_old[i]);

			return;
		}

		for (auto& _p : p.parameters)
		{
			m[_p->get_name()] = _p.get();
		}
		
		for (auto& o : j)
		{
			std::string name;
			o.at("param_name").get_to(name);
			o.get_to(*m[name]);
		}
	}
	std::vector <std::unique_ptr <parameter::ParameterInterface>>::iterator begin() { return parameters.begin(); }
	std::vector <std::unique_ptr <parameter::ParameterInterface>>::iterator end() { return parameters.end(); }

	ParameterTable& operator= (ParameterTable&& p) noexcept
	{
		parameters = std::move(p.parameters);
		return *this;
	}


	~ParameterTable() { ; }
private:

	std::vector <std::unique_ptr <parameter::ParameterInterface>> parameters;


};


}
}
}
#endif
