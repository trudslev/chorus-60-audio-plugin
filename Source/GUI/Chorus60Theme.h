#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include <BinaryData.h>
#include <array>
#include <cmath>

// Centralises every pixel constant from design/GUI-SPEC.md (palette, coordinates, the
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
#include <nf/HeaderPart.h>
#include <nf/ParameterReadout.h>

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
        // contrast: 15.24-15.37:1 vs plate:modEngineHeadingRow [functional]
        inline const juce::Colour engravedHeadingText{0xFFE6EBEE}; // engaged button letters
        inline const juce::Colour controlLabelText{0xFFA5ADB2};    //  8.04:1 - functional text
        // **Functional, not a caption.** All three uses carry live state: the MOD ENGINE status
        // note prints the engine configuration or BYPASS, the footer prints ENGAGED/BYPASS, and
        // the scope status row prints its division and state. Flavour is for text that can be
        // missed. Raised from #8A9196, which read 5.78 against the plate under a 7:1 bar.
        // contrast: 7.09-7.38:1 vs plate:modEngineHeadingRow,plate:modEngineStatusNote,plate:footerRow [functional]
        inline const juce::Colour captionTertiary{0xFF9CA2A6};

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
        /** **The Program buttons' two legends. There is no face colour here any more** - the
            plate bakes the face, and section 13 keeps only the legends at runtime, because each of
            the four lights independently while the face has no state to freeze.

            Nine constants went with the old treatment: a pale enabled face (#DBE0E3 -> #AAB1B6)
            with dark ink, a pressed face, a separate disabled face, and a disabled label that had
            itself just been rescued from #8B9297 at 1.42:1. **The pale face is what required the
            lamp-beside-legend form**; backlit legends need a dark face, so the face moved with the
            treatment, and the disabled state stopped existing rather than getting a better grey. */
        inline const juce::Colour legendLit{0xFFF1EFEA};
        /** Matte, not a dimmer ink - no bloom at all, which is what separates "unlit" from
            "slightly darker". Quoted at the worst case, the lightest part of the face.
            // contrast: 3.55:1 vs buttonCapTop [state] */
        inline const juce::Colour legendUnlit{0xFF757D82};
        /** Not drawn - the plate carries the face. Declared so the legend ratios above have a
            named ground to be measured against, and so a future re-cut has the value on record. */
        inline const juce::Colour buttonCapTop{0xFF23282B};
        inline const juce::Colour buttonCapBottom{0xFF15181A};
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
        /*  **1340 × 812, and the coordinate system is the CANVAS's now — not inside-border.**

            This was 1282 × 776 with a 1 px frame, `contentWidth/Height` at 1280 × 775, and the
            content component placed at (1, 1) so every `Layout` constant could be a literal
            inside-border figure. That was a reasonable trade while the whole panel was this
            casting's own.

            It stops being one now that the header is the shared part: `HEADER-PART.md` states its
            coordinates in **canvas** space — block at 16, 16, band at y 61 — so an inside-border
            layout would need every shared figure carried with a −1 hanging off it. **A one-pixel
            offset threaded through a coordinate system is exactly the term that survives review and
            then explains a figure that does not reproduce.** The content component sits at (0, 0) at
            full canvas size instead, and `borderInset` retires with the offset it existed for.

            The 1 px frame is still there — it is drawn by the plate, which is what it always was. */
        constexpr float canvasWidth = 1340.0f;
        constexpr float canvasHeight = 812.0f;

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
        // **The header band: y 32, outer height 34, shared by all five parts** - the LCD well,
        // both Program buttons and both meter windows. 34 is BRAND.md's suite figure rather than
        // this panel's: the castings are differently-sized units, not scales of one design. The
        // row was 28-29 here, with the LCD's border box a pixel larger than its own neighbours -
        // the drift the suite audit found in four castings.
        //
        // **Measured off the re-exported plate, not transcribed - again.** This file already
        // records the spec's table disagreeing with the plate once (x 571 against 593); it
        // disagrees again in this revision, giving the window as 451 wide at x 519 where the plate
        // has 414 at x 557. Only the RIGHT edge agrees (519 + 451 = 970, and the plate's name cell
        // ends at 969.5), so the two describe the same right-hand edge and different left-hand
        // ones. The plate wins, as it did last time.
        //
        // The whole LCD moved 36px LEFT this revision: the tag cell 594 -> 558 and the name cell
        // 654 -> 618, both by exactly 36.
        /*  **THE HEADER IS `nf::HeaderPart` NOW, AND THESE ARE ALIASES.**

            Every figure here was a literal, and the same figure was held in five sibling panels, in
            `HEADER-PART.md`, and again in the parts strip. Aliasing rather than renaming the call
            sites is the choice `FactoryPrograms.h` already made for `ProgramId`: several hundred
            references read the unqualified names, and renaming them would bury the change in noise.

            **The cell's own geometry moved substantially, and the LCD is where it shows.** The
            window was 414 wide at x 557; the part's is **641 at 357**. The tag cell was 59 and is
            **72**; the name cell was 352 and the part's name area is **538.00**. Those are not
            nudges — the shared band is a different size of part, and this casting's band had been
            built to its own canvas. */
        constexpr float programWindowX = (float) nf::HeaderGeometry::lcdX;
        constexpr float programWindowY = (float) nf::HeaderGeometry::bandY;
        constexpr float programWindowW = (float) nf::HeaderGeometry::lcdW;
        constexpr float programWindowH = (float) nf::HeaderGeometry::bandH;

        /** The bank cell and the name cell, both terms of the character budget — see below. The
            1 px divider between them is `nf::LcdCell::dividerW`. */
        constexpr float programTagCellX = programWindowX;
        constexpr float programTagCellY = programWindowY;
        constexpr float programTagCellW = nf::LcdCell::bankCellW;
        constexpr float programTagCellH = programWindowH;
        constexpr float programNameCellX = programTagCellX + programTagCellW + nf::LcdCell::dividerW;
        constexpr float programNameCellY = programWindowY;
        constexpr float programNameCellW = nf::LcdCell::nameAreaW;
        constexpr float programNameCellH = programWindowH;

        /*  **ONE RUN, ONE SOURCE. This carried THREE figures for one quantity.**

            `lcdCharacterBudget` read 36 — the 352 px cell divided by the advance. The paint path
            trimmed `lcdNameRightPadding` off that cell and drew into 326 px, which is 33. The naming
            field used a different inset again, `reduced (12, 0)` = 328 px, which is 34. And
            `readoutFormat()` handed core the **cell** figure, so the live readout believed it had
            three characters it does not have.

            None of the three was wrong about what it measured; they measured different things and
            were all called the budget. That is the recorded one-value-two-meanings shape with an
            extra head — and the cap, at 31, was derived from the middle one, so the only figure that
            constrains what a user can type was the only one nobody could find from the constant's
            name.

            **The drawn run is the source now.** The cell and the padding are stated; the run is
            their difference; the character count comes from measuring the advance of the face the
            paint path actually draws, which is the one term that genuinely belongs to the font
            rather than to the cell. `DisplayBudgetTests` does that measuring and asserts the run
            holds **cap + marker**, not merely the cap.

            When this casting's header moves onto the shared part, the cell becomes
            `nf::LcdCell::nameAreaW` (538.00) and **`lcdNameRightPadding` must go to zero**: the
            part's name area already excludes the chevron through its own 30 px trim, so keeping this
            one subtracts the same affordance twice. The arithmetic is exact and unforgiving — 538.00
            holds 49 and the shared cap is 47, so 47 + 2 fits precisely, while trimming 26 again
            leaves a 47-character run for a 47-character cap and no room for the marker. A cap may
            never shrink, so that is the one figure here that cannot be corrected after the fact. */
        constexpr float lcdCssPx = 17.0f, lcdTrackingEm = 0.10f;   // §8 / the part's LCD

        /** The name cell, and the padding that clears the chevron. */
        constexpr float lcdNameCellW_ = programNameCellW;
        /*  **ZERO NOW, and this is the figure that could not be undone if it were wrong.**

            It was 26 = the 11 px chevron + its 10 px inset + 5. That cleared the chevron out of a
            name cell which contained it. The part's name area does not: `nf::LcdCell::nameAreaW` is
            641 − 72 bank − 1 divider − **30 chevron trim**, so the chevron is already excluded
            before this constant is applied. Keeping 26 would subtract the same affordance twice.

            The arithmetic is exact and leaves no slack either way. 538.00 holds **49**; the shared
            cap is **47**; 47 + the 2-character dirty marker is 49, which fits precisely. Trim 26
            again and the run holds 47 for a 47-character cap, so an edited Program's " *" has
            nowhere to go — and **a cap may never shrink**, so that could not be corrected after
            anyone had saved against it. `DisplayBudgetTests` asserts the run against cap + marker
            rather than against the cap, which is the comparison that catches exactly this. */
        constexpr float lcdNameRightPadding = 0.0f;

        /** **The run the panel actually draws into** — every path uses this, including naming. */
        constexpr float lcdDrawnRunW = lcdNameCellW_ - lcdNameRightPadding;

        /** The trailing " *" on an edited Program. The cap must leave room for it. */
        constexpr int lcdDirtyMarkerChars = 2;

        /** The part's terms. `DisplayBudgetTests` still measures the advance off the face the paint
            path draws, because that is the one term belonging to the font rather than to the cell —
            so the two sides of the assertion keep coming from different places. */
        constexpr float lcdTrackingPx = nf::LcdCell::tracking;         // 1.700 at 17 px
        constexpr float lcdGlyphAdvance = nf::LcdCell::glyphAdvance;   // 9.180

        /*  **49 and 47 FROM CORE, where this computed its own from a 352 px cell.**

            §11's type-adoption gate: a casting does not take the shared budget until its own
            `fonts/` holds Share Tech Mono. It does — the face has been embedded here since before
            the round — so the gate is satisfied rather than waived, and with the cell now the
            part's there is nothing left to compute locally.

            The cap rises **31 → 47**, which is the only direction it may move. */
        constexpr int lcdCharacterBudget = nf::LcdCell::characterBudget();
        constexpr int maxProgramNameLength = nf::LcdCell::userNameCap();

        // Held after the gesture ends before the program name returns (section 5).
        constexpr int lcdReadoutHoldMs = 900;

        // The chevron affordance at the right of the name field, added by the handoff README's
        // "Delta since the last spec revision" (it is not in GUI-SPEC.md yet): 11 x 7,
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
        // lcdNameRightPadding lives with the LCD run above — it is a term of the character budget,
        // not a property of the chevron, and holding it beside the glyph is how it came to be
        // subtracted a second time from a cell that already excluded the chevron.

        /** **The plate bakes both button FACES now, and the build draws only the legends.**

            That is section 13's split: "the legends stay runtime text - never baked into the
            button bitmap", because each of the four can light independently and a baked legend
            would freeze one state's lighting into the face. The face itself has no state to
            freeze, so it belongs in the plate like every other static furniture.

            Measured on the re-exported plate at x 977..1047 and 1053..1123, y 32..66 - 70 x 34
            each, matching section 13's box. They were 43 x 28 and 55 x 28: two buttons of
            DIFFERENT widths, which the second legend fixes rather than causes (each is sized by
            its longest word, and DELETE and CANCEL are both six characters).

            **Nothing here draws a face.** Doing so would paint a live control over a baked copy of
            itself, which is the bug this casting's own notes keep naming. */
        /** 190px of vertical drag spans the full range, 760 while Shift is held. Suite figures:
            six castings had six drag feels - this one was on 200 - so the same hand got a different
            response from each. The Shift fine mode comes from Reflect-84. */
        constexpr int knobDragPixels = 190;
        constexpr int knobFineDragPixels = 760;

        constexpr float saveButtonX = 977.0f, saveButtonY = 32.0f, saveButtonW = 70.0f, saveButtonH = 34.0f;
        constexpr float deleteButtonX = 1053.0f, deleteButtonY = 32.0f, deleteButtonW = 70.0f, deleteButtonH = 34.0f;

        /** Two stacked legends, 10px on a 12px line box. 10px is BRAND.md's floor for functional
            text and both legends are functional, so neither is set smaller to make the pair fit. */
        constexpr float legendCssPx = 10.0f, legendTrackingEm = 0.12f, legendLineHeight = 12.0f;

        // These are text-centring boxes over wells the plate bakes, not wells the build draws.
        constexpr float inWindowX = 1139.0f, inWindowY = 32.0f, inWindowW = 54.0f, inWindowH = 34.0f;
        constexpr float outWindowX = 1203.0f, outWindowY = 32.0f, outWindowW = 54.0f, outWindowH = 34.0f;

        // Below this the IN/OUT readouts show -INF rather than a number: the plugin's own BBD clock
        // noise sits well above it, so anything lower is silence.
        constexpr float meterFloorDb = -60.0f;

    /*  The READOUT's upper bound, and it is a different quantity from any bar ceiling. Suite ruling
        2026-08-14 — the widest string the well can be asked to draw is five characters, as a
        guarantee. Without this the numerals were bounded only by how loud the signal got. */
    inline constexpr float meterCeilingDb = 99.9f;

    /*  **The IN/OUT readout's string, and it lives HERE rather than in ProgramHeader.cpp.**

        Same reason the parameter readout format does: `ProgramHeader.h` reaches `PluginProcessor.h`,
        whose `JucePlugin_*` macros exist only in the plugin target, so a test reading the format
        from there cannot link — and a test that declares its own copy asserts against itself and
        passes while the panel prints something else.

        Suite ruling 2026-08-14: floor sentinel, +99.9 ceiling, one decimal always, an explicit sign
        decision. The widest string is then FIVE characters as a guarantee rather than as a range. */
    inline juce::String formatMeterDb (float db)
    {
        if (db <= meterFloorDb)
            return "-INF";

        /*  **`> 0.0f`, and this casting printed `>= 0.0f`.** One value, two castings, no reason: at
            exactly 0.0 dB this read "+0.0" where Gatecrasher read "0.0". The plus means ABOVE unity
            and 0.0 dB is not, so `>=` printed a sign claiming something false. */
        const float clamped = juce::jlimit (meterFloorDb, meterCeilingDb, db);

        return (clamped > 0.0f ? "+" : "") + juce::String (clamped, 1);
    }


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
    /*  **The plate at 3×, and it carries far less than it did.** §Asset format: it holds the fascia
        gradient, the CHORUS badge and the box frames — and nothing else. Every label, tick, numeral,
        knob, lamp and the scope are drawn at runtime now, where the previous plate baked all of
        them. So a runtime draw that would once have double-printed over baked ink is now the only
        thing drawing it. */
    inline const juce::Image& panelBackgroundImage()
    {
        static const juce::Image image = juce::ImageFileFormat::loadFrom(
            BinaryData::chorus60backgroundplate3x_png, (size_t) BinaryData::chorus60backgroundplate3x_pngSize);
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
    /** **How this panel spells the LCD parameter readout.**

        A presentation decision, so it lives with the other presentation constants - and that
        placement is load-bearing for the test: ProgramHeader.h reaches PluginProcessor.h, which
        needs JucePlugin_* macros that only exist in the plugin target, so a test reading the format
        from there could not link. The test must read the SHIPPING format rather than a copy, or it
        asserts against itself.

        `asAuthored`: the value is left in whatever case its parameter produced. The IMAGE switch's
        MONO/STEREO already arrive upper-case from its own stringFromValue, which is where that
        decision belongs - re-casing it here would make the panel and the host's automation lane
        disagree, which is the failure this whole extraction exists to prevent.

        The revert is core's 900 ms, which is what this panel already used. */
    inline nf::ReadoutFormat readoutFormat()
    {
        nf::ReadoutFormat f;
        f.nameCharacterBudget = Layout::lcdCharacterBudget;
        return f;
    }

    /* formatParameterValue is GONE, and this note is here so its absence reads as deliberate.

       It formatted a value by switching on the parameter's LABEL - "Hz" to two places, "%" through
       roundToInt, "ms" to one, "dB" with an explicit sign - which is a SECOND formatting convention
       sitting beside the parameter's own. That is precisely the arrangement that lets a panel and a
       host's automation lane print the same control two different ways, and it is what hid a
       missing formatter in Elmer and shipped one in TapeRot.

       All four rules moved verbatim onto the parameters themselves, as stringFromValueFunctions on
       the four shared attribute sets in Parameters.h. The output is identical and the host now
       agrees with the panel. nf::describeParameter joins value and label.
    */
}
