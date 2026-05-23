/*
  ==============================================================================
    MainComponent.h
  ==============================================================================
    This header defines the MainComponent class, which serves as the core UI and
    audio processing component of the application. It manages audio playback,
    mixing, and recording; provides user interface elements including buttons,
    sliders, labels, and VU meters; and handles user events such as button clicks,
    slider changes, and timer callbacks. Custom look-and-feel classes are also
    declared here to customize the appearance of buttons and sliders.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "VUMeter.h"

// MainComponent: Core UI and audio processing component, managing playback, UI elements, and event handling.
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Button::Listener,
                       public juce::Slider::Listener,
                       public juce::Timer
{
public:
    // Public interface: construction, audio processing, UI updates, and event handling.
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;
    void sliderValueChanged (juce::Slider* slider) override;
    void timerCallback() override;

private:
    // Audio format and player management.
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbCache { 100 };

    DJAudioPlayer player1 { formatManager };
    DJAudioPlayer player2 { formatManager };

    DJAudioPlayer drumPlayer1 { formatManager };
    DJAudioPlayer drumPlayer2 { formatManager };

    juce::MixerAudioSource mixerSource;

    // UI elements for decks and playlist.
    DeckGUI deckGUI1 { &player1, formatManager, thumbCache, true };
    DeckGUI deckGUI2 { &player2, formatManager, thumbCache, false };
    PlaylistComponent playlistComponent;

    // Playback control buttons.
    juce::TextButton playButton1 { "Play" };
    juce::TextButton stopButton1 { "Stop" };
    juce::TextButton loadButton1 { "Load" };

    juce::TextButton playButton2 { "Play" };
    juce::TextButton stopButton2 { "Stop" };
    juce::TextButton loadButton2 { "Load" };

    // Sliders and labels for volume, speed, position, and EQ.
    juce::Slider volSlider1, speedSlider1, posSlider1;
    juce::Label  volLabel1, speedLabel1, posLabel1;
    juce::Slider eqLowSlider1, eqMidSlider1, eqHighSlider1;
    juce::Label  eqLowLabel1, eqMidLabel1, eqHighLabel1;

    juce::Slider volSlider2, speedSlider2, posSlider2;
    juce::Label  volLabel2, speedLabel2, posLabel2;
    juce::Slider eqLowSlider2, eqMidSlider2, eqHighSlider2;
    juce::Label  eqLowLabel2, eqMidLabel2, eqHighLabel2;

    // VU meters for visualizing audio levels.
    VUMeter vuMeter1;
    VUMeter vuMeter2;

    // File chooser for loading audio files.
    juce::FileChooser fChooser { "Select a file..." };

    // Custom LookAndFeel for buttons.
    class RealisticButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground (juce::Graphics& g,
                                   juce::Button& button,
                                   const juce::Colour& backgroundColour,
                                   bool isMouseOverButton,
                                   bool isButtonDown) override;
    };
    RealisticButtonLookAndFeel realisticButtonLookAndFeel;

    // Custom LookAndFeel for faders and rotary sliders.
    class FancyFaderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider (juce::Graphics& g,
                               int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               const juce::Slider::SliderStyle style,
                               juce::Slider& slider) override;

        void drawRotarySlider (juce::Graphics& g,
                               int x, int y, int width, int height,
                               float sliderPosProportional,
                               float rotaryStartAngle,
                               float rotaryEndAngle,
                               juce::Slider& slider) override;
    };
    FancyFaderLookAndFeel fancyFaderLookAndFeel;

    // Recorder for master output.
    class MasterRecorder;
    std::unique_ptr<MasterRecorder> masterRecorder;

    // UI initialization and layout methods.
    void initializeUI();
    void layoutUIComponents();
    void initializeSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText,
                          double minValue, double maxValue, double interval,
                          juce::Slider::SliderStyle style,
                          juce::Slider::TextEntryBoxPosition textBoxPosition = juce::Slider::TextBoxAbove);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
