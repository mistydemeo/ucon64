/*
backup.h - single header file for all backup unit dependent functions

written by 2003 NoisyB (noisyb@gmx.net)


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BACKUP_H
#define BACKUP_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  PARALLEL
#include "cd64.h"
#include "dex.h"
#include "doctor64.h"
#include "doctor64jr.h"
#include "fal.h"
//#include "ffe.h"
#include "fig.h"
#include "fpl.h"
#include "gbx.h"
#include "gd.h"
#include "interceptor.h"
#include "lynxit.h"
#include "mccl.h"
#include "md.h"
#include "mgd.h"
#include "msg.h"
//#include "psxpblib.h"
#include "smc.h"
#include "smd.h"
#include "ssc.h"
#include "swc.h"
#include "ufo.h"
#include "yoko.h"
#include "z64.h"
#endif // PARALLEL

#ifdef  USB
#endif  // USB

#ifdef  SERIAL
#endif  // SERIAL
#endif // BACKUP_H
