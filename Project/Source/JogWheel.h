/*
  ==============================================================================
    JogWheel.h
  ==============================================================================
    This header defines the JogWheel component that simulates a DJ jog wheel.
    It handles painting the wheel, processing mouse events for rotation, and
    updating the playhead position. Callback functions allow external handling
    of jog wheel actions.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <cmath>

// JogWheel: A component that simulates a DJ jog wheel for controlling track position.
class JogWheel : public juce::Component
{
public:
    // Constructor and Destructor.
    JogWheel();
    ~JogWheel() override;

    // Paint the jog wheel.
    void paint(juce::Graphics& g) override;
    
    // No special resizing logic.
    void resized() override;

    // Mouse event handlers for rotating the wheel.
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // Set the playhead position (0.0 to 1.0).
    void setPlayheadPosition(float newPosition);

    // Callbacks for when the jog wheel is turned or released.
    std::function<void(float delta)> onJogTurn;
    std::function<void()> onJogEnd;

private:
    // Private members: drag state, last recorded angle, and normalized playhead position.
    bool  isDragging       = false;
    float lastAngle        = 0.0f;
    float playheadPosition = 0.0f;

    // Checks if a point is inside the wheel.
    bool  isPointInsideWheel(juce::Point<float> point) const;
    
    // Computes the angle of a point relative to the wheel center.
    float computeAngle(const juce::Point<float>& point) const;

    // Helper drawing methods.
    void drawOuterRing    (juce::Graphics& g, const juce::Rectangle<float>& area, float diameter, juce::Colour neonBlue) const;
    void drawInnerGrooves (juce::Graphics& g, const juce::Rectangle<float>& area, float diameter) const;
    void drawCenterCircle (juce::Graphics& g, const juce::Rectangle<float>& area, float diameter, juce::Colour neonBlue) const;
    void drawNeedle       (juce::Graphics& g, const juce::Rectangle<float>& area, float diameter, juce::Colour neonBlue) const;
    void drawPlayhead     (juce::Graphics& g) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JogWheel)
};
