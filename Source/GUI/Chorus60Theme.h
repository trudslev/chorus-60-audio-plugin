#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include <BinaryData.h>
#include <array>
#include <cmath>

// Centralises every pixel constant from design/CHORUS60-GUI-SPEC.md (palette, coordinates, the
// filmstrip contract) in one place, mirroring GatecrasherTheme.h's role for Gatecrasher.
//
// REVISION 2 INVERTED THIS FILE'S SCOPE. The plate used to be bare material with every glyph drawn
// in code, so this held coordinates for two dozen strings. It now BAKES the printed scales, every
// tick ring, every numeral and unit, the wordmark, the static labels and the group headings - see
// design/CHORUS60-BUILD-HANDOFF.md section 1, which is the asset contract. What remains here is the
// geometry of the nine things drawn at runtime, plus the handful of colours those need in order to
// match the baked text beside them exactly.
//
// **Do not add a constant here for anything the plate bakes.** A second copy of a baked coordinate
// is a copy that can drift, and drawing a baked string again double-prints it at a one-pixel offset.
//
// COORDINATES ARE INSIDE-BORDER. The exported plate is 1282 x 777 and includes a 1 px outer border;
// both design documents measure from the first pixel of panel material inside it. Chorus60EditorContent
// positions its content child at (1, 1) so every constant below is a literal spec value with no
// arithmetic - see canvasWidth/contentWidth.
//
// IMPORTANT: ranges/defaults for knob scaling come from Source/Parameters.h, never from a spec
// table. This file holds *layout* only.
namespace Chorus60Theme
{
    namespace Colour
    {
        // ---- Section 2 palette -------------------------------------------------------------
        //
        // Revision 2 lifted every grey one step for contrast, measured against the group field
        // #131517. Runtime-drawn text must match the baked text beside it exactly, so these are not
        // approximations. The retired values, for anyone reading an old commit: #8A9196 as a *label*
        // colour, #7B8287, #5F666B and #5A6165 have all collapsed into the two below, and #C6CED3
        // went with the standing value readouts.
        inline const juce::Colour engravedHeadingText{0xFFE6EBEE}; // 12.9:1 - engaged button letters
        inline const juce::Colour controlLabelText{0xFFA5ADB2};    //  8.04:1 - functional text
        inline const juce::Colour captionTertiary{0xFF8A9196};     //  5.73:1 - captions, footer, scope status

        // Everything inside a display well.
        inline const juce::Colour ledWindowBg{0xFF07090A};
        inline const juce::Colour ledWindowBorder{0xFF363C41};
        inline const juce::Colour ledWindowDivider{0xFF2A3035};
        inline const juce::Colour ledWindowText{0xFFDFE6EA};    // 14.6:1 on #07090A
        inline const juce::Colour lcdParameterReadout{0xFFFFD9A0}; // 11.7:1 - only while a control moves

        // The one accent (BRAND.md's one-colour rule): the scope trace and the engine lamps.
        inline const juce::Colour chorusAccent{0xFFFF2B1C};

        // Delay-modulation scope. The annotations are drawn OPAQUE at #9BA3A8 (5.6:1 on the well);
        // the old rgba(160,178,186,.55) measured 3.11:1 and is gone.
        inline const juce::Colour scopeBorder{0xFF0A0C0D};
        inline const juce::Colour scopeBgTop{0xFF06080A};
        inline const juce::Colour scopeBgBottom{0xFF0B0F11};
        inline const juce::Colour scopeGrid{0x1A96B4BE};
        inline const juce::Colour scopeCentreLine{0x3896B4BE};
        inline const juce::Colour scopeInputUnderlay{0x38B2BEC5};
        inline const juce::Colour scopeAnnotation{0xFF9BA3A8};

        // SAVE / DELETE. Identical to Gatecrasher's and TapeRot's by design, not by accident - the
        // utility surface is shared across the suite.
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

    /** Which of the two re-rendered filmstrips a knob uses. */
    enum class KnobFilmstripSize { mod, global };

