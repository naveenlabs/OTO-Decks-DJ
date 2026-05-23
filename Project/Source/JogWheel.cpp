/*
  ==============================================================================
    JogWheel.cpp
  ==============================================================================
    This file implements the JogWheel component, which simulates a DJ jog wheel
    for controlling track position. It includes custom painting routines to
    draw the wheel's outer ring, inner grooves, center circle, needle, and playhead.
    Mouse event handlers update the playhead position as the user interacts with the wheel,
    with callback functions to handle jog turns and release events.
*/

#include "JogWheel.h"

//==============================================================================
// Constructors and Destructor
//==============================================================================

// -- Constructor --
// No extra initialization needed.
JogWheel::JogWheel()
{
}

// -- Destructor --
JogWheel::~JogWheel()
{
}

//==============================================================================
// Painting Functions
//==============================================================================

// -- paint --
// Paints the jog wheel and its components.
void JogWheel::paint(juce::Graphics& g)
{
    auto area     = getLocalBounds().toFloat().reduced(10.0f);
    float diameter = juce::jmin(area.getWidth(), area.getHeight());
    juce::Colour neonBlue = juce::Colour::fromRGB(50, 255, 255);

    drawOuterRing(g, area, diameter, neonBlue);
    drawInnerGrooves(g, area, diameter);
    drawCenterCircle(g, area, diameter, neonBlue);
    drawNeedle(g, area, diameter, neonBlue);
    drawPlayhead(g);
}

// -- resized --
// No additional resizing logic.
void JogWheel::resized()
{
}

//==============================================================================
// Helper Drawing Functions
//==============================================================================

// -- isPointInsideWheel --
// Checks if a point is inside the wheel.
bool JogWheel::isPointInsideWheel(juce::Point<float> point) const
{
    auto area     = getLocalBounds().toFloat().reduced(10.0f);
    float diameter = juce::jmin(area.getWidth(), area.getHeight());
    auto center   = area.getCentre();

    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return std::sqrt(dx * dx + dy * dy) <= (diameter * 0.5f);
}

// -- computeAngle --
// Computes the angle between the center of the wheel and a given point.
float JogWheel::computeAngle(const juce::Point<float>& point) const
{
    auto area   = getLocalBounds().toFloat().reduced(10.0f);
    auto center = area.getCentre();
    return std::atan2(point.y - center.y, point.x - center.x);
}

// -- drawOuterRing --
// Draws the outer ring with neon dots.
void JogWheel::drawOuterRing(juce::Graphics& g, const juce::Rectangle<float>& area,
                             float diameter, juce::Colour neonBlue) const
{
    float x = area.getCentreX() - (diameter * 0.5f);
    float y = area.getCentreY() - (diameter * 0.5f);

    g.setColour(neonBlue.withAlpha(0.5f));
    g.drawEllipse(x - 3.0f, y - 3.0f, diameter + 6.0f, diameter + 6.0f, 3.0f);

    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(x, y, diameter, diameter);

    int numDots = 60;
    float dotRadius = diameter * 0.48f;
    for (int i = 0; i < numDots; ++i)
    {
        float angle = juce::MathConstants<float>::twoPi * (float)i / (float)numDots;
        float dotX  = area.getCentreX() + dotRadius * std::cos(angle);
        float dotY  = area.getCentreY() + dotRadius * std::sin(angle);

        g.setColour(juce::Colours::white);
        g.fillEllipse(dotX - 2.0f, dotY - 2.0f, 4.0f, 4.0f);

        g.setColour(neonBlue.withAlpha(0.7f));
        g.drawEllipse(dotX - 2.0f, dotY - 2.0f, 4.0f, 4.0f, 1.0f);
    }
}

// -- drawInnerGrooves --
// Draws the inner grooves inside the outer ring.
void JogWheel::drawInnerGrooves(juce::Graphics& g, const juce::Rectangle<float>& area,
                                float diameter) const
{
    float x = area.getCentreX() - (diameter * 0.5f);
    float y = area.getCentreY() - (diameter * 0.5f);

    g.setColour(juce::Colours::black);
    g.fillEllipse(x + 5.0f, y + 5.0f, diameter - 10.0f, diameter - 10.0f);

    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    int numGrooves = 25;
    for (int i = 0; i < numGrooves; ++i)
    {
        float grooveDiameter = (diameter * 0.7f) - (i * (diameter * 0.7f / numGrooves));
        float gx = area.getCentreX() - (grooveDiameter * 0.5f);
        float gy = area.getCentreY() - (grooveDiameter * 0.5f);
        g.drawEllipse(gx, gy, grooveDiameter, grooveDiameter, 1.0f);
    }
}

