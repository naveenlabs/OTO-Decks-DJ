/*
  ==============================================================================
    VUMeter.h
  ==============================================================================
    This header defines the VUMeter component, which displays volume levels for
    left and right audio channels using colored blocks. It provides methods to
    update the channel levels and paints the meter based on these levels, updating
    its display regularly using a timer callback.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

// VUMeter: Displays volume levels using colored blocks.
class VUMeter : public juce::Component,
                public juce::Timer
{
public:
    // Constructor and Destructor.
    VUMeter();
    ~VUMeter() override;

    // Paint the volume meter.
    void paint(juce::Graphics& g) override;
    
    // No special resizing logic.
    void resized() override;

    // Update the left and right channel levels (normalized 0.0 to 1.0).
    void setLevels(float newLeftLevel, float newRightLevel);

    // Timer callback to update the meter.
    void timerCallback() override;

private:
    // Private member variables: leftLevel and rightLevel represent the current volume levels.
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;

    // Constants for the VUMeter: number of blocks, spacing, and sensitivity.
    static constexpr int numBlocks = 20;
    static constexpr float blockSpacing = 2.0f;
    static constexpr float meterSensitivity = 1.3f;

    // Linearly interpolate between two colours.
    static juce::Colour lerpColour(const juce::Colour& c1, const juce::Colour& c2, float alpha);
    
    // Determine the meter colour based on the level fraction.
    static juce::Colour getMeterColour(float fraction);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