    /** How a filmstrip sheet is laid out, and how big the cap inside a frame is.

        The @1x sheets are vertical strips; the @2x sheets, when they arrive, are 8-column row-major
        grids, because a 168 x 21504 strip exceeds the maximum texture height on most targets.
        Carrying the shape as data means the swap is a table edit rather than a code change.

        `capFraction` is the load-bearing field. The generator PADS each frame for the drop shadow,
        so the cap is smaller than the frame pitch and the two must not be conflated - handoff
        section 4 warns that sizing from the pitch lands every control about 15% small, and the
        padding ratio differs between sheets (Gatecrasher's Ø136 cap sits in a 160 px box).

        The generator's own header states the rule: "the frame box is LARGER than the cap ... this is
        not padding: it is where the cast shadow fades to zero. Do not re-crop, and position knobs
        from the CAP centre, not the frame box." Measured on the shipped sheets, border alpha is <= 2
        everywhere, so the shadow really does die inside the frame.

        An earlier revision shipped sheets whose pitch WAS the cap diameter, which cost the shadow
        its room; drawing those into a box of the section-8 diameter happened to match the renders of
        the day. Do not reintroduce that by "simplifying" this field away. */
    struct FilmstripSheet
    {
        int framePx;
        int columns;       // 1 = vertical strip
        float capFraction; // cap diameter as a fraction of the frame pitch
    };

    namespace Layout
    {
        // The exported plate, border included. Everything else is measured from inside it.
        constexpr float canvasWidth = 1282.0f;
        constexpr float canvasHeight = 777.0f;
        constexpr float borderInset = 1.0f;
        constexpr float contentWidth = 1280.0f;
        constexpr float contentHeight = 775.0f;

        // Rotation range for every knob: pointer at 12 o'clock = centre.
        constexpr float knobArcStartDegrees = -135.0f;
        constexpr float knobArcEndDegrees = 135.0f;

        // No tick-ring constants here, deliberately. Revision 1 drew the rings itself at even
        // angular spacing; revision 2 bakes every tick into the plate at its LABELLED value
        // (section 7), which on the skewed Rate knob is not evenly spaced at all. A drawn ring would
        // lay even ticks over uneven printed ones. There is no mark table either - the plate is the
        // single source of truth for where a mark sits, and the spec is the record of why.

        constexpr int knobFrameCount = 128;
        // frame pitch, columns (1 = vertical strip), cap-to-pitch ratio. Taken from the shipped
        // generator's own table (design/tools/render-knob-filmstrips-chorus60.mjs): the mod cap is
        // Ø84 in a 112 box and the global Ø68 in a 92 box, and the difference between those two
        // ratios is real, not rounding - the global knob's shadow needs proportionally more room.
        // The @2x sheets are the same caps at 168/136 in 224/184, laid out as 8x16 row-major grids.
        inline constexpr FilmstripSheet modSheet{112, 1, 84.0f / 112.0f};
        inline constexpr FilmstripSheet globalSheet{92, 1, 68.0f / 92.0f};

        // ---- Section 4: regions ---------------------------------------------------------------
        constexpr float headerBandH = 78.0f;
        constexpr float footerBandY = 747.0f, footerBandH = 28.0f;

        constexpr float buttonColumnX = 22.0f, buttonColumnY = 96.0f;
        constexpr float buttonColumnW = 220.0f, buttonColumnH = 629.0f;

        constexpr float scopeBlockX = 285.0f, scopeBlockY = 96.0f, scopeBlockW = 973.0f, scopeBlockH = 140.0f;
        constexpr float scopeWellX = 285.0f, scopeWellY = 116.0f, scopeWellW = 973.0f, scopeWellH = 120.0f;
        constexpr float scopeCaptionRowH = 20.0f;   // block top to well top
        constexpr float scopeInnerInset = 2.0f;

