/*
  ==============================================================================
    PlaylistComponent.cpp
  ==============================================================================
    This file implements the PlaylistComponent, which manages the track playlist
    for the DJ application. It provides functionality for adding, filtering, and
    loading tracks into decks, as well as saving and loading the playlist to/from
    an XML file. Additionally, it implements a custom table model and action buttons
    for managing each track entry.
*/

#include "PlaylistComponent.h"
#include <cmath>

//==============================================================================
// Utility Functions
//==============================================================================

// -- secondsToMinSec --
// Converts seconds to a minutes:seconds string.
static juce::String secondsToMinSec(double seconds)
{
    int totalSeconds = static_cast<int>(std::round(seconds));
    int mins = totalSeconds / 60;
    int secs = totalSeconds % 60;
    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
}

//==============================================================================
// Audio File Validation
//==============================================================================

// -- isAudioFile --
// Checks if the given file has a supported audio extension.
bool PlaylistComponent::isAudioFile(const juce::File& file)
{
    juce::String ext = file.getFileExtension().toLowerCase();
    return (ext == ".wav" || ext == ".mp3" || ext == ".aiff" || ext == ".flac" || ext == ".ogg");
}

//==============================================================================
// PlaylistButtonLookAndFeel Implementation
//==============================================================================

// -- Constructor/Destructor --
// (No additional comments necessary.)
PlaylistComponent::PlaylistButtonLookAndFeel::PlaylistButtonLookAndFeel() { }
PlaylistComponent::PlaylistButtonLookAndFeel::~PlaylistButtonLookAndFeel() { }

// -- drawButtonBackground --
// Custom drawing of the button background with gradient fill and rounded corners.
void PlaylistComponent::PlaylistButtonLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                                         juce::Button& button,
                                                                         const juce::Colour&,
                                                                         bool isMouseOverButton,
                                                                         bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    float cornerRadius = 10.0f;
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

    juce::ColourGradient outlineGrad(teal, bounds.getX(), bounds.getCentreY(),
                                     pink, bounds.getRight(), bounds.getCentreY(), false);
    outlineGrad.addColour(0.5, purple);

    juce::Path outlinePath;
    outlinePath.addRoundedRectangle(bounds, cornerRadius);

    g.setGradientFill(outlineGrad);
    g.strokePath(outlinePath, juce::PathStrokeType(2.0f));
}

//==============================================================================
// PlaylistComponent Constructor and Destructor
//==============================================================================

