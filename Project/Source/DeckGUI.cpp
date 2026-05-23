/*
  ==============================================================================
    DeckGUI.cpp
  ==============================================================================
    This file implements the DeckGUI component, which provides the user interface
    for controlling an audio deck in a DJ application. It integrates sub-components
    such as a waveform display, jog wheel for track seeking, drum pads, and an optional
    record toggle button. The file handles component layout, file drag-and-drop,
    synchronization of playhead positions, and custom painting of the deck interface.
*/

#include "DeckGUI.h"

//==============================================================================
// RecordToggleButton Implementation
//==============================================================================

// -- Constructor --
// Sets up the toggle behavior and starts a timer for blinking.
DeckGUI::RecordToggleButton::RecordToggleButton()
{
    setClickingTogglesState(true);
    startTimerHz(2);
}

// -- Custom Paint --
// Draws a circle with a blinking effect when toggled.
void DeckGUI::RecordToggleButton::paintButton(juce::Graphics& g,
                                                bool, bool)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colours::black);
    g.fillEllipse(bounds);
    g.setColour(juce::Colours::whitesmoke);
    g.drawEllipse(bounds, 1.5f);

    if (getToggleState())
    {
        float alpha = (blinkState ? 1.0f : 0.2f);
        g.setColour(juce::Colours::red.withAlpha(alpha));
        float side = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.6f;
        auto square = bounds.withSizeKeepingCentre(side, side);
        g.fillRect(square);
    }
    else
    {
        float margin = bounds.getWidth() * 0.2f;
        auto innerBounds = bounds.reduced(margin);
        g.setColour(juce::Colours::red);
        g.fillEllipse(innerBounds);
    }
}

// -- Timer Callback --
// Toggles blink state and repaints if recording is active.
void DeckGUI::RecordToggleButton::timerCallback()
{
    blinkState = !blinkState;
    if (getToggleState())
        repaint();
}

//==============================================================================
// DeckGUI Implementation
//==============================================================================

// -- Constructor --
// Initializes components, sets callbacks, and adds them to the UI.
DeckGUI::DeckGUI (DJAudioPlayer* _player,
                  juce::AudioFormatManager& formatManagerToUse,
                  juce::AudioThumbnailCache& cacheToUse,
                  bool enableRecording_)
    : player(_player),
      waveformDisplay(formatManagerToUse, cacheToUse),
      enableRecording(enableRecording_)
{
    // Setup waveform seek callback.
    waveformDisplay.onSeek = [this](double newPos)
    {
        isUserSeeking = true;
        player->setPositionRelative(newPos);
        waveformDisplay.setPositionRelative(newPos);
        jogWheel.setPlayheadPosition((float)newPos);
    };
    waveformDisplay.onSeekEnd = [this]()
    {
        isUserSeeking = false;
    };

    // Add main components.
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(jogWheel);
    addAndMakeVisible(drumPads);

    // Setup jog wheel callbacks.
    jogWheel.onJogTurn = [this](float delta)
    {
        isUserSeeking = true;
        double currentPos = player->getPositionRelative();
        double newPos = juce::jlimit(0.0, 1.0, currentPos + delta);
        player->setPositionRelative(newPos);
        waveformDisplay.setPositionRelative(newPos);
    };
    jogWheel.onJogEnd = [this]()
    {
        isUserSeeking = false;
    };

    // If recording is enabled, add the record button.
    if (enableRecording)
    {
        addAndMakeVisible(recordButton);
        recordButton.onClick = [this]()
        {
            bool recordState = recordButton.getToggleState();
            if (onRecordToggled)
                onRecordToggled(recordState);
        };
    }

    // Start timer to periodically sync positions.
    startTimer(500);
}

// -- Destructor --
// Stops the timer.
DeckGUI::~DeckGUI()
{
    stopTimer();
}

// -- Painting --
// Fills background with a gradient and draws a border and title.
void DeckGUI::paint (juce::Graphics& g)
{
    juce::ColourGradient gradient(
        juce::Colour::fromRGB(10, 10, 50), getWidth() / 2.0f, 0.0f,
        juce::Colour::fromRGB(5, 5, 20), getWidth() / 2.0f, (float)getHeight(),
        false
    );
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("DeckGUI", getLocalBounds(), juce::Justification::centred, true);
}

// -- Resizing --
// Calls layoutComponents to arrange sub-components.
void DeckGUI::resized()
{
    layoutComponents();
}

// -- File Drag-and-Drop --
// Accepts any files and loads the first one as a track.
bool DeckGUI::isInterestedInFileDrag (const juce::StringArray& files)
{
    return true;
}

void DeckGUI::filesDropped (const juce::StringArray &files, int /*x*/, int /*y*/)
{
    if (files.size() == 1)
    {
        loadTrack(juce::URL{ juce::File{ files[0] } });
    }
}

// -- Timer Callback --
// Syncs the playhead positions if the user isn't seeking.
void DeckGUI::timerCallback()
{
    if (!isUserSeeking)
        syncPositions();
}

// -- Track Loading and Syncing --

// Loads a track into the player and updates the waveform display.
void DeckGUI::loadTrack (juce::URL audioURL)
{
    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);
}

// Synchronizes the waveform and jog wheel playhead positions with the player.
void DeckGUI::syncPositions()
{
    double pos = player->getPositionRelative();
    waveformDisplay.setPositionRelative(pos);
    jogWheel.setPlayheadPosition((float)pos);
}

// -- Layout --
// Arranges the waveform, jog wheel (with record button if enabled), and drum pads.
void DeckGUI::layoutComponents()
{
    const int totalRows = 12;
    auto area = getLocalBounds();
    int rowH = area.getHeight() / totalRows;

    // Allocate top area for waveform.
    waveformDisplay.setBounds(area.removeFromTop(rowH * 2));

    int gap = 20;
    area.removeFromTop(gap);

    // Allocate area for jog wheel.
    auto jogArea = area.removeFromTop(rowH * 5);

    // Position record button if enabled.
    if (enableRecording)
    {
        int recSize = 40;
        int offset = 10;
        recordButton.setBounds(jogArea.getX() + offset,
                               jogArea.getY() - offset,
                               recSize, recSize);
    }
    jogWheel.setBounds(jogArea);

    // Use remaining area for drum pads.
    drumPads.setBounds(area);
}
