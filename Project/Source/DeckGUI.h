/*
  ==============================================================================
    DeckGUI.h
  ==============================================================================
    This header defines the DeckGUI component, which provides the graphical user
    interface for controlling an audio deck. It integrates components for waveform
    display, jog wheel-based track seeking, drum pads, and recording controls.
    The component supports file drag-and-drop for track loading and synchronizes
    playhead positions via a timer callback.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "JogWheel.h"
#include "DrumPads.h"

// DeckGUI: A custom GUI component to control an audio deck.
// It displays a waveform, a jog wheel for seeking, drum pads, and recording options.
class DeckGUI : public juce::Component,
                public juce::FileDragAndDropTarget,
                public juce::Timer
{
public:
    // Constructor: sets up the deck controls and optionally enables recording.
    DeckGUI (DJAudioPlayer* player,
             juce::AudioFormatManager& formatManagerToUse,
             juce::AudioThumbnailCache& cacheToUse,
             bool enableRecording = false);

    // Destructor.
    ~DeckGUI() override;

    // Paints the deck GUI.
    void paint (juce::Graphics& g) override;

    // Arranges the sub-components.
    void resized() override;

    // File drag-and-drop support.
    bool isInterestedInFileDrag (const juce::StringArray &files) override;
    void filesDropped (const juce::StringArray &files, int x, int y) override;

    // Timer callback for syncing playhead positions.
    void timerCallback() override;

    // Loads a track into the deck.
    void loadTrack (juce::URL audioURL);

    // Callback for toggling the record state.
    std::function<void(bool isRecording)> onRecordToggled;

    // Returns the drum pads component.
    DrumPads& getDrumPads() { return drumPads; }

private:
    DJAudioPlayer* player;
    WaveformDisplay waveformDisplay;
    JogWheel jogWheel;
    DrumPads drumPads;

    bool enableRecording = false;
    
    // RecordToggleButton: a custom toggle button with a blinking effect.
    class RecordToggleButton : public juce::ToggleButton,
                               private juce::Timer
    {
    public:
        RecordToggleButton();
        void paintButton (juce::Graphics& g,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
        void timerCallback() override;
    private:
        bool blinkState = false;
    };

    RecordToggleButton recordButton;
    bool isUserSeeking = false;

    // Sync the playhead position of the waveform and jog wheel.
    void syncPositions();

    // Layout all components within the deck.
    void layoutComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};
