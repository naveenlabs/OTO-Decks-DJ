/*
  ==============================================================================
    DrumPads.cpp
  ==============================================================================
    This file implements the DrumPads component, which provides a set of custom
    drum pad buttons for triggering DJ effects. It manages loading and reloading
    of vocal samples via a file chooser, and defines callbacks for actions such
    as vocal playback, bass drop, kick roll, skip backward, and skip forward.
    The component lays out the drum pad buttons in a grid and applies custom
    neon glow and gradient effects during painting.
*/

#include "DrumPads.h"

//==============================================================================
// Helper Functions
//==============================================================================

// Check if the file is a supported audio file.
static bool isAudioFile(const juce::File& file)
{
    return file.hasFileExtension(".wav") || file.hasFileExtension(".mp3") ||
           file.hasFileExtension(".aiff") || file.hasFileExtension(".flac") ||
           file.hasFileExtension(".ogg");
}

//==============================================================================
// DrumPadButton Implementation
//==============================================================================

// -- Constructor --
DrumPads::DrumPadButton::DrumPadButton(const juce::String& name, juce::Colour color1, juce::Colour color2, juce::Colour neon)
    : juce::TextButton(name), gradColor1(color1), gradColor2(color2), neonColor(neon)
{
    setButtonText("");
}

// -- Custom Paint --
// Draw neon glow and gradient fill.
void DrumPads::DrumPadButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw neon glow effect.
    for (int i = 5; i >= 1; --i)
    {
        auto glowBounds = bounds.expanded((float)i);
        g.setColour(neonColor.withAlpha(0.1f * i));
        g.drawRect(glowBounds, 2.0f * i);
    }
    
    // Adjust colors if the button is pressed.
    juce::Colour c1 = gradColor1;
    juce::Colour c2 = gradColor2;
    if (shouldDrawButtonAsDown)
    {
        c1 = c1.darker(0.2f);
        c2 = c2.darker(0.2f);
    }
    
    // Create and fill with a gradient.
    juce::ColourGradient grad(c1, bounds.getCentreX(), bounds.getY(),
                              c2, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRect(bounds);
}

// -- Mouse Double-Click --
// On double-click, if the pad is "Pad1" or "Pad6", reload the corresponding sample.
void DrumPads::DrumPadButton::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (getName() == "Pad1" || getName() == "Pad6")
    {
        if (auto* parent = findParentComponentOfClass<DrumPads>())
            parent->handleDrumPadDoubleClick(getName());
    }
    juce::TextButton::mouseDoubleClick(event);
}

//==============================================================================
// DrumPads Implementation
//==============================================================================

// -- File Chooser --
void DrumPads::launchFileChooser(const juce::String& prompt, std::function<void(const juce::URL&)> onSampleLoaded)
{
    padFileChooser = std::make_unique<juce::FileChooser>(prompt,
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        "*");
    
    padFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, prompt, onSampleLoaded](const juce::FileChooser& fc)
        {
            juce::File file = fc.getResult();
            if (file.existsAsFile())
            {
                if (isAudioFile(file))
                {
                    DBG("File selected for " + prompt + ": " + file.getFullPathName());
                    onSampleLoaded(juce::URL(file));
                }
                else
                {
                    DBG("Error: File " + file.getFileName() + " is not a valid audio format. " + prompt);
                }
            }
            else
            {
                DBG("Error: No valid file selected for " + prompt);
            }
            padFileChooser.reset();
        });
}

