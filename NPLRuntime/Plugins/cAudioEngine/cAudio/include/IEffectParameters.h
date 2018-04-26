// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cVector3.h"
#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

namespace cAudio
{
	//! Contains parameters for the EAX Reverb Effect.  This effect tries to simulate how sound behaves in different environments.
	struct sEAXReverbParameters
	{
		sEAXReverbParameters(
			float density = 1.0f,
			float diffusion = 1.0f, 
			float gain = 0.32f,
			float gainHF = 0.89f, 
			float gainLF = 0.0f, 
			float decayTime = 1.49f, 
			float decayHFRatio = 0.83f, 
			float decayLFRatio = 1.0f, 
			float reflectionsGain = 0.05f, 
			float reflectionsDelay = 0.007f, 
			cVector3 reflectionsPan = cVector3(0.0f, 0.0f, 0.0f), 
			float lateReverbGain = 1.26f, 
			float lateReverbDelay = 0.011f, 
			cVector3 lateReverbPan = cVector3(0.0f, 0.0f, 0.0f), 
			float echoTime = 0.25f, 
			float echoDepth = 0.0f, 
			float modulationTime = 0.25f, 
			float modulationDepth = 0.0f, 
			float airAbsorptionGainHF = 0.994f, 
			float hFReference = 5000.0f, 
			float lFReference = 250.0f, 
			float roomRolloffFactor = 0.0f, 
			bool decayHFLimit = true) : 
			Density(density), Diffusion(diffusion), Gain(gain), GainHF(gainHF), GainLF(gainLF),
			DecayTime(decayTime), DecayHFRatio(decayHFRatio), DecayLFRatio(decayLFRatio), 
			ReflectionsGain(reflectionsGain), ReflectionsDelay(reflectionsDelay),
			ReflectionsPan(reflectionsPan), LateReverbGain(lateReverbGain),
			LateReverbDelay(lateReverbDelay), LateReverbPan(lateReverbPan),
			EchoTime(echoTime), EchoDepth(echoDepth),ModulationTime(modulationTime), 
			ModulationDepth(modulationDepth), AirAbsorptionGainHF(airAbsorptionGainHF), 
			HFReference(hFReference), LFReference(lFReference), 
			RoomRolloffFactor(roomRolloffFactor), DecayHFLimit(decayHFLimit) { }

		//! Reverb Modal Density controls the coloration of the late reverb. Lowering the value adds 
		//! more coloration to the late reverb.
		//! Range: 0.0 to 1.0
		float Density;

		//! The Reverb Diffusion property controls the echo density in the reverberation decay. It's set by
		//! default to 1.0, which provides the highest density. Reducing diffusion gives the reverberation a
		//! more "grainy" character that is especially noticeable with percussive sound sources. If you set a
		//! diffusion value of 0.0, the later reverberation sounds like a succession of distinct echoes.
		//! Range: 0.0 to 1.0
		float Diffusion;

		//! The Reverb Gain property is the master volume control for the reflected sound (both early
		//! reflections and reverberation) that the reverb effect adds to all sound sources. It sets the
		//! maximum amount of reflections and reverberation added to the final sound mix. The value of the
		//! Reverb Gain property ranges from 1.0 (0db) (the maximum amount) to 0.0 (-100db) (no reflected
		//! sound at all).
		//! Range: 0.0 to 1.0
		float Gain;

		//! The Reverb Gain HF property further tweaks reflected sound by attenuating it at high frequencies.
		//! It controls a low-pass filter that applies globally to the reflected sound of all sound sources
		//! feeding the particular instance of the reverb effect. The value of the Reverb Gain HF property
		//! ranges from 1.0 (0db) (no filter) to 0.0 (-100db) (virtually no reflected sound). HF Reference sets
		//! the frequency at which the value of this property is measured.
		//! Range: 0.0 to 1.0
		float GainHF;

		//! The Reverb Gain LF property further tweaks reflected sound by attenuating it at low frequencies.
		//! It controls a high-pass filter that applies globally to the reflected sound of all sound sources
		//! feeding the particular instance of the reverb effect. The value of the Reverb Gain LF property
		//! ranges from 1.0 (0db) (no filter) to 0.0 (-100db) (virtually no reflected sound). LF Reference sets
		//! the frequency at which the value of this property is measured.
		//! Range: 0.0 to 1.0
		float GainLF;

		//! The Decay Time property sets the reverberation decay time. It ranges from 0.1 (typically a small
		//! room with very dead surfaces) to 20.0 (typically a large room with very live surfaces).
		//! Range: 0.1 to 20.0
		float DecayTime;

