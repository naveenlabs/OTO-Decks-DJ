/*
  ==============================================================================
    WaveformDisplay.h
  ==============================================================================
    This header defines the WaveformDisplay component, which displays an audio
    waveform along with a movable playhead. It supports loading an audio file,
    interactive seeking via mouse events, and notifies external listeners about
    seek actions.
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>

// WaveformDisplay: Displays an audio waveform along with a movable playhead.
class WaveformDisplay : public juce::Component, public juce::ChangeListener
{
public:
    // Constructor: Sets up the audio thumbnail and registers a change listener.
    WaveformDisplay(juce::AudioFormatManager & formatManagerToUse,
                    juce::AudioThumbnailCache & cacheToUse);
    
    // Destructor.
    ~WaveformDisplay();

    // Draws the waveform and playhead.
    void paint(juce::Graphics&) override;
    
    // Called when the component is resized.
    void resized() override;
    
    // Called when the audio thumbnail changes.
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Loads an audio file and displays its waveform.
    void loadURL(juce::URL audioURL);
    
    // Sets the playhead position (0.0 to 1.0).
    void setPositionRelative(double pos);

    // Callbacks for seeking.
    std::function<void(double)> onSeek;
    std::function<void()> onSeekEnd;

    // Mouse interactions to update the playhead.
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    // Draws the audio waveform.
    void paintWaveform(juce::Graphics& g);
    
    // Draws the playhead indicator.
    void paintPlayhead(juce::Graphics& g);
    
    // Updates the playhead position based on mouse movement.
    void updatePositionFromMouseEvent(const juce::MouseEvent& event);

    // Audio thumbnail used for drawing the waveform.
    juce::AudioThumbnail audioThumb;
    
    // True if an audio file has been successfully loaded.
    bool fileLoaded;
    
    // Current playhead position (0.0 to 1.0).
    double position;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
