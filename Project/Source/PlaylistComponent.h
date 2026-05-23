/*
  ==============================================================================
    PlaylistComponent.h
  ==============================================================================
    This header defines the PlaylistComponent, which manages the track playlist.
    It handles adding, filtering, and loading tracks into decks while providing a
    custom UI for track display and file management.
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>

// PlaylistComponent: Manages the track playlist, loading tracks into decks, and filtering.
class PlaylistComponent  : public juce::Component
{
public:
    // Public interface: construction, UI handling, and track management.
    PlaylistComponent();
    ~PlaylistComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Track loading and management.
    void openAddFileDialog(bool isDeck1, int rowNumber);
    void addTrackToDeck(bool isDeck1, int rowNumber, const std::string& title,
                        const std::string& duration, const juce::URL& fileURL);
    void loadTrackFromPlaylist(bool isDeck1, int rowNumber);

    // Callbacks for loading tracks into decks.
    std::function<void(const juce::URL&)> loadDeck1Callback;
    std::function<void(const juce::URL&)> loadDeck2Callback;

    // Custom LookAndFeel accessor.
    juce::LookAndFeel_V4* getPlaylistButtonLookAndFeel() { return &playlistButtonLookAndFeel; }

private:
    // Internal structure for storing track details.
    struct Track
    {
        std::string title;
        std::string duration;
        juce::URL   url;
    };

    // Library management: saving and loading playlists.
    void saveLibrary();
    void loadLibrary();

    // Filtering methods for both decks.
    void filterDeck1();
    void filterDeck2();
    void filterDecks(bool isDeck1);

    // Track storage.
    std::vector<Track> deck1;
    std::vector<Track> deck2;
    std::vector<Track> visibleDeck1;
    std::vector<Track> visibleDeck2;

    // UI components: tables for both decks.
    juce::TableListBox tableComponent1;
    juce::TableListBox tableComponent2;

    // Custom LookAndFeel for playlist buttons.
    class PlaylistButtonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        PlaylistButtonLookAndFeel();
        ~PlaylistButtonLookAndFeel() override;

        void drawButtonBackground (juce::Graphics& g,
                                   juce::Button& button,
                                   const juce::Colour& backgroundColour,
                                   bool isMouseOverButton,
                                   bool isButtonDown) override;
    };
    PlaylistButtonLookAndFeel playlistButtonLookAndFeel;

    // Table model for managing playlist entries.
    class Model : public juce::TableListBoxModel
    {
    public:
        Model(std::vector<Track>* tracks, bool isDeck1, PlaylistComponent* parent)
            : trackPtr(tracks), deckFlag(isDeck1), parentPlaylist(parent) {}

        int getNumRows() override;
        void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
        void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
        juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

        void updateTrackList(std::vector<Track>* newTracks) { trackPtr = newTracks; }

    private:
        std::vector<Track>* trackPtr;
        bool deckFlag;
        PlaylistComponent* parentPlaylist;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Model)
    };

    Model model1;
    Model model2;

    // Search and clear buttons for filtering the playlist.
    juce::TextEditor searchBox1;
    juce::TextButton clearButton1 { "X" };

    juce::TextEditor searchBox2;
    juce::TextButton clearButton2 { "X" };

    // Component for action buttons inside the playlist.
    class ActionButtonsComponent : public juce::Component,
                                   public juce::Button::Listener
    {
    public:
        ActionButtonsComponent(int rowNumber, bool isDeck1, PlaylistComponent* parentPlaylist);

        void resized() override;
        void buttonClicked(juce::Button* b) override;

    private:
        int rowID;
        bool belongsToDeck1;
        juce::TextButton addButton, loadButton;
        PlaylistComponent* parentPlaylist;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActionButtonsComponent)
    };

    // File handling.
    juce::AudioFormatManager formatManager;
    juce::File lastDirectory;
    std::unique_ptr<juce::FileChooser> addFileChooser;

    // Utility function to check for valid audio files.
    static bool isAudioFile(const juce::File& file);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};
