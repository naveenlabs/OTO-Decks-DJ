/*
  ==============================================================================
    MainComponent.cpp
  ==============================================================================
    This file implements the MainComponent class, which serves as the primary
    user interface and audio processing component for the application. It
    handles audio playback, mixing, and recording (via the embedded MasterRecorder
    class), sets up and manages UI components (such as buttons, sliders, and custom
    look-and-feel elements), and processes user interactions including button clicks
    and slider changes. Additionally, it implements audio callbacks for preparing,
    processing, and releasing audio resources, as well as painting the UI.
*/

#include "MainComponent.h"

//==============================================================================
// MasterRecorder Class
//==============================================================================

// -- MasterRecorder Class --
// Handles recording operations including starting, stopping, and writing audio data.

class MainComponent::MasterRecorder
{
public:
    //==============================================================================
    // Constructors and Destructor
    //==============================================================================

    // -- Constructor --
    // No extra initialization needed.
    MasterRecorder() {}

    // -- Destructor --
    // Stops recording if active.
    ~MasterRecorder() { stopRecording(); }

    //==============================================================================
    // Recording Functions
    //==============================================================================

    // -- startRecording --
    // Initiates recording, creates a WAV file on the desktop, and sets up the writer.
    bool startRecording(double sampleRate)
    {
        if (recording)
            return false;
        auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
        recordingFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                            .getChildFile("recording_" + timestamp + ".wav");
        juce::WavAudioFormat wavFormat;
        std::unique_ptr<juce::FileOutputStream> fos(recordingFile.createOutputStream());
        if (fos != nullptr)
        {
            writer.reset(wavFormat.createWriterFor(fos.get(), sampleRate, 2, 16, {}, 0));
            fos.release();
        }
        if (writer != nullptr)
        {
            recording = true;
            DBG("Master recording started at: " << recordingFile.getFullPathName());
        }
        else
        {
            DBG("Failed to start master recording!");
        }
        return recording;
    }

    // -- stopRecording --
    // Stops recording and resets the writer.
    void stopRecording()
    {
        if (recording)
        {
            writer.reset();
            recording = false;
            DBG("Master recording stopped. File: " << recordingFile.getFullPathName());
        }
    }

    // -- writeBlock --
    // Writes a block of audio samples to the file if recording.
    void writeBlock(const juce::AudioSourceChannelInfo& bufferToFill)
    {
        if (recording && writer)
        {
            writer->writeFromAudioSampleBuffer(*bufferToFill.buffer,
                                               bufferToFill.startSample,
                                               bufferToFill.numSamples);
        }
    }

    // -- isRecordingActive --
    // Returns true if recording is active.
    bool isRecordingActive() const { return recording; }

    // -- getRecordingFile --
    // Returns the recording file.
    juce::File getRecordingFile() const { return recordingFile; }

private:
    bool recording = false;
    juce::File recordingFile;
    std::unique_ptr<juce::AudioFormatWriter> writer;
};

//==============================================================================
// RealisticButtonLookAndFeel Methods
//==============================================================================

// -- RealisticButtonLookAndFeel::drawButtonBackground --
// Draws a rounded rectangle with a gradient outline as the button background.
void MainComponent::RealisticButtonLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                                      juce::Button& button,
                                                                      const juce::Colour&,
                                                                      bool isMouseOverButton,
                                                                      bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    float cornerRadius = 20.0f;
    juce::Colour deckBg = juce::Colour::fromRGB(10, 10, 50);
    if (isButtonDown)
        deckBg = deckBg.brighter(0.5f);
    else if (isMouseOverButton)
        deckBg = deckBg.brighter(0.3f);
    g.setColour(deckBg);
    g.fillRoundedRectangle(bounds, cornerRadius);
    
    juce::Colour teal   = juce::Colour::fromRGB(0, 255, 255);
    juce::Colour purple = juce::Colour::fromRGB(138, 43, 226);
    juce::Colour pink   = juce::Colour::fromRGB(255, 105, 180);
    
    if (isButtonDown)
    {
        teal   = teal.brighter(0.5f);
        purple = purple.brighter(0.5f);
        pink   = pink.brighter(0.5f);
    }
    else if (isMouseOverButton)
    {
        teal   = teal.brighter(0.3f);
        purple = purple.brighter(0.3f);
        pink   = pink.brighter(0.3f);
    }
    
    juce::ColourGradient outlineGrad(teal,
                                     bounds.getX(), bounds.getCentreY(),
                                     pink,
                                     bounds.getRight(), bounds.getCentreY(),
                                     false);
    outlineGrad.addColour(0.5, purple);
    
    juce::Path outlinePath;
    outlinePath.addRoundedRectangle(bounds, cornerRadius);
    
    g.setGradientFill(outlineGrad);
    g.strokePath(outlinePath, juce::PathStrokeType(4.0f));
}

