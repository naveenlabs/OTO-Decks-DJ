/*
  ==============================================================================
    WaveformDisplay.cpp
  ==============================================================================
    This file implements the WaveformDisplay component, which renders an audio
    waveform with a glowing gradient and a movable playhead. It loads audio files,
    updates the waveform when changes occur, and handles mouse events to enable
    interactive seeking.
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "WaveformDisplay.h"

//==============================================================================
// Constructors and Destructor
//==============================================================================

// Constructor: initializes the audio thumbnail and sets initial state.
WaveformDisplay::WaveformDisplay(juce::AudioFormatManager & formatManagerToUse,
                                 juce::AudioThumbnailCache & cacheToUse)
    : audioThumb(1000, formatManagerToUse, cacheToUse), fileLoaded(false), position(0.0)
{
    audioThumb.addChangeListener(this);
    setMouseClickGrabsKeyboardFocus(false);
}

// Destructor.
WaveformDisplay::~WaveformDisplay()
{
}

//==============================================================================
// Painting Functions
//==============================================================================

// -- Main Paint --
// Paints background, border, and either the waveform/playhead or a "file not loaded" message.
void WaveformDisplay::paint(juce::Graphics& g)
{
    // Draw background gradient.
    juce::ColourGradient backgroundGradient(juce::Colour::fromRGB(10, 10, 50),
                                              getWidth() / 2.0f, 0.0f,
                                              juce::Colour::fromRGB(5, 5, 20),
                                              getWidth() / 2.0f, (float)getHeight(),
                                              false);
    g.setGradientFill(backgroundGradient);
    g.fillAll();

    // Draw border.
    g.setColour(juce::Colour::fromRGB(192, 192, 192));
    g.drawRect(getLocalBounds(), 1);

    if (fileLoaded)
    {
        paintWaveform(g);
        paintPlayhead(g);
    }
    else
    {
        // Display message if no file is loaded.
        g.setColour(juce::Colour::fromRGB(192, 192, 192));
        g.setFont(20.0f);
        g.drawText("File not loaded...", getLocalBounds(), juce::Justification::centred, true);
    }
}

// -- Waveform Painting --
// Draws the waveform with a glow effect and a main gradient.
void WaveformDisplay::paintWaveform(juce::Graphics& g)
{
    auto mainBounds = getLocalBounds().toFloat();
    {
        juce::Colour topColour = juce::Colour::fromRGB(200, 80, 255);
        juce::Colour bottomColour = juce::Colour::fromRGB(70, 150, 255);
        auto glowBounds = mainBounds.expanded(8.0f);

        // Draw glow effect.
        juce::ColourGradient glowGrad(topColour.withAlpha(0.4f),
                                      glowBounds.getCentreX(), glowBounds.getY(),
                                      bottomColour.withAlpha(0.4f),
                                      glowBounds.getCentreX(), glowBounds.getBottom(),
                                      false);
        g.saveState();
        g.setGradientFill(glowGrad);
        audioThumb.drawChannel(g, glowBounds.toNearestInt(), 0.0, audioThumb.getTotalLength(), 0, 1.0f);
        g.restoreState();

        // Draw main waveform.
        juce::ColourGradient mainGrad(topColour,
                                      mainBounds.getCentreX(), mainBounds.getY(),
                                      bottomColour,
                                      mainBounds.getCentreX(), mainBounds.getBottom(),
                                      false);
        g.setGradientFill(mainGrad);
        audioThumb.drawChannel(g, mainBounds.toNearestInt(), 0.0, audioThumb.getTotalLength(), 0, 1.0f);
    }
}

// -- Playhead Painting --
// Draws the playhead with a glowing gradient and a solid line.
void WaveformDisplay::paintPlayhead(juce::Graphics& g)
{
    float playheadX = (float)(position * (double)getWidth());
    {
        g.saveState();
        juce::ColourGradient glowLineGradient(juce::Colour::fromRGB(255, 140, 0).withAlpha(0.3f),
                                                playheadX, 0.0f,
                                                juce::Colour::fromRGB(255, 215, 0).withAlpha(0.3f),
                                                playheadX, (float)getHeight(),
                                                false);
        g.setGradientFill(glowLineGradient);
        g.drawLine(playheadX, -5.0f, playheadX, (float)getHeight() + 5.0f, 10.0f);
        g.restoreState();
    }
    {
        juce::ColourGradient mainLineGradient(juce::Colour::fromRGB(255, 140, 0),
                                              playheadX, 0.0f,
                                              juce::Colour::fromRGB(255, 215, 0),
                                              playheadX, (float)getHeight(),
                                              false);
        g.setGradientFill(mainLineGradient);
        g.drawLine(playheadX, 0.0f, playheadX, (float)getHeight(), 4.0f);
    }
}

//==============================================================================
// Resizing and Change Listener
//==============================================================================

// No special layout changes needed on resize.
void WaveformDisplay::resized()
{
}

// When the audio thumbnail changes, repaint the component.
void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    repaint();
}

//==============================================================================
// File Loading and Position Management
//==============================================================================

// Loads an audio file and updates the waveform display.
void WaveformDisplay::loadURL(juce::URL audioURL)
{
    audioThumb.clear();
    fileLoaded = audioThumb.setSource(new juce::URLInputSource(audioURL));
    if (fileLoaded)
    {
        std::cout << "WaveformDisplay: loaded!" << std::endl;
        repaint();
    }
    else
    {
        std::cout << "WaveformDisplay: not loaded!" << std::endl;
    }
}

// Sets the playhead position (0.0 to 1.0) and repaints if changed.
void WaveformDisplay::setPositionRelative(double pos)
{
    if (pos != position)
    {
        position = pos;
        repaint();
    }
}

//==============================================================================
// Mouse Event Handling
//==============================================================================

// Updates playhead position based on mouse event and triggers onSeek callback.
void WaveformDisplay::updatePositionFromMouseEvent(const juce::MouseEvent& event)
{
    double newPos = event.x / (double)getWidth();
    setPositionRelative(newPos);
    if (onSeek)
        onSeek(newPos);
}

// On mouse press, update the playhead position.
void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    if (fileLoaded)
        updatePositionFromMouseEvent(event);
}

// On mouse drag, update the playhead position.
void WaveformDisplay::mouseDrag(const juce::MouseEvent& event)
{
    if (fileLoaded)
        updatePositionFromMouseEvent(event);
}

// On mouse release, update the playhead and trigger onSeekEnd callback.
void WaveformDisplay::mouseUp(const juce::MouseEvent& event)
{
    if (fileLoaded)
    {
        updatePositionFromMouseEvent(event);
        if (onSeekEnd)
            onSeekEnd();
    }
}
