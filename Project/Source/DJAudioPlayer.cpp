/*
  ==============================================================================
    DJAudioPlayer.cpp
  ==============================================================================
    This file implements the DJAudioPlayer class, which handles audio playback,
    EQ processing, and special DJ effects (bass drop, kick roll, and beat skipping).
    It manages file loading, playback control (gain, speed, position), and computes
    RMS levels for visual metering while processing audio in real time.
*/

#include "DJAudioPlayer.h"
#include <cmath>

//==============================================================================
// Constructors and Destructor
//==============================================================================

// -- Constructor --
// Initializes the audio player with the provided format manager.
DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
}

// -- Destructor --
DJAudioPlayer::~DJAudioPlayer()
{
}

//==============================================================================
// Playback Preparation and Audio Processing
//==============================================================================

// -- Preparation --
// Prepares audio sources, the EQ chain, and updates filters before playback.
void DJAudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlockExpected);
    spec.numChannels      = 2;

    eqChain.prepare(spec);
    eqChain.reset();

    updateFilters();
}

// -- Audio Block Processing --
// Retrieves and processes the next audio block, applies EQ, calculates RMS levels,
// and processes any active special DJ effects.
void DJAudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // If in sample mode, stop playback after sampleModeLength seconds.
    if (sampleMode)
    {
        double pos = transportSource.getCurrentPosition();
        if (pos >= sampleModeLength)
        {
            transportSource.stop();
            bufferToFill.clearActiveBufferRegion();
            return;
        }
    }
    
    // Get audio block from the resampled source.
    resampleSource.getNextAudioBlock(bufferToFill);

    // Process the EQ chain.
    {
        juce::dsp::AudioBlock<float> audioBlock(*bufferToFill.buffer);
        juce::dsp::ProcessContextReplacing<float> context(audioBlock);
        eqChain.process(context);
    }

    // Calculate RMS levels for VU metering.
    const int numChannels = bufferToFill.buffer->getNumChannels();
    const int numSamples  = bufferToFill.numSamples;
    if (numChannels > 0)
    {
        float sumL = 0.0f;
        auto* leftChan = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        if (numChannels > 1)
        {
            float sumR = 0.0f;
            auto* rightChan = bufferToFill.buffer->getReadPointer(1, bufferToFill.startSample);
            for (int i = 0; i < numSamples; ++i)
            {
                sumL += leftChan[i] * leftChan[i];
                sumR += rightChan[i] * rightChan[i];
            }
            currentRmsLeft  = juce::jmin(std::sqrt(sumL / numSamples), 1.0f);
            currentRmsRight = juce::jmin(std::sqrt(sumR / numSamples), 1.0f);
        }
        else
        {
            for (int i = 0; i < numSamples; ++i)
                sumL += leftChan[i] * leftChan[i];
            currentRmsLeft  = juce::jmin(std::sqrt(sumL / numSamples), 1.0f);
            currentRmsRight = 0.0f;
        }
    }
    else
    {
        currentRmsLeft  = 0.0f;
        currentRmsRight = 0.0f;
    }

    // Process special DJ effects.
    if (isBassDropActive)
        processBassDrop(bufferToFill);

    if (isKickRollActive)
        processKickRoll(bufferToFill);
}

// Releases audio resources.
void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
    eqChain.reset();
}

//==============================================================================
// Playback Control Functions
//==============================================================================

// -- File Loading --
// Loads an audio file from a URL and sets up the reader source.
void DJAudioPlayer::loadURL(juce::URL audioURL)
{
    auto stream = audioURL.createInputStream(false, nullptr, nullptr, {}, 0, nullptr);
    if (stream != nullptr)
    {
        if (auto* reader = formatManager.createReaderFor(std::move(stream)))
        {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource(
                new juce::AudioFormatReaderSource(reader, true)
            );
            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            readerSource.reset(newSource.release());
        }
    }
}

// -- Gain Control --
// Sets the playback gain.
void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0.0 || gain > 1.0)
        DBG("DJAudioPlayer::setGain out of [0..1] range!");
    else
        transportSource.setGain(gain);
}

// -- Speed Control --
// Sets the playback speed.
void DJAudioPlayer::setSpeed(double ratio)
{
    if (ratio < 0.01 || ratio > 100.0)
        DBG("DJAudioPlayer::setSpeed out of [0..100] range!");
    else
        resampleSource.setResamplingRatio(ratio);
}

// -- Position Control --
// Sets the playback position in seconds.
void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

// Sets the playback position relative to the track length.
void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0.0 || pos > 1.0)
        DBG("DJAudioPlayer::setPositionRelative out of [0..1] range!");
    else
    {
        double length = transportSource.getLengthInSeconds();
        double posInSecs = length * pos;
        setPosition(posInSecs);
    }
}

// -- Playback Start/Stop --
// Starts playback.
void DJAudioPlayer::start()
{
    transportSource.start();
}

// Stops playback.
void DJAudioPlayer::stop()
{
    transportSource.stop();
}

// Returns the current playback position as a fraction of track length.
double DJAudioPlayer::getPositionRelative() const
{
    double dur = transportSource.getLengthInSeconds();
    if (dur > 0.0)
        return transportSource.getCurrentPosition() / dur;
    else
        return 0.0;
}

//==============================================================================
// EQ and Filter Adjustments
//==============================================================================

// Sets the low EQ gain and updates filters.
void DJAudioPlayer::setLowGain(float newLowGainDb)
{
    lowGainDb = newLowGainDb;
    updateFilters();
}