//==============================================================================
// FancyFaderLookAndFeel Methods - Linear Slider
//==============================================================================

// -- FancyFaderLookAndFeel::drawLinearSlider --
// Draws a linear slider with a gradient track and circular thumb.
void MainComponent::FancyFaderLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                                             int x, int y, int width, int height,
                                                             float sliderPos,
                                                             float, float,
                                                             const juce::Slider::SliderStyle style,
                                                             juce::Slider&)
{
    bool vertical = (style == juce::Slider::LinearVertical || style == juce::Slider::ThreeValueVertical);
    juce::Colour teal = juce::Colour::fromRGB(0, 255, 255);
    juce::Colour pink = juce::Colour::fromRGB(255, 105, 180);
    float trackThickness = 6.0f;
    juce::Rectangle<float> trackRect;
    if (vertical)
    {
        float cx = x + (width * 0.5f) - (trackThickness * 0.5f);
        trackRect = { cx, (float)y, trackThickness, (float)height };
    }
    else
    {
        float cy = y + (height * 0.5f) - (trackThickness * 0.5f);
        trackRect = { (float)x, cy, (float)width, trackThickness };
    }
    juce::ColourGradient trackGradient;
    if (vertical)
    {
        trackGradient = juce::ColourGradient(teal,
                                             trackRect.getCentreX(), trackRect.getY(),
                                             pink,
                                             trackRect.getCentreX(), trackRect.getBottom(),
                                             false);
    }
    else
    {
        trackGradient = juce::ColourGradient(teal,
                                             trackRect.getX(), trackRect.getCentreY(),
                                             pink,
                                             trackRect.getRight(), trackRect.getCentreY(),
                                             false);
    }
    g.setGradientFill(trackGradient);
    g.fillRoundedRectangle(trackRect, 3.0f);
    
    float thumbSize = 16.0f;
    juce::Rectangle<float> thumbRect;
    if (vertical)
    {
        float cx = x + (width * 0.5f) - (thumbSize * 0.5f);
        float cy = sliderPos - (thumbSize * 0.5f);
        thumbRect = { cx, cy, thumbSize, thumbSize };
    }
    else
    {
        float cx = sliderPos - (thumbSize * 0.5f);
        float cy = y + (height * 0.5f) - (thumbSize * 0.5f);
        thumbRect = { cx, cy, thumbSize, thumbSize };
    }
    juce::ColourGradient thumbGrad(teal,
                                   thumbRect.getX(), thumbRect.getCentreY(),
                                   pink,
                                   thumbRect.getRight(), thumbRect.getCentreY(),
                                   false);
    g.setGradientFill(thumbGrad);
    g.fillEllipse(thumbRect);
    g.setColour(juce::Colours::black);
    g.drawEllipse(thumbRect, 1.0f);
}

//==============================================================================
// FancyFaderLookAndFeel Methods - Rotary Slider
//==============================================================================