		//! The Decay HF Ratio property adjusts the spectral quality of the Decay Time parameter. It is the
		//! ratio of high-frequency decay time relative to the time set by Decay Time. The Decay HF Ratio
		//! value 1.0 is neutral: the decay time is equal for all frequencies. As Decay HF Ratio increases
		//! above 1.0, the high-frequency decay time increases so it's longer than the decay time at mid
		//! frequencies. You hear a more brilliant reverberation with a longer decay at high frequencies. As
		//! the Decay HF Ratio value decreases below 1.0, the high-frequency decay time decreases so it's
		//! shorter than the decay time of the mid frequencies. You hear a more natural reverberation.
		//! Range: 0.1 to 20.0
		float DecayHFRatio;

		//! The Decay LF Ratio property adjusts the spectral quality of the Decay Time parameter. It is the
		//! ratio of low-frequency decay time relative to the time set by Decay Time. The Decay LF Ratio
		//! value 1.0 is neutral: the decay time is equal for all frequencies. As Decay LF Ratio increases
		//! above 1.0, the low-frequency decay time increases so it's longer than the decay time at mid
		//! frequencies. You hear a more booming reverberation with a longer decay at low frequencies. As
		//! the Decay LF Ratio value decreases below 1.0, the low-frequency decay time decreases so it's
		//! shorter than the decay time of the mid frequencies. You hear a more tinny reverberation.
		//! Range: 0.1 to 20.0
		float DecayLFRatio;

		//! The Reflections Gain property controls the overall amount of initial reflections relative to the Gain
		//! property. (The Gain property sets the overall amount of reflected sound: both initial reflections
		//! and later reverberation.) The value of Reflections Gain ranges from a maximum of 3.16 (+10 dB)
		//! to a minimum of 0.0 (-100 dB) (no initial reflections at all), and is corrected by the value of the
		//! Gain property. The Reflections Gain property does not affect the subsequent reverberation decay.
		//! Range: 0.0 to 3.16
		float ReflectionsGain;

		//! The Reflections Delay property is the amount of delay between the arrival time of the direct path
		//! from the source to the first reflection from the source. It ranges from 0 to 300 milliseconds. You
		//! can reduce or increase Reflections Delay to simulate closer or more distant reflective surfaces—
		//! and therefore control the perceived size of the room.
		//! Range: 0.0 to 0.3
		float ReflectionsDelay;

		//! The Reflections Pan property is a 3D vector that controls the spatial distribution of the cluster of
		//! early reflections. The direction of this vector controls the global direction of the reflections, while
		//! its magnitude controls how focused the reflections are towards this direction.
		//! It is important to note that the direction of the vector is interpreted in the coordinate system of the
		//! user, without taking into account the orientation of the virtual listener. For instance, assuming a
		//! four-point loudspeaker playback system, setting Reflections Pan to (0, 0, 0.7) means that the
		//! reflections are panned to the front speaker pair, whereas as setting of (0, 0, -0.7) pans the
		//! reflections towards the rear speakers. These vectors follow the a left-handed co-ordinate system,
		//! unlike OpenAL uses a right-handed co-ordinate system.
		//! If the magnitude of Reflections Pan is zero (the default setting), the early reflections come evenly
		//! from all directions. As the magnitude increases, the reflections become more focused in the
		//! direction pointed to by the vector. A magnitude of 1.0 would represent the extreme case, where
		//! all reflections come from a single direction.
		cVector3 ReflectionsPan;

		//! The Late Reverb Gain property controls the overall amount of later reverberation relative to the
		//! Gain property. (The Gain property sets the overall amount of both initial reflections and later
		//! reverberation.) The value of Late Reverb Gain ranges from a maximum of 10.0 (+20 dB) to a
		//! minimum of 0.0 (-100 dB) (no late reverberation at all).
		//! Range: 0.0 to 10.0
		float LateReverbGain;

		//! The Late Reverb Delay property defines the begin time of the late reverberation relative to the
		//! time of the initial reflection (the first of the early reflections). It ranges from 0 to 100 milliseconds.
		//! Reducing or increasing Late Reverb Delay is useful for simulating a smaller or larger room.
		//! Range: 0.0 to 0.1
		float LateReverbDelay;

		//! The Late Reverb Pan property is a 3D vector that controls the spatial distribution of the late
		//! reverb. The direction of this vector controls the global direction of the reverb, while its magnitude
		//! controls how focused the reverb are towards this direction. The details under Reflections Pan,
		//! above, also apply to Late Reverb Pan.
		cVector3 LateReverbPan;

		//! Echo Time controls the rate at which the cyclic echo repeats itself along the
		//! reverberation decay. For example, the default setting for Echo Time is 250 ms. causing the echo
		//! to occur 4 times per second. Therefore, if you were to clap your hands in this type of
		//! environment, you will hear four repetitions of clap per second.
		//! Range: 0.075 to 0.25
		float EchoTime;

