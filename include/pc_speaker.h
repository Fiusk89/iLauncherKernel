#pragma once
#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H
#include <kernel.h>

void pc_speaker_playsound(uint8_t *data, uint32_t size);
void pc_speaker_install();
#endif