/*   This file is part of ThemeSrv.
*
*   ThemeSrv is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Affero General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   ThemeSrv is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Affero General Public License for more details.
*
*   You should have received a copy of the GNU Affero General Public License
*   along with ThemeSrv.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PROTOCOL_COMMON_H
#define __PROTOCOL_COMMON_H

#include "protocol.h"

#include "protocol_fields_begin.inl"
#include "common.inl"
#include "protocol_fields_end.inl"

#include "protocol_structs_begin.inl"
#include "common.inl"
#include "protocol_structs_end.inl"

namespace theme
{
    namespace protocol
    {
        enum
        {
            e_protocolCli2Auth = 0x0A,
            e_protocolCli2Game = 0x0B,
            e_protocolCli2File = 0x10,
            e_protocolCli2Gate = 0x16,
        };
    };
};

#endif