		//! Echo Depth introduces a cyclic echo in the reverberation decay, which will be noticeable with
		//! transient or percussive sounds. A larger value of Echo Depth will make this effect more
		//! prominent. 
		//! Together with Reverb Diffusion, Echo Depth will control how long the echo effect will persist along
		//! the reverberation decay. In a more diffuse environment, echoes will wash out more quickly after
		//! the direct sound. In an environment that is less diffuse, you will be able to hear a larger number
		//! of repetitions of the echo, which will wash out later in the reverberation decay. If Diffusion is set
		//! to 0.0 and Echo Depth is set to 1.0, the echo will persist distinctly until the end of the
		//! reverberation decay.
		//! Range: 0.0 to 1.0
		float EchoDepth;
		
		//! Using these two properties below, you can create a pitch modulation in the reverberant sound. This will
		//! be most noticeable applied to sources that have tonal color or pitch. You can use this to make
		//! some trippy effects! Modulation Time controls the speed of the vibrato (rate of periodic changes in pitch).
		//! Range: 0.004 to 4.0
		float ModulationTime;

		//! Modulation Depth controls the amount of pitch change. Low values of Diffusion will contribute to
		//! reinforcing the perceived effect by reducing the mixing of overlapping reflections in the
		//! reverberation decay.
		//! Range: 0.0 to 1.0
		float ModulationDepth;

		//! The Air Absorption Gain HF property controls the distance-dependent attenuation at high
		//! frequencies caused by the propagation medium. It applies to reflected sound only. You can use
		//! Air Absorption Gain HF to simulate sound transmission through foggy air, dry air, smoky
		//! atmosphere, and so on. The default value is 0.994 (-0.05 dB) per meter, which roughly
		//! corresponds to typical condition of atmospheric humidity, temperature, and so on. Lowering the
		//! value simulates a more absorbent medium (more humidity in the air, for example); raising the
		//! value simulates a less absorbent medium (dry desert air, for example).
		//! Range: 0.892 to 1.0
		float AirAbsorptionGainHF;

		//! The properties HF Reference and LF Reference determine respectively the frequencies at which
		//! the high-frequency effects and the low-frequency effects created by EAX Reverb properties are
		//! measured, for example Decay HF Ratio and Decay LF Ratio.
		//! Note that it is necessary to maintain a factor of at least 10 between these two reference
		//! frequencies so that low frequency and high frequency properties can be accurately controlled and
		//! will produce independent effects. In other words, the LF Reference value should be less than
		//! 1/10 of the HF Reference value.
		//! Range: 1000.0 to 20000.0
		float HFReference;

		//! See HFReference.
		//! Range: 20.0 to 1000.0
		float LFReference;

		//! The Room Rolloff Factor property is one of two methods available to attenuate the reflected
		//! sound (containing both reflections and reverberation) according to source-listener distance. It's
		//! defined the same way as OpenAL's Rolloff Factor, but operates on reverb sound instead of
		//! direct-path sound. Setting the Room Rolloff Factor value to 1.0 specifies that the reflected sound
		//! will decay by 6 dB every time the distance doubles. Any value other than 1.0 is equivalent to a
		//! scaling factor applied to the quantity specified by ((Source listener distance) - (Reference
		//! Distance)). Reference Distance is an OpenAL source parameter that specifies the inner border
		//! for distance rolloff effects: if the source comes closer to the listener than the reference distance,
		//! the direct-path sound isn't increased as the source comes closer to the listener, and neither is the
		//! reflected sound.
		//! The default value of Room Rolloff Factor is 0.0 because, by default, the Effects Extension reverb
		//! effect naturally manages the reflected sound level automatically for each sound source to
		//! simulate the natural rolloff of reflected sound vs. distance in typical rooms.
		//! Range: 0.0 to 10.0
		float RoomRolloffFactor;

		//! When this flag is set, the high-frequency decay time automatically stays below a limit value that's
		//! derived from the setting of the property Air Absorption Gain HF. This limit applies regardless of
		//! the setting of the property Decay HF Ratio, and the limit doesn't affect the value of Decay HF
		//! Ratio. This limit, when on, maintains a natural sounding reverberation decay by allowing you to
		//! increase the value of Decay Time without the risk of getting an unnaturally long decay time at
		//! high frequencies. If this flag is set to false, high-frequency decay time isn't automatically
		//! limited.
		bool DecayHFLimit;
	};

