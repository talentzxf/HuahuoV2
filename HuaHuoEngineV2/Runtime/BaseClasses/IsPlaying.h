#pragma once
#include "Modules/ExportModules.h"

/// Is the world playmode? (Otherwise we are in editor mode)
bool EXPORT_COREMODULE IsWorldPlaying();

void SetIsWorldPlaying(bool isPlaying);
