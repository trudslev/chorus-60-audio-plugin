#include "Chorus60Theme.h"
#include "../Parameters.h"

namespace Chorus60Theme
{

std::vector<RingToDraw> ringsToDraw()
{
    std::vector<RingToDraw> rings;
    rings.reserve (9);

    // The mod row, in slot order. Its parameter is page-dependent; its RING is not, so the
    // range is the one every page of that slot shares.
    static const char* const modIDs[] { "rate", "depth", "delayCenter", "decorrelation" };
    const juce::NormalisableRange<float> modRanges[] {
        Chorus60Ranges::rate(), Chorus60Ranges::percent(),
        Chorus60Ranges::delayCentreMs(), Chorus60Ranges::percent() };

    for (size_t i = 0; i < Layout::modKnobCentreX.size(); ++i)
        rings.push_back ({ modIDs[i], Layout::slotLabels[i],
                           { Layout::modKnobCentreX[i], Layout::modKnobCentreY },
                           Layout::modKnobD, Layout::modKnobScales[i], modRanges[i] });

    for (const auto& k : Layout::knobs)
        rings.push_back ({ k.paramID, k.label, { k.cx, k.cy }, k.diameter, k.scale,
                           juce::String (k.paramID) == "trim" ? Chorus60Ranges::trimDb()
                                                              : Chorus60Ranges::percent() });

    return rings;
}

std::vector<RingToDraw> ringsInBox (const Layout::GroupBox& box)
{
    const juce::Rectangle<float> bounds { box.x, box.y, box.w, box.h };

    std::vector<RingToDraw> inside;
    for (const auto& ring : ringsToDraw())
        if (bounds.contains (ring.centre))
            inside.push_back (ring);

    return inside;
}

juce::Rectangle<float> drawGroupHeading (juce::Graphics& g, const Layout::GroupBox& box,
                                         juce::Colour ink)
{
    // §8's box title: Barlow Condensed 600, 12 on a 15 px line box, .28em. The row is 30 tall and
    // the text sits 8 px into it, `headingPad` in from the box's left — 31 where the lamp precedes
    // it, 14 where nothing does.
    const juce::Rectangle<float> line { box.x + box.headingPad,
                                        box.y + Layout::groupHeadingTextTop,
                                        box.w - box.headingPad - Layout::groupHeadingTextTop,
                                        Layout::groupHeadingLineBox };

    drawTrackedText (g, juce::String (box.title),
                     labelFont (labelFontHeightForCssPx (Layout::groupHeadingCssPx)),
                     trackingPxForEm (Layout::groupHeadingTrackingEm, Layout::groupHeadingCssPx),
                     line, juce::Justification::centredLeft, ink);

    return line;
}



int drawKnobScale (juce::Graphics& g, const RingToDraw& ring)
{
    const float r = ring.diameter * 0.5f;

    // §3: ticks start 2 px outside the sweep arc, major 2 x 9 and minor 1.5 x 5, ink #a5adb2.
    // The numeral ring is r + 29.5 — the catalogue's clearance chain, which this casting follows
    // exactly, so nothing here is a per-casting figure.
    const float tickOuter = r + Layout::knobTickInkGap + Layout::knobMajorTickLength;

    int numeralled = 0;

    for (int i = 0; i < ring.scale.count; ++i)
    {
        const auto& mark = ring.scale.marks[i];

        // **The angle comes from the parameter, never from a stored fraction.** A taper change then
        // moves the ring with the pointer instead of leaving numerals where it never reaches.
        const float f = ring.range.convertTo0to1 (mark.value);
        const float angle = knobAngleForValue01 (f);

        const float length = mark.isMajor() ? Layout::knobMajorTickLength : Layout::knobMinorTickLength;
        const float width  = mark.isMajor() ? Layout::knobMajorTickWidth  : Layout::knobMinorTickWidth;

        g.setColour (Colour::knobTick);
        g.drawLine ({ pointOnCircle (ring.centre, tickOuter - length, angle),
                      pointOnCircle (ring.centre, tickOuter, angle) }, width);

        if (! mark.isMajor())
            continue;

        ++numeralled;

        const auto at = pointOnCircle (ring.centre, r + Layout::knobNumeralRingOffset, angle);
        const auto text = Layout::withRealMinus (mark.printed);

        drawTrackedText (g, text,
                         monoFont (monoFontHeightForCssPx (Layout::knobNumeralCssPx)),
                         Layout::knobNumeralCssPx * Layout::knobNumeralTrackingEm,
                         juce::Rectangle<float> (at.x - 30.0f, at.y - 8.0f, 60.0f, 16.0f),
                         juce::Justification::centred, Colour::knobNumeral);
    }

    /*  The unit, then the control label — one centred stack under the knob, both measured from the
        knob's own box rather than from a cell.

        **The unit is Barlow Condensed, not the mono the numerals use**, which is easy to get wrong
        because it sits among them: §8 gives the printed unit its own row at 10 / 13 / .16 em while
        the scale numeral is Share Tech Mono 11 / 13 / 0. It had been drawn in the numeral's face at
        the numeral's size six px below the cap — a placeholder from the pass that had no figure for
        it. §3.1's rule that a unit never becomes a suffix on the control name is what puts it here
        rather than in the label. */
    if (ring.scale.unit != nullptr)
        drawTrackedText (g, juce::String (ring.scale.unit),
                         labelFont (labelFontHeightForCssPx (Layout::knobUnitCssPx)),
                         trackingPxForEm (Layout::knobUnitTrackingEm, Layout::knobUnitCssPx),
                         juce::Rectangle<float> (ring.centre.x - Layout::knobCaptionBoxW * 0.5f,
                                                 Layout::knobUnitTop (ring.centre.y, ring.diameter),
                                                 Layout::knobCaptionBoxW, Layout::knobUnitLineBox),
                         juce::Justification::centred, Colour::knobNumeral);

    drawTrackedText (g, juce::String (ring.label),
                     labelFont (labelFontHeightForCssPx (Layout::knobLabelCssPx)),
                     trackingPxForEm (Layout::knobLabelTrackingEm, Layout::knobLabelCssPx),
                     juce::Rectangle<float> (ring.centre.x - Layout::knobCaptionBoxW * 0.5f,
                                             Layout::knobLabelTop (ring.centre.y, ring.diameter),
                                             Layout::knobCaptionBoxW, Layout::knobLabelLineBox),
                     juce::Justification::centred, Colour::controlLabelText);

    return numeralled;
}

} // namespace Chorus60Theme