        // Group boxes, confirmed by measuring the plate's own 1px black outlines: MOD ENGINE spans
        // x 285..1257 / y 252..491, CHARACTER x 285..851, OUTPUT x 868..1257, both y 508..724.
        constexpr float modEngineGroupX = 285.0f, modEngineGroupY = 252.0f;
        constexpr float modEngineGroupW = 973.0f, modEngineGroupH = 240.0f;
        constexpr float characterGroupX = 285.0f, characterGroupY = 508.0f;
        constexpr float characterGroupW = 567.0f, characterGroupH = 218.0f;
        constexpr float outputGroupX = 868.0f, outputGroupY = 508.0f;
        constexpr float outputGroupW = 390.0f, outputGroupH = 218.0f;

        // Heading row contents, measured off chorus60-page-i@2x.png: the MOD ENGINE box carries a
        // lit Ø8 LED at (308.5, 268.5) - dark, not merely dimmed, in bypass - with its heading text
        // starting at x 322 and the status note right-aligned to x 1237, 20 px inside the box. The
        // heading row sits ABOVE the rule, so none of it dims with the OFF state.
        constexpr float modEngineLedD = 8.0f;
        constexpr float modEngineLedCentreX = 308.5f, modEngineLedCentreY = 268.5f;
        constexpr float modEngineHeadingX = 322.0f;
        constexpr float modEngineStatusRight = 1237.0f;
        constexpr float modEngineHeadingRowY = 260.0f, modEngineHeadingRowH = 16.0f;

        // Each box's heading rule, below which the OFF state's multiply applies. Measured off
        // chorus60-page-off@2x.png as the row where the ratio against the bare plate steps from
        // 1.000 to 0.500: y 283 for MOD ENGINE and y 539 for the other two - 31 px below each box
        // top in all three cases.
        constexpr float groupHeadingRuleOffset = 31.0f;

        // ---- Section 8: knob positions ---------------------------------------------------------
        //
        // Section 7's cells are 176 x 164 (mod, knob centre at 88,82) and 158 x 144 (global, centre
        // at 79,72). Only the centres are needed here: the cell exists to place the printed scale,
        // and the printed scale is baked.
        constexpr float modKnobD = 84.0f;
        constexpr float globalKnobD = 68.0f;

        constexpr float modKnobCentreY = 376.0f;
        inline constexpr std::array<float, 4> modKnobCentreX{{402.0f, 598.0f, 794.0f, 990.0f}};

        // Control label row: "below the cell, 6 px gap". The mod cell's top is centre - 82 = 294,
        // so its bottom is 458 and the label row starts at 464. Only the mod-engine labels are drawn
        // (their suffix is page-dependent); the five global labels are baked.
        constexpr float modLabelRowY = 464.0f, modLabelRowH = 15.0f;
        constexpr float modCellW = 176.0f;

        constexpr float globalKnobCentreY = 620.0f;

        struct KnobSpec
        {
            const char* paramID;
            float cx, cy, diameter;
            KnobFilmstripSize size;
        };

        // The five genuinely global knobs. Their names are baked, so no displayName here.
        inline constexpr std::array<KnobSpec, 5> knobs{{
            {"drift",       389.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global},
            {"saturation",  569.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global},
            {"noise",       749.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global},
            {"mix",         973.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global},
            {"trim",       1153.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global},
        }};

        // ---- Section 7.2: the IMAGE switch ------------------------------------------------------
        //
        // Track and thumb ship as separate sprites so the thumb travels rather than crossfading.
        // STEREO is thumb-up. The printed STEREO / MONO words sit at the thumb centres and are baked.
        constexpr float switchCellX = 1098.0f, switchCellW = 132.0f;
        constexpr float switchTrackX = 1147.5f, switchTrackY = 343.0f;
        constexpr float switchTrackW = 34.0f, switchTrackH = 68.0f;
        constexpr float switchThumbX = 1151.5f, switchThumbD = 26.0f;
        constexpr float switchThumbYStereo = 347.0f, switchThumbYMono = 381.0f;
        constexpr float switchTravelMs = 260.0f;

