//
//  AKSynthOne.swift
//  AudioKit
//
//  Created by Aurelius Prochazka, revision history on Github.
//  Copyright © 2017 Aurelius Prochazka. All rights reserved.
//

import Foundation
import AudioKit

@objc open class AKSynthOne: AKPolyphonicNode, AKComponent, AKSynthOneProtocol {
    
    public typealias AKAudioUnitType = AKSynthOneAudioUnit
    
    /// Four letter unique description of the node
    public static let ComponentDescription = AudioComponentDescription(instrument: "aks1")

    // MARK: - Properties

    public var internalAU: AKAudioUnitType?
    public var token: AUParameterObserverToken?

    fileprivate var waveformArray = [AKTable]()
    fileprivate var auParameters: [AUParameter] = []
    
    ///Hard-reset of DSP...for PANIC
    open func resetDSP() {
        internalAU?.resetDSP()
    }
    
    open func resetSequencer() {
        internalAU?.resetSequencer()
    }
    
    ///Puts all playing notes into release mode.
    open func stopAllNotes() {
        internalAU?.stopAllNotes()
    }
    
    ///These getter/setters are more efficient than using "parameter[i]"
    open func setAK1Parameter(_ param: AKSynthOneParameter, _ value : Double) {
        internalAU?.setAK1Parameter(param, value: Float(value))
    }
    
    open func getAK1Parameter(_ param: AKSynthOneParameter) -> Double {
        return Double(internalAU?.getAK1Parameter(param) ?? 0)
    }
    
    open func getParameterMin(_ param: AKSynthOneParameter) -> Double {
        return Double(internalAU?.getParameterMin(param) ?? 0)
    }

    open func getParameterMax(_ param: AKSynthOneParameter) -> Double {
        return Double(internalAU?.getParameterMax(param) ?? 1)
    }

    open func getParameterRange(_ param: AKSynthOneParameter) -> ClosedRange<Double> {
        let min = Double(internalAU?.getParameterMin(param) ?? 0)
        let max = Double(internalAU?.getParameterMax(param) ?? 1)
        return min ... max
    }
    
    open func getParameterDefault(_ param: AKSynthOneParameter) -> Double {
        return Double(internalAU?.getParameterDefault(param) ?? 0)
    }

    open func getAK1ArpSeqPattern(forIndex inputIndex : Int) -> Int {
        let index = (0...15).clamp(inputIndex)
        let aspi = Int32(Int(AKSynthOneParameter.arpSeqPattern00.rawValue) + index)
        let aspp = AKSynthOneParameter(rawValue: aspi)!
        return Int( getAK1Parameter(aspp) )
    }
    
    open func setAK1ArpSeqPattern(forIndex inputIndex : Int, _ value: Int) {
        let index = Int32((0...15).clamp(inputIndex))
        let aspi = Int32(AKSynthOneParameter.arpSeqPattern00.rawValue + index)
        let aspp = AKSynthOneParameter(rawValue: aspi)!
        internalAU?.setAK1Parameter(aspp, value: Float(value) )
    }
    
    open func getAK1SeqOctBoost(forIndex inputIndex : Int) -> Double {
        let index = (0...15).clamp(inputIndex)
        let asni = Int32(Int(AKSynthOneParameter.arpSeqOctBoost00.rawValue) + index)
        let asnp = AKSynthOneParameter(rawValue: asni)!
        return getAK1Parameter(asnp)
    }
    
    open func setAK1SeqOctBoost(forIndex inputIndex : Int, _ value: Double) {
        let index = Int32((0...15).clamp(inputIndex))
        let aspi = Int32(AKSynthOneParameter.arpSeqOctBoost00.rawValue + index)
        let aspp = AKSynthOneParameter(rawValue: aspi)!
        internalAU?.setAK1Parameter(aspp, value: Float(value) )
    }
    
    open func getAK1ArpSeqNoteOn(forIndex inputIndex : Int) -> Bool {
        let index = (0...15).clamp(inputIndex)
        let asoi = Int32(Int(AKSynthOneParameter.arpSeqNoteOn00.rawValue) + index)
        let asop = AKSynthOneParameter(rawValue: asoi)!
        return ( getAK1Parameter(asop) > 0 ) ? true : false
    }
    