// -- FancyFaderLookAndFeel::drawRotarySlider --
// Draws a rotary slider with a glow arc, circular track, and pointer indicator.
void MainComponent::FancyFaderLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                                             int x, int y, int width, int height,
                                                             float sliderPosProportional,
                                                             float rotaryStartAngle,
                                                             float rotaryEndAngle,
                                                             juce::Slider&)
{
    const float diameter = (float)juce::jmin(width, height) - 4.0f;
    const float radius   = diameter * 0.5f;
    const float centerX  = (float)x + ((float)width * 0.5f);
    const float centerY  = (float)y + ((float)height * 0.5f);
    const float rawAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float adjustedAngle = rawAngle + 1.6f;
    const float pointerAngle  = adjustedAngle + juce::MathConstants<float>::pi;
    
    g.setColour(juce::Colours::black);
    g.fillEllipse(centerX - radius, centerY - radius, diameter, diameter);
    
    {
        // Draw glow arc with gradient.
        juce::Path glowArc;
        glowArc.addArc(centerX - radius, centerY - radius, diameter, diameter,
                       rotaryStartAngle, rotaryEndAngle, true);
        juce::Colour teal   = juce::Colour::fromRGB(0, 255, 255).withAlpha(0.3f);
        juce::Colour pink   = juce::Colour::fromRGB(255, 105, 180).withAlpha(0.3f);
        juce::ColourGradient glowGrad(teal,
                                      centerX, centerY,
                                      pink,
                                      centerX + radius, centerY,
                                      true);
        g.saveState();
        g.setGradientFill(glowGrad);
        g.strokePath(glowArc, juce::PathStrokeType(6.0f));
        g.restoreState();
    }
    
    {
        // Draw main arc.
        juce::Path arc;
        arc.addArc(centerX - radius, centerY - radius, diameter, diameter,
                   rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colours::white);
        g.strokePath(arc, juce::PathStrokeType(2.0f));
    }
    
    {
        // Draw pointer line.
        const float pointerLength = radius * 0.9f;
        juce::Line<float> pointerLine({ centerX, centerY },
                                      { centerX + pointerLength * std::cos(pointerAngle),
                                        centerY + pointerLength * std::sin(pointerAngle) });
        g.setColour(juce::Colours::white);
        g.drawLine(pointerLine, 3.0f);
    }
}

//==============================================================================
// MainComponent Class
//==============================================================================

//==============================================================================
// Constructors and Destructor
//==============================================================================

