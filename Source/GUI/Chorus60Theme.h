#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include <BinaryData.h>
#include <array>
#include <cmath>

// Centralises every pixel constant from design/CHORUS60-GUI-SPEC.md (palette, coordinates, the
// filmstrip contract) in one place, mirroring GatecrasherTheme.h's role for Gatecrasher. Most of
// the fascia is a static bitmap (see Chorus60PanelBackground) rather than code-drawn, so this
// file's job is narrower: positions/sizes for the *live* pieces layered on top, plus the handful
// of colours those live pieces need to match the baked artwork around them.
//
// IMPORTANT: ranges/defaults for knob scaling and value-text formatting come from Source/Parameters.h,
// NOT from the spec's own section 9 table - see chorus-60/CLAUDE.md's explicit note on this. This
// file only holds *layout* (coordinates, colours, sizes), never a parameter range.
namespace Chorus60Theme
{
    namespace Colour
    {
        // Section 1 palette - only the values actually needed by live-drawn components (the fascia
        // gradients/grain/dividers/section-stripe colours never leave the static background bitmap).
        inline const juce::Colour engravedHeadingText{0xFFE6EBEE};
        inline const juce::Colour controlLabelText{0xFF8A9196};
        inline const juce::Colour valueText{0xFFC6CED3};
        inline const juce::Colour captionTertiary{0xFF7B8287};
        inline const juce::Colour inactiveLabel{0xFF5F666B};
        inline const juce::Colour footerText{0xFF5A6165};
        inline const juce::Colour stripeText{0xFFEEF2F4};
        inline const juce::Colour modelLinePrimary{0xFF8A9196};   // "BBD CHORUS PROCESSOR"
        inline const juce::Colour modelLineSecondary{0xFF5F666B}; // "MODEL CH-60 - STEREO"

        inline const juce::Colour ledWindowBg{0xFF07090A};
        inline const juce::Colour ledWindowBorder{0xFF363C41};
        inline const juce::Colour ledWindowText{0xFFDFE6EA};

        // "Chorus accent (ONLY colour beyond the stripes)" - reserved exclusively for the two
        // engine LEDs and the scope trace, per spec section 1's explicit rule and BRAND.md's
        // one-accent-colour rule. Never used for any knob/label/meter.
        inline const juce::Colour chorusAccent{0xFFFF2B1C};
        inline const juce::Colour ledUnlit{0xFF3A1512};

        inline const juce::Colour tickMark{0xFF78848C}; // rgba(120,132,140,x)

        // Engine button faces, section 1/4 (160deg linear gradients).
        inline const juce::Colour buttonIITop{0xFFE5A021}, buttonIIBottom{0xFFC07908};
        inline const juce::Colour buttonITop{0xFFEAD681}, buttonIBottom{0xFFD0B857};
        inline const juce::Colour buttonOffTop{0xFFEAECEC}, buttonOffBottom{0xFFC9CDCF};

        // LED lit radial gradient, section 4.
        inline const juce::Colour ledLitCore{0xFFFF2B1C}, ledLitMid{0xFFB0140C}, ledLitEdge{0xFF6D0B06};

        // Delay-modulation scope, section 5 - same construction as Gatecrasher's envelope scope, so
        // several of these numeric values match GatecrasherTheme's own literally (both were derived
        // from the same rgba(150,180,190,*)/(160,178,186,*) greys).
        inline const juce::Colour scopeBorder{0xFF0A0C0D};
        inline const juce::Colour scopeBgTop{0xFF06080A};
        inline const juce::Colour scopeBgBottom{0xFF0B0F11};
        inline const juce::Colour scopeGrid{0x1A96B4BE};        // rgba(150,180,190,.10)
        inline const juce::Colour scopeCentreLine{0x3896B4BE};  // rgba(150,180,190,.22)
        inline const juce::Colour scopeInputUnderlay{0x38B2BEC5}; // rgba(178,190,197,.22)
        inline const juce::Colour scopeAnnotation{0x8CA0B2BA};    // rgba(160,178,186,.55)