// -- Constructor --
// Initializes components, loads the library, and sets up callbacks.
PlaylistComponent::PlaylistComponent()
    : model1(&visibleDeck1, true,  this),
      model2(&visibleDeck2, false, this)
{
    formatManager.registerBasicFormats();
    lastDirectory = juce::File::getSpecialLocation(juce::File::userMusicDirectory);

    deck1.resize(6);
    deck2.resize(6);
    visibleDeck1 = deck1;
    visibleDeck2 = deck2;

    loadLibrary();

    // -- Setup Search Box and Clear Button for Deck1 --
    searchBox1.setTextToShowWhenEmpty("Search Deck1...", juce::Colours::grey);
    searchBox1.onTextChange = [this] { filterDeck1(); };
    searchBox1.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(40, 40, 60));
    searchBox1.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    searchBox1.setColour(juce::TextEditor::outlineColourId, juce::Colour::fromRGB(60, 60, 80));
    searchBox1.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour::fromRGB(138, 43, 226));
    addAndMakeVisible(searchBox1);

    clearButton1.onClick = [this]
    {
        searchBox1.setText("");
        filterDeck1();
    };
    clearButton1.setLookAndFeel(getPlaylistButtonLookAndFeel());
    addAndMakeVisible(clearButton1);

    // -- Setup Table Component for Deck1 --
    tableComponent1.getHeader().addColumn("S.No",       1, 50);
    tableComponent1.getHeader().addColumn("Title Track",2, 200);
    tableComponent1.getHeader().addColumn("Duration",   3, 80);
    tableComponent1.getHeader().addColumn("Actions",    4, 170);
    tableComponent1.setModel(&model1);
    tableComponent1.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(40, 40, 60));
    tableComponent1.getHeader().setColour(juce::TableHeaderComponent::backgroundColourId, juce::Colour::fromRGB(60, 60, 80));
    tableComponent1.getHeader().setColour(juce::TableHeaderComponent::textColourId, juce::Colours::white);
    addAndMakeVisible(tableComponent1);

    // -- Setup Search Box and Clear Button for Deck2 --
    searchBox2.setTextToShowWhenEmpty("Search Deck2...", juce::Colours::grey);
    searchBox2.onTextChange = [this] { filterDeck2(); };
    searchBox2.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(40, 40, 60));
    searchBox2.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    searchBox2.setColour(juce::TextEditor::outlineColourId, juce::Colour::fromRGB(60, 60, 80));
    searchBox2.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour::fromRGB(138, 43, 226));
    addAndMakeVisible(searchBox2);

    clearButton2.onClick = [this]
    {
        searchBox2.setText("");
        filterDeck2();
    };
    clearButton2.setLookAndFeel(getPlaylistButtonLookAndFeel());
    addAndMakeVisible(clearButton2);

    // -- Setup Table Component for Deck2 --
    tableComponent2.getHeader().addColumn("S.No",       1, 50);
    tableComponent2.getHeader().addColumn("Title Track",2, 200);
    tableComponent2.getHeader().addColumn("Duration",   3, 80);
    tableComponent2.getHeader().addColumn("Actions",    4, 170);
    tableComponent2.setModel(&model2);
    tableComponent2.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromRGB(40, 40, 60));
    tableComponent2.getHeader().setColour(juce::TableHeaderComponent::backgroundColourId, juce::Colour::fromRGB(60, 60, 80));
    tableComponent2.getHeader().setColour(juce::TableHeaderComponent::textColourId, juce::Colours::white);
    addAndMakeVisible(tableComponent2);
}

PlaylistComponent::~PlaylistComponent()
{
    tableComponent1.setModel(nullptr);
    tableComponent2.setModel(nullptr);
}

//==============================================================================
// Painting and Resizing
//==============================================================================

// -- Paint --
// Fills the background with transparent black.
void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
}

// -- Resized --
// Divides the component into two halves for Deck1 and Deck2 layout.
void PlaylistComponent::resized()
{
    auto area = getLocalBounds();
    auto halfWidth = area.getWidth() / 2;
    auto deck1Area = area.removeFromLeft(halfWidth);
    auto deck2Area = area;

    const int searchHeight = 30;
    const int clearButtonWidth = 30;
    const int spacing = 5;

    // -- Layout for Deck1 --
    auto deck1SearchArea = deck1Area.removeFromTop(searchHeight);
    searchBox1.setBounds(deck1SearchArea.removeFromLeft(deck1SearchArea.getWidth() - clearButtonWidth - spacing));
    clearButton1.setBounds(deck1SearchArea.removeFromLeft(clearButtonWidth));
    deck1Area.removeFromTop(spacing);
    tableComponent1.setBounds(deck1Area);

    // -- Layout for Deck2 --
    auto deck2SearchArea = deck2Area.removeFromTop(searchHeight);
    searchBox2.setBounds(deck2SearchArea.removeFromLeft(deck2SearchArea.getWidth() - clearButtonWidth - spacing));
    clearButton2.setBounds(deck2SearchArea.removeFromLeft(clearButtonWidth));
    deck2Area.removeFromTop(spacing);
    tableComponent2.setBounds(deck2Area);
}

//==============================================================================
// Filtering Functions
//==============================================================================

// -- filterDeck1 and filterDeck2 --
// Triggers filtering for Deck1 or Deck2.
void PlaylistComponent::filterDeck1() { filterDecks(true); }
void PlaylistComponent::filterDeck2() { filterDecks(false); }