// -- MainComponent Constructor --
// Sets up audio channels, registers formats, initializes UI components, and sets callbacks.
MainComponent::MainComponent()
{
    setSize(1000, 650);

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
         && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [this](bool granted)
            {
                if (granted)
                    setAudioChannels(0, 2);
            });
    }
    else
    {
        setAudioChannels(0, 2);
    }

    formatManager.registerBasicFormats();

    masterRecorder = std::make_unique<MasterRecorder>();

    initializeUI();

    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    addAndMakeVisible(playlistComponent);

    // Playlist callbacks for loading tracks.
    playlistComponent.loadDeck1Callback = [this](const juce::URL& url)
    {
        deckGUI1.loadTrack(url);
    };
    playlistComponent.loadDeck2Callback = [this](const juce::URL& url)
    {
        deckGUI2.loadTrack(url);
    };

    // Record toggle callback for deckGUI1.
    deckGUI1.onRecordToggled = [this](bool recOn)
    {
        if (recOn)
            masterRecorder->startRecording(44100.0);
        else
            masterRecorder->stopRecording();
    };

    drumPlayer1.sampleMode = true;
    drumPlayer2.sampleMode = true;

    // Drum pad callbacks for deckGUI1.
    {
        auto& dp1 = deckGUI1.getDrumPads();
        dp1.onBassDropTriggered = [this] { player1.triggerBassDrop(); };
        dp1.onKickRollTriggered = [this] { player1.triggerKickRoll(); };
        dp1.onSkipBackwardTriggered = [this] { player1.skipBackward(8, 120); };
        dp1.onSkipForwardTriggered = [this] { player1.skipForward(8, 120); };

        dp1.onVocal1Triggered = [this, &dp1]
        {
            juce::URL url = dp1.getVocal1URL();
            DBG("Left Deck - onVocal1Triggered: " << url.toString(true));
            if (!url.isEmpty())
            {
                drumPlayer1.loadURL(url);
                drumPlayer1.start();
            }
        };
        dp1.onVocal2Triggered = [this, &dp1]
        {
            juce::URL url = dp1.getVocal2URL();
            DBG("Left Deck - onVocal2Triggered: " << url.toString(true));
            if (!url.isEmpty())
            {
                drumPlayer1.loadURL(url);
                drumPlayer1.start();
            }
        };
    }
    // Drum pad callbacks for deckGUI2.
    {
        auto& dp2 = deckGUI2.getDrumPads();
        dp2.onBassDropTriggered = [this] { player2.triggerBassDrop(); };
        dp2.onKickRollTriggered = [this] { player2.triggerKickRoll(); };
        dp2.onSkipBackwardTriggered = [this] { player2.skipBackward(8, 120); };
        dp2.onSkipForwardTriggered = [this] { player2.skipForward(8, 120); };

        dp2.onVocal1Triggered = [this, &dp2]
        {
            juce::URL url = dp2.getVocal1URL();
            DBG("Right Deck - onVocal1Triggered: " << url.toString(true));
            if (!url.isEmpty())
            {
                drumPlayer2.loadURL(url);
                drumPlayer2.start();
            }
        };
        dp2.onVocal2Triggered = [this, &dp2]
        {
            juce::URL url = dp2.getVocal2URL();
            DBG("Right Deck - onVocal2Triggered: " << url.toString(true));
            if (!url.isEmpty())
            {
                drumPlayer2.loadURL(url);
                drumPlayer2.start();
            }
        };
    }

    // Initialize buttons and set look-and-feel.
    addAndMakeVisible(playButton1);
    playButton1.setLookAndFeel(&realisticButtonLookAndFeel);
    playButton1.addListener(this);

    addAndMakeVisible(stopButton1);
    stopButton1.setLookAndFeel(&realisticButtonLookAndFeel);
    stopButton1.addListener(this);

    addAndMakeVisible(loadButton1);
    loadButton1.setLookAndFeel(&realisticButtonLookAndFeel);
    loadButton1.addListener(this);

    addAndMakeVisible(playButton2);
    playButton2.setLookAndFeel(&realisticButtonLookAndFeel);
    playButton2.addListener(this);

    addAndMakeVisible(stopButton2);
    stopButton2.setLookAndFeel(&realisticButtonLookAndFeel);
    stopButton2.addListener(this);

    addAndMakeVisible(loadButton2);
    loadButton2.setLookAndFeel(&realisticButtonLookAndFeel);
    loadButton2.addListener(this);

    // Initialize sliders, labels, and VU meters.
    addAndMakeVisible(volSlider1); addAndMakeVisible(speedSlider1); addAndMakeVisible(posSlider1);
    addAndMakeVisible(volLabel1); addAndMakeVisible(speedLabel1); addAndMakeVisible(posLabel1);
    addAndMakeVisible(eqLowSlider1); addAndMakeVisible(eqMidSlider1); addAndMakeVisible(eqHighSlider1);
    addAndMakeVisible(eqLowLabel1); addAndMakeVisible(eqMidLabel1); addAndMakeVisible(eqHighLabel1);

    addAndMakeVisible(volSlider2); addAndMakeVisible(speedSlider2); addAndMakeVisible(posSlider2);
    addAndMakeVisible(volLabel2); addAndMakeVisible(speedLabel2); addAndMakeVisible(posLabel2);
    addAndMakeVisible(eqLowSlider2); addAndMakeVisible(eqMidSlider2); addAndMakeVisible(eqHighSlider2);
    addAndMakeVisible(eqLowLabel2); addAndMakeVisible(eqMidLabel2); addAndMakeVisible(eqHighLabel2);

    addAndMakeVisible(vuMeter1);
    addAndMakeVisible(vuMeter2);

    initializeSlider(volSlider1, volLabel1, "Volume", 0.0, 1.0, 0.01, juce::Slider::LinearVertical);
    initializeSlider(speedSlider1, speedLabel1, "Speed", 0.01, 2.0, 0.01, juce::Slider::LinearVertical);
    initializeSlider(posSlider1, posLabel1, "Position", 0.0, 1.0, 0.01, juce::Slider::LinearHorizontal);
    initializeSlider(eqLowSlider1, eqLowLabel1, "Low", -12.0, 12.0, 0.1, juce::Slider::Rotary);
    initializeSlider(eqMidSlider1, eqMidLabel1, "Mid", -12.0, 12.0, 0.1, juce::Slider::Rotary);
    initializeSlider(eqHighSlider1, eqHighLabel1, "High", -12.0, 12.0, 0.1, juce::Slider::Rotary);

    initializeSlider(volSlider2, volLabel2, "Volume", 0.0, 1.0, 0.01, juce::Slider::LinearVertical);
    initializeSlider(speedSlider2, speedLabel2, "Speed", 0.01, 2.0, 0.01, juce::Slider::LinearVertical);
    initializeSlider(posSlider2, posLabel2, "Position", 0.0, 1.0, 0.01, juce::Slider::LinearHorizontal);
    initializeSlider(eqLowSlider2, eqLowLabel2, "Low", -12.0, 12.0, 0.1, juce::Slider::Rotary);
    initializeSlider(eqMidSlider2, eqMidLabel2, "Mid", -12.0, 12.0, 0.1, juce::Slider::Rotary);
    initializeSlider(eqHighSlider2, eqHighLabel2, "High", -12.0, 12.0, 0.1, juce::Slider::Rotary);

    startTimerHz(20);
}

