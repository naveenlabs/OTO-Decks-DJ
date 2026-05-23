/*
  ==============================================================================
    DrumPads.h
  ==============================================================================
    This header defines the DrumPads component, which provides a set of custom
    drum pad buttons for triggering DJ effects. It supports callbacks for actions
    like vocal playback, bass drop, kick roll, skip backward, and skip forward.
    Additionally, it manages loading vocal samples via a file chooser and handles
    double-click events on the pads.
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <array>
#include <memory>

// DrumPads: A component that contains drum pad buttons to trigger DJ effects.
class DrumPads : public juce::Component
{
public:
    // Constructor: Initializes the drum pad buttons and sets up callbacks.
    DrumPads();

    // Destructor.
    ~DrumPads() override;

    // Resizes and lays out the drum pad buttons.
    void resized() override;

    // Callbacks for various drum pad actions.
    std::function<void()> onVocal1Triggered;
    std::function<void()> onBassDropTriggered;
    std::function<void()> onKickRollTriggered;
    std::function<void()> onSkipBackwardTriggered;
    std::function<void()> onSkipForwardTriggered;
    std::function<void()> onVocal2Triggered;

    // Get the URL for the vocal samples.
    juce::URL getVocal1URL() const { return vocal1URL; }
    juce::URL getVocal2URL() const { return vocal2URL; }

    // Handle a double-click on a drum pad.
    void handleDrumPadDoubleClick(const juce::String& padName);

private:
    // DrumPadButton: A custom button with gradient and neon effects.
    class DrumPadButton : public juce::TextButton
    {
    public:
        // Constructor: Sets the button's name and colors.
        DrumPadButton(const juce::String& name, juce::Colour color1, juce::Colour color2, juce::Colour neon);

        // Custom painting of the button.
        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
        
        // Handle double-click events on the button.
        void mouseDoubleClick(const juce::MouseEvent& event) override;
    private:
        juce::Colour gradColor1, gradColor2, neonColor;
    };

    // Opens a file chooser to load an audio sample.
    void launchFileChooser(const juce::String& prompt, std::function<void(const juce::URL&)> onSampleLoaded);

    // Holds the drum pad buttons.
    std::array<std::unique_ptr<DrumPadButton>, 6> drumPadButtons;
    // Stores URLs for the vocal samples.
    juce::URL vocal1URL;
    juce::URL vocal2URL;
    // File chooser used for selecting audio samples.
    std::unique_ptr<juce::FileChooser> padFileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumPads)
};