    open func setAK1ArpSeqNoteOn(forIndex inputIndex : Int, _ value: Bool) {
        let index = Int32((0...15).clamp(inputIndex))
        let aspi = Int32(AKSynthOneParameter.arpSeqNoteOn00.rawValue + index)
        let aspp = AKSynthOneParameter(rawValue: aspi)!
        internalAU?.setAK1Parameter(aspp, value: Float(value == true ? 1 : 0) )
    }

    /// "parameter[i]" syntax is inefficient...use getter/setters above
    open var parameters: [Double] {
        get {
            var result: [Double] = []
            if let floatParameters = internalAU?.parameters as? [NSNumber] {
                for number in floatParameters {
                    result.append(number.doubleValue)
                }
            }
            return result
        }
        set {
            internalAU?.parameters = newValue
            
            if internalAU?.isSetUp ?? false {
                if let existingToken = token {
                    for (index, parameter) in auParameters.enumerated() {
                        if Double(parameter.value) != newValue[index] {
                            parameter.setValue( Float( newValue[index]), originator: existingToken)
                        }
                    }
                }
            } else {
                internalAU?.parameters = newValue
            }
        }
    }

    /// Ramp Time represents the speed at which parameters are allowed to change
    @objc open dynamic var rampTime: Double = 0.0 {
        willSet {
            internalAU?.rampTime = newValue
        }
    }

    // MARK: - Initialization
    
    /// Initialize the synth with defaults
    public convenience override init() {
        let squareWithHighPWM = AKTable()
        let size = squareWithHighPWM.count
        for i in 0..<size {
            if i < size / 8 {
                squareWithHighPWM[i] = -1.0
            } else {
                squareWithHighPWM[i] = 1.0
            }
        }
        self.init(waveformArray: [AKTable(.triangle), AKTable(.square), squareWithHighPWM, AKTable(.sawtooth)])
    }

    /// Initialize this synth
    ///
    /// - Parameters:
    ///   - waveformArray:      An array of 4 waveforms
    ///
    public init(waveformArray: [AKTable]) {
        
        self.waveformArray = waveformArray
        _Self.register()

        super.init()
        AVAudioUnit._instantiate(with: _Self.ComponentDescription) { [weak self] avAudioUnit in

            self?.avAudioNode = avAudioUnit
            self?.midiInstrument = avAudioUnit as? AVAudioUnitMIDIInstrument
            self?.internalAU = avAudioUnit.auAudioUnit as? AKAudioUnitType
            
            for (i, waveform) in waveformArray.enumerated() {
                self?.internalAU?.setupWaveform(UInt32(i), size: Int32(UInt32(waveform.count)))
                for (j, sample) in waveform.enumerated() {
                    self?.internalAU?.setWaveform(UInt32(i), withValue: sample, at: UInt32(j))
                }
            }
            self?.internalAU?.parameters = self?.parameters
            self?.internalAU?.aks1Delegate = self
        }

        guard let tree = internalAU?.parameterTree else {
            return
        }
        auParameters = tree.allParameters

        token = tree.token(byAddingParameterObserver: { address, value in
            guard let param: AKSynthOneParameter = AKSynthOneParameter(rawValue: Int32(address)) else {
                return
            }
            self.notifyDelegateOfParamChange(param, Double(value) )
        })
        
        internalAU?.aks1Delegate = self
    }

    @objc open var delegate: AKSynthOneProtocol?
    
    internal func notifyDelegateOfParamChange(_ param: AKSynthOneParameter, _ value: Double) {
        delegate?.paramDidChange(param, value: value)
    }
    
    /// stops all notes
    open func reset() {
        internalAU?.reset()
    }

    // MARK: - AKPolyphonic

    // Function to start, play, or activate the node at frequency
    open override func play(noteNumber: MIDINoteNumber, velocity: MIDIVelocity, frequency: Double) {
        internalAU?.startNote(noteNumber, velocity: velocity, frequency: Float(frequency))
    }

    /// Function to stop or bypass the node, both are equivalent
    open override func stop(noteNumber: MIDINoteNumber) {
        internalAU?.stopNote(noteNumber)
    }
    
    //MARK: - AKSynthOneProtocol passthroughs
    @objc public func paramDidChange(_ param: AKSynthOneParameter, value: Double) {
        delegate?.paramDidChange(param, value: value)
    }
    
    @objc public func arpBeatCounterDidChange(_ beat: Int) {
        delegate?.arpBeatCounterDidChange(beat)
    }
    
    @objc public func heldNotesDidChange() {
        delegate?.heldNotesDidChange()
    }
    
    @objc public func playingNotesDidChange() {
        delegate?.playingNotesDidChange()
    }

}