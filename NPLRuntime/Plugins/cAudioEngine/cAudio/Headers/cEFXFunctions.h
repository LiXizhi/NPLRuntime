// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

#include <al.h>
#include <alc.h>
#include <efx.h>
#include <efx-creative.h>

#include "cMutex.h"

//To use EFX in linux.
#ifndef ALC_EXT_EFX
#define AL_FILTER_TYPE                                     0x8001
#define AL_EFFECT_TYPE                                     0x8001
#define AL_FILTER_NULL                                     0x0000
#define AL_FILTER_LOWPASS                                  0x0001
#define AL_FILTER_HIGHPASS                                 0x0002
#define AL_FILTER_BANDPASS                                 0x0003
#define AL_EFFECT_NULL                                     0x0000
#define AL_EFFECT_EAXREVERB                                0x8000
#define AL_EFFECT_REVERB                                   0x0001
#define AL_EFFECT_CHORUS                                   0x0002
#define AL_EFFECT_DISTORTION                               0x0003
#define AL_EFFECT_ECHO                                     0x0004
#define AL_EFFECT_FLANGER                                  0x0005
#define AL_EFFECT_FREQUENCY_SHIFTER                        0x0006
#define AL_EFFECT_VOCAL_MORPHER                            0x0007
#define AL_EFFECT_PITCH_SHIFTER                            0x0008
#define AL_EFFECT_RING_MODULATOR                           0x0009
#define AL_EFFECT_AUTOWAH                                  0x000A
#define AL_EFFECT_COMPRESSOR                               0x000B
#define AL_EFFECT_EQUALIZER                                0x000C
#define ALC_EFX_MAJOR_VERSION                              0x20001
#define ALC_EFX_MINOR_VERSION                              0x20002
#define ALC_MAX_AUXILIARY_SENDS                            0x20003
#endif  

namespace cAudio
{
	//! EFX Extension function pointers and setup
	struct cEFXFunctions
	{
		cEFXFunctions()
		{
			alGenEffects = NULL;
			alDeleteEffects = NULL;
			alIsEffect = NULL;
			alEffecti = NULL;
			alEffectiv = NULL;
			alEffectf = NULL;
			alEffectfv = NULL;
			alGetEffecti = NULL;
			alGetEffectiv = NULL;
			alGetEffectf = NULL;
			alGetEffectfv = NULL;

			alGenFilters = NULL;
			alDeleteFilters = NULL;
			alIsFilter = NULL;
			alFilteri = NULL;
			alFilteriv = NULL;
			alFilterf = NULL;
			alFilterfv = NULL;
			alGetFilteri = NULL;
			alGetFilteriv = NULL;
			alGetFilterf = NULL;
			alGetFilterfv = NULL;

			alGenAuxiliaryEffectSlots = NULL;
			alDeleteAuxiliaryEffectSlots = NULL;
			alIsAuxiliaryEffectSlot = NULL;
			alAuxiliaryEffectSloti = NULL;
			alAuxiliaryEffectSlotiv = NULL;
			alAuxiliaryEffectSlotf = NULL;
			alAuxiliaryEffectSlotfv = NULL;
			alGetAuxiliaryEffectSloti = NULL;
			alGetAuxiliaryEffectSlotiv = NULL;
			alGetAuxiliaryEffectSlotf = NULL;
			alGetAuxiliaryEffectSlotfv = NULL;

			Supported = false;
		}

		// Effect objects
		LPALGENEFFECTS alGenEffects;
		LPALDELETEEFFECTS alDeleteEffects;
		LPALISEFFECT alIsEffect;
		LPALEFFECTI alEffecti;
		LPALEFFECTIV alEffectiv;
		LPALEFFECTF alEffectf;
		LPALEFFECTFV alEffectfv;
		LPALGETEFFECTI alGetEffecti;
		LPALGETEFFECTIV alGetEffectiv;
		LPALGETEFFECTF alGetEffectf;
		LPALGETEFFECTFV alGetEffectfv;

		// Filter objects
		LPALGENFILTERS alGenFilters;
		LPALDELETEFILTERS alDeleteFilters;
		LPALISFILTER alIsFilter;
		LPALFILTERI alFilteri;
		LPALFILTERIV alFilteriv;
		LPALFILTERF alFilterf;
		LPALFILTERFV alFilterfv;
		LPALGETFILTERI alGetFilteri;
		LPALGETFILTERIV alGetFilteriv;
		LPALGETFILTERF alGetFilterf;
		LPALGETFILTERFV alGetFilterfv;

		// Auxiliary slot object
		LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
		LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
		LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
		LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
		LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
		LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
		LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
		LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
		LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
		LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
		LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;

		cAudioMutex Mutex;

		bool Supported;

		bool CheckEFXSupport(ALCdevice* device)
		{
			Supported = false;
			if (alcIsExtensionPresent(device, "ALC_EXT_EFX"))
			{
				// Get function pointers
				alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
				alDeleteEffects = (LPALDELETEEFFECTS )alGetProcAddress("alDeleteEffects");
				alIsEffect = (LPALISEFFECT )alGetProcAddress("alIsEffect");
				alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
				alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
				alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
				alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
				alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
				alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
				alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
				alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
				alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
				alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
				alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
				alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
				alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
				alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
				alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
				alGetFilteri = (LPALGETFILTERI )alGetProcAddress("alGetFilteri");
				alGetFilteriv= (LPALGETFILTERIV )alGetProcAddress("alGetFilteriv");
				alGetFilterf = (LPALGETFILTERF )alGetProcAddress("alGetFilterf");
				alGetFilterfv= (LPALGETFILTERFV )alGetProcAddress("alGetFilterfv");
				alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
				alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
				alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
				alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
				alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
				alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
				alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
				alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
				alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
				alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
				alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

				if (alGenEffects &&	alDeleteEffects && alIsEffect && alEffecti && alEffectiv &&	alEffectf &&
					alEffectfv && alGetEffecti && alGetEffectiv && alGetEffectf && alGetEffectfv &&	alGenFilters &&
					alDeleteFilters && alIsFilter && alFilteri && alFilteriv &&	alFilterf && alFilterfv &&
					alGetFilteri &&	alGetFilteriv && alGetFilterf && alGetFilterfv && alGenAuxiliaryEffectSlots &&
					alDeleteAuxiliaryEffectSlots &&	alIsAuxiliaryEffectSlot && alAuxiliaryEffectSloti &&
					alAuxiliaryEffectSlotiv && alAuxiliaryEffectSlotf && alAuxiliaryEffectSlotfv &&
					alGetAuxiliaryEffectSloti && alGetAuxiliaryEffectSlotiv && alGetAuxiliaryEffectSlotf &&
					alGetAuxiliaryEffectSlotfv)
					Supported = true;
			}

			return Supported;
		}
	};
};

#endif
