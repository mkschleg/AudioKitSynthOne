//
//  AKSynthOneDSPKernel.hpp
//  AudioKit
//
//  Created by Aurelius Prochazka, revision history on Github.
//  Copyright © 2017 Aurelius Prochazka. All rights reserved.
//
//  20170926: Super HD refactor by Marcus Hobbs

#pragma once

#import <vector>
#import <list>
#include <string>

#import "AKSoundpipeKernel.hpp"
#import "AKSynthOneAudioUnit.h"
#import "AKSynthOneParameter.h"

@class AEArray;
@class AEMessageQueue;

#define AKS1_MAX_POLYPHONY (6)
#define AKS1_NUM_MIDI_NOTES (128)
#define AKS1_FTABLE_SIZE (4096)
#define AKS1_NUM_FTABLES (4)

#ifdef __cplusplus

class AKSynthOneDSPKernel : public AKSoundpipeKernel, public AKOutputBuffered {
    
public:

    // MARK: AKSynthOneDSPKernel Member Functions
    
    AKSynthOneDSPKernel();
    
    ~AKSynthOneDSPKernel();

    void setAK1Parameter(AKSynthOneParameter param, float inputValue);
    
    float getAK1Parameter(AKSynthOneParameter param);

    // AUParameter/AUValue
    void setParameters(float params[]);
    
    void setParameter(AUParameterAddress address, AUValue value);
    
    AUValue getParameter(AUParameterAddress address);
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration) override;
    
    void print_debug();
    
    ///panic...hard-resets DSP.  artifacts.
    void resetDSP();
    
    ///puts all notes in release mode...no artifacts
    void stopAllNotes();
    
    void handleTempoSetting(float currentTempo);
    
    ///can be called from within the render loop
    void beatCounterDidChange();
    
    ///can be called from within the render loop
    void playingNotesDidChange();
    
    ///can be called from within the render loop
    void heldNotesDidChange();
    
    ///PROCESS
    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset) override;
    
    // turnOnKey is called by render thread in "process", so access note via AEArray
    void turnOnKey(int noteNumber, int velocity);
    
    // turnOnKey is called by render thread in "process", so access note via AEArray
    void turnOnKey(int noteNumber, int velocity, float frequency);
    
    // turnOffKey is called by render thread in "process", so access note via AEArray
    void turnOffKey(int noteNumber);
    
    // NOTE ON
    // startNote is not called by render thread, but turnOnKey is
    void startNote(int noteNumber, int velocity);
    
    // NOTE ON
    // startNote is not called by render thread, but turnOnKey is
    void startNote(int noteNumber, int velocity, float frequency);
    
    // NOTE OFF...put into release mode
    void stopNote(int noteNumber);
    
    /// Puts all notes in release mode
    void reset();
    
    /// Sets beatcounter to 0 and clears sequence
    void resetSequencer();
    
    // MIDI
    virtual void handleMIDIEvent(AUMIDIEvent const& midiEvent) override;
    
    void init(int _channels, double _sampleRate) override;
    
    void destroy();
    
    // initializeNoteStates() must be called AFTER init returns
    void initializeNoteStates();
    
    void setupWaveform(uint32_t waveform, uint32_t size);
    
    void setWaveformValue(uint32_t waveform, uint32_t index, float value);
    
    ///parameter min
    float parameterMin(AKSynthOneParameter i);
    
    ///parameter max
    float parameterMax(AKSynthOneParameter i);
    
    ///parameter defaults
    float parameterDefault(AKSynthOneParameter i);
    
    ///parameter unit
    AudioUnitParameterUnit parameterUnit(AKSynthOneParameter i);
    
    ///parameter clamp
    float parameterClamp(AKSynthOneParameter i, float inputValue);

    ///friendly description of parameter
    std::string parameterFriendlyName(AKSynthOneParameter i);
    
    ///C string friendly description of parameter
    const char* parameterCStr(AKSynthOneParameter i);

    ///parameter presetKey
    std::string parameterPresetKey(AKSynthOneParameter i);

    // MARK: Member Variables
