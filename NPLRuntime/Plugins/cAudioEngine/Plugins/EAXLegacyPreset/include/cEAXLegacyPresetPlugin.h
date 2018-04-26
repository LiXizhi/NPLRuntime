#ifndef CEAXLEGACYPRESETPLUGIN_H
#define CEAXLEGACYPRESETPLUGIN_H

#include "IAudioPlugin.h"

using namespace cAudio;

class cEAXLegacyPresetPlugin : public IAudioPlugin
{
	bool installPlugin(ILogger* logger)
	{
		//This plugin has no first time initialization to do, so this is an easy function
		return true;
	}

	const char* getPluginName()
	{
		return "EAXLegacyPresets";
	}

	void uninstallPlugin()
	{
		//Nothing to clean up
	}

	void onCreateAudioManager(IAudioManager* manager)
	{
#ifdef CAUDIO_EFX_ENABLED
		sEAXReverbParameters genericPreset;
		genericPreset.Gain = 0.316f;
		genericPreset.GainLF = 1.0f;
		genericPreset.GainHF = 0.891f;
		genericPreset.RoomRolloffFactor = 0.0f;
		genericPreset.AirAbsorptionGainHF = 0.994f;
		genericPreset.LFReference = 250.0f;
		genericPreset.HFReference = 5000.0f;
		genericPreset.Density = 1.0f;
		genericPreset.Diffusion = 1.0f;
		genericPreset.EchoTime = 0.25f;
		genericPreset.EchoDepth = 0.0f;
		genericPreset.ModulationTime = 0.25f;
		genericPreset.ModulationDepth = 0.0f;
		genericPreset.DecayTime = 1.49f;
		genericPreset.DecayLFRatio = 1.0f;
		genericPreset.DecayHFRatio = 0.83f;
		genericPreset.DecayHFLimit = true;
		genericPreset.ReflectionsGain = 0.05f;
		genericPreset.ReflectionsDelay = 0.01f;
		genericPreset.LateReverbGain = 1.259f;
		genericPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters paddedCellPreset;
		paddedCellPreset.Gain = 0.316f;
		paddedCellPreset.GainLF = 1.0f;
		paddedCellPreset.GainHF = 0.001f;
		paddedCellPreset.RoomRolloffFactor = 0.0f;
		paddedCellPreset.AirAbsorptionGainHF = 0.994f;
		paddedCellPreset.LFReference = 250.0f;
		paddedCellPreset.HFReference = 5000.0f;
		paddedCellPreset.Density = 0.17f;
		paddedCellPreset.Diffusion = 1.0f;
		paddedCellPreset.EchoTime = 0.25f;
		paddedCellPreset.EchoDepth = 0.0f;
		paddedCellPreset.ModulationTime = 0.25f;
		paddedCellPreset.ModulationDepth = 0.0f;
		paddedCellPreset.DecayTime = 0.17f;
		paddedCellPreset.DecayLFRatio = 1.0f;
		paddedCellPreset.DecayHFRatio = 0.10f;
		paddedCellPreset.DecayHFLimit = true;
		paddedCellPreset.ReflectionsGain = 0.250f;
		paddedCellPreset.ReflectionsDelay = 0.0f;
		paddedCellPreset.LateReverbGain = 1.268f;
		paddedCellPreset.LateReverbDelay = 0.0f;

		sEAXReverbParameters roomPreset;
		roomPreset.Gain = 0.316f;
		roomPreset.GainLF = 1.0f;
		roomPreset.GainHF = 0.593f;
		roomPreset.RoomRolloffFactor = 0.0f;
		roomPreset.AirAbsorptionGainHF = 0.994f;
		roomPreset.LFReference = 250.0f;
		roomPreset.HFReference = 5000.0f;
		roomPreset.Density = 0.43f;
		roomPreset.Diffusion = 1.0f;
		roomPreset.EchoTime = 0.25f;
		roomPreset.EchoDepth = 0.0f;
		roomPreset.ModulationTime = 0.25f;
		roomPreset.ModulationDepth = 0.0f;
		roomPreset.DecayTime = 0.40f;
		roomPreset.DecayLFRatio = 1.0f;
		roomPreset.DecayHFRatio = 0.83f;
		roomPreset.DecayHFLimit = true;
		roomPreset.ReflectionsGain = 0.150f;
		roomPreset.ReflectionsDelay = 0.0f;
		roomPreset.LateReverbGain = 1.062f;
		roomPreset.LateReverbDelay = 0.0f;

		sEAXReverbParameters bathroomPreset;
		bathroomPreset.Gain = 0.316f;
		bathroomPreset.GainLF = 1.0f;
		bathroomPreset.GainHF = 0.251f;
		bathroomPreset.RoomRolloffFactor = 0.0f;
		bathroomPreset.AirAbsorptionGainHF = 0.994f;
		bathroomPreset.LFReference = 250.0f;
		bathroomPreset.HFReference = 5000.0f;
		bathroomPreset.Density = 0.17f;
		bathroomPreset.Diffusion = 1.0f;
		bathroomPreset.EchoTime = 0.25f;
		bathroomPreset.EchoDepth = 0.0f;
		bathroomPreset.ModulationTime = 0.25f;
		bathroomPreset.ModulationDepth = 0.0f;
		bathroomPreset.DecayTime = 1.49f;
		bathroomPreset.DecayLFRatio = 1.0f;
		bathroomPreset.DecayHFRatio = 0.54f;
		bathroomPreset.DecayHFLimit = true;
		bathroomPreset.ReflectionsGain = 0.653f;
		bathroomPreset.ReflectionsDelay = 0.01f;
		bathroomPreset.LateReverbGain = 3.270f;
		bathroomPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters livingroomPreset;
		livingroomPreset.Gain = 0.316f;
		livingroomPreset.GainLF = 1.0f;
		livingroomPreset.GainHF = 0.001f;
		livingroomPreset.RoomRolloffFactor = 0.0f;
		livingroomPreset.AirAbsorptionGainHF = 0.994f;
		livingroomPreset.LFReference = 250.0f;
		livingroomPreset.HFReference = 5000.0f;
		livingroomPreset.Density = 0.97f;
		livingroomPreset.Diffusion = 1.0f;
		livingroomPreset.EchoTime = 0.25f;
		livingroomPreset.EchoDepth = 0.0f;
		livingroomPreset.ModulationTime = 0.25f;
		livingroomPreset.ModulationDepth = 0.0f;
		livingroomPreset.DecayTime = 0.50f;
		livingroomPreset.DecayLFRatio = 1.0f;
		livingroomPreset.DecayHFRatio = 0.10f;
		livingroomPreset.DecayHFLimit = true;
		livingroomPreset.ReflectionsGain = 0.205f;
		livingroomPreset.ReflectionsDelay = 0.0f;
		livingroomPreset.LateReverbGain = 0.281f;
		livingroomPreset.LateReverbDelay = 0.0f;

		sEAXReverbParameters stoneroomPreset;
		stoneroomPreset.Gain = 0.316f;
		stoneroomPreset.GainLF = 1.0f;
		stoneroomPreset.GainHF = 0.708f;
		stoneroomPreset.RoomRolloffFactor = 0.0f;
		stoneroomPreset.AirAbsorptionGainHF = 0.994f;
		stoneroomPreset.LFReference = 250.0f;
		stoneroomPreset.HFReference = 5000.0f;
		stoneroomPreset.Density = 1.0f;
		stoneroomPreset.Diffusion = 1.0f;
		stoneroomPreset.EchoTime = 0.25f;
		stoneroomPreset.EchoDepth = 0.0f;
		stoneroomPreset.ModulationTime = 0.25f;
		stoneroomPreset.ModulationDepth = 0.0f;
		stoneroomPreset.DecayTime = 2.31f;
		stoneroomPreset.DecayLFRatio = 1.0f;
		stoneroomPreset.DecayHFRatio = 0.64f;
		stoneroomPreset.DecayHFLimit = true;
		stoneroomPreset.ReflectionsGain = 0.441f;
		stoneroomPreset.ReflectionsDelay = 0.01f;
		stoneroomPreset.LateReverbGain = 1.099f;
		stoneroomPreset.LateReverbDelay = 0.02f;

		sEAXReverbParameters auditoriumPreset;
		auditoriumPreset.Gain = 0.316f;
		auditoriumPreset.GainLF = 1.0f;
		auditoriumPreset.GainHF = 0.578f;
		auditoriumPreset.RoomRolloffFactor = 0.0f;
		auditoriumPreset.AirAbsorptionGainHF = 0.994f;
		auditoriumPreset.LFReference = 250.0f;
		auditoriumPreset.HFReference = 5000.0f;
		auditoriumPreset.Density = 1.0f;
		auditoriumPreset.Diffusion = 1.0f;
		auditoriumPreset.EchoTime = 0.25f;
		auditoriumPreset.EchoDepth = 0.0f;
		auditoriumPreset.ModulationTime = 0.25f;
		auditoriumPreset.ModulationDepth = 0.0f;
		auditoriumPreset.DecayTime = 4.32f;
		auditoriumPreset.DecayLFRatio = 1.0f;
		auditoriumPreset.DecayHFRatio = 0.59f;
		auditoriumPreset.DecayHFLimit = true;
		auditoriumPreset.ReflectionsGain = 0.403f;
		auditoriumPreset.ReflectionsDelay = 0.02f;
		auditoriumPreset.LateReverbGain = 0.717f;
		auditoriumPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters concertHallPreset;
		concertHallPreset.Gain = 0.316f;
		concertHallPreset.GainLF = 1.0f;
		concertHallPreset.GainHF = 0.562f;
		concertHallPreset.RoomRolloffFactor = 0.0f;
		concertHallPreset.AirAbsorptionGainHF = 0.994f;
		concertHallPreset.LFReference = 250.0f;
		concertHallPreset.HFReference = 5000.0f;
		concertHallPreset.Density = 1.0f;
		concertHallPreset.Diffusion = 1.0f;
		concertHallPreset.EchoTime = 0.25f;
		concertHallPreset.EchoDepth = 0.0f;
		concertHallPreset.ModulationTime = 0.25f;
		concertHallPreset.ModulationDepth = 0.0f;
		concertHallPreset.DecayTime = 3.92f;
		concertHallPreset.DecayLFRatio = 1.0f;
		concertHallPreset.DecayHFRatio = 0.70f;
		concertHallPreset.DecayHFLimit = true;
		concertHallPreset.ReflectionsGain = 0.243f;
		concertHallPreset.ReflectionsDelay = 0.02f;
		concertHallPreset.LateReverbGain = 0.998f;
		concertHallPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters cavePreset;
		cavePreset.Gain = 0.316f;
		cavePreset.GainLF = 1.0f;
		cavePreset.GainHF = 1.0f;
		cavePreset.RoomRolloffFactor = 0.0f;
		cavePreset.AirAbsorptionGainHF = 0.994f;
		cavePreset.LFReference = 250.0f;
		cavePreset.HFReference = 5000.0f;
		cavePreset.Density = 1.0f;
		cavePreset.Diffusion = 1.0f;
		cavePreset.EchoTime = 0.25f;
		cavePreset.EchoDepth = 0.0f;
		cavePreset.ModulationTime = 0.25f;
		cavePreset.ModulationDepth = 0.0f;
		cavePreset.DecayTime = 2.91f;
		cavePreset.DecayLFRatio = 1.0f;
		cavePreset.DecayHFRatio = 1.30f;
		cavePreset.DecayHFLimit = false;
		cavePreset.ReflectionsGain = 0.500f;
		cavePreset.ReflectionsDelay = 0.01f;
		cavePreset.LateReverbGain = 0.706f;
		cavePreset.LateReverbDelay = 0.02f;

		sEAXReverbParameters arenaPreset;
		arenaPreset.Gain = 0.316f;
		arenaPreset.GainLF = 1.0f;
		arenaPreset.GainHF = 0.448f;
		arenaPreset.RoomRolloffFactor = 0.0f;
		arenaPreset.AirAbsorptionGainHF = 0.994f;
		arenaPreset.LFReference = 250.0f;
		arenaPreset.HFReference = 5000.0f;
		arenaPreset.Density = 1.0f;
		arenaPreset.Diffusion = 1.0f;
		arenaPreset.EchoTime = 0.25f;
		arenaPreset.EchoDepth = 0.0f;
		arenaPreset.ModulationTime = 0.25f;
		arenaPreset.ModulationDepth = 0.0f;
		arenaPreset.DecayTime = 7.24f;
		arenaPreset.DecayLFRatio = 1.0f;
		arenaPreset.DecayHFRatio = 0.33f;
		arenaPreset.DecayHFLimit = true;
		arenaPreset.ReflectionsGain = 0.261f;
		arenaPreset.ReflectionsDelay = 0.02f;
		arenaPreset.LateReverbGain = 1.019f;
		arenaPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters hangarPreset;
		hangarPreset.Gain = 0.316f;
		hangarPreset.GainLF = 1.0f;
		hangarPreset.GainHF = 0.316f;
		hangarPreset.RoomRolloffFactor = 0.0f;
		hangarPreset.AirAbsorptionGainHF = 0.994f;
		hangarPreset.LFReference = 250.0f;
		hangarPreset.HFReference = 5000.0f;
		hangarPreset.Density = 1.0f;
		hangarPreset.Diffusion = 1.0f;
		hangarPreset.EchoTime = 0.25f;
		hangarPreset.EchoDepth = 0.0f;
		hangarPreset.ModulationTime = 0.25f;
		hangarPreset.ModulationDepth = 0.0f;
		hangarPreset.DecayTime = 10.05f;
		hangarPreset.DecayLFRatio = 1.0f;
		hangarPreset.DecayHFRatio = 0.23f;
		hangarPreset.DecayHFLimit = true;
		hangarPreset.ReflectionsGain = 0.500f;
		hangarPreset.ReflectionsDelay = 0.02f;
		hangarPreset.LateReverbGain = 1.256f;
		hangarPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters carpetedHallwayPreset;
		carpetedHallwayPreset.Gain = 0.316f;
		carpetedHallwayPreset.GainLF = 1.0f;
		carpetedHallwayPreset.GainHF = 0.010f;
		carpetedHallwayPreset.RoomRolloffFactor = 0.0f;
		carpetedHallwayPreset.AirAbsorptionGainHF = 0.994f;
		carpetedHallwayPreset.LFReference = 250.0f;
		carpetedHallwayPreset.HFReference = 5000.0f;
		carpetedHallwayPreset.Density = 0.43f;
		carpetedHallwayPreset.Diffusion = 1.0f;
		carpetedHallwayPreset.EchoTime = 0.25f;
		carpetedHallwayPreset.EchoDepth = 0.0f;
		carpetedHallwayPreset.ModulationTime = 0.25f;
		carpetedHallwayPreset.ModulationDepth = 0.0f;
		carpetedHallwayPreset.DecayTime = 0.30f;
		carpetedHallwayPreset.DecayLFRatio = 1.0f;
		carpetedHallwayPreset.DecayHFRatio = 0.10f;
		carpetedHallwayPreset.DecayHFLimit = true;
		carpetedHallwayPreset.ReflectionsGain = 0.121f;
		carpetedHallwayPreset.ReflectionsDelay = 0.0f;
		carpetedHallwayPreset.LateReverbGain = 0.153f;
		carpetedHallwayPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters hallwayPreset;
		hallwayPreset.Gain = 0.316f;
		hallwayPreset.GainLF = 1.0f;
		hallwayPreset.GainHF = 0.708f;
		hallwayPreset.RoomRolloffFactor = 0.0f;
		hallwayPreset.AirAbsorptionGainHF = 0.994f;
		hallwayPreset.LFReference = 250.0f;
		hallwayPreset.HFReference = 5000.0f;
		hallwayPreset.Density = 0.36f;
		hallwayPreset.Diffusion = 1.0f;
		hallwayPreset.EchoTime = 0.25f;
		hallwayPreset.EchoDepth = 0.0f;
		hallwayPreset.ModulationTime = 0.25f;
		hallwayPreset.ModulationDepth = 0.0f;
		hallwayPreset.DecayTime = 1.49f;
		hallwayPreset.DecayLFRatio = 1.0f;
		hallwayPreset.DecayHFRatio = 0.59f;
		hallwayPreset.DecayHFLimit = true;
		hallwayPreset.ReflectionsGain = 0.246f;
		hallwayPreset.ReflectionsDelay = 0.01f;
		hallwayPreset.LateReverbGain = 1.662f;
		hallwayPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters stoneCorridorPreset;
		stoneCorridorPreset.Gain = 0.316f;
		stoneCorridorPreset.GainLF = 1.0f;
		stoneCorridorPreset.GainHF = 0.762f;
		stoneCorridorPreset.RoomRolloffFactor = 0.0f;
		stoneCorridorPreset.AirAbsorptionGainHF = 0.994f;
		stoneCorridorPreset.LFReference = 250.0f;
		stoneCorridorPreset.HFReference = 5000.0f;
		stoneCorridorPreset.Density = 1.0f;
		stoneCorridorPreset.Diffusion = 1.0f;
		stoneCorridorPreset.EchoTime = 0.25f;
		stoneCorridorPreset.EchoDepth = 0.0f;
		stoneCorridorPreset.ModulationTime = 0.25f;
		stoneCorridorPreset.ModulationDepth = 0.0f;
		stoneCorridorPreset.DecayTime = 2.70f;
		stoneCorridorPreset.DecayLFRatio = 1.0f;
		stoneCorridorPreset.DecayHFRatio = 0.79f;
		stoneCorridorPreset.DecayHFLimit = true;
		stoneCorridorPreset.ReflectionsGain = 0.247f;
		stoneCorridorPreset.ReflectionsDelay = 0.01f;
		stoneCorridorPreset.LateReverbGain = 1.576f;
		stoneCorridorPreset.LateReverbDelay = 0.02f;

		sEAXReverbParameters alleyPreset;
		alleyPreset.Gain = 0.316f;
		alleyPreset.GainLF = 1.0f;
		alleyPreset.GainHF = 0.733f;
		alleyPreset.RoomRolloffFactor = 0.0f;
		alleyPreset.AirAbsorptionGainHF = 0.994f;
		alleyPreset.LFReference = 250.0f;
		alleyPreset.HFReference = 5000.0f;
		alleyPreset.Density = 1.0f;
		alleyPreset.Diffusion = 0.3f;
		alleyPreset.EchoTime = 0.13f;
		alleyPreset.EchoDepth = 0.95f;
		alleyPreset.ModulationTime = 0.25f;
		alleyPreset.ModulationDepth = 0.0f;
		alleyPreset.DecayTime = 1.49f;
		alleyPreset.DecayLFRatio = 1.0f;
		alleyPreset.DecayHFRatio = 0.86f;
		alleyPreset.DecayHFLimit = true;
		alleyPreset.ReflectionsGain = 0.250f;
		alleyPreset.ReflectionsDelay = 0.01f;
		alleyPreset.LateReverbGain = 0.997f;
		alleyPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters forestPreset;
		forestPreset.Gain = 0.316f;
		forestPreset.GainLF = 1.0f;
		forestPreset.GainHF = 0.022f;
		forestPreset.RoomRolloffFactor = 0.0f;
		forestPreset.AirAbsorptionGainHF = 0.994f;
		forestPreset.LFReference = 250.0f;
		forestPreset.HFReference = 5000.0f;
		forestPreset.Density = 1.0f;
		forestPreset.Diffusion = 0.3f;
		forestPreset.EchoTime = 0.13f;
		forestPreset.EchoDepth = 1.0f;
		forestPreset.ModulationTime = 0.25f;
		forestPreset.ModulationDepth = 0.0f;
		forestPreset.DecayTime = 1.49f;
		forestPreset.DecayLFRatio = 1.0f;
		forestPreset.DecayHFRatio = 0.54f;
		forestPreset.DecayHFLimit = true;
		forestPreset.ReflectionsGain = 0.052f;
		forestPreset.ReflectionsDelay = 0.16f;
		forestPreset.LateReverbGain = 0.768f;
		forestPreset.LateReverbDelay = 0.09f;

		sEAXReverbParameters cityPreset;
		cityPreset.Gain = 0.316f;
		cityPreset.GainLF = 1.0f;
		cityPreset.GainHF = 0.398f;
		cityPreset.RoomRolloffFactor = 0.0f;
		cityPreset.AirAbsorptionGainHF = 0.994f;
		cityPreset.LFReference = 250.0f;
		cityPreset.HFReference = 5000.0f;
		cityPreset.Density = 1.0f;
		cityPreset.Diffusion = 0.5f;
		cityPreset.EchoTime = 0.25f;
		cityPreset.EchoDepth = 0.0f;
		cityPreset.ModulationTime = 0.25f;
		cityPreset.ModulationDepth = 0.0f;
		cityPreset.DecayTime = 1.49f;
		cityPreset.DecayLFRatio = 1.0f;
		cityPreset.DecayHFRatio = 0.67f;
		cityPreset.DecayHFLimit = true;
		cityPreset.ReflectionsGain = 0.073f;
		cityPreset.ReflectionsDelay = 0.01f;
		cityPreset.LateReverbGain = 0.143f;
		cityPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters mountainsPreset;
		mountainsPreset.Gain = 0.316f;
		mountainsPreset.GainLF = 1.0f;
		mountainsPreset.GainHF = 0.056f;
		mountainsPreset.RoomRolloffFactor = 0.0f;
		mountainsPreset.AirAbsorptionGainHF = 0.994f;
		mountainsPreset.LFReference = 250.0f;
		mountainsPreset.HFReference = 5000.0f;
		mountainsPreset.Density = 1.0f;
		mountainsPreset.Diffusion = 0.27f;
		mountainsPreset.EchoTime = 0.25f;
		mountainsPreset.EchoDepth = 1.0f;
		mountainsPreset.ModulationTime = 0.25f;
		mountainsPreset.ModulationDepth = 0.0f;
		mountainsPreset.DecayTime = 1.49f;
		mountainsPreset.DecayLFRatio = 1.0f;
		mountainsPreset.DecayHFRatio = 0.21f;
		mountainsPreset.DecayHFLimit = false;
		mountainsPreset.ReflectionsGain = 0.041f;
		mountainsPreset.ReflectionsDelay = 0.30f;
		mountainsPreset.LateReverbGain = 0.192f;
		mountainsPreset.LateReverbDelay = 0.1f;

		sEAXReverbParameters quarryPreset;
		quarryPreset.Gain = 0.316f;
		quarryPreset.GainLF = 1.0f;
		quarryPreset.GainHF = 0.316f;
		quarryPreset.RoomRolloffFactor = 0.0f;
		quarryPreset.AirAbsorptionGainHF = 0.994f;
		quarryPreset.LFReference = 250.0f;
		quarryPreset.HFReference = 5000.0f;
		quarryPreset.Density = 1.0f;
		quarryPreset.Diffusion = 1.0f;
		quarryPreset.EchoTime = 0.13f;
		quarryPreset.EchoDepth = 0.70f;
		quarryPreset.ModulationTime = 0.25f;
		quarryPreset.ModulationDepth = 0.0f;
		quarryPreset.DecayTime = 1.49f;
		quarryPreset.DecayLFRatio = 1.0f;
		quarryPreset.DecayHFRatio = 0.83f;
		quarryPreset.DecayHFLimit = true;
		quarryPreset.ReflectionsGain = 0.0f;
		quarryPreset.ReflectionsDelay = 0.06f;
		quarryPreset.LateReverbGain = 1.778f;
		quarryPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters plainPreset;
		plainPreset.Gain = 0.316f;
		plainPreset.GainLF = 1.0f;
		plainPreset.GainHF = 0.100f;
		plainPreset.RoomRolloffFactor = 0.0f;
		plainPreset.AirAbsorptionGainHF = 0.994f;
		plainPreset.LFReference = 250.0f;
		plainPreset.HFReference = 5000.0f;
		plainPreset.Density = 1.0f;
		plainPreset.Diffusion = 0.21f;
		plainPreset.EchoTime = 0.25f;
		plainPreset.EchoDepth = 1.0f;
		plainPreset.ModulationTime = 0.25f;
		plainPreset.ModulationDepth = 0.0f;
		plainPreset.DecayTime = 1.49f;
		plainPreset.DecayLFRatio = 1.0f;
		plainPreset.DecayHFRatio = 0.50f;
		plainPreset.DecayHFLimit = true;
		plainPreset.ReflectionsGain = 0.058f;
		plainPreset.ReflectionsDelay = 0.18f;
		plainPreset.LateReverbGain = 0.109f;
		plainPreset.LateReverbDelay = 0.1f;

		sEAXReverbParameters parkingLotPreset;
		parkingLotPreset.Gain = 0.316f;
		parkingLotPreset.GainLF = 1.0f;
		parkingLotPreset.GainHF = 1.0f;
		parkingLotPreset.RoomRolloffFactor = 0.0f;
		parkingLotPreset.AirAbsorptionGainHF = 0.994f;
		parkingLotPreset.LFReference = 250.0f;
		parkingLotPreset.HFReference = 5000.0f;
		parkingLotPreset.Density = 1.0f;
		parkingLotPreset.Diffusion = 1.0f;
		parkingLotPreset.EchoTime = 0.25f;
		parkingLotPreset.EchoDepth = 0.0f;
		parkingLotPreset.ModulationTime = 0.25f;
		parkingLotPreset.ModulationDepth = 0.0f;
		parkingLotPreset.DecayTime = 1.65f;
		parkingLotPreset.DecayLFRatio = 1.0f;
		parkingLotPreset.DecayHFRatio = 1.50f;
		parkingLotPreset.DecayHFLimit = false;
		parkingLotPreset.ReflectionsGain = 0.208f;
		parkingLotPreset.ReflectionsDelay = 0.01f;
		parkingLotPreset.LateReverbGain = 0.265f;
		parkingLotPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters sewerPipePreset;
		sewerPipePreset.Gain = 0.316f;
		sewerPipePreset.GainLF = 1.0f;
		sewerPipePreset.GainHF = 0.316f;
		sewerPipePreset.RoomRolloffFactor = 0.0f;
		sewerPipePreset.AirAbsorptionGainHF = 0.994f;
		sewerPipePreset.LFReference = 250.0f;
		sewerPipePreset.HFReference = 5000.0f;
		sewerPipePreset.Density = 0.31f;
		sewerPipePreset.Diffusion = 0.8f;
		sewerPipePreset.EchoTime = 0.25f;
		sewerPipePreset.EchoDepth = 0.0f;
		sewerPipePreset.ModulationTime = 0.25f;
		sewerPipePreset.ModulationDepth = 0.0f;
		sewerPipePreset.DecayTime = 2.81f;
		sewerPipePreset.DecayLFRatio = 1.0f;
		sewerPipePreset.DecayHFRatio = 0.14f;
		sewerPipePreset.DecayHFLimit = true;
		sewerPipePreset.ReflectionsGain = 1.637f;
		sewerPipePreset.ReflectionsDelay = 0.01f;
		sewerPipePreset.LateReverbGain = 3.247f;
		sewerPipePreset.LateReverbDelay = 0.02f;

		sEAXReverbParameters underWaterPreset;
		underWaterPreset.Gain = 0.316f;
		underWaterPreset.GainLF = 1.0f;
		underWaterPreset.GainHF = 0.010f;
		underWaterPreset.RoomRolloffFactor = 0.0f;
		underWaterPreset.AirAbsorptionGainHF = 0.994f;
		underWaterPreset.LFReference = 250.0f;
		underWaterPreset.HFReference = 5000.0f;
		underWaterPreset.Density = 0.36f;
		underWaterPreset.Diffusion = 1.0f;
		underWaterPreset.EchoTime = 0.25f;
		underWaterPreset.EchoDepth = 0.0f;
		underWaterPreset.ModulationTime = 1.18f;
		underWaterPreset.ModulationDepth = 0.35f;
		underWaterPreset.DecayTime = 1.49f;
		underWaterPreset.DecayLFRatio = 1.0f;
		underWaterPreset.DecayHFRatio = 0.10f;
		underWaterPreset.DecayHFLimit = true;
		underWaterPreset.ReflectionsGain = 0.597f;
		underWaterPreset.ReflectionsDelay = 0.01f;
		underWaterPreset.LateReverbGain = 7.080f;
		underWaterPreset.LateReverbDelay = 0.01f;

		sEAXReverbParameters druggedPreset;
		druggedPreset.Gain = 0.316f;
		druggedPreset.GainLF = 1.0f;
		druggedPreset.GainHF = 1.0f;
		druggedPreset.RoomRolloffFactor = 0.0f;
		druggedPreset.AirAbsorptionGainHF = 0.994f;
		druggedPreset.LFReference = 250.0f;
		druggedPreset.HFReference = 5000.0f;
		druggedPreset.Density = 0.43f;
		druggedPreset.Diffusion = 0.5f;
		druggedPreset.EchoTime = 0.25f;
		druggedPreset.EchoDepth = 0.0f;
		druggedPreset.ModulationTime = 0.25f;
		druggedPreset.ModulationDepth = 1.0f;
		druggedPreset.DecayTime = 8.39f;
		druggedPreset.DecayLFRatio = 1.0f;
		druggedPreset.DecayHFRatio = 1.39f;
		druggedPreset.DecayHFLimit = false;
		druggedPreset.ReflectionsGain = 0.876f;
		druggedPreset.ReflectionsDelay = 0.0f;
		druggedPreset.LateReverbGain = 3.108f;
		druggedPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters dizzyPreset;
		dizzyPreset.Gain = 0.316f;
		dizzyPreset.GainLF = 1.0f;
		dizzyPreset.GainHF = 0.632f;
		dizzyPreset.RoomRolloffFactor = 0.0f;
		dizzyPreset.AirAbsorptionGainHF = 0.994f;
		dizzyPreset.LFReference = 250.0f;
		dizzyPreset.HFReference = 5000.0f;
		dizzyPreset.Density = 0.36f;
		dizzyPreset.Diffusion = 0.6f;
		dizzyPreset.EchoTime = 0.25f;
		dizzyPreset.EchoDepth = 1.0f;
		dizzyPreset.ModulationTime = 0.81f;
		dizzyPreset.ModulationDepth = 0.31f;
		dizzyPreset.DecayTime = 17.23f;
		dizzyPreset.DecayLFRatio = 1.0f;
		dizzyPreset.DecayHFRatio = 0.56f;
		dizzyPreset.DecayHFLimit = false;
		dizzyPreset.ReflectionsGain = 0.139f;
		dizzyPreset.ReflectionsDelay = 0.02f;
		dizzyPreset.LateReverbGain = 0.494f;
		dizzyPreset.LateReverbDelay = 0.03f;

		sEAXReverbParameters psychoticPreset;
		psychoticPreset.Gain = 0.316f;
		psychoticPreset.GainLF = 1.0f;
		psychoticPreset.GainHF = 0.841f;
		psychoticPreset.RoomRolloffFactor = 0.0f;
		psychoticPreset.AirAbsorptionGainHF = 0.994f;
		psychoticPreset.LFReference = 250.0f;
		psychoticPreset.HFReference = 5000.0f;
		psychoticPreset.Density = 0.06f;
		psychoticPreset.Diffusion = 0.5f;
		psychoticPreset.EchoTime = 0.25f;
		psychoticPreset.EchoDepth = 0.0f;
		psychoticPreset.ModulationTime = 4.0f;
		psychoticPreset.ModulationDepth = 1.0f;
		psychoticPreset.DecayTime = 7.56f;
		psychoticPreset.DecayLFRatio = 1.0f;
		psychoticPreset.DecayHFRatio = 0.91f;
		psychoticPreset.DecayHFLimit = false;
		psychoticPreset.ReflectionsGain = 0.487f;
		psychoticPreset.ReflectionsDelay = 0.02f;
		psychoticPreset.LateReverbGain = 2.438f;
		psychoticPreset.LateReverbDelay = 0.03f;

		manager->getEffects()->addEAXReverbEffectPreset("Generic", genericPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Padded Cell", paddedCellPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Room", roomPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Bath Room", bathroomPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Living Room", livingroomPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Stone Room", stoneroomPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Auditorium", auditoriumPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Concert Hall", concertHallPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Cave", cavePreset);
		manager->getEffects()->addEAXReverbEffectPreset("Arena", arenaPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Hangar", hangarPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Carpeted Hallway", carpetedHallwayPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Hallway", hallwayPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Stone Corridor", stoneCorridorPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Alley", alleyPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Forest", forestPreset);
		manager->getEffects()->addEAXReverbEffectPreset("City", cityPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Mountains", mountainsPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Quarry", quarryPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Plain", plainPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Parking Lot", parkingLotPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Sewer Pipe", sewerPipePreset);
		manager->getEffects()->addEAXReverbEffectPreset("Under Water", underWaterPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Drugged", druggedPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Dizzy", dizzyPreset);
		manager->getEffects()->addEAXReverbEffectPreset("Psychotic", psychoticPreset);
#endif
	}

	void onCreateAudioCapture(IAudioCapture* capture)
	{
		//Do nothing with this "event"
	}

	void onDestroyAudioManager(IAudioManager* manager)
	{
#ifdef CAUDIO_EFX_ENABLED
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Generic");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Padded Cell");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Room");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Bath Room");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Living Room");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Stone Room");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Auditorium");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Concert Hall");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Cave");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Arena");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Hangar");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Carpeted Hallway");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Hallway");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Stone Corridor");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Alley");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Forest");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "City");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Mountains");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Quarry");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Plain");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Parking Lot");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Sewer Pipe");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Under Water");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Drugged");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Dizzy");
		manager->getEffects()->removeEffectPreset(EET_EAX_REVERB, "Psychotic");
#endif
	}

	void onDestoryAudioCapture(IAudioCapture* capture)
	{
		//Do nothing with this "event"
	}
};

#endif //! CEAXLEGACYPRESETPLUGIN_H