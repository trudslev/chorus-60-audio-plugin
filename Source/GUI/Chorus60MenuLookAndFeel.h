#pragma once

#include <nf/MenuMetrics.h>

#include "Chorus60Theme.h"

/**
    Dresses the Program dropdown in the panel's own language.

    The menu is the one piece of this GUI that cannot be a bitmap: its size depends on how many
    User Programs exist, so it has to be drawn. Left to JUCE's default it renders as a system-grey
    list that belongs to a different product entirely - the glass, the segment face and the display
    ink all stop at the edge of the LCD.

    So this paints the menu as an extension of the PROGRAM glass: the same near-black #07090A fill,
    the same #DFE6EA display ink, Share Tech Mono throughout, and the well's own #363C41 border
    reused as the rule. The lit pip is the red accent, matching the engine lamps - this plugin's
    accent is not reserved the way Gatecrasher's is, and a red pip reads as the same lit indicator
    the panel uses everywhere else. Nothing here invents a colour - every value comes from
    Chorus60Theme::Colour.
*/
class Chorus60MenuLookAndFeel final : public nf::MenuMetricsLookAndFeel
{
public:
    Chorus60MenuLookAndFeel()
        : nf::MenuMetricsLookAndFeel (metrics())
    {
        // Covers the few bits JUCE draws without asking us first (the drop shadow's backdrop).
        setColour (juce::PopupMenu::backgroundColourId, glass);
        setColour (juce::PopupMenu::textColourId, Chorus60Theme::Colour::ledWindowText);
        setColour (juce::PopupMenu::highlightedBackgroundColourId,
                   Chorus60Theme::Colour::chorusAccent.withAlpha (0.20f));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    }

protected:
    /** Plain, untracked measurement - this panel draws its rows with a plain run. */
    float measureMenuItemText (const juce::String& text) override
    {
        return juce::GlyphArrangement::getStringWidth (getPopupMenuFont(), text);
    }

public:
    juce::Font getPopupMenuFont() override
    {
        return Chorus60Theme::monoFont (Chorus60Theme::monoFontHeightForCssPx (itemTextSize));
    }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        const juce::Rectangle<float> r (0.0f, 0.0f, (float) width, (float) height);
        g.setColour (glass);
        g.fillRoundedRectangle (r, 3.0f);
        g.setColour (rule);
        g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);
    }


    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon,
                            const juce::Colour* textColourToUse) override
    {
        juce::ignoreUnused (hasSubMenu, shortcutKeyText, icon, textColourToUse);

        if (isSeparator)
        {
            auto line = area.toFloat().reduced (8.0f, 0.0f).withHeight (1.0f)
                            .withY (area.toFloat().getCentreY());
            g.setColour (rule);
            g.fillRect (line);
            return;
        }

        auto r = area.toFloat().reduced (3.0f, 1.0f);

        if (isHighlighted && isActive)
        {
            g.setColour (Chorus60Theme::Colour::chorusAccent.withAlpha (0.18f));
            g.fillRoundedRectangle (r, 2.0f);
        }

        // A filled amber pip rather than a tick glyph: the panel has no checkmark idiom anywhere,
        // but it is full of lit lamps and segments, so this reads as "this one is lit".
        if (isTicked)
        {
            const float d = 5.0f;
            g.setColour (Chorus60Theme::Colour::chorusAccent);
            g.fillEllipse (r.getX() + 7.0f, r.getCentreY() - d * 0.5f, d, d);
        }

        auto ink = isActive ? Chorus60Theme::Colour::ledWindowText
                            : Chorus60Theme::Colour::ledWindowText.withAlpha (0.35f);

        if (isHighlighted && isActive)
            ink = juce::Colours::white;

        Chorus60Theme::drawTrackedText (g, text, getPopupMenuFont(), tracking,
                                         r.withTrimmedLeft ((float) tickColumn),
                                         juce::Justification::left, ink);
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area,
                                     const juce::String& sectionName) override
    {
        auto r = area.toFloat().reduced (3.0f, 0.0f);

        // **Drawn as authored, never re-cased.** The caption is authored FACTORY/USER at the
        // addSectionHeader call in ProgramHeader.cpp, per BRAND.md's rule that case belongs at
        // the source. It held a .toUpperCase() here until 2026-08-13. Nothing else reads these
        // two strings today, which is a fact about today rather than a property of the code:
        // the moment a caption comes from data, the site that re-cases it is the site that
        // gets it wrong. Re-arguing the exception each time costs more than the rule.
        Chorus60Theme::drawTrackedText (g, sectionName,
                                         Chorus60Theme::monoFont (Chorus60Theme::monoFontHeightForCssPx (headerTextSize)),
                                         headerTracking,
                                         r.withTrimmedLeft ((float) tickColumn),
                                         juce::Justification::left,
                                         // **Opaque.** BRAND.md permits opacity for STATE and
                                         // forbids it for HIERARCHY; a section header is
                                         // hierarchy. Size and tracking already carry it.
                                         Chorus60Theme::Colour::ledWindowText);

        g.setColour (rule);
        g.fillRect (r.reduced (5.0f, 0.0f).withHeight (1.0f).withY (r.getBottom() - 1.0f));
    }