	//! Similar to the above EAX Reverb Effect, but has less features, meaning it may be better supported on lower end hardware.
	struct sReverbParameters
	{
		sReverbParameters(
			float density = 1.0f,
			float diffusion = 1.0f, 
			float gain = 0.32f,
			float gainHF = 0.89f, 
			float decayTime = 1.49f, 
			float decayHFRatio = 0.83f, 
			float reflectionsGain = 0.05f, 
			float reflectionsDelay = 0.007f, 
			float lateReverbGain = 1.26f, 
			float lateReverbDelay = 0.011f, 
			float airAbsorptionGainHF = 0.994f, 
			float roomRolloffFactor = 0.0f, 
			bool decayHFLimit = true) : 
			Density(density), Diffusion(diffusion), Gain(gain), GainHF(gainHF),
			DecayTime(decayTime), DecayHFRatio(decayHFRatio), 
			ReflectionsGain(reflectionsGain), ReflectionsDelay(reflectionsDelay),
			LateReverbGain(lateReverbGain), LateReverbDelay(lateReverbDelay),
			AirAbsorptionGainHF(airAbsorptionGainHF), RoomRolloffFactor(roomRolloffFactor), 
			DecayHFLimit(decayHFLimit) { }

		//! Reverb Modal Density controls the coloration of the late reverb. Lowering the value adds more
		//! coloration to the late reverb.
		//! Range: 0.0 to 1.0
		float Density;

		//! The Reverb Diffusion property controls the echo density in the reverberation decay. It's set by
		//! default to 1.0, which provides the highest density. Reducing diffusion gives the reverberation a
		//! more "grainy" character that is especially noticeable with percussive sound sources. If you set a
		//! diffusion value of 0.0, the later reverberation sounds like a succession of distinct echoes.
		//! Range: 0.0 to 1.0
		float Diffusion;

		//! The Reverb Gain property is the master volume control for the reflected sound (both early
		//! reflections and reverberation) that the reverb effect adds to all sound sources. It sets the
		//! maximum amount of reflections and reverberation added to the final sound mix. The value of the
		//! Reverb Gain property ranges from 1.0 (0db) (the maximum amount) to 0.0 (-100db) (no reflected
		//! sound at all).
		//! Range: 0.0 to 1.0
		float Gain;

		//! The Reverb Gain HF property further tweaks reflected sound by attenuating it at high frequencies.
		//! It controls a low-pass filter that applies globally to the reflected sound of all sound sources
		//! feeding the particular instance of the reverb effect. The value of the Reverb Gain HF property
		//! ranges from 1.0 (0db) (no filter) to 0.0 (-100db) (virtually no reflected sound).
		//! Range: 0.0 to 1.0
		float GainHF;

		//! The Decay Time property sets the reverberation decay time. It ranges from 0.1 (typically a small
		//! room with very dead surfaces) to 20.0 (typically a large room with very live surfaces).
		//! Range: 0.1 to 20.0
		float DecayTime;

		//! The Decay HF Ratio property sets the spectral quality of the Decay Time parameter. It is the
		//! ratio of high-frequency decay time relative to the time set by Decay Time. The Decay HF Ratio
		//! value 1.0 is neutral: the decay time is equal for all frequencies. As Decay HF Ratio increases
		//! above 1.0, the high-frequency decay time increases so it's longer than the decay time at low
		//! frequencies. You hear a more brilliant reverberation with a longer decay at high frequencies. As
		//! the Decay HF Ratio value decreases below 1.0, the high-frequency decay time decreases so it's
		//! shorter than the decay time of the low frequencies. You hear a more natural reverberation.
		//! Range: 0.1 to 2.0
		float DecayHFRatio;

		//! The Reflections Gain property controls the overall amount of initial reflections relative to the Gain
		//! property. (The Gain property sets the overall amount of reflected sound: both initial reflections
		//! and later reverberation.) The value of Reflections Gain ranges from a maximum of 3.16 (+10 dB)
		//! to a minimum of 0.0 (-100 dB) (no initial reflections at all), and is corrected by the value of the
		//! Gain property. The Reflections Gain property does not affect the subsequent reverberation
		//! decay.
		//! Range: 0.0 to 3.16
		float ReflectionsGain;

		//! The Reflections Delay property is the amount of delay between the arrival time of the direct path
		//! from the source to the first reflection from the source. It ranges from 0 to 300 milliseconds. You
		//! can reduce or increase Reflections Delay to simulate closer or more distant reflective surfaces—
		//! and therefore control the perceived size of the room.
		//! Range: 0.0 to 0.3
		float ReflectionsDelay;

		//! The Late Reverb Gain property controls the overall amount of later reverberation relative to the
		//! Gain property. (The Gain property sets the overall amount of both initial reflections and later
		//! reverberation.) The value of Late Reverb Gain ranges from a maximum of 10.0 (+20 dB) to a
		//! minimum of 0.0 (-100 dB) (no late reverberation at all).
		//! Range: 0.0 to 10.0
		float LateReverbGain;

		//! The Late Reverb Delay property defines the begin time of the late reverberation relative to the
		//! time of the initial reflection (the first of the early reflections). It ranges from 0 to 100 milliseconds.
		//! Reducing or increasing Late Reverb Delay is useful for simulating a smaller or larger room.
		//! Range: 0.0 to 0.1
		float LateReverbDelay;

