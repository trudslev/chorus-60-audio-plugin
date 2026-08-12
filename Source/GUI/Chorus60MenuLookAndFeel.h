#pragma once

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
class Chorus60MenuLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    Chorus60MenuLookAndFeel()
    {
        // Covers the few bits JUCE draws without asking us first (the drop shadow's backdrop).
        setColour (juce::PopupMenu::backgroundColourId, glass);
        setColour (juce::PopupMenu::textColourId, Chorus60Theme::Colour::ledWindowText);
        setColour (juce::PopupMenu::highlightedBackgroundColourId,
                   Chorus60Theme::Colour::chorusAccent.withAlpha (0.20f));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    }

    juce::Font getPopupMenuFont() override
    {
        return Chorus60Theme::monoFont (itemTextSize);
    }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        const juce::Rectangle<float> r (0.0f, 0.0f, (float) width, (float) height);
        g.setColour (glass);
        g.fillRoundedRectangle (r, 3.0f);
        g.setColour (rule);
        g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);
    }

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator,
                                    int standardMenuItemHeight, int& idealWidth,
                                    int& idealHeight) override
    {
        if (isSeparator)
        {
            idealWidth = 50;
            idealHeight = 9;
            return;
        }

        const auto f = getPopupMenuFont();
        idealWidth = (int) std::ceil (juce::GlyphArrangement::getStringWidth (f, text))
                     + tickColumn + 26;
        idealHeight = standardMenuItemHeight > 0 ? juce::jmax (rowHeight, standardMenuItemHeight)
                                                 : rowHeight;
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

        Chorus60Theme::drawTrackedText (g, sectionName.toUpperCase(),
                                         Chorus60Theme::monoFont (headerTextSize), headerTracking,
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

    static constexpr float itemTextSize   = 15.0f;
    static constexpr float tracking       = 1.0f;
    static constexpr float headerTextSize = 11.0f;
    static constexpr float headerTracking = 1.6f;
    static constexpr int   rowHeight      = 24;
    static constexpr int   tickColumn     = 22;
};