// -- filterDecks --
// Filters tracks in the specified deck based on the search query.
void PlaylistComponent::filterDecks(bool isDeck1)
{
    juce::String query;
    std::vector<Track>* source;
    std::vector<Track>* visible;

    // -- Determine which deck to filter --
    if (isDeck1)
    {
        query = searchBox1.getText().trim().toLowerCase();
        source = &deck1;
        visible = &visibleDeck1;
    }
    else
    {
        query = searchBox2.getText().trim().toLowerCase();
        source = &deck2;
        visible = &visibleDeck2;
    }

    visible->clear();
    for (const auto& track : *source)
    {
        juce::String titleLower = juce::String(track.title).toLowerCase();
        if (titleLower.contains(query))
            visible->push_back(track);
    }

    // -- Update table component --
    if (isDeck1)
    {
        model1.updateTrackList(&visibleDeck1);
        tableComponent1.updateContent();
    }
    else
    {
        model2.updateTrackList(&visibleDeck2);
        tableComponent2.updateContent();
    }
}

//==============================================================================
// File Dialog and Track Management
//==============================================================================

// -- openAddFileDialog --
// Opens a file chooser dialog to add a track to the specified deck.
void PlaylistComponent::openAddFileDialog(bool isDeck1, int rowNumber)
{
    juce::String prompt = "Select a Track to add";
    addFileChooser = std::make_unique<juce::FileChooser>(
        prompt,
        lastDirectory,
        "*"
    );
    addFileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, isDeck1, rowNumber, prompt](const juce::FileChooser& fc)
        {
            juce::File file = fc.getResult();
            if (file.existsAsFile())
            {
                lastDirectory = file.getParentDirectory();

                if (!isAudioFile(file))
                {
                    DBG("Error: File " << file.getFileName() << " is not a valid audio format. " << prompt);
                    return;
                }

                auto title = file.getFileName().toStdString();
                std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
                std::string duration;
                if (reader != nullptr)
                {
                    double seconds = reader->lengthInSamples / reader->sampleRate;
                    duration = secondsToMinSec(seconds).toStdString();
                }
                else
                {
                    duration = "Unknown";
                }
                addTrackToDeck(isDeck1, rowNumber, title, duration, juce::URL{ file });
                saveLibrary();
            }
            addFileChooser.reset();
        }
    );
}

// -- addTrackToDeck --
// Adds a track to the specified deck at the given row.
void PlaylistComponent::addTrackToDeck(bool isDeck1, int rowNumber, const std::string& title,
                                       const std::string& duration, const juce::URL& fileURL)
{
    Track track { title, duration, fileURL };

    // -- Update selected deck --
    if (isDeck1)
    {
        if (rowNumber >= 0 && rowNumber < static_cast<int>(deck1.size()))
        {
            deck1[rowNumber] = track;
            visibleDeck1[rowNumber] = track;
            tableComponent1.updateContent();
        }
    }
    else
    {
        if (rowNumber >= 0 && rowNumber < static_cast<int>(deck2.size()))
        {
            deck2[rowNumber] = track;
            visibleDeck2[rowNumber] = track;
            tableComponent2.updateContent();
        }
    }
    saveLibrary();
}

// -- loadTrackFromPlaylist --
// Loads a track from the specified deck.
void PlaylistComponent::loadTrackFromPlaylist(bool isDeck1, int rowNumber)
{
    juce::URL fileURL;
    // -- Determine which deck to load from --
    if (isDeck1)
    {
        if (rowNumber >= 0 && rowNumber < static_cast<int>(deck1.size()))
            fileURL = deck1[rowNumber].url;
        if (fileURL.isEmpty()) return;

        if (loadDeck1Callback)
            loadDeck1Callback(fileURL);
    }
    else
    {
        if (rowNumber >= 0 && rowNumber < static_cast<int>(deck2.size()))
            fileURL = deck2[rowNumber].url;
        if (fileURL.isEmpty()) return;

        if (loadDeck2Callback)
            loadDeck2Callback(fileURL);
    }
}