		//! The Air Absorption Gain HF property controls the distance-dependent attenuation at high
		//! frequencies caused by the propagation medium. It applies to reflected sound only. You can use
		//! Air Absorption Gain HF to simulate sound transmission through foggy air, dry air, smoky
		//! atmosphere, and so on. The default value is 0.994 (-0.05 dB) per meter, which roughly
		//! corresponds to typical condition of atmospheric humidity, temperature, and so on. Lowering the
		//! value simulates a more absorbent medium (more humidity in the air, for example); raising the
		//! value simulates a less absorbent medium (dry desert air, for example).
		//! Range: 0.892 to 1.0
		float AirAbsorptionGainHF;

		//! The Room Rolloff Factor property is one of two methods available to attenuate the reflected
		//! sound (containing both reflections and reverberation) according to source-listener distance. It's
		//! defined the same way as OpenAL's Rolloff Factor, but operates on reverb sound instead of
		//! direct-path sound. Setting the Room Rolloff Factor value to 1.0 specifies that the reflected sound
		//! will decay by 6 dB every time the distance doubles. Any value other than 1.0 is equivalent to a
		//! scaling factor applied to the quantity specified by ((Source listener distance) - (Reference
		//! Distance)). Reference Distance is an OpenAL source parameter that specifies the inner border
		//! for distance rolloff effects: if the source comes closer to the listener than the reference distance,
		//! the direct-path sound isn't increased as the source comes closer to the listener, and neither is the
		//! reflected sound.
		//! The default value of Room Rolloff Factor is 0.0 because, by default, the Effects Extension reverb
		//! effect naturally manages the reflected sound level automatically for each sound source to
		//! simulate the natural rolloff of reflected sound vs. distance in typical rooms.
		//! Range: 0.0 to 10.0
		float RoomRolloffFactor;

		//! When this flag is set, the high-frequency decay time automatically stays below a limit value that's
		//! derived from the setting of the property Air Absorption Gain HF. This limit applies regardless of
		//! the setting of the property Decay HF Ratio, and the limit doesn't affect the value of Decay HF
		//! Ratio. This limit, when on, maintains a natural sounding reverberation decay by allowing you to
		//! increase the value of Decay Time without the risk of getting an unnaturally long decay time at
		//! high frequencies. If this flag is set to false, high-frequency decay time isn't automatically
		//! limited.
		bool DecayHFLimit;
	};

	//! The chorus effect essentially replays the input audio accompanied by another slightly delayed version of the signal, creating a "doubling" effect.
	struct sChorusParameters
	{
		enum ChorusWaveform
		{
			ECW_SINUSOID,
			ECW_TRIANGLE,
			ECW_COUNT
		};
		sChorusParameters(
			ChorusWaveform waveform = ECW_TRIANGLE,
			int phase = 90,
			float rate = 1.1f, 
			float depth = 0.1f,
			float feedback = 0.25f, 
			float delay = 0.016f) : 
			Waveform(waveform), Phase(phase), Rate(rate), Depth(depth), Feedback(feedback), 
			Delay(delay) { }

		//! This property sets the waveform shape of the LFO that controls the delay time of the delayed signals.
		ChorusWaveform Waveform;

		//! This property controls the phase difference between the left and right LFO's. At zero degrees the
		//! two LFOs are synchronized. Use this parameter to create the illusion of an expanded stereo field
		//! of the output signal.
		//! Range: -180 to 180
		int Phase;

		//! This property sets the modulation rate of the LFO that controls the delay time of the delayed signals.
		//! Range: 0.0 to 10.0
		float Rate;

		//! This property controls the amount by which the delay time is modulated by the LFO.
		//! Range: 0.0 to 1.0
		float Depth;

		//! This property controls the amount of processed signal that is fed back to the input of the chorus
		//! effect. Negative values will reverse the phase of the feedback signal. At full magnitude the
		//! identical sample will repeat endlessly. At lower magnitudes the sample will repeat and fade out
		//! over time. Use this parameter to create a "cascading" chorus effect.
		//! Range: -1.0 to 1.0
		float Feedback;

		//! This property controls the average amount of time the sample is delayed before it is played back,
		//! and with feedback, the amount of time between iterations of the sample. Larger values lower the
		//! pitch. Smaller values make the chorus sound like a flanger, but with different frequency
		//! characteristics.
		//! Range: 0.0 to 0.016
		float Delay;
	};

	//! The distortion effect simulates turning up (overdriving) the gain stage on a guitar amplifier or adding a distortion pedal to an instrument's output.
	struct sDistortionParameters
	{
		sDistortionParameters(
			float edge = 0.2f,
			float gain = 0.05f, 
			float lowpassCutoff = 8000.0f,
			float eqCenter = 3600.0f, 
			float eqBandwidth = 3600.0f) : 
			Edge(edge), Gain(gain), LowpassCutoff(lowpassCutoff), EqCenter(eqCenter), 
			EqBandwidth(eqBandwidth) { }

