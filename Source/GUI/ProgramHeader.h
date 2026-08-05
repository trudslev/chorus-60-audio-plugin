#pragma once

#include "../PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// The program section (CHORUS60-GUI-SPEC.md section 6): "Contract is identical to Gatecrasher
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

    int displayedProgramIndex = -1;
    juce::String displayedProgramName;
    bool displayedIsFactory = true;

    bool namingMode = false;
    juce::String typedName;

    HeaderButton pressedButton = HeaderButton::none;

    juce::Rectangle<float> saveButtonRect, deleteButtonRect, headerClusterRect;
    juce::Rectangle<float> tagCellRect, nameCellRect, inWindowRect, outWindowRect;
};