//==============================================================================
// Destructors
//==============================================================================

// -- MainComponent Destructor --
// Releases look-and-feel pointers and shuts down the audio system.
MainComponent::~MainComponent()
{
    playButton1.setLookAndFeel(nullptr);
    stopButton1.setLookAndFeel(nullptr);
    loadButton1.setLookAndFeel(nullptr);

    playButton2.setLookAndFeel(nullptr);
    stopButton2.setLookAndFeel(nullptr);
    loadButton2.setLookAndFeel(nullptr);

    volSlider1.setLookAndFeel(nullptr);
    speedSlider1.setLookAndFeel(nullptr);
    posSlider1.setLookAndFeel(nullptr);
    eqLowSlider1.setLookAndFeel(nullptr);
    eqMidSlider1.setLookAndFeel(nullptr);
    eqHighSlider1.setLookAndFeel(nullptr);

    volSlider2.setLookAndFeel(nullptr);
    speedSlider2.setLookAndFeel(nullptr);
    posSlider2.setLookAndFeel(nullptr);
    eqLowSlider2.setLookAndFeel(nullptr);
    eqMidSlider2.setLookAndFeel(nullptr);
    eqHighSlider2.setLookAndFeel(nullptr);

    shutdownAudio();
}

//==============================================================================
// UI Initialization and Layout
//==============================================================================

// -- initializeUI --
// Sets up additional UI elements if needed.
void MainComponent::initializeUI()
{
    // Additional UI initialization logic can be added here.
}

// -- initializeSlider --
// Configures a slider with its label, range, style, and listener.
void MainComponent::initializeSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText,
                                     double minValue, double maxValue, double interval,
                                     juce::Slider::SliderStyle style,
                                     juce::Slider::TextEntryBoxPosition textBoxPosition)
{
    slider.setLookAndFeel(&fancyFaderLookAndFeel);
    slider.setRange(minValue, maxValue, interval);
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(textBoxPosition, false, 50, 20);
    slider.setNumDecimalPlacesToDisplay(2);
    slider.addListener(this);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
}

