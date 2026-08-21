#include "../Source/PluginProcessor.h"
#include "../Source/GUI/Chorus60Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

/*  **What `ProgramHeader` redraws per tick, before why it costs.**

    It is the second of this editor's two continuous costs: 20 Hz, unconditional, 1.546 ms per paint.
    Unconditional is not a defect here and its own `timerCallback` says why — *"the IN/OUT meters
    need continuous redraw regardless of whether the current program changed."* That is true, so
    **the timer is right and the question is what the paint does with it.**

    The distinction decides the fix. A component redrawing UNCHANGED content unconditionally wants
    its timer guarded, the way `EngineButtonComponent` and `ImageSwitch` already guard theirs. A
    component redrawing a large static half to update a small live one wants a cache. This is the
    second, and this file measures how lopsided it is.

    **Split by what varies, not by what is drawn where:**

      - **Never** — the header block, the wordmark, the descriptor, the model line, the three wells,
        both button faces.
      - **On a Program change** — the bank tag, the Program name, the caption, the legends.
      - **Every tick** — two meter numbers, in two 64 px wells.
*/
class HeaderPaintSplitTests final : public juce::UnitTest
{
public:
    HeaderPaintSplitTests() : juce::UnitTest ("Header paint split", "Performance") {}

    void runTest() override
    {
        using namespace Chorus60Theme;
        using namespace Chorus60Theme::Layout;

        beginTest ("The static half against what actually changes per tick");

        juce::Image canvas (juce::Image::ARGB, (int) canvasWidth, 140, true);

        const auto time = [&] (int repeats, auto&& body)
        {
            juce::Graphics g (canvas);
            body (g);
            const auto start = juce::Time::getMillisecondCounterHiRes();
            for (int i = 0; i < repeats; ++i)
                body (g);
            return (juce::Time::getMillisecondCounterHiRes() - start) / (double) repeats;
        };

        constexpr int n = 300;

        const double blockMs = time (n, [&] (juce::Graphics& g) { paintHeaderBlock (g); });

        const double nameplateMs = time (n, [&] (juce::Graphics& g)
        {
            drawTrackedText (g, Layout::wordmarkText, wordmarkFont (Layout::wordmarkCssPx),
                              trackingPxForEm (Layout::wordmarkTrackingEm, Layout::wordmarkCssPx),
                              juce::Rectangle<float> (Layout::nameplateX, Layout::nameplateY,
                                                       Layout::nameplateW, Layout::wordmarkLineBox),
                              juce::Justification::centredLeft, Colour::engravedHeadingText);
            drawTrackedText (g, Layout::descriptorText,
                              labelFont (labelFontHeightForCssPx (Layout::descriptorCssPx)),
                              trackingPxForEm (Layout::descriptorTrackingEm, Layout::descriptorCssPx),
                              juce::Rectangle<float> (Layout::nameplateX, Layout::descriptorY,
                                                       Layout::nameplateW, Layout::descriptorLineBox),
                              juce::Justification::centredLeft, Colour::engravedHeadingText);
        });

        const double wellsMs = time (n, [&] (juce::Graphics& g)
        {
            paintDisplayWell (g, juce::Rectangle<float> (programWindowX, programWindowY,
                                                          programWindowW, programWindowH));
            paintDisplayWell (g, juce::Rectangle<float> ((float) nf::HeaderGeometry::inWellX,
                                                          (float) nf::HeaderGeometry::bandY,
                                                          (float) nf::HeaderGeometry::meterWellW,
                                                          (float) nf::HeaderGeometry::bandH));
            paintDisplayWell (g, juce::Rectangle<float> ((float) nf::HeaderGeometry::outWellX,
                                                          (float) nf::HeaderGeometry::bandY,
                                                          (float) nf::HeaderGeometry::meterWellW,
                                                          (float) nf::HeaderGeometry::bandH));
            paintProgramButtonFace (g, nf::HeaderGeometry::saveButton().toFloat());
            paintProgramButtonFace (g, nf::HeaderGeometry::deleteButton().toFloat());
        });

        /*  EVERY TICK: the two meter numbers, and the entire reason the timer is unconditional. It
            is the figure everything above has to be weighed against. */
        const auto lcdFont = monoFont (monoFontHeightForCssPx (lcdCssPx));
        const float lcdTracking = trackingPxForEm (lcdTrackingEm, lcdCssPx);

        const double metersMs = time (n, [&] (juce::Graphics& g)
        {
            for (const auto& text : { juce::String ("-12.4"), juce::String ("-6.2") })
                drawTrackedText (g, text, lcdFont, lcdTracking,
                                  juce::Rectangle<float> ((float) nf::HeaderGeometry::inWellX,
                                                           (float) nf::HeaderGeometry::bandY,
                                                           (float) nf::HeaderGeometry::meterWellW,
                                                           (float) nf::HeaderGeometry::bandH),
                                  juce::Justification::centred, Colour::ledWindowText);
        });

        const double staticMs = blockMs + nameplateMs + wellsMs;

        logMessage ("  header block   " + juce::String (blockMs * 1000.0, 1) + " us   never changes");
        logMessage ("  nameplate      " + juce::String (nameplateMs * 1000.0, 1) + " us   never changes");
        logMessage ("  wells + caps   " + juce::String (wellsMs * 1000.0, 1) + " us   never changes");
        logMessage ("  --------");
        logMessage ("  static total   " + juce::String (staticMs * 1000.0, 1) + " us");
        logMessage ("  meter values   " + juce::String (metersMs * 1000.0, 1)
                    + " us   the only thing the 20 Hz timer is FOR");
        logMessage ("  ratio          " + juce::String (metersMs > 0.0 ? staticMs / metersMs : 0.0, 1)
                    + "x static per live");
        logMessage ("  at 20 Hz that is " + juce::String (staticMs * 20.0, 2)
                    + " ms/s of unchanging pixels to deliver "
                    + juce::String (metersMs * 20.0, 2) + " ms/s of numbers");

        expectGreaterThan (staticMs, 0.0);
        expectGreaterThan (metersMs, 0.0);
        expectNotEquals (juce::String (staticMs / metersMs, 1), juce::String ("1.0"),
                         "static and live cost the same, so this fixture distinguishes nothing");
    }
};

static HeaderPaintSplitTests headerPaintSplitTests;
