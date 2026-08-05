#include "ProgramHeader.h"
#include "Chorus60Theme.h"

namespace
{
    // "Signed dBFS to one decimal" (section 6) - explicit +/- sign, unlike the knob value labels'
    // Hz/%/ms formatting in Chorus60Theme::formatParameterValue.
    //
    // Below the floor the number stops meaning anything and starts overflowing the 54px window
    // ("-100.0" is six glyphs of 13px mono, wider than the well), so it collapses to -INF. Same
    // convention as Gatecrasher's own IN/OUT windows, and as this panel's scope annotations.
    juce::String formatMeterDb(float db)
    {
        if (db <= Chorus60Theme::Layout::meterFloorDb)
            return "-INF";

        return (db >= 0.0f ? "+" : "") + juce::String(db, 1);
    }
}

ProgramHeader::ProgramHeader(Chorus60AudioProcessor& processor) : processorRef(processor)
{
    using namespace Chorus60Theme::Layout;

    saveButtonRect = {saveButtonX, saveButtonY, saveButtonW, saveButtonH};
    deleteButtonRect = {deleteButtonX, deleteButtonY, deleteButtonW, deleteButtonH};
    headerClusterRect = {headerCropX, headerCropY, headerCropW, headerCropH};
    tagCellRect = {programTagCellX, programTagCellY, programTagCellW, programTagCellH};
    nameCellRect = {programNameCellX, programNameCellY, programNameCellW, programNameCellH};
    inWindowRect = {inWindowX, inWindowY, inWindowW, inWindowH};
    outWindowRect = {outWindowX, outWindowY, outWindowW, outWindowH};

    displayedProgramIndex = processorRef.getCurrentProgram();
    displayedProgramName = processorRef.getProgramName(displayedProgramIndex);
    displayedIsFactory = processorRef.isFactoryProgram(displayedProgramIndex);
    displayedIsModified = processorRef.isCurrentProgramModified();

    setWantsKeyboardFocus(true);
    startTimerHz(20); // comfortably exceeds section 6's "~6Hz" IN/OUT update requirement
}

ProgramHeader::~ProgramHeader()
{
    stopTimer();
}

bool ProgramHeader::hitTest(int x, int y)
{
    return headerClusterRect.contains((float) x, (float) y);
}

void ProgramHeader::timerCallback()
{
    refreshDisplayFromProcessor();

    // Unlike Gatecrasher's own ProgramHeader (which only repaints on a program change or naming
    // caret blink), this one repaints unconditionally every tick - the IN/OUT meters need
    // continuous redraw regardless of whether the current program changed.
    repaint();
}

void ProgramHeader::refreshDisplayFromProcessor()
{
    const int index = processorRef.getCurrentProgram();
    if (index != displayedProgramIndex)
    {
        displayedProgramIndex = index;
        displayedProgramName = processorRef.getProgramName(index);
        displayedIsFactory = processorRef.isFactoryProgram(index);
    }

    // Naming mode shows STORE, which stays enabled regardless of whether anything was modified (its
    // own typed value is what's being stored), so the modified flag isn't tracked while naming and
    // isButtonEnabled short-circuits on namingMode before ever reading it.
    if (! namingMode)
        displayedIsModified = processorRef.isCurrentProgramModified();
}

// The name cell is only a menu trigger while idle - during name entry it's the text field being
// typed into, so clicking it must not replace the half-typed name with a program list.
bool ProgramHeader::isProgramMenuAvailableAt(juce::Point<float> position) const
{
    return ! namingMode && nameCellRect.contains(position);
}

void ProgramHeader::showProgramMenu()
{
    const int numPrograms = processorRef.getNumPrograms();
    const int currentIndex = processorRef.getCurrentProgram();

    // Item IDs are index + 1 because PopupMenu reserves 0 for "dismissed without choosing" - which
    // also happens to match the 1-based bank numbering the LCD shows.
    juce::PopupMenu menu;
    bool hasUserPrograms = false;

    menu.addSectionHeader("Factory");
    for (int i = 0; i < numPrograms; ++i)
    {
        if (processorRef.isFactoryProgram(i))
            menu.addItem(i + 1, juce::String(i + 1).paddedLeft('0', 2) + "  " + processorRef.getProgramName(i),
                          true, i == currentIndex);
        else
            hasUserPrograms = true;
    }

    if (hasUserPrograms)
    {
        menu.addSeparator();
        menu.addSectionHeader("User");
        for (int i = 0; i < numPrograms; ++i)
            if (! processorRef.isFactoryProgram(i))
                menu.addItem(i + 1, juce::String(i + 1).paddedLeft('0', 2) + "  " + processorRef.getProgramName(i),
                              true, i == currentIndex);
    }

    menu.showMenuAsync(juce::PopupMenu::Options()
                           .withTargetComponent(this)
                           .withTargetScreenArea(localAreaToGlobal(nameCellRect.getSmallestIntegerContainer())),
                       [safeThis = juce::Component::SafePointer<ProgramHeader>(this)](int result)
                       {
                           if (safeThis == nullptr || result == 0)
                               return;

                           // Goes through ProgramManager's async apply path - the timerCallback
                           // picks the change up and repaints, so no forced refresh here.
                           safeThis->processorRef.setCurrentProgram(result - 1);
                       });
}