// -- layoutUIComponents --
// Arranges the UI components including deck areas, buttons, sliders, and labels.
void MainComponent::layoutUIComponents()
{
    auto area = getLocalBounds();
    auto topArea = area.removeFromTop((int)(area.getHeight() * 0.70f));
    auto bottomArea = area;

    playlistComponent.setBounds(bottomArea);

    auto colWidth = topArea.getWidth() / 4;
    auto col1 = topArea.removeFromLeft(colWidth);
    auto col2 = topArea.removeFromLeft(colWidth);
    auto col3 = topArea.removeFromLeft(colWidth);
    auto col4 = topArea;

    deckGUI1.setBounds(col1);
    deckGUI2.setBounds(col4);

    // Layout for Deck GUI1
    {
        auto deck1Area = col2.reduced(10);
        int btnHeight = 50, btnSpacing = 10;
        int rowWidth = deck1Area.getWidth();
        int halfWidth = (rowWidth - btnSpacing) / 2;
        auto rowArea = deck1Area.removeFromTop(btnHeight);

        playButton1.setBounds(rowArea.removeFromLeft(halfWidth));
        rowArea.removeFromLeft(btnSpacing);
        stopButton1.setBounds(rowArea.removeFromLeft(halfWidth));

        deck1Area.removeFromTop(10);
        int loadBtnHeight = 40;
        loadButton1.setBounds(deck1Area.removeFromTop(loadBtnHeight));

        deck1Area.removeFromTop(10);
        int rowH = 150;
        auto row = deck1Area.removeFromTop(rowH);
        int colW = row.getWidth() / 3;

        {
            auto volArea = row.removeFromLeft(colW).reduced(5);
            int sliderHeight = volArea.getHeight() - 20;
            volSlider1.setBounds(volArea.removeFromTop(sliderHeight));
            volLabel1.setBounds(volArea);
        }
        {
            auto vuArea = row.removeFromLeft(colW).reduced(5);
            vuMeter1.setBounds(vuArea);
        }
        {
            auto speedArea = row.removeFromLeft(colW).reduced(5);
            int sliderHeight = speedArea.getHeight() - 20;
            speedSlider1.setBounds(speedArea.removeFromTop(sliderHeight));
            speedLabel1.setBounds(speedArea);
        }

        deck1Area.removeFromTop(10);
        int posSliderH = 60;
        auto posArea = deck1Area.removeFromTop(posSliderH).reduced(5);
        {
            int sliderHeight = posArea.getHeight() - 20;
            posSlider1.setBounds(posArea.removeFromTop(sliderHeight));
            posLabel1.setBounds(posArea);
        }

        deck1Area.removeFromTop(10);
        int eqKnobHeight = 100;
        auto eqArea = deck1Area.removeFromTop(eqKnobHeight);
        int eqWidth = eqArea.getWidth() / 3;

        {
            auto lowArea = eqArea.removeFromLeft(eqWidth).reduced(5);
            int knobH = static_cast<int>(lowArea.getHeight() * 0.8f);
            eqLowSlider1.setBounds(lowArea.removeFromTop(knobH));
            eqLowLabel1.setBounds(lowArea);
        }
        {
            auto midArea = eqArea.removeFromLeft(eqWidth).reduced(5);
            int knobH = static_cast<int>(midArea.getHeight() * 0.8f);
            eqMidSlider1.setBounds(midArea.removeFromTop(knobH));
            eqMidLabel1.setBounds(midArea);
        }
        {
            auto highArea = eqArea.reduced(5);
            int knobH = static_cast<int>(highArea.getHeight() * 0.8f);
            eqHighSlider1.setBounds(highArea.removeFromTop(knobH));
            eqHighLabel1.setBounds(highArea);
        }
    }

    // Layout for Deck GUI2
    {
        auto deck2Area = col3.reduced(10);
        int btnHeight = 50, btnSpacing = 10;
        int rowWidth = deck2Area.getWidth();
        int halfWidth = (rowWidth - btnSpacing) / 2;
        auto rowArea = deck2Area.removeFromTop(btnHeight);

        playButton2.setBounds(rowArea.removeFromLeft(halfWidth));
        rowArea.removeFromLeft(btnSpacing);
        stopButton2.setBounds(rowArea.removeFromLeft(halfWidth));

        deck2Area.removeFromTop(10);
        int loadBtnHeight = 40;
        loadButton2.setBounds(deck2Area.removeFromTop(loadBtnHeight));

        deck2Area.removeFromTop(10);
        int rowH = 150;
        auto row = deck2Area.removeFromTop(rowH);
        int colW = row.getWidth() / 3;

        {
            auto volArea = row.removeFromLeft(colW).reduced(5);
            int sliderHeight = volArea.getHeight() - 20;
            volSlider2.setBounds(volArea.removeFromTop(sliderHeight));
            volLabel2.setBounds(volArea);
        }
        {
            auto vuArea = row.removeFromLeft(colW).reduced(5);
            vuMeter2.setBounds(vuArea);
        }
        {
            auto speedArea = row.removeFromLeft(colW).reduced(5);
            int sliderHeight = speedArea.getHeight() - 20;
            speedSlider2.setBounds(speedArea.removeFromTop(sliderHeight));
            speedLabel2.setBounds(speedArea);
        }

        deck2Area.removeFromTop(10);
        int posSliderH = 60;
        auto posArea = deck2Area.removeFromTop(posSliderH).reduced(5);
        {
            int sliderHeight = posArea.getHeight() - 20;
            posSlider2.setBounds(posArea.removeFromTop(sliderHeight));
            posLabel2.setBounds(posArea);
        }

        deck2Area.removeFromTop(10);
        int eqKnobHeight = 100;
        auto eqArea = deck2Area.removeFromTop(eqKnobHeight);
        int eqWidth = eqArea.getWidth() / 3;

        {
            auto lowArea = eqArea.removeFromLeft(eqWidth).reduced(5);
            int knobH = static_cast<int>(lowArea.getHeight() * 0.8f);
            eqLowSlider2.setBounds(lowArea.removeFromTop(knobH));
            eqLowLabel2.setBounds(lowArea);
        }
        {
            auto midArea = eqArea.removeFromLeft(eqWidth).reduced(5);
            int knobH = static_cast<int>(midArea.getHeight() * 0.8f);
            eqMidSlider2.setBounds(midArea.removeFromTop(knobH));
            eqMidLabel2.setBounds(midArea);
        }
        {
            auto highArea = eqArea.reduced(5);
            int knobH = static_cast<int>(highArea.getHeight() * 0.8f);
            eqHighSlider2.setBounds(highArea.removeFromTop(knobH));
            eqHighLabel2.setBounds(highArea);
        }
    }
}