public:
    
    AKSynthOneAudioUnit* audioUnit;
    
    bool resetted = false;
    
    int arpBeatCounter = 0;
    
    /// dsp params
    float p[AKSynthOneParameter::AKSynthOneParameterCount];
    
    // Portamento values
    float monoFrequency = 440.f * exp2((60.f - 69.f)/12.f);
    
    // phasor values
    float lfo1 = 0.f;
    float lfo2 = 0.f;
    
    // midi
    bool notesHeld = false;
    
private:
    
    struct NoteNumber;
    
    struct SeqNoteNumber;
    
    struct NoteState;
    
    struct AKS1Param {
        AKSynthOneParameter param;
        float min;
        float defaultValue;
        float max;
        std::string presetKey;
        std::string friendlyName;
        AudioUnitParameterUnit unit;
        bool usePortamento;
        sp_port *portamento;
        float portamentoTarget;
    };
    
    // array of struct NoteState of count MAX_POLYPHONY
    AKSynthOneDSPKernel::NoteState* noteStates;
    
    // monophonic: single instance of NoteState
    AKSynthOneDSPKernel::NoteState* monoNote;
    
    bool initializedNoteStates = false;
    
    // AKS1_MAX_POLYPHONY is the limit of hard-coded number of simultaneous notes to render to manage computation.
    // New noteOn events will steal voices to keep this number.
    // For now "polyphony" is constant equal to AKS1_MAX_POLYPHONY, but with some refactoring we could make it dynamic.
    const int polyphony = AKS1_MAX_POLYPHONY;
    
    int playingNoteStatesIndex = 0;
    sp_ftbl *ft_array[AKS1_NUM_FTABLES];
    UInt32 tbl_size = AKS1_FTABLE_SIZE;
    sp_phasor *lfo1Phasor;
    sp_phasor *lfo2Phasor;
    sp_ftbl *sine;
    sp_bitcrush *bitcrush;
    sp_pan2 *pan;
    sp_osc *panOscillator;
    sp_phaser *phaser0;
    sp_smoothdelay *delayL;
    sp_smoothdelay *delayR;
    sp_smoothdelay *delayRR;
    sp_smoothdelay *delayFillIn;
    sp_crossfade *delayCrossfadeL;
    sp_crossfade *delayCrossfadeR;
    sp_revsc *reverbCostello;
    sp_buthp *butterworthHipassL;
    sp_buthp *butterworthHipassR;
    sp_crossfade *revCrossfadeL;
    sp_crossfade *revCrossfadeR;
    sp_compressor *compressor0;
    sp_compressor *compressor1;
    sp_compressor *compressor2;
    sp_compressor *compressor3;
    sp_port *monoFrequencyPort;
    float monoFrequencySmooth = 261.6255653006f;
    float tempo = 120.f;
    float previousProcessMonoPolyStatus = 0.f;
    float lfo1_0_1;
    float lfo1_1_0;
    float lfo2_0_1;
    float lfo2_1_0;
    float lfo3_0_1;
    float lfo3_1_0;

    // Arp/Seq
    double arpSampleCounter = 0;
    double arpTime = 0;
    int notesPerOctave = 12;
    
    ///once init'd: arpSeqNotes can be accessed and mutated only within process and resetDSP
    std::vector<SeqNoteNumber> arpSeqNotes;
    std::vector<NoteNumber> arpSeqNotes2;
    const int maxArpSeqNotes = 1024; // 128 midi note numbers * 4 arp octaves * up+down
    
    ///once init'd: arpSeqLastNotes can be accessed and mutated only within process and resetDSP
    std::list<int> arpSeqLastNotes;
    
    // Array of midi note numbers of NoteState's which have had a noteOn event but not yet a noteOff event.
    NSMutableArray<NSNumber*>* heldNoteNumbers;
    AEArray* heldNoteNumbersAE;
    
    const float bpm_min = 1.f;
    const float bpm_max = 256.f;
    const float min_division_of_beat = 1.f/64.f; // 1 bar * 64th note
    const float max_division_of_beat = 4.f * 8.f; // 8 bars * 4 beats
    const float rate_min = (bpm_min/60.f) / max_division_of_beat; // Hz 0.000520
    const float rate_max = (bpm_max/60.f) / min_division_of_beat; // Hz 273.0666
    AKS1Param aks1p[AKSynthOneParameter::AKSynthOneParameterCount] = {
        { index1,                0, 1, 1, "index1", "Index 1", kAudioUnitParameterUnit_Generic, true, NULL},
        { index2,                0, 1, 1, "index2", "Index 2", kAudioUnitParameterUnit_Generic, true, NULL},
        { morphBalance,          0, 0.5, 1, "morphBalance", "morphBalance", kAudioUnitParameterUnit_Generic, true, NULL},
        { morph1SemitoneOffset,  -12, 0, 12, "morph1SemitoneOffset", "morph1SemitoneOffset", kAudioUnitParameterUnit_RelativeSemiTones, false, NULL},
        { morph2SemitoneOffset,  -12, 0, 12, "morph2SemitoneOffset", "morph2SemitoneOffset", kAudioUnitParameterUnit_RelativeSemiTones, false, NULL},
        { morph1Volume,          0, 0.8, 1, "morph1Volume", "morph1Volume", kAudioUnitParameterUnit_Generic, true, NULL},
        { morph2Volume,          0, 0.8, 1, "morph2Volume", "morph2Volume", kAudioUnitParameterUnit_Generic, true, NULL},
        { subVolume,             0, 0, 1, "subVolume", "subVolume", kAudioUnitParameterUnit_Generic, true, NULL},
        { subOctaveDown,         0, 0, 1, "subOctaveDown", "subOctaveDown", kAudioUnitParameterUnit_Generic, false, NULL},
        { subIsSquare,           0, 0, 1, "subIsSquare", "subIsSquare", kAudioUnitParameterUnit_Generic, false, NULL},
        { fmVolume,              0, 0, 1, "fmVolume", "fmVolume", kAudioUnitParameterUnit_Generic, true, NULL},
        { fmAmount,              0, 0, 15, "fmAmount", "fmAmount", kAudioUnitParameterUnit_Generic, true, NULL},
        { noiseVolume,           0, 0, 1, "noiseVolume", "noiseVolume", kAudioUnitParameterUnit_Generic, true, NULL},
        { lfo1Index,             0, 0, 3, "lfo1Index", "lfo1Index", kAudioUnitParameterUnit_Generic, false, NULL},
        { lfo1Amplitude,         0, 0, 1, "lfo1Amplitude", "lfo1Amplitude", kAudioUnitParameterUnit_Generic, true, NULL},
        { lfo1Rate,              rate_min, 0.25, rate_max, "lfo1Rate", "lfo1Rate", kAudioUnitParameterUnit_Rate, true, NULL},
        { cutoff,                64, 20000, 22050, "cutoff", "cutoff", kAudioUnitParameterUnit_Hertz, true, NULL},
        { resonance,             0, 0.1, 0.75, "resonance", "resonance", kAudioUnitParameterUnit_Generic, true, NULL},
        { filterMix,             0, 1, 1, "filterMix", "filterMix", kAudioUnitParameterUnit_Generic, true, NULL},
        { filterADSRMix,         0, 0, 1.2, "filterADSRMix", "filterADSRMix", kAudioUnitParameterUnit_Generic, true, NULL},
        { isMono,                0, 0, 1, "isMono", "isMono", kAudioUnitParameterUnit_Generic, false, NULL},
        { glide,                 0, 0, 0.2, "glide", "glide", kAudioUnitParameterUnit_Generic, false, NULL},
        { filterAttackDuration,  0.0005, 0.05, 2, "filterAttackDuration", "filterAttackDuration", kAudioUnitParameterUnit_Seconds, true, NULL},
        { filterDecayDuration,   0.005, 0.05, 2, "filterDecayDuration", "filterDecayDuration", kAudioUnitParameterUnit_Seconds, true, NULL},
        { filterSustainLevel,    0, 1, 1, "filterSustainLevel", "filterSustainLevel", kAudioUnitParameterUnit_Generic, true, NULL},
        { filterReleaseDuration, 0, 0.5, 2, "filterReleaseDuration", "filterReleaseDuration", kAudioUnitParameterUnit_Seconds, true, NULL},
        { attackDuration,        0.0005, 0.05, 2, "attackDuration", "attackDuration", kAudioUnitParameterUnit_Seconds, true, NULL},
        { decayDuration,         0, 0.005, 2, "decayDuration", "decayDuration", kAudioUnitParameterUnit_Seconds, true, NULL},
        { sustainLevel,          0, 0.8, 1, "sustainLevel", "sustainLevel", kAudioUnitParameterUnit_Generic, true, NULL},
        { releaseDuration,       0.004, 0.05, 2, "releaseDuration", "releaseDuration", kAudioUnitParameterUnit_Seconds, true, NULL},
        { morph2Detuning,        -4, 0, 4, "morph2Detuning", "morph2Detuning", kAudioUnitParameterUnit_Generic, true, NULL},
        { detuningMultiplier,    1, 1, 2, "detuningMultiplier", "detuningMultiplier", kAudioUnitParameterUnit_Generic, true, NULL},
        { masterVolume,          0, 0.5, 2, "masterVolume", "masterVolume", kAudioUnitParameterUnit_Generic, true, NULL},
        { bitCrushDepth,         1, 24, 24, "bitCrushDepth", "bitCrushDepth", kAudioUnitParameterUnit_Generic, false, NULL},
        { bitCrushSampleRate,    4096, 44100, 48000, "bitCrushSampleRate", "bitCrushSampleRate", kAudioUnitParameterUnit_Hertz, true, NULL},
        { autoPanAmount,         0, 0, 1, "autoPanAmount", "autoPanAmount", kAudioUnitParameterUnit_Generic, true, NULL},
        { autoPanFrequency,      0, 0.25, 10, "autoPanFrequency", "autoPanFrequency", kAudioUnitParameterUnit_Hertz, true, NULL},
        { reverbOn,              0, 1, 1, "reverbOn", "reverbOn", kAudioUnitParameterUnit_Generic, false, NULL},
        { reverbFeedback,        0, 0.5, 1, "reverbFeedback", "reverbFeedback", kAudioUnitParameterUnit_Generic, true, NULL},
        { reverbHighPass,        80, 700, 900, "reverbHighPass", "reverbHighPass", kAudioUnitParameterUnit_Generic, true, NULL},
        { reverbMix,             0, 0, 1, "reverbMix", "reverbMix", kAudioUnitParameterUnit_Generic, true, NULL},
        { delayOn,               0, 0, 1, "delayOn", "delayOn", kAudioUnitParameterUnit_Generic, false, NULL},
        { delayFeedback,         0, 0.1, 0.9, "delayFeedback", "delayFeedback", kAudioUnitParameterUnit_Generic, true, NULL},
        
        { delayTime,             0.1, 0.5, 1.5, "delayTime", "delayTime", kAudioUnitParameterUnit_Seconds, true, NULL},
        
        { delayMix,              0, 0.125, 1, "delayMix", "delayMix", kAudioUnitParameterUnit_Generic, true, NULL},
        { lfo2Index,             0, 0, 3, "lfo2Index", "lfo2Index", kAudioUnitParameterUnit_Generic, false, NULL},
        { lfo2Amplitude,         0.016666, 0, 1, "lfo2Amplitude", "lfo2Amplitude", kAudioUnitParameterUnit_Generic, true, NULL},
        { lfo2Rate,              rate_min, 0.25, rate_max, "lfo2Rate", "lfo2Rate", kAudioUnitParameterUnit_Generic, true, NULL},
        { cutoffLFO,             0, 0, 3, "cutoffLFO", "cutoffLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { resonanceLFO,          0, 0, 3, "resonanceLFO", "resonanceLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { oscMixLFO,             0, 0, 3, "oscMixLFO", "oscMixLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { sustainLFO,            0, 0, 3, "sustainLFO", "sustainLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { decayLFO,              0, 0, 3, "decayLFO", "decayLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { noiseLFO,              0, 0, 3, "noiseLFO", "noiseLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { fmLFO,                 0, 0, 3, "fmLFO", "fmLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { detuneLFO,             0, 0, 3, "detuneLFO", "detuneLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { filterEnvLFO,          0, 0, 3, "filterEnvLFO", "filterEnvLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { pitchLFO,              0, 0, 3, "pitchLFO", "pitchLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { bitcrushLFO,           0, 0, 3, "bitcrushLFO", "bitcrushLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { autopanLFO,            0, 0, 3, "autopanLFO", "autopanLFO", kAudioUnitParameterUnit_Generic, false, NULL},
        { arpDirection,          0, 1, 2, "arpDirection", "arpDirection", kAudioUnitParameterUnit_Generic, false, NULL},
        { arpInterval,           0, 12, 12, "arpInterval", "arpInterval", kAudioUnitParameterUnit_Generic, false, NULL},
        { arpIsOn,               0, 0, 1, "arpIsOn", "arpIsOn", kAudioUnitParameterUnit_Generic, false, NULL},
        { arpOctave,             0, 1, 3, "arpOctave", "arpOctave", kAudioUnitParameterUnit_Generic, false, NULL},
        { arpRate,               bpm_min, 120, bpm_max, "arpRate", "arpRate", kAudioUnitParameterUnit_BPM, false, NULL},
        { arpIsSequencer,        0, 0, 1, "arpIsSequencer", "arpIsSequencer", kAudioUnitParameterUnit_Generic, false, NULL},
        { arpTotalSteps,         1, 4, 16, "arpTotalSteps", "arpTotalSteps" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern00,       -24, 0, 24, "arpSeqPattern00", "arpSeqPattern00" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern01,       -24, 0, 24, "arpSeqPattern01", "arpSeqPattern01" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern02,       -24, 0, 24, "arpSeqPattern02", "arpSeqPattern02" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern03,       -24, 0, 24, "arpSeqPattern03", "arpSeqPattern03" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern04,       -24, 0, 24, "arpSeqPattern04", "arpSeqPattern04" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern05,       -24, 0, 24, "arpSeqPattern05", "arpSeqPattern05" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern06,       -24, 0, 24, "arpSeqPattern06", "arpSeqPattern06" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern07,       -24, 0, 24, "arpSeqPattern07", "arpSeqPattern07" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern08,       -24, 0, 24, "arpSeqPattern08", "arpSeqPattern08" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern09,       -24, 0, 24, "arpSeqPattern09", "arpSeqPattern09" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern10,       -24, 0, 24, "arpSeqPattern10", "arpSeqPattern10" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern11,       -24, 0, 24, "arpSeqPattern11", "arpSeqPattern11" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern12,       -24, 0, 24, "arpSeqPattern12", "arpSeqPattern12" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern13,       -24, 0, 24, "arpSeqPattern13", "arpSeqPattern13" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern14,       -24, 0, 24, "arpSeqPattern14", "arpSeqPattern14" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqPattern15,       -24, 0, 24, "arpSeqPattern15", "arpSeqPattern15" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost00,      0, 0, 1, "arpSeqOctBoost00", "arpSeqOctBoost00" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost01,      0, 0, 1, "arpSeqOctBoost01", "arpSeqOctBoost01" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost02,      0, 0, 1, "arpSeqOctBoost02", "arpSeqOctBoost02" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost03,      0, 0, 1, "arpSeqOctBoost03", "arpSeqOctBoost03" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost04,      0, 0, 1, "arpSeqOctBoost04", "arpSeqOctBoost04" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost05,      0, 0, 1, "arpSeqOctBoost05", "arpSeqOctBoost05" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost06,      0, 0, 1, "arpSeqOctBoost06", "arpSeqOctBoost06" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost07,      0, 0, 1, "arpSeqOctBoost07", "arpSeqOctBoost07" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost08,      0, 0, 1, "arpSeqOctBoost08", "arpSeqOctBoost08" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost09,      0, 0, 1, "arpSeqOctBoost09", "arpSeqOctBoost09" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost10,      0, 0, 1, "arpSeqOctBoost10", "arpSeqOctBoost10" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost11,      0, 0, 1, "arpSeqOctBoost11", "arpSeqOctBoost11" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost12,      0, 0, 1, "arpSeqOctBoost12", "arpSeqOctBoost12" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost13,      0, 0, 1, "arpSeqOctBoost13", "arpSeqOctBoost13" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost14,      0, 0, 1, "arpSeqOctBoost14", "arpSeqOctBoost14" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqOctBoost15,      0, 0, 1, "arpSeqOctBoost15", "arpSeqOctBoost15" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn00,        0, 0, 1, "arpSeqNoteOn00", "arpSeqNoteOn00" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn01,        0, 0, 1, "arpSeqNoteOn01", "arpSeqNoteOn01" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn02,        0, 0, 1, "arpSeqNoteOn02", "arpSeqNoteOn02" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn03,        0, 0, 1, "arpSeqNoteOn03", "arpSeqNoteOn03" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn04,        0, 0, 1, "arpSeqNoteOn04", "arpSeqNoteOn04" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn05,        0, 0, 1, "arpSeqNoteOn05", "arpSeqNoteOn05" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn06,        0, 0, 1, "arpSeqNoteOn06", "arpSeqNoteOn06" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn07,        0, 0, 1, "arpSeqNoteOn07", "arpSeqNoteOn07" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn08,        0, 0, 1, "arpSeqNoteOn08", "arpSeqNoteOn08" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn09,        0, 0, 1, "arpSeqNoteOn09", "arpSeqNoteOn09" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn10,        0, 0, 1, "arpSeqNoteOn10", "arpSeqNoteOn10" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn11,        0, 0, 1, "arpSeqNoteOn11", "arpSeqNoteOn11" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn12,        0, 0, 1, "arpSeqNoteOn12", "arpSeqNoteOn12" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn13,        0, 0, 1, "arpSeqNoteOn13", "arpSeqNoteOn13" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn14,        0, 0, 1, "arpSeqNoteOn14", "arpSeqNoteOn14" , kAudioUnitParameterUnit_Generic, false, NULL},
        { arpSeqNoteOn15,        0, 0, 1, "arpSeqNoteOn15", "arpSeqNoteOn15" , kAudioUnitParameterUnit_Generic, false, NULL},
        { filterType,            0, 0, 2, "filterType", "filterType" , kAudioUnitParameterUnit_Generic, false, NULL},
        { phaserMix,             0, 0, 1, "phaserMix", "phaserMix" , kAudioUnitParameterUnit_Generic, true, NULL},
        { phaserRate,            1, 12, 300, "phaserRate", "phaserRate" , kAudioUnitParameterUnit_Hertz, true, NULL},
        { phaserFeedback,        0, 0.0, 0.8, "phaserFeedback", "phaserFeedback" , kAudioUnitParameterUnit_Generic, true, NULL},
        { phaserNotchWidth,      100, 800, 1000, "phaserNotchWidth", "phaserNotchWidth" , kAudioUnitParameterUnit_Hertz, true, NULL},
        { monoIsLegato,          0, 0, 1, "monoIsLegato", "monoIsLegato" , kAudioUnitParameterUnit_Generic, false, NULL}
    };
};
#endif