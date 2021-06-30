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

#ifndef KRAPS_HDR_H
#define KRAPS_HDR_H

#include "serialize/base64/base64.hpp"
#include "processor/processor_matrix.hpp"
#include "processor/processor_types.hpp"
#include "misc/misc.hpp"
namespace kraps 
{

namespace kraps_info
{
	const char* const  version_str = "0.2"; // totally useless;
	const int          version_num = 0x2;
}

}


#endif