// -- drawCenterCircle --
// Draws the center circle with a gradient fill and neon outline.
void JogWheel::drawCenterCircle(juce::Graphics& g, const juce::Rectangle<float>& area,
                                float diameter, juce::Colour neonBlue) const
{
    float innerDiameter = diameter * 0.3f;
    float ix = area.getCentreX() - (innerDiameter * 0.5f);
    float iy = area.getCentreY() - (innerDiameter * 0.5f);

    juce::ColourGradient innerGrad(juce::Colours::black,
                                   area.getCentreX(), area.getCentreY(),
                                   juce::Colours::darkgrey,
                                   ix + innerDiameter, iy + innerDiameter,
                                   true);
    g.setGradientFill(innerGrad);
    g.fillEllipse(ix, iy, innerDiameter, innerDiameter);

    g.setColour(neonBlue.withAlpha(0.9f));
    g.drawEllipse(ix, iy, innerDiameter, innerDiameter, 4.0f);

    g.setColour(juce::Colours::grey);
    g.drawEllipse(ix, iy, innerDiameter, innerDiameter, 2.0f);
}

// -- drawNeedle --
// Draws the needle at the top of the wheel.
void JogWheel::drawNeedle(juce::Graphics& g, const juce::Rectangle<float>& area,
                          float diameter, juce::Colour neonBlue) const
{
    float needleLength = diameter * 0.1f;
    float needleX      = area.getCentreX();
    float topY         = area.getCentreY() - (diameter * 0.5f);
    float needleY      = topY + (diameter * 0.15f);

    g.setColour(neonBlue.withAlpha(0.7f));
    g.drawLine(needleX, needleY, needleX, needleY + needleLength, 6.0f);

    g.setColour(juce::Colours::white);
    g.drawLine(needleX, needleY, needleX, needleY + needleLength, 3.0f);
}

// -- drawPlayhead --
// Draws the playhead indicator.
void JogWheel::drawPlayhead(juce::Graphics& g) const
{
    auto area     = getLocalBounds().toFloat().reduced(10.0f);
    float diameter = juce::jmin(area.getWidth(), area.getHeight());

    float angle = -juce::MathConstants<float>::halfPi +
                  (juce::MathConstants<float>::twoPi * playheadPosition);

    float radius  = diameter * 0.35f;
    float centerX = area.getCentreX();
    float centerY = area.getCentreY();
    float endX    = centerX + radius * std::cos(angle);
    float endY    = centerY + radius * std::sin(angle);

    juce::Colour neonBlue = juce::Colour::fromRGB(50, 255, 255);

    g.setColour(neonBlue.withAlpha(0.6f));
    g.drawLine(centerX, centerY, endX, endY, 6.0f);

    g.setColour(juce::Colours::white);
    g.drawLine(centerX, centerY, endX, endY, 3.0f);
}

//==============================================================================
// Mouse Event Handling
//==============================================================================

// -- mouseDown --
// Starts dragging if the mouse press is within the wheel.
void JogWheel::mouseDown(const juce::MouseEvent& event)
{
    if (isPointInsideWheel(event.position))
    {
        isDragging = true;
        lastAngle  = computeAngle(event.position);
    }
    else
    {
        isDragging = false;
    }
}

// -- mouseDrag --
// Updates the playhead position based on the change in angle.
void JogWheel::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        if (!isPointInsideWheel(event.position))
            return;

        float currentAngle = computeAngle(event.position);
        float deltaAngle   = currentAngle - lastAngle;

        // Adjust deltaAngle if wrapping around.
        if (deltaAngle > juce::MathConstants<float>::pi)
            deltaAngle -= juce::MathConstants<float>::twoPi;
        else if (deltaAngle < -juce::MathConstants<float>::pi)
            deltaAngle += juce::MathConstants<float>::twoPi;

        lastAngle = currentAngle;

        float deltaPosition = deltaAngle / juce::MathConstants<float>::twoPi;
        playheadPosition    = juce::jlimit(0.0f, 1.0f, playheadPosition + deltaPosition);

        if (onJogTurn)
            onJogTurn(deltaPosition);

        repaint();
    }
}

// -- mouseUp --
// Ends dragging and triggers the onJogEnd callback.
void JogWheel::mouseUp(const juce::MouseEvent&)
{
    if (isDragging && onJogEnd)
        onJogEnd();

    isDragging = false;
}

//==============================================================================
// Setter Function
//==============================================================================

// -- setPlayheadPosition --
// Sets the normalized playhead position and repaints the wheel.
void JogWheel::setPlayheadPosition(float newPosition)
{
    playheadPosition = juce::jlimit(0.0f, 1.0f, newPosition);
    repaint();
}