private:
    /** The PROGRAM glass itself (section 5), so the menu reads as the same pane of dark plastic. */
    const juce::Colour glass { 0xFF07090A };
    /** The well's own border, reused as the menu's rule. */
    const juce::Colour rule  { 0xFF363C41 };

    /*  **THESE ARE CSS px AND WERE BEING PASSED AS JUCE HEIGHTS — the type-scale finding, fixed.**

        Every other size in this casting goes through `monoFontHeightForCssPx` or
        `labelFontHeightForCssPx`, because a spec's `font-size` is an **em** size and
        `FontOptions(h)` sets **ascent + descent**. These three sites passed the constant straight
        to `monoFont`, so the same nominal number meant CSS px on the panel and a JUCE height in the
        dropdown. Measured through this casting's own converter (ratio 1.12700), the caption was
        rendering at **9.76** effective CSS px and the rows at **13.31** — the caption under
        BRAND.md's ~9-10 px functional floor, in a list a user reads to pick a Program.

        **The diagnosis was already written down, one function below, and the fix was not made.**
        `metrics()`'s comment says in terms that "this casting builds its mono type from a JUCE
        height, where those two pass a CSS px through withPointHeight, so the same constant produces
        a smaller line box", having predicted 19 for the caption line box and measured 18. Knowing
        why a figure is wrong is not the same as it being right, and the correction waited for the
        type pass because it moves the dropdown's whole scale.

        **The trackings stay ABSOLUTE PX and are not converted to em**, deliberately. They were
        chosen against the rendered size, and the rendered size is what changes here — so their
        ratio to the type does drop. No spec states a menu tracking, and inventing an em figure to
        preserve a ratio nobody authored would be a figure with nothing behind it. Left as measured
        values with this note, for the designers rather than for a refactor. */
    static constexpr float itemTextSize   = 15.0f;   // CSS px
    static constexpr float tracking       = 1.0f;    // absolute px, see above
    static constexpr float headerTextSize = 11.0f;   // CSS px
    static constexpr float headerTracking = 1.6f;    // absolute px
    /** **This panel's dropdown sizes, stated rather than inherited.**

        `sectionHeaderHeight` is 36 because that is what this panel has ALWAYS drawn - JUCE's
        `LookAndFeel_V2` sizes a section caption at the row height plus half again, and this casting
        took that default by omission. It is written down now so it is a number somebody can see and
        argue with; **it is deliberately not changed here**, because altering it moves every row
        below FACTORY and that is the designers' call, not a refactor's. Elmer designed 19 against
        22px rows and is the only casting that chose.

        The row height is pinned and never grows to the platform's standard item height - see
        nf::MenuMetricsLookAndFeel. */
    static nf::MenuMetrics metrics()
    {
        nf::MenuMetrics m;
        m.rowHeight = rowHeight;
        // **The construction, not a literal.** padding + the line box this panel's own caption
        // type produces - nf::captionHeight. It comes out **18** here, where TapeRot's and Fifth
        // Member's come out 19 from the same construction and the same nominal 11px.
        //
        // That difference is not a caption question at all: this casting builds its mono type from
        // a JUCE height, where those two pass a CSS px through withPointHeight, so the same
        // constant produces a smaller line box. Predicting 19 for all four and measuring 18 here is
        // what surfaced it - see the type-scale finding in the root CLAUDE.md.
        //
        // Which is the argument for the construction over a literal, twice over: the number is a
        // property of the face AND of how the font is built, and writing either 18 or 19 would go
        // silently wrong at the first change to either - which is how this caption came to inherit
        // JUCE's rowHeight + rowHeight / 2 in the first place.
        //
        // 3px above and 4px below is the suite's ADOPTED default, not a derived figure: only
        // Elmer and Reflect-84 have designer-authored caption padding and they disagree (3/4
        // against 9/4), so there is no suite constant to derive. See BRAND.md.
        m.sectionHeaderHeight = nf::captionHeight (Chorus60Theme::monoFont (
                                              Chorus60Theme::monoFontHeightForCssPx (headerTextSize)), 3, 4);
        m.separatorHeight = 9;
        m.leadingColumn = tickColumn;
        m.horizontalPadding = 26;
        return m;
    }

    static constexpr int   rowHeight      = 24;
    static constexpr int   tickColumn     = 22;
};