// -- Constructor --
DrumPads::DrumPads()
{
    drumPadButtons[0] = std::make_unique<DrumPadButton>("Pad1",
        juce::Colour::fromRGB(255, 140, 0),
        juce::Colour::fromRGB(255, 215, 0),
        juce::Colour::fromRGB(255, 140, 0));
    drumPadButtons[1] = std::make_unique<DrumPadButton>("Pad2",
        juce::Colour::fromRGB(255, 20, 147),
        juce::Colour::fromRGB(255, 105, 180),
        juce::Colour::fromRGB(255, 20, 147));
    drumPadButtons[2] = std::make_unique<DrumPadButton>("Pad3",
        juce::Colour::fromRGB(50, 205, 50),
        juce::Colour::fromRGB(0, 255, 127),
        juce::Colour::fromRGB(50, 205, 50));
    drumPadButtons[3] = std::make_unique<DrumPadButton>("Pad4",
        juce::Colour::fromRGB(30, 144, 255),
        juce::Colour::fromRGB(135, 206, 250),
        juce::Colour::fromRGB(30, 144, 255));
    drumPadButtons[4] = std::make_unique<DrumPadButton>("Pad5",
        juce::Colour::fromRGB(138, 43, 226),
        juce::Colour::fromRGB(147, 112, 219),
        juce::Colour::fromRGB(138, 43, 226));
    drumPadButtons[5] = std::make_unique<DrumPadButton>("Pad6",
        juce::Colour::fromRGB(255, 69, 0),
        juce::Colour::fromRGB(255, 99, 71),
        juce::Colour::fromRGB(255, 69, 0));

    // Make buttons visible.
    for (auto& btn : drumPadButtons)
        addAndMakeVisible(btn.get());

    // Set up onClick actions for each pad.
    drumPadButtons[0]->onClick = [this]
    {
        DBG("DrumPad #1 clicked");
        if (vocal1URL.isEmpty())
            launchFileChooser("Select a track for Vocal #1", [this](const juce::URL& url)
            {
                vocal1URL = url;
                DBG("Loaded Vocal1 sample: " + vocal1URL.toString(true));
            });
        else
        {
            if (onVocal1Triggered)
            {
                DBG("Triggering Vocal1 sample playback");
                onVocal1Triggered();
            }
        }
    };

    drumPadButtons[1]->onClick = [this]
    {
        DBG("DrumPad #2 clicked (Bass Drop)");
        if (onBassDropTriggered)
            onBassDropTriggered();
    };

    drumPadButtons[2]->onClick = [this]
    {
        DBG("DrumPad #3 clicked (Kick Roll)");
        if (onKickRollTriggered)
            onKickRollTriggered();
    };

    drumPadButtons[3]->onClick = [this]
    {
        DBG("DrumPad #4 clicked (Skip Backward)");
        if (onSkipBackwardTriggered)
            onSkipBackwardTriggered();
    };

    drumPadButtons[4]->onClick = [this]
    {
        DBG("DrumPad #5 clicked (Skip Forward)");
        if (onSkipForwardTriggered)
            onSkipForwardTriggered();
    };

    drumPadButtons[5]->onClick = [this]
    {
        DBG("DrumPad #6 clicked");
        if (vocal2URL.isEmpty())
            launchFileChooser("Select a track for Vocal #2", [this](const juce::URL& url)
            {
                vocal2URL = url;
                DBG("Loaded Vocal2 sample: " + vocal2URL.toString(true));
            });
        else
        {
            if (onVocal2Triggered)
            {
                DBG("Triggering Vocal2 sample playback");
                onVocal2Triggered();
            }
        }
    };
}

// -- Double-Click Handling --
// Reload vocal samples for Pad1 or Pad6 on double-click.
void DrumPads::handleDrumPadDoubleClick(const juce::String& padName)
{
    if (padName == "Pad1")
    {
        DBG("DrumPad #1 double-clicked: Reload Vocal1 sample");
        vocal1URL = juce::URL();
        launchFileChooser("Select a new track for Vocal #1", [this](const juce::URL& url)
        {
            vocal1URL = url;
            DBG("Reloaded Vocal1 sample: " + vocal1URL.toString(true));
        });
    }
    else if (padName == "Pad6")
    {
        DBG("DrumPad #6 double-clicked: Reload Vocal2 sample");
        vocal2URL = juce::URL();
        launchFileChooser("Select a new track for Vocal #2", [this](const juce::URL& url)
        {
            vocal2URL = url;
            DBG("Reloaded Vocal2 sample: " + vocal2URL.toString(true));
        });
    }
}

// -- Destructor --
DrumPads::~DrumPads()
{
}

// -- Layout --
void DrumPads::resized()
{
    int padSize = 50;
    int spacing = 10;
    int totalWidth = getWidth();
    int totalHeight = getHeight();
    int gridWidth = 3 * padSize + 2 * spacing;
    int gridHeight = 2 * padSize + spacing;
    int startX = (totalWidth - gridWidth) / 2;
    int startY = (totalHeight - gridHeight) / 2;
    
    drumPadButtons[0]->setBounds(startX, startY, padSize, padSize);
    drumPadButtons[1]->setBounds(startX + padSize + spacing, startY, padSize, padSize);
    drumPadButtons[2]->setBounds(startX + 2 * (padSize + spacing), startY, padSize, padSize);
    drumPadButtons[3]->setBounds(startX, startY + padSize + spacing, padSize, padSize);
    drumPadButtons[4]->setBounds(startX + padSize + spacing, startY + padSize + spacing, padSize, padSize);
    drumPadButtons[5]->setBounds(startX + 2 * (padSize + spacing), startY + padSize + spacing, padSize, padSize);
}
