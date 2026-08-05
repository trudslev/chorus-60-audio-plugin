#include "PanelChrome.h"
#include "Chorus60Theme.h"

using namespace Chorus60Theme;

namespace
{
    // Every silkscreened string is drawn the same way: take the spec's CSS size, convert it to a
    // juce::Font height, convert the CSS em tracking to absolute px at that size, then place it by a
    // point rather than a box - the panel aligns labels to their control's axis (or to a column
    // edge), which is what the measurements off the dressed render actually record.
    void drawText(juce::Graphics& g, const juce::String& text, float x, float centreY, float cssPx,
                   float trackingEm, juce::Colour colour, juce::Justification::Flags anchor)
    {
        const auto font = labelFont(labelFontHeightForCssPx(cssPx));
        const float tracking = trackingPxForEm(trackingEm, cssPx);
        const float width = trackedTextWidth(text, font, tracking);

        const float left = anchor == juce::Justification::horizontallyCentred ? x - width * 0.5f
                          : anchor == juce::Justification::right              ? x - width
                                                                              : x;

        drawTrackedText(g, text, font, tracking, {left, centreY - cssPx, width, cssPx * 2.0f},
                         juce::Justification::centred, colour);
    }
}

PanelChrome::PanelChrome()
{
    // Sits between the plate and every real control - must never swallow a click.
    setInterceptsMouseClicks(false, false);
}

void PanelChrome::paint(juce::Graphics& g)
{
    paintHeader(g);
    paintGroupTitles(g);
    paintKnobLabels(g);
    paintFooter(g);
}

void PanelChrome::paintHeader(juce::Graphics& g)
{
    // Model lines beside the wordmark (section 8). WordmarkComponent owns "CHORUS-60" itself.
    drawText(g, "BBD CHORUS PROCESSOR", Layout::modelLineX, Layout::modelLine1CentreY,
              11.0f, 0.24f, Colour::modelLinePrimary, juce::Justification::left);
    drawText(g, juce::String(juce::CharPointer_UTF8("MODEL CH-60 \xc2\xb7 STEREO")),
              Layout::modelLineX, Layout::modelLine2CentreY,
              11.0f, 0.24f, Colour::modelLineSecondary, juce::Justification::left);

    // The blue stripe's caption (section 4). Barlow Condensed 700 rather than 600, and the spec's
    // text-indent trick is unnecessary here because drawTrackedText never appends a trailing
    // letter-space in the first place - it only puts tracking *between* glyphs.
    {
        const auto font = labelFontBold(labelFontHeightForCssPx(13.0f));
        const float tracking = trackingPxForEm(0.40f, 13.0f);
        const float width = trackedTextWidth("CHORUS", font, tracking);
        drawTrackedText(g, "CHORUS", font, tracking,
                         {Layout::stripeCaptionCentreX - width * 0.5f,
                          Layout::stripeCaptionCentreY - 13.0f, width, 26.0f},
                         juce::Justification::centred, Colour::stripeText);
    }

    // IN / OUT captions above their windows (section 2's 9px / .24em caption size). PROGRAM's own
    // caption belongs to ProgramHeader, which owns that whole cluster.
    drawText(g, "IN", Layout::inWindowX + Layout::inWindowW * 0.5f, Layout::headerCaptionCentreY,
              9.0f, 0.24f, Colour::captionTertiary, juce::Justification::horizontallyCentred);
    drawText(g, "OUT", Layout::outWindowX + Layout::outWindowW * 0.5f, Layout::headerCaptionCentreY,
              9.0f, 0.24f, Colour::captionTertiary, juce::Justification::horizontallyCentred);
}

void PanelChrome::paintGroupTitles(juce::Graphics& g)
{
    // Section 7's group table. MOD ENGINE I/II carry a title-row LED (drawn live elsewhere, since it
    // follows engine state), so their titles start further right to clear it.
    struct GroupTitle { const char* text; float x, y; bool hasLed; };
    static constexpr GroupTitle titles[] = {
        {"MOD ENGINE I",  Layout::modEngineIGroupX,  Layout::modEngineIGroupY,  true},
        {"MOD ENGINE II", Layout::modEngineIIGroupX, Layout::modEngineIIGroupY, true},
        {"BBD LINE",      302.0f, 430.0f, false},
        {"CHARACTER",     620.0f, 430.0f, false},
        {"OUTPUT",        1075.0f, 430.0f, false},
    };

    for (const auto& title : titles)
        drawText(g, title.text,
                  title.x + (title.hasLed ? Layout::groupTitleInsetXWithLed : Layout::groupTitleInsetX),
                  title.y + Layout::groupTitleCentreBelowTop,
                  10.0f, 0.28f, Colour::engravedHeadingText, juce::Justification::left);
}

void PanelChrome::paintKnobLabels(juce::Graphics& g)
{
    // Section 7: "Label stack beneath each knob: name (Barlow Condensed 600, 10px, .18em, #8A9196)
    // then value, 9px gaps". The value row is a live component (KnobValueLabel) placed from the same
    // constants, so the two stay in step.
    for (const auto& spec : Layout::knobs)
    {
        const float nameCentreY = spec.cy + spec.diameter * 0.5f + Layout::knobLabelGap
                                   + Layout::knobNameRowH * 0.5f;

        drawText(g, spec.displayName, spec.cx, nameCentreY, 10.0f, 0.18f, Colour::controlLabelText,
                  juce::Justification::horizontallyCentred);
    }
}

void PanelChrome::paintFooter(juce::Graphics& g)
{
    // The BBD line's stage count is fixed by the DSP (dsp/BBDLine's 1024 stages), so the whole
    // string is static here. The engine state shown in the scope's caption row is the live one.
    drawText(g, juce::String(juce::CharPointer_UTF8("BBD 1024 STAGE \xc2\xb7 BYPASS \xc2\xb7 v1.0")),
              Layout::footerRight, Layout::footerCentreY, 9.0f, 0.24f, Colour::footerText,
              juce::Justification::right);
}