        // ---- Section 10: the paged MOD ENGINE box ----------------------------------------------
        struct EnginePage
        {
            const char* rateID;
            const char* depthID;
            const char* centreID;
            const char* decorrID;
            const char* imageID;
            const char* title;      // group heading, e.g. "MOD ENGINE I+II"
            const char* statusNote; // right-aligned note in the heading row
            const char* suffix;     // appended to each slot label, e.g. "I+II"
        };

        inline constexpr EnginePage pageI{
            "rate1", "depth1", "center1", "decorr1", "image1",
            "MOD ENGINE I", "ENGINE I ENGAGED", "I"};
        inline constexpr EnginePage pageII{
            "rate2", "depth2", "center2", "decorr2", "image2",
            "MOD ENGINE II", "ENGINE II ENGAGED", "II"};
        inline constexpr EnginePage pageBoth{
            "rateB", "depthB", "centerB", "decorrB", "imageB",
            "MOD ENGINE I+II", "BOTH ENGAGED \xc2\xb7 MONO BBD PAIR", "I+II"};

        inline constexpr std::array<const char*, 4> slotLabels{
            {"RATE", "DEPTH", "DELAY CENTER", "DECORRELATION"}};
        constexpr const char* imageSlotLabel = "IMAGE";

        // Shown while nothing is engaged. Revision 2 dropped the "SETTINGS RETAINED" half: the
        // pointers no longer wind to zero, so there is no false impression left to correct.
        constexpr const char* bypassStatusNote = "BYPASS";

        // Page-change slew: "1 - 0.002^(dt/380ms)", time-based so travel takes the same wall time
        // whatever the frame rate and a dropped frame doesn't shorten it. A drag bypasses it.
        constexpr float slewSettleMs = 380.0f;
        constexpr float slewRemainderAtSettle = 0.002f;

        // ---- Section 9: the OFF state ----------------------------------------------------------
        constexpr float powerDownFadeMs = 340.0f;
        constexpr float powerDownMultiply = 0.50f;

        // ---- Section 4: the engine button column -----------------------------------------------
        //
        // 132 x 132 at x 26, distributed rather than centred: all four vertical gaps come out at
        // 41 px, which is the JN-80's own packed-block rhythm. Do not re-centre the stack.
        constexpr float buttonX = 26.0f, buttonW = 132.0f, buttonH = 132.0f;
        constexpr float buttonIIY = 183.0f, buttonIY = 356.0f, buttonOffY = 528.0f;
        constexpr float pressAnimMs = 110.0f;
        constexpr float pressOffsetPx = 3.0f;

        // Lamp sprites are 96 x 96 with the glow baked into the transparent margin, drawn CENTRED on
        // the lamp position rather than placed by their top-left. The OFF button has no lamp - that
        // position is empty on the hardware and stays empty here.
        constexpr float lampSpriteD = 96.0f;
        constexpr float lampIICentreX = 182.5f, lampIICentreY = 250.0f;
        constexpr float lampICentreX = 182.5f, lampICentreY = 422.5f;

        // Letter block: 16 px right of the button, then 12 px past the 15 px lamp.
        constexpr float engineLetterX = 202.0f, engineLetterW = 80.0f;

        // ---- Section 5: the PROGRAM LCD ---------------------------------------------------------
        //
        // Measured off the plate's own well borders (#363C41) rather than transcribed: the spec's
        // table gives x 571 for the window and 631 for the name field, but the plate has them at 593
        // and 654. The two numbers the spec and the plate agree on are the ones the character budget
        // rests on - a 59 px bank cell and a 352 px glyph run - so those are exact either way.
        constexpr float programWindowX = 593.0f, programWindowY = 33.0f;
        constexpr float programWindowW = 414.0f, programWindowH = 29.0f;
        constexpr float programTagCellX = 594.0f, programTagCellY = 34.0f;
        constexpr float programTagCellW = 59.0f, programTagCellH = 28.0f;
        constexpr float programNameCellX = 654.0f, programNameCellY = 34.0f;
        constexpr float programNameCellW = 352.0f, programNameCellH = 28.0f;

