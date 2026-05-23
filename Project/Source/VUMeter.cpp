/*
  ==============================================================================
    VUMeter.cpp
  ==============================================================================
    This file implements the VUMeter component, which visually displays audio
    levels for the left and right channels using colored blocks. It provides
    functionality for interpolating colours, updating meter levels, and repainting
    the display regularly via a timer callback.
*/

#include "VUMeter.h"
#include <cmath>

//==============================================================================
// Utility Functions
//==============================================================================

// -- lerpColour --
// Linearly interpolates between two colours.
juce::Colour VUMeter::lerpColour(const juce::Colour& c1, const juce::Colour& c2, float alpha)
{
    return c1.interpolatedWith(c2, alpha);
}

// -- getMeterColour --
// Determines the meter colour based on the level fraction.
juce::Colour VUMeter::getMeterColour(float fraction)
{
    juce::Colour teal   = juce::Colour::fromRGB(0, 255, 255);
    juce::Colour purple = juce::Colour::fromRGB(138, 43, 226);
    juce::Colour pink   = juce::Colour::fromRGB(255, 105, 180);

    if (fraction <= 0.5f)
    {
        float localAlpha = fraction / 0.5f;
        return lerpColour(teal, purple, localAlpha);
    }
    else
    {
        float localAlpha = (fraction - 0.5f) / 0.5f;
        return lerpColour(purple, pink, localAlpha);
    }
}

//==============================================================================
// Constructors and Destructor
//==============================================================================

// -- Constructor --
// Starts the timer to update the VU meter.
VUMeter::VUMeter()
{
    startTimerHz(20);
}

// -- Destructor --
// Stops the timer.
VUMeter::~VUMeter()
{
    stopTimer();
}

//==============================================================================
// Painting Functions
//==============================================================================

// -- Paint --
// Draws the VU meter blocks for left and right channels.
void VUMeter::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    auto bounds = getLocalBounds().toFloat();
    float halfWidth = bounds.getWidth() * 0.5f;
    const float totalSpacing = blockSpacing * (numBlocks - 1);
    float blockHeight = (bounds.getHeight() - totalSpacing) / static_cast<float>(numBlocks);

    float leftX = bounds.getX() + 4.0f;
    float rightX = bounds.getX() + halfWidth + 4.0f;
    float barWidth = (halfWidth - 8.0f);

    // Scale levels using sensitivity.
    float scaledLeftLevel = juce::jlimit(0.0f, 1.0f, leftLevel * meterSensitivity);
    float scaledRightLevel = juce::jlimit(0.0f, 1.0f, rightLevel * meterSensitivity);

    // Draw blocks for both channels.
    for (int i = 0; i < numBlocks; ++i)
    {
        float fraction = static_cast<float>(i) / static_cast<float>(numBlocks - 1);
        float y = bounds.getBottom() - (i + 1) * blockHeight - i * blockSpacing;

        juce::Rectangle<float> leftBlock(leftX, y, barWidth, blockHeight);
        if (fraction <= scaledLeftLevel)
        {
            g.setColour(getMeterColour(fraction));
            g.fillRect(leftBlock);
        }

        juce::Rectangle<float> rightBlock(rightX, y, barWidth, blockHeight);
        if (fraction <= scaledRightLevel)
        {
            g.setColour(getMeterColour(fraction));
            g.fillRect(rightBlock);
        }
    }

    // Draw border and a dividing line.
    g.setColour(juce::Colours::white);
    g.drawRect(bounds, 2.0f);
    g.setColour(juce::Colours::grey);
    g.drawLine(bounds.getX() + halfWidth, bounds.getY(),
               bounds.getX() + halfWidth, bounds.getBottom(), 2.0f);
}

//==============================================================================
// Resizing
//==============================================================================

// No special resizing logic needed.
void VUMeter::resized()
{
}

//==============================================================================
// Level Management and Timer Callback
//==============================================================================

// -- setLevels --
// Updates the left and right volume levels.
void VUMeter::setLevels(float newLeftLevel, float newRightLevel)
{
    leftLevel = juce::jlimit(0.0f, 1.0f, newLeftLevel);
    rightLevel = juce::jlimit(0.0f, 1.0f, newRightLevel);
}

// -- Timer Callback --
// Repaints the component at regular intervals.
void VUMeter::timerCallback()
{
    repaint();
}