//==============================================================================
// Library Persistence
//==============================================================================

// -- saveLibrary --
// Saves the current state of both decks to an XML file.
void PlaylistComponent::saveLibrary()
{
    auto root = std::make_unique<juce::XmlElement>("Playlist");

    // -- Save Deck1 --
    auto deck1Elem = std::make_unique<juce::XmlElement>("Deck1");
    for (const auto& track : deck1)
    {
        auto* trackElem = new juce::XmlElement("Track");
        trackElem->setAttribute("title",    track.title);
        trackElem->setAttribute("duration", track.duration);
        trackElem->setAttribute("url",      track.url.toString(false));
        deck1Elem->addChildElement(trackElem);
    }
    root->addChildElement(deck1Elem.release());

    // -- Save Deck2 --
    auto deck2Elem = std::make_unique<juce::XmlElement>("Deck2");
    for (const auto& track : deck2)
    {
        auto* trackElem = new juce::XmlElement("Track");
        trackElem->setAttribute("title",    track.title);
        trackElem->setAttribute("duration", track.duration);
        trackElem->setAttribute("url",      track.url.toString(false));
        deck2Elem->addChildElement(trackElem);
    }
    root->addChildElement(deck2Elem.release());

    juce::File libraryFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("OtoDecks")
                               .getChildFile("playlist.xml");

    libraryFile.getParentDirectory().createDirectory();
    juce::String xmlString = root->toString();
    libraryFile.replaceWithText(xmlString, false, false);
}

// -- loadLibrary --
// Loads the deck state from the XML file and ensures each deck has 6 tracks.
void PlaylistComponent::loadLibrary()
{
    juce::File libraryFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("OtoDecks")
                               .getChildFile("playlist.xml");
    if (libraryFile.existsAsFile())
    {
        std::unique_ptr<juce::XmlElement> root(juce::XmlDocument::parse(libraryFile));
        if (root != nullptr && root->hasTagName("Playlist"))
        {
            deck1.clear();
            deck2.clear();

            // -- Load Deck1 --
            if (auto* deck1Elem = root->getChildByName("Deck1"))
            {
                for (int i = 0; i < deck1Elem->getNumChildElements(); ++i)
                {
                    auto* trackElem = deck1Elem->getChildElement(i);
                    if (trackElem->hasTagName("Track"))
                    {
                        Track t;
                        t.title    = trackElem->getStringAttribute("title", "Add Track").toStdString();
                        t.duration = trackElem->getStringAttribute("duration", "0:00").toStdString();
                        t.url      = juce::URL(trackElem->getStringAttribute("url"));
                        deck1.push_back(t);
                    }
                }
            }

            // -- Load Deck2 --
            if (auto* deck2Elem = root->getChildByName("Deck2"))
            {
                for (int i = 0; i < deck2Elem->getNumChildElements(); ++i)
                {
                    auto* trackElem = deck2Elem->getChildElement(i);
                    if (trackElem->hasTagName("Track"))
                    {
                        Track t;
                        t.title    = trackElem->getStringAttribute("title", "Add Track").toStdString();
                        t.duration = trackElem->getStringAttribute("duration", "0:00").toStdString();
                        t.url      = juce::URL(trackElem->getStringAttribute("url"));
                        deck2.push_back(t);
                    }
                }
            }
        }
    }

    // -- Ensure each deck has 6 tracks --
    while (deck1.size() < 6)
        deck1.push_back(Track{"Add Track", "0:00", juce::URL()});
    while (deck2.size() < 6)
        deck2.push_back(Track{"Add Track", "0:00", juce::URL()});

    visibleDeck1 = deck1;
    visibleDeck2 = deck2;
}

//==============================================================================
// Table Model Functions
//==============================================================================

