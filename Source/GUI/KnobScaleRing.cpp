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
        rings.push_back ({ modIDs[i],
                           { Layout::modKnobCentreX[i], Layout::modKnobCentreY },
                           Layout::modKnobD, Layout::modKnobScales[i], modRanges[i] });

    for (const auto& k : Layout::knobs)
        rings.push_back ({ k.paramID, { k.cx, k.cy }, k.diameter, k.scale,
                           juce::String (k.paramID) == "trim" ? Chorus60Ranges::trimDb()
                                                              : Chorus60Ranges::percent() });

    return rings;
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

    // The unit prints inside the arc's bottom gap — §3.1 is explicit that it never becomes a suffix
    // on the control's name. The sweep is 270 degrees, so the gap is the 90 below the pivot.
    if (ring.scale.unit != nullptr)
        drawTrackedText (g, juce::String (ring.scale.unit),
                         monoFont (monoFontHeightForCssPx (Layout::knobNumeralCssPx)),
                         Layout::knobNumeralCssPx * Layout::knobNumeralTrackingEm,
                         juce::Rectangle<float> (ring.centre.x - 30.0f,
                                                 ring.centre.y + r + Layout::knobUnitDrop,
                                                 60.0f, 16.0f),
                         juce::Justification::centred, Colour::knobNumeral);

    return numeralled;
}

} // namespace Chorus60Theme