//==============================================================================
// Resizing
//==============================================================================

// -- resized --
// Called when the component's size changes.
void MainComponent::resized()
{
    layoutUIComponents();
}

//==============================================================================
// Audio Callbacks
//==============================================================================

// -- prepareToPlay --
// Prepares all audio sources and the mixer for playback.
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
    drumPlayer1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    drumPlayer2.prepareToPlay(samplesPerBlockExpected, sampleRate);

    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
    mixerSource.addInputSource(&drumPlayer1, false);
    mixerSource.addInputSource(&drumPlayer2, false);
}

// -- getNextAudioBlock --
// Processes and outputs the next block of audio, recording if active.
void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
    if (masterRecorder && masterRecorder->isRecordingActive())
    {
        masterRecorder->writeBlock(bufferToFill);
    }
}

// -- releaseResources --
// Releases audio resources from all sources.
void MainComponent::releaseResources()
{
    player1.releaseResources();
    player2.releaseResources();
    drumPlayer1.releaseResources();
    drumPlayer2.releaseResources();
    mixerSource.releaseResources();
}

//==============================================================================
// Painting
//==============================================================================

// -- paint --
// Draws the background gradient and UI separators.
void MainComponent::paint (juce::Graphics& g)
{
    juce::ColourGradient gradient(
        juce::Colour::fromRGB(10, 10, 50), getWidth() / 2.0f, 0.0f,
        juce::Colour::fromRGB(5, 5, 20),   getWidth() / 2.0f, (float)getHeight(),
        false
    );
    g.setGradientFill(gradient);
    g.fillAll();

    auto area = getLocalBounds();
    auto topArea = area.removeFromTop((int)(area.getHeight() * 0.70f));
    auto colWidth = topArea.getWidth() / 4.0f;
    int boundaryX1 = topArea.getX() + (int)colWidth;
    int boundaryX2 = topArea.getX() + (int)(colWidth * 2.0f);
    int boundaryX3 = topArea.getX() + (int)(colWidth * 3.0f);

    g.setColour(juce::Colour::fromRGB(192, 192, 192));
    g.drawLine((float)boundaryX1, (float)topArea.getY(),
               (float)boundaryX1, (float)topArea.getBottom(), 2.0f);
    g.drawLine((float)boundaryX2, (float)topArea.getY(),
               (float)boundaryX2, (float)topArea.getBottom(), 2.0f);
    g.drawLine((float)boundaryX3, (float)topArea.getY(),
               (float)boundaryX3, (float)topArea.getBottom(), 2.0f);
}