		//! This property controls the shape of the distortion. The higher the value for Edge, the "dirtier" and "fuzzier" the effect.
		//! Range: 0.0 to 1.0
		float Edge;

		//! This property allows you to attenuate the distorted sound.
		//! Range: 0.01 to 1.0
		float Gain;

		//! Input signal can have a low pass filter applied, to limit the amount of high frequency signal feeding into the distortion effect.
		//! Range: 80.0 to 24000.0
		float LowpassCutoff;

		//! This property controls the frequency at which the post-distortion attenuation (Gain) is active.
		//! Range: 80.0 to 24000.0
		float EqCenter;

		//! This property controls the bandwidth of the post-distortion attenuation.
		//! Range: 80.0 to 24000.0
		float EqBandwidth;
	};

	//! The echo effect generates discrete, delayed instances of the input signal.
	struct sEchoParameters
	{
		sEchoParameters(
			float delay = 0.1f,
			float lRDelay = 0.1f, 
			float damping = 0.5f,
			float feedback = 0.5f, 
			float spread = -1.0f) : 
			Delay(delay), LRDelay(lRDelay), Damping(damping), Feedback(feedback), 
			Spread(spread) { }

		//! This property controls the delay between the original sound and the first "tap", or echo instance.
		//! Range: 0.0 to 0.207
		float Delay;

		//! This property controls the delay between the first "tap" and the second "tap".
		//! Range: 0.0 to 0.404
		float LRDelay;

		//! This property controls the amount of high frequency damping applied to each echo. As the sound
		//! is subsequently fed back for further echoes, damping results in an echo which progressively gets
		//! softer in tone as well as intensity.
		//! Range: 0.0 to 0.99
		float Damping;

		//! This property controls the amount of feedback the output signal fed back into the input. Use this
		//! parameter to create "cascading" echoes. At full magnitude, the identical sample will repeat
		//! endlessly. Below full magnitude, the sample will repeat and fade.
		//! Range: 0.0 to 1.0
		float Feedback;

		//! This property controls how hard panned the individual echoes are. With a value of 1.0, the first
		//! "tap" will be panned hard left, and the second "tap" hard right. A value of -1.0 gives the opposite
		//! result. Settings nearer to 0.0 result in less emphasized panning.
		//! Range: -1.0 to 1.0
		float Spread;
	};

	//! The flanger effect creates a "tearing" or "whooshing" sound (like a jet flying overhead).
	struct sFlangerParameters
	{
		enum FlangerWaveform
		{
			EFW_SINUSOID,
			EFW_TRIANGLE,
			EFW_COUNT
		};
		sFlangerParameters(
			FlangerWaveform waveform = EFW_TRIANGLE,
			int phase = 0,
			float rate = 0.27f, 
			float depth = 1.0f,
			float feedback = -0.5f, 
			float delay = 0.002f) : 
			Waveform(waveform), Phase(phase), Rate(rate), Depth(depth), Feedback(feedback), 
			Delay(delay) { }

		//! Selects the shape of the LFO waveform that controls the amount of the delay of the sampled signal.
		FlangerWaveform Waveform;

		//! This changes the phase difference between the left and right LFO's. At zero degrees the two LFOs are synchronized.
		//! Range: -180 to 180
		int Phase;

		//! The number of times per second the LFO controlling the amount of delay repeats. Higher values increase the pitch modulation.
		//! Range: 0.0 to 10.0
		float Rate;

		//! The ratio by which the delay time is modulated by the LFO. Use this parameter to increase the pitch modulation.
		//! Range: 0.0 to 1.0
		float Depth;

		//! This is the amount of the output signal level fed back into the effect's input. 
		//! A negative value will reverse the phase of the feedback signal. Use this parameter 
		//! to create an "intense metallic" effect. At full magnitude, the identical sample will 
		//! repeat endlessly. At less than full magnitude, the sample will repeat and fade out over time.
		//! Range: -1.0 to 1.0
		float Feedback;

		//! The average amount of time the sample is delayed before it is played back; with feedback, the amount of time between iterations of the sample.
		//! Range: 0.0 to 0.004
		float Delay;
	};

	//! The frequency shifter is a single-sideband modulator, which translates all the component frequencies of the input signal by an equal amount.
	struct sFrequencyShiftParameters
	{
		enum ShiftDirection
		{
			ESD_DOWN,
			ESD_UP,
			ESD_OFF,
			ESD_COUNT
		};
		sFrequencyShiftParameters(
			float frequency = 0.0f,
			ShiftDirection left = ESD_DOWN,
			ShiftDirection right = ESD_DOWN) : 
			Frequency(frequency), Left(left), Right(right) { }