        // Program header, section 6 - identical contract/values to Gatecrasher's own.
        inline const juce::Colour tagFactory{0xFF6F797F};
        inline const juce::Colour tagUser{0xFFCFD7DC};
        inline const juce::Colour headerName{0xFFDFE6EA};

        // SAVE / DELETE, section 6: "Gatecrasher's exact treatment ... the only light-steel
        // elements on this panel and that is intentional - the utility surface is shared across the
        // suite." Values are identical to GatecrasherTheme's own by design, not by accident.
        inline const juce::Colour buttonEnabledTop{0xFFDBE0E3};
        inline const juce::Colour buttonEnabledBottom{0xFFAAB1B6};
        inline const juce::Colour buttonEnabledBorder{0xFF6D7478};
        inline const juce::Colour buttonEnabledLabel{0xFF22272B};
        inline const juce::Colour buttonPressedTop{0xFFA9B0B5};
        inline const juce::Colour buttonPressedBottom{0xFFC9D0D4};
        inline const juce::Colour buttonDisabledTop{0xFFC2C8CC};
        inline const juce::Colour buttonDisabledBottom{0xFFA8AFB3};
        inline const juce::Colour buttonDisabledBorder{0xFF8D9498};
        inline const juce::Colour buttonDisabledLabel{0xFF8B9297};
    }

    enum class KnobFilmstripSize { large, small };

    namespace Layout
    {
        constexpr float canvasWidth = 1400.0f;
        constexpr float canvasHeight = 632.0f;

        // Rotation range for every knob: pointer at 12 o'clock = centre (section 7).
        constexpr float knobArcStartDegrees = -135.0f;
        constexpr float knobArcEndDegrees = 135.0f;

        // Section 7: "every 15 on the large knobs / 20 on the small ones".
        constexpr float largeKnobTickSpacingDegrees = 15.0f;
        constexpr float smallKnobTickSpacingDegrees = 20.0f;

        // Section 7: tick ring "from r+3 to r+9".
        constexpr float tickInnerOffset = 3.0f;
        constexpr float tickOuterOffset = 9.0f;

        // Filmstrip frames are square with transparent margin for the baked cast shadow - draw
        // into the full bounding box, not just the knob circle. Same bleed factor as Gatecrasher's
        // own port, since these are literally the same two shared filmstrip PNGs.
        constexpr float knobBoundingBoxBleed = 1.07f;

        // Knob label stack (section 7: "name then value, 9px gaps") - measured against
        // chorus60-panel-bypass@2x.png's baked RATE I example (knob bottom ~y357, name centre
        // ~y372, value centre ~y393.5 at the panel's 1x scale), which lines up almost exactly with
        // gap=9 / nameRowH=13 / valueRowH=14.
        constexpr float knobLabelGap = 9.0f;
        constexpr float knobNameRowH = 13.0f;
        constexpr float knobValueRowH = 14.0f;

        struct KnobSpec
        {
            const char* paramID;
            const char* displayName;
            float cx, cy, diameter;
            KnobFilmstripSize size;
        };