void ProgramHeader::mouseMove(const juce::MouseEvent& e)
{
    // Position-dependent, so it can't be a one-off setMouseCursor in the constructor: this component
    // spans the whole canvas and only the one cell is clickable.
    setMouseCursor(isProgramMenuAvailableAt(e.position) ? juce::MouseCursor::PointingHandCursor
                                                        : juce::MouseCursor::NormalCursor);
}

ProgramHeader::HeaderButton ProgramHeader::buttonAt(juce::Point<float> position) const
{
    if (saveButtonRect.contains(position))
        return HeaderButton::save;
    if (deleteButtonRect.contains(position))
        return HeaderButton::deleteOrCancel;
    return HeaderButton::none;
}

bool ProgramHeader::isButtonEnabled(HeaderButton button) const
{
    if (namingMode)
        return button == HeaderButton::save || button == HeaderButton::deleteOrCancel; // STORE/CANCEL, both enabled

    if (button == HeaderButton::save)
        return displayedIsModified; // nothing changed since the program loaded = nothing to save
    if (button == HeaderButton::deleteOrCancel)
        return !displayedIsFactory; // DELETE disabled for read-only factory programs
    return false;
}

void ProgramHeader::mouseDown(const juce::MouseEvent& e)
{
    pressedNameCell = isProgramMenuAvailableAt(e.position);

    const auto candidate = buttonAt(e.position);
    pressedButton = isButtonEnabled(candidate) ? candidate : HeaderButton::none;
    if (pressedButton != HeaderButton::none)
        repaint();
}

void ProgramHeader::mouseUp(const juce::MouseEvent& e)
{
    if (pressedNameCell)
    {
        pressedNameCell = false;
        if (isProgramMenuAvailableAt(e.position))
        {
            showProgramMenu();
            return;
        }
    }

    const auto released = buttonAt(e.position);
    if (released != HeaderButton::none && released == pressedButton)
    {
        if (released == HeaderButton::save)
        {
            if (namingMode)
                commitStore();
            else
                enterNamingMode();
        }
        else // deleteOrCancel
        {
            if (namingMode)
                cancelNaming();
            else if (!displayedIsFactory)
                processorRef.deleteUserProgram(displayedProgramIndex);
        }
    }

    if (pressedButton != HeaderButton::none)
    {
        pressedButton = HeaderButton::none;
        repaint();
    }
}

void ProgramHeader::enterNamingMode()
{
    namingMode = true;
    typedName.clear();
    grabKeyboardFocus();
    repaint();
}

void ProgramHeader::commitStore()
{
    // Empty name -> ProgramManager's own "NEW PROGRAM" fallback handles it (section 6) - not
    // duplicated here.
    processorRef.saveNewUserProgram(typedName.trim());

    namingMode = false;
    typedName.clear();
    giveAwayKeyboardFocus();

    // saveNewUserProgram applies synchronously (see ProgramManager::saveNewUserProgram), so the
    // new program is already current by the time this returns.
    refreshDisplayFromProcessor();
    repaint();
}

void ProgramHeader::cancelNaming()
{
    // Must NOT touch APVTS parameters - the user's tweaked-but-unsaved knob values survive a
    // Cancel (section 6). displayedProgramIndex/Name/IsFactory were never written to while naming,
    // so simply leaving naming mode reverts the display to whatever was loaded before SAVE.
    namingMode = false;
    typedName.clear();
    giveAwayKeyboardFocus();
    repaint();
}

bool ProgramHeader::keyPressed(const juce::KeyPress& key)
{
    if (!namingMode)
        return false;

    if (key.isKeyCode(juce::KeyPress::returnKey))
    {
        commitStore();
        return true;
    }
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        cancelNaming();
        return true;
    }
    if (key.isKeyCode(juce::KeyPress::backspaceKey))
    {
        if (typedName.isNotEmpty())
            typedName = typedName.dropLastCharacters(1);
        repaint();
        return true;
    }

    const juce::juce_wchar character = key.getTextCharacter();
    if (character >= 32 && character != 127
        && typedName.length() < Chorus60Theme::Layout::maxProgramNameLength)
    {
        typedName += juce::String::charToString(character).toUpperCase();
        repaint();
        return true;
    }

    return false;
}