		//! This is the carrier frequency. For carrier frequencies below the audible range, the singlesideband
		//! modulator may produce phaser effects, spatial effects or a slight pitch-shift. As the
		//! carrier frequency increases, the timbre of the sound is affected; a piano or guitar note becomes
		//! like a bell's chime, and a human voice sounds extraterrestrial!
		//! Range: 0.0 to 24000.0
		float Frequency;

		//! These select which internal signals are added together to produce the output. Different
		//! combinations of values will produce slightly different tonal and spatial effects.
		ShiftDirection Left;

		//! These select which internal signals are added together to produce the output. Different
		//! combinations of values will produce slightly different tonal and spatial effects.
		ShiftDirection Right;
	};

	//! The vocal morpher consists of a pair of 4-band formant filters, used to impose vocal tract effects upon the input signal.
	struct sVocalMorpherParameters
	{
		enum MorpherPhoneme
		{
			EMP_A,
			EMP_E,
			EMP_I,
			EMP_O,
			EMP_U,
			EMP_AA,
			EMP_AE,
			EMP_AH,
			EMP_AO,
			EMP_EH,
			EMP_ER,
			EMP_IH,
			EMP_IY,
			EMP_UH,
			EMP_UW,
			EMP_B,
			EMP_D,
			EMP_F,
			EMP_G,
			EMP_J,
			EMP_K,
			EMP_L,
			EMP_M,
			EMP_N,
			EMP_P,
			EMP_R,
			EMP_S,
			EMP_T,
			EMP_V,
			EMP_Z,
			ESD_COUNT
		};

		enum MorpherWaveform
		{
			EMW_SINUSOID,
			EMW_TRIANGLE,
			EMW_SAW,
			EMW_COUNT
		};
		sVocalMorpherParameters(
			MorpherPhoneme phonemeA = EMP_A,
			MorpherPhoneme phonemeB = EMP_ER,
			int phonemeACoarseTune = 0,
			int phonemeBCoarseTune = 0,
			MorpherWaveform waveform = EMW_SINUSOID,
			float rate = 1.41f) : 
			PhonemeA(phonemeA), PhonemeB(phonemeB), PhonemeACoarseTune(phonemeACoarseTune),
			PhonemeBCoarseTune(phonemeBCoarseTune), Waveform(waveform), Rate(rate) { }

		//! If both parameters are set to the same phoneme, that determines the filtering effect that will be
		//! heard. If these two parameters are set to different phonemes, the filtering effect will morph
		//! between the two settings at a rate specified by Rate.
		MorpherPhoneme PhonemeA;

		//! If both parameters are set to the same phoneme, that determines the filtering effect that will be
		//! heard. If these two parameters are set to different phonemes, the filtering effect will morph
		//! between the two settings at a rate specified by Rate.
		MorpherPhoneme PhonemeB;

		//! This is used to adjust the pitch of phoneme filter A in 1-semitone increments.
		//! Range: -24 to 24
		int PhonemeACoarseTune;

		//! This is used to adjust the pitch of phoneme filter B in 1-semitone increments.
		//! Range: -24 to 24
		int PhonemeBCoarseTune;

		//! This controls the shape of the low-frequency oscillator used to morph between the two phoneme filters.
		MorpherWaveform Waveform;

		//! This controls the frequency of the low-frequency oscillator used to morph between the two phoneme filters.
		//! Range: 0.0 to 10.0
		float Rate;
	};

	//! The pitch shifter applies time-invariant pitch shifting to the input signal, over a one octave range and controllable at a semi-tone and cent resolution.
	struct sPitchShifterParameters
	{
		sPitchShifterParameters(
			int coarseTune = 12,
			int fineTune = 0) : 
			CoarseTune(coarseTune), FineTune(fineTune) { }

		//! This sets the number of semitones by which the pitch is shifted. There are 12 semitones per
		//! octave. Negative values create a downwards shift in pitch, positive values pitch the sound
		//! upwards.
		//! Range: -12 to 12
		int CoarseTune;

		//! This sets the number of cents between Semitones a pitch is shifted. A Cent is 1/100th of a
		//! Semitone. Negative values create a downwards shift in pitch, positive values pitch the sound
		//! upwards.
		//! Range: -50 to 50
		int FineTune;
	};

	//! The ring modulator multiplies an input signal by a carrier signal in the time domain, resulting in tremolo or inharmonic effects.
	struct sRingModulatorParameters
	{
		enum ModulatorWaveform
		{
			EMW_SINUSOID,
			EMW_SAW,
			EMW_SQUARE,
			EMW_COUNT
		};
		sRingModulatorParameters(
			float frequency = 440.0f,
			float highPassCutoff = 800.0f,
			ModulatorWaveform waveform = EMW_SINUSOID) : 
			Frequency(frequency), HighPassCutoff(highPassCutoff), Waveform(waveform) { }

