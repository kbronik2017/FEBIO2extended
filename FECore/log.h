#pragma once

#include "Logfile.h"

//! get the one-and-only log file (defined in LogFile.cpp)
//extern Logfile& felog;
#define felog (*Logfile::GetInstance())