        // Share Tech Mono 15 px at .10em advances 9.6 px per character, so the 352 px run holds 36.
        // The longest strings that can appear are a 27-character "NN " + 24-char name and a
        // 25-character parameter readout, so both clear it with 90+ px to spare. Do not narrow the
        // window without re-checking those two.
        constexpr float lcdCssPx = 15.0f, lcdTrackingEm = 0.10f;
        constexpr int lcdCharacterBudget = 36;
        constexpr int maxProgramNameLength = 31; // mirrors ProgramManager::maxProgramNameLength

        // Held after the gesture ends before the program name returns (section 5).
        constexpr int lcdReadoutHoldMs = 900;

        // The chevron affordance at the right of the name field, added by the handoff README's
        // "Delta since the last spec revision" (it is not in CHORUS60-GUI-SPEC.md yet): 11 x 7,
        // 1.4 px stroke with square caps, currentColor at 0.75, vertically centred, inset 10 px
        // from the field's right edge, and the field takes 26 px of right padding to clear it.
        //
        // It is DRAWN, not baked, and only while the LCD is showing a stored program - it marks the
        // window as a picker, and there is nothing to pick during name entry or a parameter
        // readout. That state-dependence is exactly why it cannot live in the plate.
        constexpr float lcdChevronW = 11.0f, lcdChevronH = 7.0f;
        constexpr float lcdChevronStroke = 1.4f;
        constexpr float lcdChevronInsetRight = 10.0f;
        constexpr float lcdChevronAlpha = 0.75f;
        constexpr float lcdNameRightPadding = 26.0f;

        // SAVE / DELETE aren't wells, so the plate has nothing for them; these are measured off
        // chorus60-page-i@2x.png, which draws both.
        constexpr float saveButtonX = 1015.0f, saveButtonY = 34.0f, saveButtonW = 43.0f, saveButtonH = 28.0f;
        constexpr float deleteButtonX = 1066.0f, deleteButtonY = 34.0f, deleteButtonW = 55.0f, deleteButtonH = 28.0f;

        constexpr float inWindowX = 1139.0f, inWindowY = 34.0f, inWindowW = 54.0f, inWindowH = 28.0f;
        constexpr float outWindowX = 1203.0f, outWindowY = 34.0f, outWindowW = 54.0f, outWindowH = 28.0f;

        // Below this the IN/OUT readouts show -INF rather than a number: the plugin's own BBD clock
        // noise sits well above it, so anything lower is silence.
        constexpr float meterFloorDb = -60.0f;

        // ---- Scope internals --------------------------------------------------------------------
        constexpr float scopeHistorySeconds = 2.0f;
        constexpr float scopeFps = 60.0f;
        constexpr int scopeNumDivisions = 8;
        constexpr float scopeAmplitudeFraction = 0.34f;
        constexpr float scopeCentreOffsetFraction = 0.10f;

        // Delay Center's own range, from Parameters.h's delayCentreMs() - the scope offsets its
        // centre line by where the active configuration sits within it.
        constexpr float delayCenterRangeMid = 8.0f, delayCenterRangeHalf = 6.0f;

        // ModulationEngine's maxExcursionMs (5 ms) plus CharacterStage's maxDriftMs (0.15 ms). Both
        // live in anonymous namespaces in DSP .cpp files so aren't includable here; this is their
        // documented sum, named so the provenance is clear rather than a bare magic number.
        constexpr float scopeReferenceExcursionMs = 5.0f + 0.15f;

        // Footer status line, right-aligned in the footer band. It carries live engine state
        // ("... ENGAGED ..." / "... BYPASS ..."), which is why it is drawn rather than baked.
        constexpr float footerRight = 1258.0f, footerCentreY = 761.0f;
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