//==============================================================================
// Button Handling
//==============================================================================

// -- buttonClicked --
// Handles button click events to start, stop, or load tracks.
void MainComponent::buttonClicked (juce::Button* button)
{
    if      (button == &playButton1) player1.start();
    else if (button == &stopButton1) player1.stop();
    else if (button == &loadButton1)
    {
        auto flags = juce::FileBrowserComponent::canSelectFiles;
        fChooser.launchAsync(flags, [this] (const juce::FileChooser& fc)
        {
            if (fc.getResult().existsAsFile())
                deckGUI1.loadTrack(juce::URL{ fc.getResult() });
        });
    }
    else if (button == &playButton2) player2.start();
    else if (button == &stopButton2) player2.stop();
    else if (button == &loadButton2)
    {
        auto flags = juce::FileBrowserComponent::canSelectFiles;
        fChooser.launchAsync(flags, [this] (const juce::FileChooser& fc)
        {
            if (fc.getResult().existsAsFile())
                deckGUI2.loadTrack(juce::URL{ fc.getResult() });
        });
    }
}

//==============================================================================
// Slider Handling
//==============================================================================

// -- sliderValueChanged --
// Updates player parameters based on slider changes.
void MainComponent::sliderValueChanged (juce::Slider* slider)
{
    if      (slider == &volSlider1)    player1.setGain(slider->getValue());
    else if (slider == &speedSlider1)  player1.setSpeed(slider->getValue());
    else if (slider == &posSlider1)    player1.setPositionRelative(slider->getValue());
    else if (slider == &eqLowSlider1)  player1.setLowGain((float) slider->getValue());
    else if (slider == &eqMidSlider1)  player1.setMidGain((float) slider->getValue());
    else if (slider == &eqHighSlider1) player1.setHighGain((float) slider->getValue());
    else if (slider == &volSlider2)    player2.setGain(slider->getValue());
    else if (slider == &speedSlider2)  player2.setSpeed(slider->getValue());
    else if (slider == &posSlider2)    player2.setPositionRelative(slider->getValue());
    else if (slider == &eqLowSlider2)  player2.setLowGain((float) slider->getValue());
    else if (slider == &eqMidSlider2)  player2.setMidGain((float) slider->getValue());
    else if (slider == &eqHighSlider2) player2.setHighGain((float) slider->getValue());
}

//==============================================================================
// Timer Callback
//==============================================================================

// -- timerCallback --
// Updates the VU meters based on the current audio levels.
void MainComponent::timerCallback()
{
    vuMeter1.setLevels(player1.getRmsLevelLeft(), player1.getRmsLevelRight());
    vuMeter2.setLevels(player2.getRmsLevelLeft(), player2.getRmsLevelRight());
}