		//! This is the frequency of the carrier signal. If the carrier signal is slowly varying (less than 20 Hz),
		//! the result is a tremolo (slow amplitude variation) effect. If the carrier signal is in the audio range,
		//! audible upper and lower sidebands begin to appear, causing an inharmonic effect. The carrier
		//! signal itself is not heard in the output.
		//! Range: 0.0 to 8000.0
		float Frequency;

		//! This controls the cutoff frequency at which the input signal is high-pass filtered before being ring
		//! modulated. If the cutoff frequency is 0, the entire signal will be ring modulated. If the cutoff
		//! frequency is high, very little of the signal (only those parts above the cutoff) will be ring
		//! modulated.
		//! Range: 0.0 to 24000.0
		float HighPassCutoff;

		//! This controls which waveform is used as the carrier signal. Traditional ring modulator and
		//! tremolo effects generally use a sinusoidal carrier. Sawtooth and square waveforms are may
		//! cause unpleasant aliasing.
		ModulatorWaveform Waveform;
	};

	//! The Auto-wah effect emulates the sound of a wah-wah pedal used with an electric guitar, or a mute on a brass instrument.
	struct sAutowahParameters
	{
		sAutowahParameters(
			float attackTime = 0.06f,
			float releaseTime = 0.06f, 
			float resonance = 1000.0f,
			float peakGain = 11.22f) : 
			AttackTime(attackTime), ReleaseTime(releaseTime), Resonance(resonance), 
			PeakGain(peakGain) { }

		//! This property controls the time the filtering effect takes to sweep from minimum to maximum center frequency when it is triggered by input signal.
		//! Range: 0.0001 to 1.0
		float AttackTime;

		//! This property controls the time the filtering effect takes to sweep from maximum back to base center frequency, when the input signal ends.
		//! Range: 0.0001 to 1.0
		float ReleaseTime;

		//! This property controls the resonant peak, sometimes known as emphasis or Q, of the auto-wah
		//! band-pass filter. Resonance occurs when the effect boosts the frequency content of the sound
		//! around the point at which the filter is working. A high value promotes a highly resonant, sharp
		//! sounding effect.
		//! Range: 2.0 to 1000.0
		float Resonance;

		//! This property controls the input signal level at which the band-pass filter will be fully opened.
		//! Range: 0.00003 to 31621.0
		float PeakGain;
	};

	//! The Automatic Gain Control effect performs the same task as a studio compressor, evening out the audio dynamic range of an input sound.
	struct sCompressorParameters
	{
		sCompressorParameters(
			bool active = true) : 
			Active(active) { }

		//! The Compressor can only be switched on and off – it cannot be adjusted.
		bool Active;
	};

	//! The OpenAL Effects Extension EQ is very flexible, providing tonal control over four different adjustable frequency ranges.
	struct sEqualizerParameters
	{
		sEqualizerParameters(
			float lowGain = 1.0f,
			float lowCutoff = 200.0f,
			float mid1Gain = 1.0f,
			float mid1Center = 500.0f,
			float mid1Width = 1.0f,
			float mid2Gain = 1.0f,
			float mid2Center = 3000.0f,
			float mid2Width = 1.0f,
			float highGain = 1.0f,
			float highCutoff = 6000.0f) : 
			LowGain(lowGain), LowCutoff(lowCutoff), Mid1Gain(mid1Gain), 
			Mid1Center(mid1Center), Mid1Width(mid1Width), Mid2Gain(mid2Gain), 
			Mid2Center(mid2Center), Mid2Width(mid2Width), HighGain(highGain),
			HighCutoff(highCutoff) { }

		//! This property controls amount of cut or boost on the low frequency range.
		//! Range: 0.126 to 7.943
		float LowGain;

		//! This property controls the low frequency below which signal will be cut off.
		//! Range: 50.0 to 800.0
		float LowCutoff;

		//! This property allows you to cut / boost signal on the "mid1" range.
		//! Range: 0.126 to 7.943
		float Mid1Gain;

		//! This property sets the center frequency for the "mid1" range.
		//! Range: 200.0 to 3000.0
		float Mid1Center;

		//! This property controls the width of the "mid1" range.
		//! Range: 0.01 to 1.0
		float Mid1Width;

		//! This property allows you to cut / boost signal on the "mid2" range.
		//! Range: 0.126 to 7.943
		float Mid2Gain;

		//! This property sets the center frequency for the "mid2" range.
		//! Range: 1000.0 to 8000.0
		float Mid2Center;

		//! This property controls the width of the "mid2" range.
		//! Range: 0.01 to 1.0
		float Mid2Width;

		//! This property allows you to cut / boost the signal at high frequencies.
		//! Range: 0.126 to 7.943
		float HighGain;

		//! This property controls the high frequency above which signal will be cut off.
		//! Range: 4000.0 to 16000.0
		float HighCutoff;
	};
};

#endif
