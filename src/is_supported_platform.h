/*
    This file is part of darktable,
    Copyright (C) 2016-2022 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// WARNING: do not #include anything in here!

#if !defined(__APPLE__)
#error "DarkTableNext supports macOS only."
#endif

#if !defined(__x86_64__) && !defined(__aarch64__)
#error "DarkTableNext supports only Apple Silicon and Intel 64-bit Macs."
#endif

#if !defined(__SSE2__) || !defined(__SSE__)
#pragma message "Building without SSE2.  Some functionality will be noticeably slower."
#endif

// double check for 32-bit architecture
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ < 8
#error "DarkTableNext requires a 64-bit macOS target."
#endif
