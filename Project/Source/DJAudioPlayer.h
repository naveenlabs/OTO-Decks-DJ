/*
  ==============================================================================
    DJAudioPlayer.h
  ==============================================================================
    This header defines the DJAudioPlayer class, which handles audio playback and
    manipulation for DJ applications. It supports file loading, playback control
    (gain, speed, position), EQ adjustments, and special effects such as bass drops,
    kick rolls, and beat skipping. The class also provides RMS level monitoring for
    visual feedback and uses JUCE's audio transport, resampling, and DSP modules to
    process audio in real time.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <juce_dsp/juce_dsp.h>
#include <algorithm>

// DJAudioPlayer: Handles audio playback, EQ, speed, and special DJ effects.
class DJAudioPlayer : public juce::AudioSource
{
public:
    // Public interface: construction, playback control, effects, and EQ.
    DJAudioPlayer(juce::AudioFormatManager& _formatManager);
    ~DJAudioPlayer() override;

    // AudioSource overrides.
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // File loading and playback controls.
    void loadURL(juce::URL audioURL);
    void setGain(double gain);
    void setSpeed(double ratio);
    void setPosition(double posInSecs);
    void setPositionRelative(double pos);
    void start();
    void stop();
    double getPositionRelative() const;

    // Special effects.
    void triggerBassDrop();
    void triggerKickRoll();
    void skipForward(int beats, double bpm);
    void skipBackward(int beats, double bpm);

    // EQ adjustments.
    void setLowGain(float newLowGainDb);
    void setMidGain(float newMidGainDb);
    void setHighGain(float newHighGainDb);

    // RMS level getters.
    float getRmsLevelLeft() const { return currentRmsLeft; }
    float getRmsLevelRight() const { return currentRmsRight; }

    // Sample mode for playing short samples.
    bool sampleMode = false;
    double sampleModeLength = 10.0;

private:
    // Private members: audio sources, EQ chain, and internal playback/effect variables.
    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource { &transportSource, false, 2 };

    juce::dsp::ProcessorChain<
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                       juce::dsp::IIR::Coefficients<float>>,
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                       juce::dsp::IIR::Coefficients<float>>,
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                       juce::dsp::IIR::Coefficients<float>>
    > eqChain;

    float lowGainDb = 0.0f, midGainDb = 0.0f, highGainDb = 0.0f;
    void updateFilters();

    double currentSampleRate = 44100.0;

    // Bass drop effect variables.
    bool   isBassDropActive   = false;
    double bassDropPhase      = 0.0;
    double bassDropElapsed    = 0.0;
    double bassDropDuration   = 4.0;
    double bassDropStartFreq  = 120.0;
    double bassDropEndFreq    = 60.0;

    // Kick roll effect variables.
    bool   isKickRollActive   = false;
    double kickRollPhase      = 0.0;
    double kickRollFrequency  = 80.0;
    double kickRollAmplitude  = 0.0;
    double kickRollAmpDecay   = 0.98;
    int    kickRollRepetitions = 0;
    int    maxKickRollRepetitions = 5;
    int    kickRollGapSamples = 0;
    int    kickRollGapCounter = 0;

    // Current RMS levels.
    float currentRmsLeft  = 0.0f;
    float currentRmsRight = 0.0f;

    // Process special effects.
    void processBassDrop(const juce::AudioSourceChannelInfo& bufferToFill);
    void processKickRoll(const juce::AudioSourceChannelInfo& bufferToFill);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DJAudioPlayer)
};
