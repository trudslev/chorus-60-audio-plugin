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
    // The cluster hitTest claims: the program window plus both buttons. Derived from section 5's
    // own rects rather than from a bitmap crop - the header-state PNGs that crop existed for were
    // abandoned (see paint), and a rect kept in step with nothing was free to drift.
    programWindowRect = {programWindowX, programWindowY, programWindowW, programWindowH};
    headerClusterRect = programWindowRect
                            .getUnion({saveButtonX, saveButtonY, saveButtonW, saveButtonH})
                            .getUnion({deleteButtonX, deleteButtonY, deleteButtonW, deleteButtonH});
    tagCellRect = {programTagCellX, programTagCellY, programTagCellW, programTagCellH};
    nameCellRect = {programNameCellX, programNameCellY, programNameCellW, programNameCellH};
    inWindowRect = {inWindowX, inWindowY, inWindowW, inWindowH};
    outWindowRect = {outWindowX, outWindowY, outWindowW, outWindowH};

    displayedId = processorRef.getProgramManager().getCurrentProgramId();
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
    // The live readout reverts on its own clock rather than a second timer (section 5: held 900 ms
    // after release, then the program name returns).
    if (readoutRevertAtMs != 0 && juce::Time::getMillisecondCounter() >= readoutRevertAtMs)
    {
        readoutRevertAtMs = 0;
        liveReadout.clear();
    }

    refreshDisplayFromProcessor();

    // Unlike Gatecrasher's own ProgramHeader (which only repaints on a program change or naming
    // caret blink), this one repaints unconditionally every tick - the IN/OUT meters need
    // continuous redraw regardless of whether the current program changed.
    repaint();
}

void ProgramHeader::showParameter(const juce::RangedAudioParameter& param)
{
    using namespace Chorus60Theme;

    if (namingMode)
        return;   // the glass belongs to the name field until it commits or cancels

    // Straight through the parameter's own name and Chorus60Theme's formatter, so the LCD and the
    // host never disagree about what a control reads. Section 5's examples set the shape:
    // "DELAY CENTER I+II: 6.4 ms", "RATE I: 0.45 Hz", "IMAGE I: STEREO".
    const auto name = param.getName(Layout::lcdCharacterBudget).toUpperCase();
    const auto value = formatParameterValue(param, param.convertFrom0to1(param.getValue()));

    // The NAME is capitalised, the value is not. Section 5's examples set that split -
    // "DELAY CENTER I+II: 6.4 ms" and "RATE I: 0.45 Hz" keep their units as written, and the
    // IMAGE switch's MONO/STEREO already arrive upper-case from its own stringFromValue.
    const auto text = name + ": " + value;
    if (text != liveReadout)
    {
        liveReadout = text;
        repaint(nameCellRect.getSmallestIntegerContainer());
    }
    readoutRevertAtMs = 0;
}

void ProgramHeader::releaseParameter()
{
    using namespace Chorus60Theme;

    if (liveReadout.isNotEmpty())
        readoutRevertAtMs = juce::Time::getMillisecondCounter()
                                + (juce::uint32) Layout::lcdReadoutHoldMs;
}

void ProgramHeader::refreshDisplayFromProcessor()
{
    if (const auto id = processorRef.getProgramManager().getCurrentProgramId(); id != displayedId)
        displayedId = id;

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
    // Anywhere in the window, not just the name cell: the chevron is a static affordance marking
    // the window as a selector, not a button of its own.
    return ! namingMode && programWindowRect.contains(position);
}