        // Section 7's full 11-knob table, in the exact ParamIDs from Source/Parameters.h (the
        // authoritative parameter list - engine1/engine2 are the only two APVTS parameters with no
        // knob, they bind to the button column instead).
        inline constexpr std::array<KnobSpec, 11> knobs{ {
            {"rate1",         "RATE I",        454.0f, 328.0f, 58.0f, KnobFilmstripSize::large},
            {"depth1",        "DEPTH I",       680.0f, 328.0f, 58.0f, KnobFilmstripSize::large},
            {"rate2",         "RATE II",       999.0f, 328.0f, 58.0f, KnobFilmstripSize::large},
            {"depth2",        "DEPTH II",     1226.0f, 328.0f, 58.0f, KnobFilmstripSize::large},
            {"delayCenter",   "DELAY CENTER",  385.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
            {"decorrelation", "DECORRELATION", 521.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
            {"drift",         "DRIFT",         703.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
            {"saturation",    "SATURATION",    839.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
            {"noise",         "NOISE",         976.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
            {"mix",           "MIX",          1158.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
            {"trim",          "OUTPUT TRIM",  1294.0f, 495.0f, 42.0f, KnobFilmstripSize::small},
        } };

        // Button column, section 4.
        constexpr float buttonIIX = 31.0f, buttonIIY = 144.0f, buttonW = 116.0f, buttonH = 116.0f;
        constexpr float buttonIX = 31.0f, buttonIY = 286.0f;
        constexpr float buttonOffX = 31.0f, buttonOffY = 428.0f;
        constexpr float buttonCornerRadius = 5.0f;

        constexpr float ledIIX = 167.0f, ledIIY = 195.0f, ledD = 15.0f;
        constexpr float ledIX = 167.0f, ledIY = 337.0f;

        // "Roman labels sit 27px right of each LED" - gap between the LED's right edge and the
        // label's left edge.
        constexpr float engineLabelX = ledIIX + ledD + 27.0f;
        constexpr float engineLabelW = 70.0f;

        constexpr float pressAnimMs = 110.0f;
        constexpr float pressOffsetPx = 3.0f;

        // MOD ENGINE I/II group panels' own Ø8 title-row LED (section 7's group table: "Ø8 LED
        // (engine I state) + title"). Not given exact sub-coordinates in the spec's tables - derived
        // from the group panel's own 9/14/14 padding convention (section 7) and cross-checked
        // against chorus60-panel@2x.png's baked dot position.
        constexpr float groupLedD = 8.0f;
        constexpr float modEngineIGroupX = 302.0f, modEngineIGroupY = 255.0f;
        constexpr float modEngineIIGroupX = 848.0f, modEngineIIGroupY = 255.0f;
        constexpr float groupLedPadX = 9.0f, groupLedPadY = 18.0f; // centre inset from group top-left

        // Delay-modulation scope, section 5.
        constexpr float captionRowX = 302.0f, captionRowY = 94.0f, captionRowW = 1077.0f, captionRowH = 21.0f;
        constexpr float scopeX = 302.0f, scopeY = 115.0f, scopeW = 1077.0f, scopeH = 124.0f;
        constexpr float scopeInnerInset = 2.0f;

        // "2.0s of history, scrolling right-to-left at 60fps... 8 vertical divisions... hence
        // 250ms/DIV" - derived (not spec-literal) constants: pixelsPerFrame = width / (2.0s * 60fps),
        // gridSpacing = width / 8, both computed from the actual inner rect at paint time so they
        // stay self-consistent if the rect above is ever tuned.
        constexpr float scopeHistorySeconds = 2.0f;
        constexpr float scopeFps = 60.0f;
        constexpr int scopeNumDivisions = 8;

        // Section 5: "amplitude 0.34 x h" and the delay-centre vertical-offset formula, adapted from
        // the spec's own [2,14]ms/centre-8/half-width-6 reference to Parameters.h's actual
        // [5,15]ms delayCenter range (centre 10, half-width 5) - see chorus-60/CLAUDE.md's note that
        // Parameters.h's ranges are authoritative, not the spec table's.
        constexpr float scopeAmplitudeFraction = 0.34f;
        constexpr float scopeCentreOffsetFraction = 0.10f;
        constexpr float delayCenterRangeMid = 8.0f, delayCenterRangeHalf = 6.0f;

        // The real DSP's own theoretical ceiling for combined modulation + drift offset (ms), used
        // as the value that maps to the full scopeAmplitudeFraction*h swing. ModulationEngine.cpp's
        // maxExcursionMs is 2.5ms per engine (both engines can sum), CharacterStage.cpp's
        // maxDriftMs is 0.15ms - those constants live in anonymous namespaces in DSP .cpp files so
        // aren't directly includable here; this is their documented sum, kept as one named constant
        // so the provenance is clear rather than a bare magic number.
        constexpr float scopeReferenceExcursionMs = 2.5f * 2.0f + 0.15f;

        // Program header (section 6). The three header-state bitmaps are full-width renders of the
        // whole header band (wordmark included) - ProgramHeader only ever blits the "program
        // cluster" sub-rect below (PROGRAM caption through the OUT window), leaving the wordmark to
        // WordmarkComponent so the two never double-paint the same pixels - same split as
        // Gatecrasher's own ProgramHeader/WordmarkComponent. This crop rect is a generous bounding
        // box around section 6's coordinate table, not a pixel-measured exact crop - safe because
        // the surrounding fascia is pixel-identical to the static panel background in every
        // direction, so a slightly loose crop still blends seamlessly.
        constexpr float headerAssetSrcScale = 3.0f; // shipped @3x
        constexpr float headerCropX = 800.0f, headerCropY = 8.0f, headerCropW = 590.0f, headerCropH = 65.0f;

        constexpr float programWindowX = 832.0f, programWindowY = 34.0f, programWindowW = 307.0f, programWindowH = 27.0f;
        constexpr float programTagCellX = 833.0f, programTagCellY = 35.0f, programTagCellW = 43.0f, programTagCellH = 25.0f;
        constexpr float programNameCellX = 876.0f, programNameCellY = 35.0f, programNameCellW = 262.0f, programNameCellH = 25.0f;

        constexpr float saveButtonX = 1145.0f, saveButtonY = 34.0f, saveButtonW = 41.0f, saveButtonH = 27.0f;
        constexpr float deleteButtonX = 1192.0f, deleteButtonY = 34.0f, deleteButtonW = 51.0f, deleteButtonH = 27.0f;

        constexpr float inWindowX = 1261.0f, inWindowY = 34.0f, inWindowW = 54.0f, inWindowH = 27.0f;
        constexpr float outWindowX = 1323.0f, outWindowY = 34.0f, outWindowW = 54.0f, outWindowH = 27.0f;

        constexpr int maxProgramNameLength = 22; // mirrors ProgramManager::maxProgramNameLength

        // Wordmark, section 8 - owned separately from ProgramHeader (see headerCrop comment above).
        constexpr float wordmarkX = 25.0f, wordmarkY = 30.0f, wordmarkW = 308.0f, wordmarkH = 31.0f;

        // ---- Static chrome (PanelChrome) ----------------------------------------------------
        // Model lines, section 8: "right of the wordmark at x 351 ... Barlow Condensed 600, 11px,
        // .24em, 4px apart", over the wordmark's own 31px block.
        constexpr float modelLineX = 351.0f;
        constexpr float modelLine1CentreY = 38.0f;
        constexpr float modelLine2CentreY = 53.0f;

        // Blue stripe caption, section 4: centred in the top stripe (x 25, y 94, 232 x 24).
        constexpr float stripeCaptionCentreX = 141.0f, stripeCaptionCentreY = 106.0f;

        // Group panel titles sit in each box's title row, over the plate's own hairline rule.
        // Section 7 gives the boxes' rects and a 9/14/14 padding convention; the title baseline is
        // measured off the dressed render, which puts the text centre 19px below the box top.
        constexpr float groupTitleCentreBelowTop = 19.0f;
        constexpr float groupTitleInsetX = 14.0f;      // left inset for title-only boxes
        constexpr float groupTitleInsetXWithLed = 24.0f; // MOD ENGINE I/II clear their O8 LED

        // The IN / OUT captions above their windows.
        constexpr float headerCaptionCentreY = 22.0f;

        // Below this the IN/OUT readouts show -INF rather than a number: the plugin's own noise
        // floor (BBD clock noise is always running) sits well above it, so anything lower is
        // silence, and "-100.0" overflows the 54px window anyway.
        constexpr float meterFloorDb = -60.0f;

        // Footer status line, right-aligned in the footer band (y 602, h 29).
        constexpr float footerRight = 1377.0f, footerCentreY = 616.0f;
    }

    // Angle (degrees, clockwise from 12 o'clock) for a normalised 0..1 value across the knob arc.
    inline float knobAngleForValue01(float value01) noexcept
    {
        return Layout::knobArcStartDegrees
             + value01 * (Layout::knobArcEndDegrees - Layout::knobArcStartDegrees);
    }

    inline juce::Point<float> directionForAngleDegrees(float degrees) noexcept
    {
        const float radians = juce::degreesToRadians(degrees);
        return {std::sin(radians), -std::cos(radians)};
    }

    inline juce::Point<float> pointOnCircle(juce::Point<float> centre, float radius, float angleDegrees) noexcept
    {
        return centre + directionForAngleDegrees(angleDegrees) * radius;
    }

    // MOD ENGINE I/II group panels' own Ø8 title-row LED bounds (see Layout::groupLedPadX/Y's
    // comment for how these were derived).
    inline juce::Rectangle<float> modEngineILedRect() noexcept
    {
        using namespace Layout;
        return { modEngineIGroupX + groupLedPadX - groupLedD * 0.5f, modEngineIGroupY + groupLedPadY - groupLedD * 0.5f,
                 groupLedD, groupLedD };
    }
    inline juce::Rectangle<float> modEngineIILedRect() noexcept
    {
        using namespace Layout;
        return { modEngineIIGroupX + groupLedPadX - groupLedD * 0.5f, modEngineIIGroupY + groupLedPadY - groupLedD * 0.5f,
                 groupLedD, groupLedD };
    }

    // Number of ticks (inclusive of both arc endpoints) whose even spacing across the full 270
    // sweep comes closest to targetSpacingDegrees, landing exactly on -135 and +135.
    inline int tickCountForSpacing(float targetSpacingDegrees) noexcept
    {
        const float sweep = Layout::knobArcEndDegrees - Layout::knobArcStartDegrees;
        const int intervals = juce::jmax(1, (int) std::round(sweep / targetSpacingDegrees));
        return intervals + 1;
    }

    // CSS-style angled linear gradient (0deg = to top, 90deg = to right, clockwise), matching the
    // spec's "160deg" gradient notation for the engine buttons exactly rather than approximating
    // with a plain vertical gradient.
    inline juce::ColourGradient angledGradient(juce::Rectangle<float> bounds, juce::Colour start,
                                                 juce::Colour end, float cssAngleDegrees)
    {
        const float a = juce::degreesToRadians(cssAngleDegrees);
        const juce::Point<float> dir(std::sin(a), -std::cos(a));
        const float halfLength = 0.5f * (std::abs(bounds.getWidth() * dir.x) + std::abs(bounds.getHeight() * dir.y));
        const auto centre = bounds.getCentre();
        const auto p0 = centre - dir * halfLength;
        const auto p1 = centre + dir * halfLength;
        return juce::ColourGradient(start, p0.x, p0.y, end, p1.x, p1.y, false);
    }

    inline float trackedTextWidth(const juce::String& text, const juce::Font& font, float trackingPx)
    {
        float width = 0.0f;
        for (int i = 0; i < text.length(); ++i)
        {
            width += juce::GlyphArrangement::getStringWidth(font, juce::String::charToString(text[i]));
            if (i < text.length() - 1)
                width += trackingPx;
        }
        return width;
    }

    // juce::Font has no absolute-pixel letter-spacing, so this draws glyph-by-glyph to reproduce
    // the spec's tracking values (e.g. ".18-.28em" on labels) - same technique as
    // GatecrasherTheme::drawTrackedText.
    inline void drawTrackedText(juce::Graphics& g, const juce::String& text, const juce::Font& font,
                                 float trackingPx, juce::Rectangle<float> area,
                                 juce::Justification justification, juce::Colour colour)
    {
        g.setFont(font);
        g.setColour(colour);

        const float totalWidth = trackedTextWidth(text, font, trackingPx);
        float x = area.getX();
        if (justification.testFlags(juce::Justification::horizontallyCentred))
            x = area.getCentreX() - totalWidth * 0.5f;
        else if (justification.testFlags(juce::Justification::right))
            x = area.getRight() - totalWidth;

        for (int i = 0; i < text.length(); ++i)
        {
            const auto ch = juce::String::charToString(text[i]);
            const float charWidth = juce::GlyphArrangement::getStringWidth(font, ch);
            g.drawText(ch, juce::Rectangle<float>(x, area.getY(), charWidth + 1.0f, area.getHeight()),
                       juce::Justification::centredLeft, false);
            x += charWidth + trackingPx;
        }
    }

    // Barlow Condensed SemiBold (600, labels) / Bold (700, group/lamp text), Share Tech Mono
    // Regular (numeric/LED readouts), Librestile Extended Bold (wordmark only) - section 2. Loaded
    // once per process via function-local statics, same caching pattern as GatecrasherTheme.h.
    inline juce::Typeface::Ptr barlowSemiBoldTypeface()
    {
        static const juce::Typeface::Ptr typeface =
            juce::Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensedSemiBold_ttf,
                                                      (size_t) BinaryData::BarlowCondensedSemiBold_ttfSize);
        return typeface;
    }
    inline juce::Typeface::Ptr barlowBoldTypeface()
    {
        static const juce::Typeface::Ptr typeface =
            juce::Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensedBold_ttf,
                                                      (size_t) BinaryData::BarlowCondensedBold_ttfSize);
        return typeface;
    }
    inline juce::Typeface::Ptr shareTechMonoTypeface()
    {
        static const juce::Typeface::Ptr typeface =
            juce::Typeface::createSystemTypefaceFor(BinaryData::ShareTechMonoRegular_ttf,
                                                      (size_t) BinaryData::ShareTechMonoRegular_ttfSize);
        return typeface;
    }
    inline juce::Typeface::Ptr librestileExtBoldTypeface()
    {
        static const juce::Typeface::Ptr typeface =
            juce::Typeface::createSystemTypefaceFor(BinaryData::LibrestileExtBold_ttf,
                                                      (size_t) BinaryData::LibrestileExtBold_ttfSize);
        return typeface;
    }

    inline juce::Font labelFont(float heightPx)
    {
        return juce::Font(juce::FontOptions(heightPx).withTypeface(barlowSemiBoldTypeface()));
    }
    inline juce::Font labelFontBold(float heightPx)
    {
        return juce::Font(juce::FontOptions(heightPx).withTypeface(barlowBoldTypeface()));
    }
    inline juce::Font monoFont(float heightPx)
    {
        return juce::Font(juce::FontOptions(heightPx).withTypeface(shareTechMonoTypeface()));
    }
    // The spec quotes every type size as CSS px, but a juce::Font's height parameter is
    // ascent+descent - for a given typeface a fixed multiple of the CSS em size, not equal to it.
    // Passing a spec size straight to labelFont() therefore renders noticeably small. These convert,
    // calibrating the ratio once off a reference string whose rendered width was measured directly
    // from the dressed reference render (the baked "DECORRELATION" knob label, 13 glyphs of
    // section 7's 10px / .18em, spanning 79.4px in canvas units) - so one real measurement scales
    // every size on the panel. Same approach, and same underlying trap, as Gatecrasher's.
    inline float fontHeightForTrackedWidth(const juce::Font& probeFont, float probeHeight,
                                            const juce::String& text, float trackingPx, float targetWidthPx)
    {
        const float glyphsAtProbe = trackedTextWidth(text, probeFont, 0.0f);
        const float trackingTotal = trackingPx * (float) juce::jmax(0, text.length() - 1);
        if (glyphsAtProbe <= 0.0f)
            return probeHeight;
        return juce::jmax(1.0f, (targetWidthPx - trackingTotal) * probeHeight / glyphsAtProbe);
    }

    inline float trackingPxForEm(float em, float cssPx) { return em * cssPx; }

    inline float labelFontHeightForCssPx(float cssPx)
    {
        static const float ratio = [&]
        {
            constexpr float probeHeight = 40.0f, refCssPx = 10.0f, refWidth = 79.4f;
            return fontHeightForTrackedWidth(labelFont(probeHeight), probeHeight, "DECORRELATION",
                                              trackingPxForEm(0.18f, refCssPx), refWidth)
                 / refCssPx;
        }();
        return cssPx * ratio;
    }

    inline juce::Font monoFontBold(float heightPx)
    {
        return juce::Font(juce::FontOptions(heightPx).withTypeface(shareTechMonoTypeface())).boldened();
    }
    inline juce::Font wordmarkFont(float heightPx)
    {
        return juce::Font(juce::FontOptions(heightPx).withTypeface(librestileExtBoldTypeface()));
    }

    // Binary-data-backed images, decoded once per process via function-local statics (avoids
    // repeated PNG decode on every repaint/instantiation - the knob filmstrips in particular are
    // 128x16384 sheets). Centralised here rather than in each component, same rationale as
    // GatecrasherTheme.h's own equivalent block.
    // The background plate (spec preamble): the full static fascia with NO controls and no text -
    // panel material and frame, header chrome with empty PROGRAM / IN / OUT wells, the blue stripes
    // and blank CHORUS strip, section divider, empty scope well, and the empty group boxes with
    // their heading rules. Every glyph and every control is drawn on top of it.
    //
    // This replaced chorus60-panel-bypass@2x.png, a fully dressed render that carried baked copies
    // of the knobs (pointers included), all the labels and all the value readouts. Compositing live
    // elements over that meant each one fought a frozen copy of itself - the same class of bug that
    // had to be unpicked across the whole of Gatecrasher's GUI before it got its own bare chassis.
    // The dressed renders stay in design/assets/ as pixel-matching acceptance targets.
    inline const juce::Image& panelBackgroundImage()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::chorus60backgroundplate2x_png, (size_t) BinaryData::chorus60backgroundplate2x_pngSize);
        return image;
    }

    inline const juce::Image& knobLargeFilmstrip()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::knob_large_128px_128f_png, (size_t) BinaryData::knob_large_128px_128f_pngSize);
        return image;
    }