// Sets the mid EQ gain and updates filters.
void DJAudioPlayer::setMidGain(float newMidGainDb)
{
    midGainDb = newMidGainDb;
    updateFilters();
}

// Sets the high EQ gain and updates filters.
void DJAudioPlayer::setHighGain(float newHighGainDb)
{
    highGainDb = newHighGainDb;
    updateFilters();
}

// Updates the EQ filters using the current gain settings.
void DJAudioPlayer::updateFilters()
{
    if (currentSampleRate <= 0.0)
        return;

    {
        float frequency = 100.0f;
        float Q         = 0.7f;
        float lowLinear = juce::Decibels::decibelsToGain(lowGainDb);
        auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, frequency, Q, lowLinear);
        *eqChain.get<0>().state = *lowCoeffs;
    }

    {
        float frequency = 1000.0f;
        float Q         = 1.0f;
        float midLinear = juce::Decibels::decibelsToGain(midGainDb);
        auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, frequency, Q, midLinear);
        *eqChain.get<1>().state = *midCoeffs;
    }

    {
        float frequency = 8000.0f;
        float Q         = 0.7f;
        float highLinear = juce::Decibels::decibelsToGain(highGainDb);
        auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, frequency, Q, highLinear);
        *eqChain.get<2>().state = *highCoeffs;
    }
}

//==============================================================================
// Special Effects: Triggering Effects
//==============================================================================

// Activates the bass drop effect.
void DJAudioPlayer::triggerBassDrop()
{
    isBassDropActive = true;
    bassDropPhase    = 0.0;
    bassDropElapsed  = 0.0;
    bassDropDuration = 4.0;
    bassDropStartFreq = 120.0;
    bassDropEndFreq   = 60.0;
}

// Activates the kick roll effect.
void DJAudioPlayer::triggerKickRoll()
{
    isKickRollActive  = true;
    kickRollPhase     = 0.0;
    kickRollFrequency = 80.0;
    kickRollAmplitude = 10.0;
    kickRollAmpDecay  = 0.98;
    kickRollRepetitions = 0;
    maxKickRollRepetitions = 7;
    kickRollGapSamples = static_cast<int>(0.1 * currentSampleRate);
    kickRollGapCounter = 0;
}

//==============================================================================
// Skipping Functions
//==============================================================================

// Skips forward a given number of beats based on BPM.
void DJAudioPlayer::skipForward(int beats, double bpm)
{
    double offset = beats * (60.0 / bpm);
    double newPos = transportSource.getCurrentPosition() + offset;
    double trackLength = transportSource.getLengthInSeconds();
    if (newPos > trackLength)
        newPos = trackLength;
    setPosition(newPos);
}

// Skips backward a given number of beats based on BPM.
void DJAudioPlayer::skipBackward(int beats, double bpm)
{
    double offset = beats * (60.0 / bpm);
    double newPos = transportSource.getCurrentPosition() - offset;
    if (newPos < 0.0)
        newPos = 0.0;
    setPosition(newPos);
}

//==============================================================================
// Processing Special Effects
//==============================================================================

// Processes and applies the bass drop effect to the audio block.
void DJAudioPlayer::processBassDrop(const juce::AudioSourceChannelInfo& bufferToFill)
{
    const int numChannels = bufferToFill.buffer->getNumChannels();
    const int numSamples  = bufferToFill.numSamples;
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* writePtr = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        for (int i = 0; i < numSamples; ++i)
        {
            double dt = 1.0 / currentSampleRate;
            bassDropElapsed += dt;
            double t = bassDropElapsed / bassDropDuration;
            if (t > 1.0) t = 1.0;
            double currentFreq = bassDropStartFreq * (1.0 - t) + bassDropEndFreq * t;
            double currentAmp  = 1.0 - t;
            float sampleValue = static_cast<float>(currentAmp * std::sin(bassDropPhase));
            writePtr[i] += sampleValue;
            bassDropPhase += (2.0 * juce::MathConstants<double>::pi * currentFreq) / currentSampleRate;
            if (bassDropPhase > 2.0 * juce::MathConstants<double>::pi)
                bassDropPhase -= 2.0 * juce::MathConstants<double>::pi;
        }
    }
    if (bassDropElapsed >= bassDropDuration)
        isBassDropActive = false;
}

// Processes and applies the kick roll effect to the audio block.
void DJAudioPlayer::processKickRoll(const juce::AudioSourceChannelInfo& bufferToFill)
{
    const int numChannels = bufferToFill.buffer->getNumChannels();
    const int numSamples  = bufferToFill.numSamples;
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* writePtr = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        for (int i = 0; i < numSamples; ++i)
        {
            if (kickRollGapCounter > 0)
            {
                kickRollGapCounter--;
                continue;
            }
            float sampleValue = static_cast<float>(kickRollAmplitude * std::sin(kickRollPhase));
            writePtr[i] += sampleValue;
            kickRollPhase += (2.0 * juce::MathConstants<double>::pi * kickRollFrequency) / currentSampleRate;
            if (kickRollPhase > 2.0 * juce::MathConstants<double>::pi)
                kickRollPhase -= 2.0 * juce::MathConstants<double>::pi;
            kickRollAmplitude *= kickRollAmpDecay;
            if (kickRollAmplitude < 0.01)
            {
                kickRollRepetitions++;
                if (kickRollRepetitions >= maxKickRollRepetitions)
                {
                    isKickRollActive = false;
                    break;
                }
                else
                {
                    kickRollPhase = 0.0;
                    kickRollFrequency = 80.0;
                    kickRollAmplitude = 2.0;
                    kickRollGapCounter = kickRollGapSamples;
                }
            }
        }
    }
}