void ProgramHeader::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;
    using namespace Chorus60Theme::Layout;

    // The background plate provides the empty PROGRAM / IN / OUT wells (their frames and recesses);
    // everything inside them, and both buttons, are drawn here.
    //
    // An earlier version instead composited design/assets/header-{factory,user,name-entry}@3x.png
    // over this region as a second bitmap layer, taking the window frames and the SAVE/DELETE faces
    // from whichever of the three matched the current state. That was abandoned for the same reason
    // it was in Gatecrasher: those bitmaps' internal coordinate frame isn't calibrated to the panel's
    // own, so every rect computed from the spec's (correct) coordinates landed slightly off the
    // bitmap's content, and each one carried a baked example program name and meter reading that had
    // to be covered before the live values could be drawn. Drawing the whole cluster live needs no
    // second coordinate system to agree with anything.
    //
    // Cell interiors are cleared first so the previous frame's text doesn't accumulate - inset 1px
    // so the plate's own window borders stay intact.
    g.setColour(Colour::ledWindowBg);
    g.fillRect(tagCellRect.reduced(1.0f));
    g.fillRect(nameCellRect.reduced(1.0f));
    g.fillRect(inWindowRect.reduced(1.0f));
    g.fillRect(outWindowRect.reduced(1.0f));

    const bool showUserTag = namingMode || !displayedIsFactory;
    g.setColour(showUserTag ? Colour::tagUser : Colour::tagFactory);
    g.setFont(monoFont(9.0f));
    g.drawText(showUserTag ? "USER" : "FACT", tagCellRect, juce::Justification::centred, false);

    g.setColour(Colour::headerName);
    g.setFont(monoFont(13.0f));
    if (namingMode)
    {
        // Left-aligned, cleared, with a blinking block caret (1s period, 50% duty - section 6).
        const bool caretOn = (juce::Time::getMillisecondCounter() % 1000) < 500;
        const juce::String text = typedName + (caretOn ? juce::String(juce::CharPointer_UTF8("\xe2\x96\x88"))
                                                         : juce::String());
        g.drawText(text, nameCellRect.reduced(6.0f, 0.0f), juce::Justification::centredLeft, false);
    }
    else
    {
        // "NN NAME" - two-digit program number, space, name (section 6). The number is 1-based, so
        // the first factory program reads "01 EIGHTY-TWO" rather than "00": it's a bank position a
        // player reads off the panel and counts from one, not the zero-based index the code uses.
        // User programs continue the same run past the factory bank (10, 11, ...).
        const juce::String indexText = juce::String(displayedProgramIndex + 1).paddedLeft('0', 2);
        g.drawText(indexText + " " + displayedProgramName, nameCellRect, juce::Justification::centred, false);

        // Small chevron marking the cell as a menu trigger. Dimmed and tucked into the right edge so
        // it reads as an affordance rather than competing with the program name; the name stays
        // centred in the full cell, and the widest name this bank can show still ends clear of it.
        const float chevronW = 7.0f, chevronH = 4.0f;
        const float chevronRight = nameCellRect.getRight() - 8.0f;
        const float chevronTop = nameCellRect.getCentreY() - chevronH * 0.5f;

        juce::Path chevron;
        chevron.startNewSubPath(chevronRight - chevronW, chevronTop);
        chevron.lineTo(chevronRight - chevronW * 0.5f, chevronTop + chevronH);
        chevron.lineTo(chevronRight, chevronTop);

        g.setColour(Colour::ledWindowText.withAlpha(0.55f));
        g.strokePath(chevron, juce::PathStrokeType(1.0f));
    }

    // IN / OUT LED windows: signed dBFS to one decimal, numeric only (section 6).
    g.setColour(Colour::ledWindowText);
    g.setFont(monoFont(13.0f));
    g.drawText(formatMeterDb(processorRef.getInputMeterDb()), inWindowRect, juce::Justification::centred, false);
    g.drawText(formatMeterDb(processorRef.getOutputMeterDb()), outWindowRect, juce::Justification::centred, false);

    // SAVE / DELETE (STORE / CANCEL while naming), drawn live per section 6's state table.
    auto drawButton = [&] (juce::Rectangle<float> rect, const juce::String& label, bool enabled, bool pressed)
    {
        const auto top = pressed ? Colour::buttonPressedTop
                                  : (enabled ? Colour::buttonEnabledTop : Colour::buttonDisabledTop);
        const auto bottom = pressed ? Colour::buttonPressedBottom
                                     : (enabled ? Colour::buttonEnabledBottom : Colour::buttonDisabledBottom);

        g.setGradientFill({top, rect.getX(), rect.getY(), bottom, rect.getX(), rect.getBottom(), false});
        g.fillRect(rect);

        g.setColour(enabled ? Colour::buttonEnabledBorder : Colour::buttonDisabledBorder);
        g.drawRect(rect, 1.0f);

        // Section 6: Barlow Condensed 600, 9px, .12em tracking.
        drawTrackedText(g, label, labelFont(labelFontHeightForCssPx(9.0f)), trackingPxForEm(0.12f, 9.0f),
                         rect, juce::Justification::centred,
                         enabled ? Colour::buttonEnabledLabel : Colour::buttonDisabledLabel);
    };

    drawButton(saveButtonRect, namingMode ? "STORE" : "SAVE",
                isButtonEnabled(HeaderButton::save), pressedButton == HeaderButton::save);
    drawButton(deleteButtonRect, namingMode ? "CANCEL" : "DELETE",
                isButtonEnabled(HeaderButton::deleteOrCancel), pressedButton == HeaderButton::deleteOrCancel);
}
