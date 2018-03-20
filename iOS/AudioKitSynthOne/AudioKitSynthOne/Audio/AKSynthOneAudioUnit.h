//
//  AKSynthOneAudioUnit.h
//  AudioKit
//
//  Created by Aurelius Prochazka, revision history on Github.
//  Copyright © 2017 Aurelius Prochazka. All rights reserved.
//

#pragma once

#import <AudioKit/AKAudioUnit.h>
#import "AKSynthOneParameter.h"
#import "AKSynthOneProtocol.h"

@class AEMessageQueue;

@interface AKSynthOneAudioUnit : AKAudioUnit
{
    @public
    AEMessageQueue  *_messageQueue;
}

@property (nonatomic) NSArray *parameters;
@property (nonatomic, weak) id<AKSynthOneProtocol> aks1Delegate;

///auv3, not yet used
- (void)setParameter:(AUParameterAddress)address value:(AUValue)value;
- (AUValue)getParameter:(AUParameterAddress)address;
- (void)createParameters;

- (void)setAK1Parameter:(AKSynthOneParameter)param value:(float)value;
- (float)getAK1Parameter:(AKSynthOneParameter)param;
- (float)getParameterMin:(AKSynthOneParameter)param;
- (float)getParameterMax:(AKSynthOneParameter)param;
- (float)getParameterDefault:(AKSynthOneParameter)param;

- (void)setupWaveform:(UInt32)waveform size:(int)size;
- (void)setWaveform:(UInt32)waveform withValue:(float)value atIndex:(UInt32)index;

- (void)stopNote:(uint8_t)note;
- (void)startNote:(uint8_t)note velocity:(uint8_t)velocity;
- (void)startNote:(uint8_t)note velocity:(uint8_t)velocity frequency:(float)frequency;

- (void)reset;
- (void)stopAllNotes;
- (void)resetDSP;
- (void)resetSequencer;

// protected...called by DSP.  This audiounit will call out to delegate "aks1Delegate"
- (void)paramDidChange:(AKSynthOneParameter)param value:(double)value;
- (void)arpBeatCounterDidChange;
- (void)heldNotesDidChange;
- (void)playingNotesDidChange;

@end