    inline const juce::Image& knobSmallFilmstrip()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::knob_small_128px_128f_png, (size_t) BinaryData::knob_small_128px_128f_pngSize);
        return image;
    }

    // Chorus-60-specific value-text formatting, matching the exact display conventions baked into
    // the reference renders (design/assets/chorus60-panel*@2x.png) rather than Gatecrasher's own
    // popup-only formatter: Hz -> 2 decimals ("0.45 Hz"), % -> 0 decimals with a space before the
    // sign ("38 %"), ms -> 1 decimal ("5.6 ms"), dB -> 1 decimal, explicitly signed ("+0.0 dB").
    inline juce::String formatParameterValue(const juce::RangedAudioParameter& param, double value)
    {
        const auto label = param.getLabel();
        if (label == "Hz")
            return juce::String(value, 2) + " Hz";
        // roundToInt, NOT juce::String(value, 0): JUCE treats a decimal-place count of 0 as "use the
        // default conversion" rather than "round to a whole number", so that spelling prints the
        // full value (a Depth of 68.5916 rendered as "68.5916 %"). It only looked correct while the
        // displayed values happened to land on whole numbers, which every factory program's do.
        if (label == "%")
            return juce::String(juce::roundToInt(value)) + " %";
        if (label == "ms")
            return juce::String(value, 1) + " ms";
        if (label == "dB")
            return (value >= 0.0 ? "+" : "") + juce::String(value, 1) + " dB";

        return param.getText(param.convertTo0to1((float) value), 0);
    }
}