// -- getNumRows --
// Returns the number of rows in the track list.
int PlaylistComponent::Model::getNumRows()
{
    return static_cast<int>(trackPtr->size());
}

// -- paintRowBackground --
// Paints the background for a table row, highlighting it if selected.
void PlaylistComponent::Model::paintRowBackground(juce::Graphics& g, int /*rowNumber*/, int width, int height, bool rowIsSelected)
{
    g.fillAll(rowIsSelected ? juce::Colour::fromRGB(138, 43, 226)
                            : juce::Colour::fromRGB(30, 30, 50));
}

// -- paintCell --
// Paints the content of a table cell based on the column.
void PlaylistComponent::Model::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(trackPtr->size()))
        return;

    g.setColour(juce::Colours::white);

    // -- Column 1: Serial Number --
    if (columnId == 1)
    {
        juce::String sNo = juce::String(rowNumber + 1);
        g.drawText(sNo, 2, 0, width - 4, height, juce::Justification::centred, true);
    }
    // -- Column 2: Title --
    else if (columnId == 2)
    {
        juce::String title = juce::String((*trackPtr)[rowNumber].title);
        g.drawText(title, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    // -- Column 3: Duration --
    else if (columnId == 3)
    {
        juce::String duration = juce::String((*trackPtr)[rowNumber].duration);
        g.drawText(duration, 2, 0, width - 4, height, juce::Justification::centred, true);
    }
    g.setColour(juce::Colour(0xFFA8ADAF));
    g.drawLine(width - 1, 0.0f, width - 1, static_cast<float>(height), 0.8f);
}

// -- refreshComponentForCell --
// Provides a custom component for cells in the Actions column.
juce::Component* PlaylistComponent::Model::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate)
{
    // -- Column 4: Action Buttons --
    if (columnId == 4)
    {
        if (existingComponentToUpdate == nullptr)
        {
            auto* comp = new ActionButtonsComponent(rowNumber, deckFlag, parentPlaylist);
            existingComponentToUpdate = comp;
        }
    }
    else
    {
        delete existingComponentToUpdate;
        existingComponentToUpdate = nullptr;
    }
    return existingComponentToUpdate;
}

//==============================================================================
// ActionButtonsComponent Implementation
//==============================================================================

// -- Constructor --
// Sets up "Add" and "Load" buttons for the track row.
PlaylistComponent::ActionButtonsComponent::ActionButtonsComponent(int rowNumber, bool isDeck1, PlaylistComponent* parent)
    : rowID(rowNumber), belongsToDeck1(isDeck1), parentPlaylist(parent)
{
    // -- Add Button --
    addButton.setButtonText("Add");
    addButton.setLookAndFeel(parentPlaylist->getPlaylistButtonLookAndFeel());
    addButton.addListener(this);
    addAndMakeVisible(addButton);

    // -- Load Button --
    loadButton.setButtonText("Load");
    loadButton.setLookAndFeel(parentPlaylist->getPlaylistButtonLookAndFeel());
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);
}

// -- resized --
// Arranges the "Add" and "Load" buttons side-by-side within the available area.
void PlaylistComponent::ActionButtonsComponent::resized()
{
    auto area = getLocalBounds();
    int halfWidth = area.getWidth() / 2;
    addButton.setBounds(area.removeFromLeft(halfWidth).reduced(2));
    loadButton.setBounds(area.reduced(2));
}

// -- buttonClicked --
// Handles click events: opens file dialog on "Add" or loads track on "Load".
void PlaylistComponent::ActionButtonsComponent::buttonClicked(juce::Button* b)
{
    if (b == &addButton)
    {
        if (parentPlaylist != nullptr)
            parentPlaylist->openAddFileDialog(belongsToDeck1, rowID);
    }
    else if (b == &loadButton)
    {
        if (parentPlaylist != nullptr)
            parentPlaylist->loadTrackFromPlaylist(belongsToDeck1, rowID);
    }
}