void ProgramHeader::showProgramMenu()
{
    auto& manager = processorRef.getProgramManager();
    const auto current = manager.getCurrentProgramId();

    juce::PopupMenu menu;
    menu.setLookAndFeel(&menuLookAndFeel);

    // **Row IDs are positions in THIS menu, not Program indices.** PopupMenu needs an int per row
    // and reserves 0 for "dismissed"; the callback maps the row back to the ProgramId it was built
    // from, so no Program is addressed by a bank position here.
    menuRows = manager.listPrograms();

    bool factoryHeaderDone = false;
    bool userHeaderDone = false;

    for (size_t i = 0; i < menuRows.size(); ++i)
    {
        const auto& id = menuRows[i];

        if (id.bank == ProgramBank::factory && ! std::exchange(factoryHeaderDone, true))
        {
            menu.addSeparator();
            menu.addSectionHeader("Factory");
        }

        // The User group is absent entirely when empty, header included.
        if (id.bank == ProgramBank::user && ! std::exchange(userHeaderDone, true))
        {
            menu.addSeparator();
            menu.addSectionHeader("User");
        }

        menu.addItem((int) i + 1, manager.displayLabelFor(id), true, id == current);
    }

    // Anchored to, and at least as wide as, the whole program window. localAreaToGlobal keeps this
    // right on a scaled or moved editor.
    const auto glassOnScreen = localAreaToGlobal(programWindowRect.getSmallestIntegerContainer());
    const auto window = programWindowRect.getSmallestIntegerContainer();

    auto options = juce::PopupMenu::Options()
                       .withTargetComponent(this)
                       .withTargetScreenArea(glassOnScreen)
                       .withMaximumNumColumns(1);

    if (menuParent != nullptr)
    {
        // The list is laid out INSIDE menuHost rather than as its own desktop window. JUCE fits a
        // menu to its parent area, so an area running from the window's bottom edge to the panel's
        // gives both guarantees at once: the top cannot move and the height cannot exceed the
        // panel. A bank too long to fit scrolls. See ../../CLAUDE.md, "The Program dropdown".
        //
        // Anchor to a 1px strip on the window's bottom EDGE, not the window. With a parent, JUCE
        // first does constrainedWithin(parentArea), which slides the whole 29px window down into
        // the host before measuring and opens the list 29px too low. 1px and not zero: a
        // zero-height rectangle is isEmpty(), which drops the list out of align-to-rectangle into
        // the sideways placement meant for submenus.
        const juce::Rectangle<int> anchor { window.getX(), menuAnchorY() - 1, window.getWidth(), 1 };

        options = options.withTargetScreenArea(localAreaToGlobal(anchor))
                         .withParentComponent(menuParent)
                         .withMinimumWidth(window.getWidth());
    }
    else
    {
        options = options.withMinimumWidth(glassOnScreen.getWidth());
    }

    menuOpen = true;
    repaint();

    menu.showMenuAsync(options,
                       [safeThis = juce::Component::SafePointer<ProgramHeader>(this)](int result)
                       {
                           if (safeThis == nullptr)
                               return;

                           // Cleared here rather than on selection: JUCE runs this callback on a
                           // dismissal too, so clicking away cannot leave the mark inverted.
                           safeThis->menuOpen = false;
                           safeThis->repaint();

                           if (result == 0)
                               return;

                           // Goes through ProgramManager's async apply path - the timerCallback
                           // picks the change up and repaints, so no forced refresh here.
                           const auto row = (size_t) (result - 1);

                           if (row < safeThis->menuRows.size())
                               safeThis->processorRef.getProgramManager()
                                   .requestProgramChange(safeThis->menuRows[row]);
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
        // Only a User Program can be deleted. INIT and an unresolved id are not stored things.
        return displayedId.bank == ProgramBank::user;
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
            else if (displayedId.bank == ProgramBank::user)
                processorRef.deleteUserProgram(displayedId);
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
    liveReadout.clear();
    readoutRevertAtMs = 0;

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
    // Cancel (section 6). displayedId was never written to while naming,
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

    // Section 5: FACT / USER is set in the SAME face, size, tracking and colour as the program
    // name. It sits inside a display, so it is display text - it is no longer dimmed relative to
    // the name the way revision 1 had it.
    const auto lcdFont = monoFont(monoFontHeightForCssPx(Layout::lcdCssPx));
    const float lcdTracking = trackingPxForEm(Layout::lcdTrackingEm, Layout::lcdCssPx);

    // **An em-dash where the Program is in neither bank** - INIT, or an unresolved identifier.
    const bool onInit = !namingMode && (displayedId.bank == ProgramBank::init
                                         || displayedId.bank == ProgramBank::unresolved);
    const bool showUserTag = namingMode || displayedId.bank == ProgramBank::user;
    const auto tagText = onInit ? juce::String::charToString((juce::juce_wchar) 0x2014)
                                : juce::String(showUserTag ? "USER" : "FACT");

    drawTrackedText(g, tagText, lcdFont, lcdTracking, tagCellRect,
                     juce::Justification::centred,
                     onInit ? Colour::ledWindowText.withAlpha(0.42f) : Colour::ledWindowText);

    if (namingMode)
    {
        // Left-aligned, cleared, with a blinking block caret (1s period, 50% duty).
        const bool caretOn = (juce::Time::getMillisecondCounter() % 1000) < 500;
        const juce::String text = typedName + (caretOn ? juce::String(juce::CharPointer_UTF8("\xe2\x96\x88"))
                                                         : juce::String());
        drawTrackedText(g, text, lcdFont, lcdTracking, nameCellRect.reduced(12.0f, 0.0f),
                         juce::Justification::left, Colour::ledWindowText);
    }
    else if (liveReadout.isNotEmpty())
    {
        // Section 5's second content: the parameter readout in #FFD9A0 while a control is being
        // moved. This is the panel's ONLY live numeric display - the standing per-knob readouts
        // revision 1 had under every knob are gone.
        //
        // CENTRED, where section 5 says left-aligned. Deliberate: the readout replaces the program
        // name in the same cell, and centring both means the text stays put as it swaps instead of
        // jumping to the left margin and back on every gesture. Only name ENTRY is left-aligned,
        // where it has to be, because a caret has to sit after the last character typed.
        drawTrackedText(g, liveReadout, lcdFont, lcdTracking, nameCellRect,
                         juce::Justification::centred, Colour::lcdParameterReadout);
    }
    else
    {
        // "NN NAME" - two-digit program number, space, name. The number is 1-based, so the first
        // factory program reads "01 EIGHTY-TWO" rather than "00": it's a bank position a player
        // reads off the panel and counts from one, not the zero-based index the code uses. User
        // programs continue the same run past the factory bank (10, 11, ...).
        const auto& manager = processorRef.getProgramManager();

        // A trailing " *" while the loaded Program has been edited, matching TapeRot and
        // Gatecrasher. It clears on store, on delete and on loading another Program - all three of
        // which reset displayedIsModified through refreshDisplayFromProcessor, so nothing extra is
        // needed here. Worst case is 27 characters of "NN " + a 24-char name plus 2 for the marker,
        // which is 29 of the field's 36.
        // An identifier the session named but the bank no longer has: the VALUES are correct and
        // untouched, only the name is unknown, so the panel says so rather than pretending. No
        // dirty asterisk either - there is no baseline to differ from.
        //
        // Otherwise the number is a label computed from the Factory position at paint time. INIT
        // and User Programs carry none.
        const juce::String shown =
            displayedId.bank == ProgramBank::unresolved
                ? displayedId.displayName + "?"
                : manager.displayLabelFor(displayedId) + (displayedIsModified ? " *" : "");

        // Centred in the field less its right padding, so the name stays clear of the chevron.
        drawTrackedText(g, shown, lcdFont, lcdTracking,
                         nameCellRect.withTrimmedRight(Layout::lcdNameRightPadding),
                         juce::Justification::centred, Colour::ledWindowText);

        // The chevron. Square caps and a bare two-segment path rather than a closed shape - it is a
        // stroke mark, not an arrowhead.
        const float right = nameCellRect.getRight() - Layout::lcdChevronInsetRight;
        const float left = right - Layout::lcdChevronW;
        const float top = nameCellRect.getCentreY() - Layout::lcdChevronH * 0.5f;

        // It inverts while the list is open, mirrored about the mark's own centre line rather than
        // rotated, so the apex stays on one vertical axis and it reads as flipping in place. Without
        // it the mark still points down at a list that is already down.
        const float outerY = menuOpen ? top + Layout::lcdChevronH : top;
        const float apexY = menuOpen ? top : top + Layout::lcdChevronH;

        juce::Path chevron;
        chevron.startNewSubPath(left, outerY);
        chevron.lineTo((left + right) * 0.5f, apexY);
        chevron.lineTo(right, outerY);

        g.setColour(Colour::ledWindowText.withAlpha(Layout::lcdChevronAlpha));
        g.strokePath(chevron, juce::PathStrokeType(Layout::lcdChevronStroke,
                                                    juce::PathStrokeType::mitered,
                                                    juce::PathStrokeType::square));
    }

    // IN / OUT LED windows: signed dBFS to one decimal, numeric only (section 6).
    const auto meterFont = monoFont(monoFontHeightForCssPx(13.0f));
    const float meterTracking = trackingPxForEm(0.06f, 13.0f);
    drawTrackedText(g, formatMeterDb(processorRef.getInputMeterDb()), meterFont, meterTracking,
                     inWindowRect, juce::Justification::centred, Colour::ledWindowText);
    drawTrackedText(g, formatMeterDb(processorRef.getOutputMeterDb()), meterFont, meterTracking,
                     outWindowRect, juce::Justification::centred, Colour::ledWindowText);

    // SAVE / STORE and DELETE / CANCEL, per section 13.
    //
    // **The face is baked; only the legends are drawn.** Each of the four legends lights
    // independently, so baking one would freeze that state's lighting into the bitmap - but the
    // face itself has no state to freeze, so it sits in the plate with the rest of the static
    // furniture. Nothing below fills or borders the button: painting a face here would put a live
    // control over a baked copy of itself.
    auto drawButton = [&] (juce::Rectangle<float> rect,
                           const juce::String& upperLegend, const juce::String& lowerLegend,
                           bool upperLit, bool lowerLit)
    {
        const auto font = labelFont(labelFontHeightForCssPx(Layout::legendCssPx));
        const float tracking = trackingPxForEm(Layout::legendTrackingEm, Layout::legendCssPx);

        const float lineH = Layout::legendLineHeight;
        const float blockTop = rect.getCentreY() - lineH;   // two 12px line boxes, centred as a pair

        auto legend = [&] (const juce::String& text, float y, bool lit)
        {
            const juce::Rectangle<float> line { rect.getX(), y, rect.getWidth(), lineH };

            if (lit)
            {
                // Section 13's bloom is deliberately WARM against the cool-neutral face: a warm
                // bloom reads as backlit where a neutral one reads as merely brighter ink, which is
                // the distinction the brand rule draws. It is not the accent - the engine reds and
                // yellows stay with I / II / I+II and appear nowhere in the header.
                //
                // JUCE has no text-shadow and no cheap blur for a string, so each radius is the
                // same tracked text drawn at eight points around a circle. Alphas are tuned, not
                // quoted: eight copies at alpha a reach 1-(1-a)^8 where they coincide.
                for (auto [radius, alpha, colour] : {
                         std::tuple { 3.5f, 0.055f, juce::Colour (0xFFFFE5BC) },
                         std::tuple { 1.0f, 0.070f, juce::Colour (0xFFFFF6E8) } })
                    for (int i = 0; i < 8; ++i)
                    {
                        const float angle = juce::MathConstants<float>::twoPi * (float) i / 8.0f;

                        drawTrackedText(g, text, font, tracking,
                                        line.translated(std::cos(angle) * radius,
                                                        std::sin(angle) * radius),
                                        juce::Justification::centred, colour.withAlpha(alpha));
                    }
            }

            drawTrackedText(g, text, font, tracking, line, juce::Justification::centred,
                            lit ? Colour::legendLit : Colour::legendUnlit);
        };

        const juce::Graphics::ScopedSaveState state(g);
        g.reduceClipRegion(rect.getSmallestIntegerContainer());

        legend(upperLegend, blockTop, upperLit);
        legend(lowerLegend, blockTop + lineH, lowerLit);
    };

    /*  Section 13's state table:

        | Panel state                 | SAVE | STORE | DELETE | CANCEL |
        | Factory Program, unmodified | dark | dark  | dark   | dark   |
        | Factory Program, modified   | LIT  | dark  | dark   | dark   |
        | User Program, unmodified    | dark | dark  | LIT    | dark   |
        | User Program, modified      | LIT  | dark  | LIT    | dark   |
        | Naming a Program            | dark | LIT   | dark   | LIT    |

        Escape or CANCEL out of naming leaves the Program still modified - nothing was stored, so
        SAVE lights again the moment naming exits. DELETE on a Factory Program is dark and inert,
        which is why that row exists rather than being folded into "unmodified". */
    drawButton(saveButtonRect, "SAVE", "STORE",
               ! namingMode && isButtonEnabled(HeaderButton::save), namingMode);
    drawButton(deleteButtonRect, "DELETE", "CANCEL",
               ! namingMode && isButtonEnabled(HeaderButton::deleteOrCancel), namingMode);
}
