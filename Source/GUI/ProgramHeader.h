#pragma once

#include "../PluginProcessor.h"
#include "Chorus60MenuLookAndFeel.h"
#include "Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

// The program section (GUI-SPEC.md section 6): "Contract is identical to Gatecrasher
// section 6" per the spec's own words - ported directly from Gatecrasher's ProgramHeader, re-pointed
// at this plugin's own coordinates (section 6's table), its own three header-state bitmaps
// (header-factory-program@3x.png / header-user-program@3x.png / header-name-entry@3x.png), and
// saveNewUserProgram()/deleteUserProgram() on Chorus60AudioProcessor. "Current program" isn't an
// APVTS parameter, so this polls getCurrentProgram()/getProgramName()/isFactoryProgram() on a timer
// to stay in sync with host-driven program changes, same pattern as Gatecrasher's own.
//
// One addition beyond a straight port: Chorus-60's header row also carries the IN/OUT LED meter
// windows (section 6's table) - folded into this component rather than split into a separate
// LevelReadout class, since they sit inside the same blitted header-cluster crop right next to the
// tag/name cells and need the same poll-and-redraw treatment.
//
// Sized to the full canvas (matching ModScope/Chorus60PanelBackground's convention for absolute-
// coordinate overlays), with hitTest narrowed to just the header cluster's own bounds so it doesn't
// swallow clicks meant for the knobs/buttons elsewhere on the panel.
class ProgramHeader final : public juce::Component, private juce::Timer
{
public:
    explicit ProgramHeader(Chorus60AudioProcessor& processor);
    ~ProgramHeader() override;

    void paint(juce::Graphics&) override;
    bool hitTest(int x, int y) override;
    void focusLost (FocusChangeType) override;

    /** Section 5's parameter readout: while a control is being moved the name cell shows
        `NAME: VALUE UNIT` in #FFD9A0, reverting to the program name 900 ms after the gesture ends.

        **The CALLER guards on the control's own drag state.** A SliderAttachment also fires when a
        Program is applied and on every host automation step, and without that guard the display
        latches onto whichever parameter was written last and flickers for the length of a song.
        Naming mode wins over both - the glass belongs to the name field until it commits or
        cancels. */
    void showParameter(const juce::RangedAudioParameter& param);
    void releaseParameter();

    /** The component the Program list is laid out inside. Its bounds become the list's parent area,
        which is what fixes the list's top edge and caps its height - layout, not plumbing. Passing
        nullptr returns the list to being a free desktop window sized to its own content, which for
        a long bank overhangs the panel. See ../../CLAUDE.md, "The Program dropdown". */
    void setMenuParent(juce::Component* parent) noexcept { menuParent = parent; }

    /** The row the list's top edge lands on: the program window's own bottom edge, so the two read
        as one object rather than a bar with a list floating under it. */
    static int menuAnchorY() noexcept
    {
        return (int) std::floor(Chorus60Theme::Layout::programWindowY
                                + Chorus60Theme::Layout::programWindowH);
    }

    /** Where menuHost has to start, and it is NOT the anchor: JUCE clamps a menu to
        `jmax(parentArea.getY() + 1, ...)`, so a host beginning exactly at the anchor can only open
        one pixel below it, leaving a hairline of panel between the bar and its list.

        The lead has a floor and a ceiling. Too small and the clamp bites again; too large and the
        list can grow past the panel, because JUCE sizes it to `parentArea.getHeight() - 24` while
        the room actually below the anchor is the window's own height less than that. */
    static int menuHostTop() noexcept { return menuAnchorY() - 8; }

    void mouseDown(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;

private:
    enum class HeaderButton { none, save, deleteOrCancel };

    void timerCallback() override;
    void refreshDisplayFromProcessor();
    void enterNamingMode();
    void commitStore();
    void cancelNaming();
    void showProgramMenu();
    bool isProgramMenuAvailableAt(juce::Point<float>) const;
    HeaderButton buttonAt(juce::Point<float>) const;
    bool isButtonEnabled(HeaderButton) const;

    Chorus60AudioProcessor& processorRef;

    juce::Component* menuParent = nullptr;
    bool menuOpen = false;

    // Mirrors whatever program was loaded before SAVE was pressed - CANCEL reverts the display to
    // this without ever touching APVTS (the user's tweaked-but-unsaved knob values must survive a
    // Cancel, per section 6). Never written to while namingMode is true.
    // Polled alongside the program index rather than read straight from the processor inside
    // paint()/isButtonEnabled(): it changes on any parameter move, from the GUI or from host
    // automation, so it needs the same repaint-on-change handling the program index gets.
    bool displayedIsModified = false;

    // Tracks a press that began on the program-name cell, so releasing outside it (a drag-off)
    // cancels rather than opening the menu - the same press/release contract the buttons use.
    bool pressedNameCell = false;

    /** The Program the panel is showing, mirrored so the poll only repaints on a real change. An
        identity, not a position - so a bank that changed underneath cannot make this name the
        wrong sound. */
    ProgramId displayedId;

    /** The Programs the open menu was built from, in row order. */
    std::vector<ProgramId> menuRows;

    bool namingMode = false;
    juce::String typedName;

    HeaderButton pressedButton = HeaderButton::none;

    // Section 5's live parameter readout. Empty = showing the program name.
    juce::String liveReadout;
    juce::uint32 readoutRevertAtMs = 0;

    // Dresses the dropdown as an extension of the PROGRAM glass. Owned here so it outlives every
    // menu this component opens.
    Chorus60MenuLookAndFeel menuLookAndFeel;

    juce::Rectangle<float> saveButtonRect, deleteButtonRect, headerClusterRect, programWindowRect;
    juce::Rectangle<float> tagCellRect, nameCellRect, inWindowRect, outWindowRect;
};