    /** The rect of one group box below its heading rule - the region the OFF state multiplies. */
    inline juce::Rectangle<float> groupDimRect(float x, float y, float w, float h) noexcept
    {
        return {x, y + Layout::groupHeadingRuleOffset, w, h - Layout::groupHeadingRuleOffset};
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
    // the spec's tracking values - same technique as GatecrasherTheme::drawTrackedText.
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

    // Barlow Condensed SemiBold (600 - control labels, captions, scope status) and Bold (700 - group
    // headings, engine button letters), Share Tech Mono Regular (everything inside a display) -
    // section 3. Librestile is gone with the wordmark, which the plate bakes now. Loaded once per
    // process via function-local statics, same caching pattern as GatecrasherTheme.h.
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
    // calibrating the ratio once off a reference string measured directly from the artwork, so one
    // real measurement scales every size on the panel. Same trap as Gatecrasher's.
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

    inline float monoFontHeightForCssPx(float cssPx)
    {
        // Share Tech Mono's own ratio, calibrated the same way: section 5 states 9.6 px of advance
        // per character at 15 CSS px with .10em tracking, so the glyph advance alone is 8.1 px.
        static const float ratio = [&]
        {
            constexpr float probeHeight = 40.0f, refCssPx = 15.0f;
            const float refWidth = 8.1f * 10.0f + trackingPxForEm(0.10f, refCssPx) * 9.0f;
            return fontHeightForTrackedWidth(monoFont(probeHeight), probeHeight, "0000000000",
                                              trackingPxForEm(0.10f, refCssPx), refWidth)
                 / refCssPx;
        }();
        return cssPx * ratio;
    }

    // Binary-data-backed images, decoded once per process via function-local statics - the knob
    // filmstrips in particular are tall sheets and must not be re-decoded per repaint.
    //
    // The background plate carries all the static furniture, glyphs included (handoff section 1).
    inline const juce::Image& panelBackgroundImage()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::chorus60backgroundplate2x_png, (size_t) BinaryData::chorus60backgroundplate2x_pngSize);
        return image;
    }

    inline const juce::Image& knobModFilmstrip()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::knob_mod_84px_128f_png, (size_t) BinaryData::knob_mod_84px_128f_pngSize);
        return image;
    }

    inline const juce::Image& knobGlobalFilmstrip()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::knob_global_68px_128f_png, (size_t) BinaryData::knob_global_68px_128f_pngSize);
        return image;
    }

    inline const juce::Image& engineButtonImage(int index) // 0 = II, 1 = I, 2 = OFF
    {
        static const juce::Image images[3] = {
            juce::ImageFileFormat::loadFrom(BinaryData::buttonii2x_png, (size_t) BinaryData::buttonii2x_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::buttoni2x_png, (size_t) BinaryData::buttoni2x_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::buttonoff2x_png, (size_t) BinaryData::buttonoff2x_pngSize)};
        return images[juce::jlimit(0, 2, index)];
    }

    inline const juce::Image& lampImage(bool lit)
    {
        static const juce::Image on = juce::ImageFileFormat::loadFrom(
            BinaryData::lampon2x_png, (size_t) BinaryData::lampon2x_pngSize);
        static const juce::Image off = juce::ImageFileFormat::loadFrom(
            BinaryData::lampoff2x_png, (size_t) BinaryData::lampoff2x_pngSize);
        return lit ? on : off;
    }

    inline const juce::Image& switchTrackImage()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::switchtrack2x_png, (size_t) BinaryData::switchtrack2x_pngSize);
        return image;
    }

    inline const juce::Image& switchThumbImage()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::switchthumb2x_png, (size_t) BinaryData::switchthumb2x_pngSize);
        return image;
    }

    // Value-text formatting for the LCD's parameter readout - the only live numeric display on the
    // panel (section 5). Hz -> 2 decimals ("0.45 Hz"), % -> whole numbers ("38 %"), ms -> 1 decimal
    // ("6.4 ms"), dB -> 1 decimal explicitly signed ("+0.0 dB"), and the IMAGE switch through its
    // own MONO/STEREO strings.
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
