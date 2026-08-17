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

        /** §3's tick ink and §3.1's numerals — the same `#a5adb2` the control labels use, which is
            why they are aliases rather than three constants that happen to agree. One value, one
            meaning: if the label ink moves, the ring moves with it, because they are the same ink. */
        inline const juce::Colour& knobTick = controlLabelText;
        inline const juce::Colour& knobNumeral = controlLabelText;

        /** §6's `#b6bec2`: the nameplate's model line, and the PROGRAM / IN / OUT captions. §10's
            item 6 raised all four together — the body had been carrying `#8a9196` at **4.60**
            against its own header block, which that round records as the worst functional figure it
            found, while the six-material strip already held the corrected hex.
            // contrast: 7.79:1 vs plate:headerBlock [functional] */
        inline const juce::Colour captionSecondary{0xFFB6BEC2};
        // **Functional, not a caption.** All three uses carry live state: the MOD ENGINE status
        // note prints the engine configuration or BYPASS, the footer prints ENGAGED/BYPASS, and
        // the scope status row prints its division and state. Flavour is for text that can be
        // missed. Raised from #8A9196, which read 5.78 against the plate under a 7:1 bar.
        // contrast: 7.09-7.38:1 vs plate:modEngineHeadingRow,plate:modEngineStatusNote,plate:footerRow [functional]
        inline const juce::Colour captionTertiary{0xFF9CA2A6};

        /*  **THE HEADER BLOCK'S OWN MATERIAL. §1: `linear-gradient(180deg, #24292c, #171a1c)`.**

            Not on the plate — drawn. It was silkscreen for one revision, which is why these
            constants did not exist and why `ProgramHeader` had a comment saying the plate provided
            the wells.

            **This is the ground every header contrast figure in §6 is quoted against**, so it is
            named rather than inlined: `#24292c` is the block's lightest row and therefore the worst
            case for anything printed on it, which is the figure §6 publishes. */
        inline const juce::Colour headerBlockTop{0xFF24292C};
        inline const juce::Colour headerBlockBottom{0xFF171A1C};
        inline const juce::Colour headerBlockRing{0xFF0A0C0D};       // inset 0 0 0 1px
        inline const juce::Colour headerBlockTopLight{0x12FFFFFF};   // inset 0 1px 0 rgba(255,255,255,.07)
        inline const juce::Colour headerBlockFootShade{0x80000000};  // inset 0 -3px 8px rgba(0,0,0,.5)

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
        /** **The Program buttons' two legends**, and — again — the face they sit on.

            Nine constants went with the revision-1 treatment: a pale enabled face (#DBE0E3 ->
            #AAB1B6) with dark ink, a pressed face, a separate disabled face, and a disabled label
            that had itself just been rescued from #8B9297 at 1.42:1. **The pale face is what
            required the lamp-beside-legend form**; backlit legends need a dark face, so the face
            moved with the treatment, and the disabled state stopped existing rather than getting a
            better grey. That reasoning stands and is why the face below is dark.

            What changed twice is only *who draws it*. Revision 2 baked it, and this file said "there
            is no face colour here any more". Revision 4's plate does not, so `buttonCapTop` and
            `buttonCapBottom` below have gone from a documentation record back to live paint. */
        inline const juce::Colour legendLit{0xFFF1EFEA};
        /** Matte, not a dimmer ink - no bloom at all, which is what separates "unlit" from
            "slightly darker". Quoted at the worst case, the lightest part of the face.
            // contrast: 3.55:1 vs buttonCapTop [state] */
        inline const juce::Colour legendUnlit{0xFF757D82};
        /*  **The Program buttons' face, drawn again as of the revision-4 plate — and BOTH VALUES
            WERE WRONG WHILE NOTHING DREW THEM.**

            They read `#23282B` / `#15181A`; §1 and the delivered prototype both say
            `#23282C` / `#14181B`. One digit out in each, in a constant whose stated purpose was
            *"declared so the legend ratios above have a named ground, and so a future re-cut has the
            value on record"*.

            **A record kept for accuracy, drifting from the thing it recorded, invisibly, because
            nothing consumed it.** That is correct-by-nobody-looking rather than correct-by-
            coincidence: there was no line making it right, and no line that would have gone wrong
            if it moved. A value with no consumer has no way to be checked — which is an argument
            for deriving such a record from its source or deleting it, not for writing it down more
            carefully.

            It cost nothing here: the error is below a JND and the ratio it grounds moves in the
            third decimal. What it demonstrates is the mechanism, on a value that was written down
            *precisely so it could be trusted later*, and later is now. */
        inline const juce::Colour buttonCapTop{0xFF23282C};
        inline const juce::Colour buttonCapBottom{0xFF14181B};
        inline const juce::Colour buttonCapRing{0xFF0B0D0F};        // inset 0 0 0 1px
        inline const juce::Colour buttonCapTopLight{0x1AFFFFFF};    // inset 0 1px 0 rgba(255,255,255,.10)
        inline const juce::Colour buttonCapFootShade{0x8C000000};   // inset 0 -2px 5px rgba(0,0,0,.55)

        /** The recess inside a well: `inset 0 2px 6px rgba(0,0,0,.9)` under the 1 px `#363c41`
            ring. The LCD and both meter wells carry it. */
        inline const juce::Colour ledWindowRecess{0xE6000000};
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

        // ---- Section 1: regions -----------------------------------------------------------------
        //
        /*  **Every figure here is §1's, and the PLATE was measured against §1 rather than the other
            way round.** The three box frames are the only body geometry the new plate still carries,
            so they are the one place the asset can contradict the spec — and it does not: measured
            off `chorus60-background-plate@3x.png`, all twelve edges land on §1 to within a third of
            a canvas pixel (the residue is one 3x subpixel on the two soft bottom edges).

            That is the declaration-as-known-case check, and it is worth stating which way it ran:
            the spec is the authority and the plate agreeing is what makes transcribing it safe.
            A disagreement here would have been information about the delivery, not licence to
            follow the artwork. */
        constexpr float badgeX = 24.0f, badgeY = 136.0f, badgeW = 238.0f, badgeH = 46.0f;
        constexpr float badgeFootY = 740.0f, badgeFootH = 26.0f;

        constexpr float buttonColumnX = 24.0f, buttonColumnY = 196.0f;
        constexpr float buttonColumnW = 132.0f, buttonColumnH = 534.0f;

        constexpr float scopeCaptionRowY = 136.0f, scopeCaptionRowH = 20.0f;
        constexpr float scopeWellX = 285.0f, scopeWellY = 160.0f, scopeWellW = 1039.0f, scopeWellH = 120.0f;
        constexpr float scopeInnerInset = 2.0f;

        constexpr float footerY = 782.0f, footerLineBox = 13.0f;

        /*  **The three group boxes, and their heading row is part of the box now.**

            Each box carries a 30 px heading row — title at `x + pad`, `y + 8`, on a 15 px line box —
            closed by a rule at `y + 30`. `pad` is 31 where a lamp precedes the title and 14 where
            nothing does, which is the only difference between the three rows.

            **`hasLamp` is not decoration: it is what makes the two pads different**, so the pad is
            carried beside it rather than as a bare number somebody would later even out. */
        struct GroupBox
        {
            const char* title;
            float x, y, w, h;
            float headingPad;
            bool hasLamp;
        };

        inline constexpr std::array<GroupBox, 3> groupBoxes{{
            { "MOD ENGINE", 285.0f, 296.0f, 1039.0f, 240.0f, 31.0f, true  },
            { "CHARACTER",  285.0f, 552.0f,  567.0f, 214.0f, 14.0f, false },
            { "OUTPUT",     868.0f, 552.0f,  456.0f, 214.0f, 14.0f, false },
        }};

        constexpr float groupHeadingRowH = 30.0f;
        constexpr float groupHeadingTextTop = 8.0f, groupHeadingLineBox = 15.0f;
        constexpr float groupHeadingCssPx = 12.0f, groupHeadingTrackingEm = 0.28f;

        // The MOD ENGINE box's lamp, inside its heading row at (+14, +11), Ø9.
        constexpr float groupLampD = 9.0f, groupLampX = 14.0f, groupLampY = 11.0f;

        /*  **THE WHOLE BOX DIMS NOW, HEADING INCLUDED — this was "from the heading rule down".**

            Revision 2 dimmed each box from its rule downward, and that boundary was *measured*: the
            ratio against the bare plate stepped from 1.000 to 0.500 exactly 31 px below each box top
            in `chorus60-page-off@2x.png`. The measurement was sound and it is now superseded — §7.2
            gives the OFF row as "**brightness 0.5**, specular off, MOD ENGINE title to `#8a9196`"
            against a knob-group column, and the delivered prototype wraps the boxes, the knobs and
            the IMAGE switch in ONE `filter: brightness(0.5)` layer.

            **Its provenance is the prototype, not a render, because bundle 4 delivered no artwork.**
            The four `chorus60-page-*@2x.png` composites are the previous canvas and the previous
            treatment; measuring the new behaviour off them would return the old boundary confidently.
            Stated here rather than left implicit, because a figure with a render behind it and a
            figure with a prototype behind it are different claims. */
        inline juce::Rectangle<float> groupDimRect (const GroupBox& box) noexcept
        {
            return { box.x, box.y, box.w, box.h };
        }

        // ---- Section 8: knob positions ---------------------------------------------------------
        //
        // Section 7's cells are 176 x 164 (mod, knob centre at 88,82) and 158 x 144 (global, centre
        // at 79,72). Only the centres are needed here: the cell exists to place the printed scale,
        // and the printed scale is baked.
        /*  **§3's two classes: primary Ø76, standard Ø56.** These were Ø84 and Ø68 — this casting's
            own diameters, chosen per panel, which call 3 replaces with the suite's pair.

            The rows moved differently and it is worth stating, because one of them is uniform and
            the other is not. **The primary row shifts +23 across all four.** The standard row does
            not: DRIFT, SATURATION and NOISE stay exactly where they were, while **MIX and OUTPUT
            TRIM move +33** — the two lower boxes absorbing call 1's +58 of width. Three of five
            staying put makes the two that move look like transcription slips when the row is checked
            as a group, so they are checked one at a time against §3. */
        /** §3's tick and numeral figures. The 29.5 offset is the catalogue's clearance chain —
            r + 8 ink gap + 9 major tick + 6 clearance + 6.5 half line box — which this casting
            follows exactly, so none of these is a per-casting term. */
        constexpr float knobTickInkGap = 2.0f;
        constexpr float knobMajorTickLength = 9.0f, knobMajorTickWidth = 2.0f;
        constexpr float knobMinorTickLength = 5.0f, knobMinorTickWidth = 1.5f;
        constexpr float knobNumeralRingOffset = 29.5f;
        constexpr float knobNumeralCssPx = 11.0f, knobNumeralTrackingEm = 0.0f;
        constexpr float knobUnitDrop = 6.0f;

        constexpr float modKnobD = 76.0f;
        constexpr float globalKnobD = 56.0f;

        constexpr float modKnobCentreY = 416.0f;
        inline constexpr std::array<float, 4> modKnobCentreX{{425.0f, 621.0f, 817.0f, 1013.0f}};

        constexpr float globalKnobCentreY = 660.0f;

        /*  **THE UNIT AND THE LABEL ARE ONE STACK UNDER EVERY KNOB, MEASURED FROM ITS OWN BOX.**

            The delivered prototype places both as offsets inside the knob's own d x d box, which
            sits at `(cx - r, cy - r)`:

                unit   top: d + 20   ->  cy + r + 20,  on a 13 px line box
                label  top: d + 34   ->  cy + r + 34,  on a 15 px line box

            One expression for all nine, primary and standard alike, which is why they are functions
            of `(cy, d)` rather than a constant per row.

            **THIS REPLACES A DERIVED `modLabelRowY` THAT WAS 16 PX OUT, AND ITS OWN COMMENT HAD
            CALLED THE SHOT.** That figure was 504, reached by taking the previous 464 and adding the
            40 the pivot had moved — holding the label's gap to the pivot because "the spec does not
            restate this casting's mod cell". The comment ended: *a figure computed from a cell
            nobody restated would be an invention wearing arithmetic.* It was raised as an ask on
            exactly that ground, and the bundle answered it before the ask was written: 416 + 38 + 34
            is **488**.

            The lesson is not that the arithmetic was careless — it was carefully done, and it
            preserved a relationship that was real in revision 2. It is that **preserving a
            relationship is not the same as knowing the figure**, and the only thing that could tell
            them apart was a source restating it. So the check for the next one of these is not "is
            the derivation sound" but "**is there anything left that states this, and have I read
            it**". */
        constexpr float knobUnitTopOffset = 20.0f, knobUnitLineBox = 13.0f;
        constexpr float knobLabelTopOffset = 34.0f, knobLabelLineBox = 15.0f;
        constexpr float knobUnitCssPx = 10.0f, knobUnitTrackingEm = 0.16f;
        constexpr float knobLabelCssPx = 12.0f, knobLabelTrackingEm = 0.18f;

        /** Width to lay a centred label in. Wide enough for DECORRELATION and OUTPUT TRIM, and
            centred on the knob, so it is a drawing box rather than a cell anything else measures
            from — the figure the line above exists to stop being invented. */
        constexpr float knobCaptionBoxW = 180.0f;

        constexpr float knobUnitTop (float centreY, float diameter) noexcept
        {
            return centreY + diameter * 0.5f + knobUnitTopOffset;
        }

        constexpr float knobLabelTop (float centreY, float diameter) noexcept
        {
            return centreY + diameter * 0.5f + knobLabelTopOffset;
        }


        /*  **THE PRINTED SCALES, AUTHORED FROM §3.1 — there is nothing left to check them against.**

            This casting's own notes said *"the plate is the single source of truth for where a mark
            sits"*, and that plate is being replaced by one carrying only the fascia, the badge and
            the box frames. So these tables are not a transcription of anything: they are the marks,
            and §3.1 is their only authority.

            **Stored as VALUES, not as rotation fractions**, which is stronger than either. A mark's
            angle comes from `range.convertTo0to1 (value)` on the parameter that actually drives the
            pointer, so a taper change moves the ring with the pointer instead of leaving numerals
            pointing where the pointer never goes — BRAND.md's correctness requirement, and the same
            construction `nf::printedScaleDefects` checks.

            **RATE is why this matters and why it could not be inferred.** Its range is 0.05–16 Hz at
            skew 0.35, so its five marks land at f 0 / 0.286852 / 0.479232 / 0.783722 / 1 —
            reproduced to six decimals from the range, and nothing like even spacing. An evenly
            spaced ring would look entirely plausible: the pointer would still land on marks, the
            marks would still look deliberate, and every value between them would be wrong. Same trap
            as Reflect-84's DAMPING HF, which *gained* a minor no reasoning from the dropped numerals
            would have produced.

            `printed == nullptr` is a **minor**: a tick with no numeral. §3.1 gives the standard class
            three numerals with the demoted positions keeping their ticks — what is dropped is the
            numeral, never the mark. */
        struct ScaleMark
        {
            float value;              // in the parameter's own units
            const char* printed;      // nullptr = minor tick, no numeral

            constexpr bool isMajor() const noexcept { return printed != nullptr; }
        };

        /** RATE — §3.2's five, skewed. Do NOT even these out. */
        inline constexpr ScaleMark rateMarks[] {
            { 0.05f, "0.05" }, { 0.5f, "0.5" }, { 2.0f, "2" }, { 8.0f, "8" }, { 16.0f, "16" } };

        /** DEPTH · DECORRELATION — even fifths, all five numeralled (primary class). */
        inline constexpr ScaleMark percentPrimaryMarks[] {
            { 0.0f, "0" }, { 25.0f, "25" }, { 50.0f, "50" }, { 75.0f, "75" }, { 100.0f, "100" } };

        /** DELAY CENTER — even fifths in ms. */
        inline constexpr ScaleMark delayCentreMarks[] {
            { 2.0f, "2" }, { 5.0f, "5" }, { 8.0f, "8" }, { 11.0f, "11" }, { 14.0f, "14" } };

        /** DRIFT · SATURATION · NOISE · MIX — three numerals, minors holding .25 and .75. */
        inline constexpr ScaleMark percentStandardMarks[] {
            { 0.0f, "0" }, { 25.0f, nullptr }, { 50.0f, "50" }, { 75.0f, nullptr }, { 100.0f, "100" } };

        /** OUTPUT TRIM — three numerals with the leading plus kept, minors at the quarters. */
        inline constexpr ScaleMark trimMarks[] {
            { -12.0f, "-12" }, { -6.0f, nullptr }, { 0.0f, "0" },
            { 6.0f, nullptr }, { 12.0f, "+12" } };

        /** **The printed minus is U+2212 and the tables store ASCII `-`, deliberately.**

            `juce::String`'s `const char*` constructor decodes **Latin-1, not UTF-8**, so a
            `"\xe2\x88\x92"` literal reaches the panel as three stray glyphs rather than a minus —
            the trap this suite already records for a middle dot rendering as `\u00c2\u00b7`. Storing
            ASCII and substituting from a codepoint at the draw call keeps the table readable and the
            glyph correct, and it is the same shape as Reflect-84's `Text::withRealMinus`. */
        inline juce::String withRealMinus (const char* printed)
        {
            return juce::String (printed).replaceCharacter ('-', juce::juce_wchar (0x2212));
        }

        /** A ring: the marks, how many, and the unit that prints in the arc's bottom gap.
            `unit == nullptr` means the scale is bare — §3.1 gives six of the nine a unit. */
        struct KnobScale
        {
            const ScaleMark* marks;
            int count;
            const char* unit;
        };

        struct KnobSpec
        {
            const char* paramID;
            const char* label;      // the printed control name — plain, never page-suffixed
            float cx, cy, diameter;
            KnobFilmstripSize size;
            KnobScale scale;
        };

        /** The mod row's four rings, in slot order — RATE, DEPTH, DELAY CENTER, DECORRELATION.
            They are a separate table from `knobs` because their PARAMETER is page-dependent while
            their ring is not: the page moves the pointer, never the marks. §2.1 is explicit that
            selecting a page moves pointers rather than adding or removing controls. */
        inline constexpr std::array<KnobScale, 4> modKnobScales{{
            { rateMarks,           5, "Hz" },
            { percentPrimaryMarks, 5, "%"  },
            { delayCentreMarks,    5, "ms" },
            { percentPrimaryMarks, 5, "%"  },
        }};

        // The five genuinely global knobs. Their names were baked into the previous plate and are
        // drawn now, which is why `label` is here at all.
        inline constexpr std::array<KnobSpec, 5> knobs{{
            {"drift",      "DRIFT",        389.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global,
                 { percentStandardMarks, 5, "%" }},
            {"saturation", "SATURATION",   569.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global,
                 { percentStandardMarks, 5, "%" }},
            {"noise",      "NOISE",        749.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global,
                 { percentStandardMarks, 5, "%" }},
            {"mix",        "MIX",         1006.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global,
                 { percentStandardMarks, 5, "%" }},
            {"trim",       "OUTPUT TRIM", 1186.0f, globalKnobCentreY, globalKnobD, KnobFilmstripSize::global,
                 { trimMarks, 5, "dB" }},
        }};

        // ---- Section 5: the IMAGE switch --------------------------------------------------------
        //
        /*  §5: a 128-wide cell at (1112, 382) holding a 34 x 68 sprite and both legends to its
            right, STEREO above MONO, on 13 px line boxes 3 px inside a 68-tall column.

            **The horizontal placement is a flex centring, so it is COMPUTED rather than stored.**
            The prototype's row is `justify-content:center` with a 12 px gap, which means the pair
            is centred on the cell as a unit and the sprite's x depends on how wide the wider legend
            renders. Storing an x would freeze one font's metrics into the layout — the same defect
            as a stored rotation fraction freezing one taper. `switchSpriteX()` measures instead.

            **Both legends are printed permanently and neither moves or re-inks**: the sprite's own
            position is the state (§4B applied to a sprite part). So there is no lit/unlit ink here,
            deliberately — one colour, and it is the printed-unit grey. */
        constexpr float switchCellX = 1112.0f, switchCellY = 382.0f, switchCellW = 128.0f;
        constexpr float switchSpriteW = 34.0f, switchSpriteH = 68.0f;
        constexpr float switchLegendGap = 12.0f;
        constexpr float switchLegendInset = 3.0f, switchLegendLineBox = 13.0f;
        constexpr float switchLegendCssPx = 10.0f, switchLegendTrackingEm = 0.14f;
        constexpr const char* switchLegendStereo = "STEREO";
        constexpr const char* switchLegendMono = "MONO";

        // STEREO sits at the column's top inset; MONO at its bottom, which is space-between on a
        // 68-tall column padded 3 px at each end.
        constexpr float switchLegendStereoTop = switchCellY + switchLegendInset;
        constexpr float switchLegendMonoTop =
            switchCellY + switchSpriteH - switchLegendInset - switchLegendLineBox;

        /*  **THE TWO-PART SPRITE, WHICH §5 SUPERSEDES AND THIS PASS DID NOT REPLACE.**

            `ImageSwitch` draws an empty track with a Ø26 thumb travelling 34 px over it on a spring
            ease, and these are that artwork's terms. §7.3 replaces both with **one composite per
            state** — `switch-stereo@2x.png` / `switch-mono@2x.png`, delivered at 102 x 204 (3x of
            34 x 68, and their `@2x` names are stale per §5) — selected by which image is rendered.

            **A whole-switch composite cannot have a separately travelling thumb**, so adopting the
            delivered pair deletes the travel rather than restyling it. That is a design consequence
            and not a tidy-up, so it is raised with the designers rather than decided here; the
            printed legends beside it are this pass's row and are drawn. Left standing so the switch
            keeps working meanwhile, and marked so nobody reads it as current. */
        constexpr float switchThumbInset = 4.0f, switchThumbTravel = 34.0f, switchThumbD = 26.0f;
        constexpr float switchTravelMs = 260.0f;

        // ---- Section 2.1: the paged MOD ENGINE box ----------------------------------------------
        //
        /*  **A PAGE IS NOW FOUR PARAMETER IDs AND NOTHING ELSE — §2.1 DELETED EVERY STRING ON IT.**

            It carried three: a heading (`MOD ENGINE I+II`), a right-aligned status note
            (`BOTH ENGAGED · MONO BBD PAIR`) and a suffix appended to each slot label
            (`DELAY CENTER I+II`). §2.1 replaces all three with one sentence — *"No panel text
            relabels itself on a page change. Knob labels are plain control names; the lamps say
            which engine is live. That is the whole mechanism, and it is why the row can be one set
            of four dials rather than three."*

            **Two independent sources and a before/after**, which is what makes this a reading of the
            design rather than an inference from a prototype default: §2.1 states the rule, the
            delivered prototype passes `title: 'MOD ENGINE'` with `tag: ''` and names its dials
            `RATE` / `DEPTH` / `DELAY CENTER` / `DECORRELATION`, and the SUPERSEDED prototype in
            `design/prototype/` still carries `suffix:' I+II'` and `note:'BOTH ENGAGED …'` — so the
            three strings did not fail to be written, they were taken out.

            **What replaces the status note is §7.2's re-ink**, not nothing: on OFF the MOD ENGINE
            title goes to `#8a9196`. The note said "BYPASS" in words; the title says it in ink.

            The consequence worth naming: with the suffix gone, the four paged labels and the five
            global ones are the same kind of string, so they are drawn by one pass rather than two.
            `ModSlotLabels` existed only to carry the suffix and is deleted with it. */
        struct EnginePage
        {
            const char* rateID;
            const char* depthID;
            const char* centreID;
            const char* decorrID;
            const char* imageID;
        };

        inline constexpr EnginePage pageI   { "rate1", "depth1", "center1", "decorr1", "image1" };
        inline constexpr EnginePage pageII  { "rate2", "depth2", "center2", "decorr2", "image2" };
        inline constexpr EnginePage pageBoth{ "rateB", "depthB", "centerB", "decorrB", "imageB" };

        /** The mod row's four printed names, in slot order. Plain, and the same on every page —
            which is §2.1's whole point, so do not re-introduce a per-page variant here. */
        inline constexpr std::array<const char*, 4> slotLabels{
            {"RATE", "DEPTH", "DELAY CENTER", "DECORRELATION"}};

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

        /*  **THE BLOCK'S MATERIAL, WHICH THE PLATE STOPPED CARRYING AND NOTHING DREW.**

            `ProgramHeader` painted the LCD's contents, both button legends and the meter values on
            to a block it assumed was silkscreen — its own comment said so: *"the background plate
            provides the empty PROGRAM / IN / OUT wells (their frames and recesses)"*. True of the
            revision-2 plate. The revision-4 plate carries no header at all: a full-width scan across
            the band's centre line returns **one flat value from x 1 to x 1338**.

            So the block, four wells and two button faces are drawn here now. Geometry is core's —
            every rect below is `nf::HeaderGeometry`'s — and only the *material* is this casting's,
            which is the split `HeaderPart.h` §I draws.

            All of it is §1 and the delivered prototype, transcribed once. */
        constexpr float blockCornerRadius = 5.0f;
        constexpr float wellCornerRadius = 3.0f;
        constexpr float buttonCornerRadius = 4.0f;

        /** §8's Program legend row, reused for the three captions above the band — Barlow Condensed
            600 at 10 / 13. The two trackings differ deliberately; see `ProgramHeader::paint`. */
        constexpr float headerCaptionCssPx = 10.0f;
        constexpr float programCaptionTrackingEm = 0.24f;
        constexpr float meterCaptionTrackingEm = 0.28f;
        constexpr const char* programCaption = "PROGRAM";
        constexpr const char* programCaptionNaming = "NAME PROGRAM";

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

        /*  **THE NAMEPLATE STACK, AND THE MODEL LINE'S y IS DERIVED FROM CORE RATHER THAN READ OFF
            THE PROTOTYPE.**

            Three lines inside the part's 303 x 84 nameplate zone: the wordmark, the function
            descriptor, and the model line under it. The zone is core's; what goes in it is this
            casting's, per `HeaderPart.h` §I — six metaphors are six paint routines.

            The stack closes on the shared anchor, and that is checkable rather than asserted:

                wordmark   top nameplateY = 30, Librestile Ext 28 on a 32 px line box
                + leading  16
                descriptor top 78 == nf::HeaderGeometry::descriptorY          <- the anchor
                + its own  17 == nf::HeaderGeometry::descriptorH
                model line top 95

            So the model line is `descriptorY + descriptorH`, and `HeaderPartTests` already owns
            `landsOnDescriptorAnchor (30, 32, 16)`. **Read this as catching divergence, not as
            asserting provenance**: a re-typed 95 and this sum are indistinguishable while they
            agree, and the arm's whole value is the moment §4 moves the anchor and this casting does
            not follow.

            **The wordmark and the descriptor above it are ABSENT from the panel today** — not baked
            on the new plate and with no drawing site — which the plate survey found and the
            enumeration in this casting's CLAUDE.md now carries as its own rows. They are not drawn
            here because the wordmark additionally needs `LibrestileExtBold.ttf` in BinaryData,
            which is a CMakeLists change and its own step. */
        constexpr float nameplateX = (float) nf::HeaderGeometry::nameplateX;
        constexpr float nameplateY = (float) nf::HeaderGeometry::nameplateY;
        constexpr float nameplateW = (float) nf::HeaderGeometry::nameplateW;

        constexpr float wordmarkCssPx = 28.0f, wordmarkLineBox = 32.0f, wordmarkTrackingEm = 0.02f;
        constexpr float nameplateLeading = 16.0f;

        /** **A literal, NOT `JucePlugin_Name`**, and that is not a style preference. The
            `JucePlugin_*` macros are defined in the plugin target only, and `ProgramHeader.cpp`
            compiles into the test target as well — so reading the wordmark from the macro would not
            link. Same reason the readout format lives in this theme rather than in `ProgramHeader`.

            It matches `CHORUS60_PRODUCT_NAME` in CMakeLists, and `HeaderPartTests` has no way to
            check that from here: two spellings of one name is the cost of the target split. */
        constexpr const char* wordmarkText = "CHORUS-60";

        constexpr float descriptorY = (float) nf::HeaderGeometry::descriptorY;
        constexpr float descriptorLineBox = (float) nf::HeaderGeometry::descriptorH;
        constexpr float descriptorCssPx = 14.0f, descriptorTrackingEm = 0.26f;
        constexpr const char* descriptorText = "BBD CHORUS PROCESSOR";

        /*  **CORE STATES THIS; AN EARLIER VERSION OF THIS FILE DERIVED IT.** It read
            `descriptorY + descriptorLineBox`, which is 95 and is right — and `nf::HeaderGeometry`
            already carries `modelLineY = 95` and `modelLineH = 14` as §2 figures. Deriving a value
            the shared part states is a second source for one quantity, which is the thing this
            casting spent a whole round removing from its LCD budget.

            The derivation is not lost: it is an assertion in `PrintedScaleTests` now, where it
            catches the two disagreeing rather than quietly picking one. */
        constexpr float modelLineY = (float) nf::HeaderGeometry::modelLineY;
        constexpr float modelLineBox = (float) nf::HeaderGeometry::modelLineH;
        constexpr float modelLineCssPx = 11.0f, modelLineTrackingEm = 0.20f;

        /** `MODEL CH-60 · STEREO`. The middle dot is a CODEPOINT, never a UTF-8 literal —
            `juce::String`'s `const char*` constructor decodes Latin-1, so `"\xc2\xb7"` reaches the
            panel as two stray glyphs. Same handling as `withRealMinus`. */
        inline juce::String modelLineText()
        {
            return "MODEL CH-60 " + juce::String::charToString (juce::juce_wchar (0x00B7)) + " STEREO";
        }

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

        /*  **THE BUTTONS AND THE METER WELLS WERE STILL ON THE PREVIOUS CANVAS, AND ONLY DRAWING
            THE MATERIAL SHOWED IT.**

            The header pass aliased the LCD to `nf::HeaderGeometry` and left these six rects as
            literals from the old panel:

                SAVE     977, 32, 70 x 34   ->  core  1006, 61, 62 x 34
                DELETE  1053, 32, 70 x 34   ->  core  1076, 61, 70 x 34
                IN      1139, 32, 54 x 34   ->  core  1164, 61, 64 x 34
                OUT     1203, 32, 54 x 34   ->  core  1238, 61, 64 x 34

            **29 px right and 29 px down**, and two of the four are the wrong width as well.

            Nothing could see it. The plate baked the wells and the faces, so the only consequence
            was text centred inside a box nobody drew — a legend a few pixels off a printed face
            reads as kerning, not as a coordinate error. The moment the material had to be painted
            from these rects, the legends appeared one whole band above their faces.

            **That is what an alias is FOR, and the pass only did half of it.** `programWindow*`
            moved to core and these did not, so the header held two coordinate systems that agreed
            about nothing. They are core's now, with no literal left to drift. */
        constexpr float saveButtonX = (float) nf::HeaderGeometry::saveX;
        constexpr float saveButtonY = (float) nf::HeaderGeometry::bandY;
        constexpr float saveButtonW = (float) nf::HeaderGeometry::saveW;
        constexpr float saveButtonH = (float) nf::HeaderGeometry::bandH;
        constexpr float deleteButtonX = (float) nf::HeaderGeometry::deleteX;
        constexpr float deleteButtonY = (float) nf::HeaderGeometry::bandY;
        constexpr float deleteButtonW = (float) nf::HeaderGeometry::deleteW;
        constexpr float deleteButtonH = (float) nf::HeaderGeometry::bandH;

        /** Two stacked legends. §8 gives the Program legend **11 on a 13 px line box**; this read
            10 / 12, which is the previous revision's and is below the pair the shared part sizes
            its 34 px band for — `HeaderGeometry::bandH`'s own comment says the band is 34 because
            "two 11 px legends with leading and padding need about 27". So the band was built for a
            size this casting had stopped using. 11 clears BRAND.md's ~10 px functional floor with
            room, where 10 sat exactly on it. */
        constexpr float legendCssPx = 11.0f, legendTrackingEm = 0.12f, legendLineHeight = 13.0f;

        constexpr float inWindowX = (float) nf::HeaderGeometry::inWellX;
        constexpr float inWindowY = (float) nf::HeaderGeometry::bandY;
        constexpr float inWindowW = (float) nf::HeaderGeometry::meterWellW;
        constexpr float inWindowH = (float) nf::HeaderGeometry::bandH;
        constexpr float outWindowX = (float) nf::HeaderGeometry::outWellX;
        constexpr float outWindowY = (float) nf::HeaderGeometry::bandY;
        constexpr float outWindowW = (float) nf::HeaderGeometry::meterWellW;
        constexpr float outWindowH = (float) nf::HeaderGeometry::bandH;

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

        /*  §4's title and the three annotations inside the well, all absent from the revision-4
            plate and drawn now.

            **The annotations are drawn OPAQUE at `#9ba3a8`, 7.52:1.** They were
            `rgba(160,178,186,.55)` at **3.11** — opacity-driven hierarchy, which BRAND.md forbids —
            and §10's item 8 replaced the treatment rather than the colour: hierarchy here is carried
            by position alone. Chorus-60 is the casting whose comment recorded deleting that, and
            Gatecrasher still carried it at 3.40 afterwards, which is the divergence table's fourth
            row. Do not reintroduce an alpha on these. */
        constexpr const char* scopeTitle = "DELAY MODULATION";
        constexpr float scopeTitleCssPx = 12.0f, scopeTitleTrackingEm = 0.28f;

        constexpr float scopeAnnotationCssPx = 11.0f, scopeAnnotationTrackingEm = 0.06f;
        constexpr float scopeAnnotationInsetX = 12.0f, scopeAnnotationInsetY = 8.0f;
        constexpr float scopeAnnotationLineBox = 14.0f;
        constexpr const char* scopeAnnotationSignal = "DLY MOD";

        /** `+ MAX` and `− MAX`. The minus is U+2212 and is substituted at the draw call for the
            same reason `withRealMinus` exists — a UTF-8 literal reaches the panel as stray glyphs,
            because `juce::String`'s `const char*` constructor decodes Latin-1. */
        inline juce::String scopeAnnotationMax (bool positive)
        {
            return (positive ? juce::String ("+")
                             : juce::String::charToString (juce::juce_wchar (0x2212))) + " MAX";
        }
        constexpr float scopeAmplitudeFraction = 0.34f;
        constexpr float scopeCentreOffsetFraction = 0.10f;

        // Delay Center's own range, from Parameters.h's delayCentreMs() - the scope offsets its
        // centre line by where the active configuration sits within it.
        constexpr float delayCenterRangeMid = 8.0f, delayCenterRangeHalf = 6.0f;

        // ModulationEngine's maxExcursionMs (5 ms) plus CharacterStage's maxDriftMs (0.15 ms). Both
        // live in anonymous namespaces in DSP .cpp files so aren't includable here; this is their
        // documented sum, named so the provenance is clear rather than a bare magic number.
        constexpr float scopeReferenceExcursionMs = 5.0f + 0.15f;

        /*  The footer's two strings, both on §1's y 782 with a 13 px line box.

            The RIGHT one carries live engine state (`... ENGAGED ...` / `... BYPASS ...`), which is
            why it was drawn even when the plate baked everything else. The LEFT one — `CH-60 · SN
            0061` — is static, was baked, and is **absent from the new plate with no drawing site**;
            it is one of the rows the plate survey added to this casting's enumeration. */
        constexpr float footerLeftX = 24.0f;
        constexpr float footerRight = 1324.0f;

        /** `CH-60 · SN 0061`. Middle dot from a codepoint, never a UTF-8 literal — see
            `modelLineText`. The serial is the prototype's and is the hardware conceit, not a real
            unit number; it is the same on every instance. */
        inline juce::String footerLeftText()
        {
            return "CH-60 " + juce::String::charToString (juce::juce_wchar (0x00B7)) + " SN 0061";
        }
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

    /*  **ONE ENUMERABLE SOURCE FOR THE NINE RINGS, so the pass can be COUNTED rather than looked
        at.** A paint pass that draws "the rings" looks complete on a panel while one knob silently
        has none: the knob still draws, the pointer still moves, and only a comparison against the
        old plate shows it. The enumeration has nine ring rows and nine numeral rows and each strikes
        on its own, so the drawing has to be enumerable too.

        Both tables feed this — the four paged mod rings and the five global ones — and nothing else
        draws a mark. `Tests/PrintedScaleTests` walks what this returns and names any knob missing
        from it. */
    struct RingToDraw
    {
        const char* paramID;                    // for naming a failure, not for lookup
        const char* label;                      // the printed control name, §2.1-plain
        juce::Point<float> centre;
        float diameter;
        Layout::KnobScale scale;
        juce::NormalisableRange<float> range;   // what actually drives the pointer
    };

    /** The nine rings the panel draws, in one enumerable list. **Defined in `KnobScaleRing.cpp`**
        rather than here, because it reads the parameter ranges and this header must not depend on
        the APVTS — the theme describes the panel, not the plugin. */
    std::vector<RingToDraw> ringsToDraw();

    /** The rings whose knob sits inside one group box.

        **The partition is DERIVED, not assigned.** Each box draws its own printed layer, so the
        nine rings have to be split three ways — and hand-listing which knob belongs to which box
        is exactly how one goes missing while every box looks populated. Filtering the one
        enumerable list by containment means the three subsets provably cover it: `PrintedScaleTests`
        asserts they sum to nine and share no member. */
    std::vector<RingToDraw> ringsInBox (const Layout::GroupBox& box);

    /** Draws one knob's whole printed stack — ticks, numerals, the unit and the control label — and
        returns how many MAJORS it numeralled, so a caller can count what it produced instead of
        trusting that it ran. */
    int drawKnobScale (juce::Graphics& g, const RingToDraw& ring);

    /** Draws one group box's heading. Returns the heading's own line-box rect, so a caller can
        assert where it landed rather than re-deriving it. */
    juce::Rectangle<float> drawGroupHeading (juce::Graphics& g, const Layout::GroupBox& box,
                                             juce::Colour ink);

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
    // section 3. Loaded once per process via function-local statics, same caching pattern as
    // GatecrasherTheme.h.
    //
    // **Librestile is BACK.** It was dropped when the wordmark was baked; this file said so for one
    // revision. The revision-4 plate carries no nameplate, so the wordmark is drawn again and the
    // face has to be embedded again with it. It is the only user of this typeface on the panel.
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

    inline juce::Typeface::Ptr librestileTypeface()
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
    /*  **The wordmark takes a CSS px directly, via `withPointHeight`, and does NOT get a calibrated
        ratio like the other two faces.**

        `labelFontHeightForCssPx` and `monoFontHeightForCssPx` exist because a spec's `font-size` is
        an **em** size while `FontOptions(h)` sets **ascent + descent**; each fits a reference string
        to a reference width to recover its face's ratio. That machinery needs a measured reference,
        and there is none for Librestile — this casting has published none, and inventing one would
        be exactly the figure-with-no-measurement-behind-it this suite keeps finding.

        `withPointHeight (px)` is the call that already means what a spec means, so it needs no
        reference at all. Root CLAUDE.md says so where it records the trap; the two ratio converters
        predate that being known and are kept because their calibrations ARE measured. */
    inline juce::Font wordmarkFont(float cssPx)
    {
        return juce::Font(juce::FontOptions().withTypeface(librestileTypeface())
                                              .withPointHeight(cssPx));
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

    /*  **THE HEADER'S MATERIAL. Every one of these was silkscreen for one revision.**

        `HeaderPart.h` §I is explicit that material is the casting's and geometry is core's, so the
        rects come from `nf::HeaderGeometry` and the treatments come from §1. They are functions
        rather than a component because three different things draw parts of this header —
        `ProgramHeader` owns the LCD and the buttons, the block sits behind all of it — and a
        treatment used three times is one routine or it is three that drift.

        **The shading is CSS `inset` box-shadow, which has no JUCE equivalent**, so each is built
        from the two pieces JUCE does have: a 1 px ring for a zero-blur inset, and a short vertical
        gradient clipped to the shape for a blurred one. That approximation is stated rather than
        hidden — a `blur 8` inset is not a linear ramp — and it is the same construction the group
        boxes and the scope well already use. */
    inline void fillRoundedVertical (juce::Graphics& g, juce::Rectangle<float> r, float radius,
                                     juce::Colour top, juce::Colour bottom)
    {
        g.setGradientFill ({ top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false });
        g.fillRoundedRectangle (r, radius);
    }

    /** A blurred inset shadow along one horizontal edge, clipped to the rounded shape. `depth` is
        the CSS blur; `fromTop` picks which edge it hangs from. */
    inline void insetEdgeShade (juce::Graphics& g, juce::Rectangle<float> r, float radius,
                                juce::Colour ink, float depth, bool fromTop)
    {
        juce::Path shape;
        shape.addRoundedRectangle (r, radius);

        juce::Graphics::ScopedSaveState saved (g);
        g.reduceClipRegion (shape);

        const float y0 = fromTop ? r.getY() : r.getBottom();
        const float y1 = fromTop ? r.getY() + depth : r.getBottom() - depth;
        g.setGradientFill ({ ink, r.getX(), y0, ink.withAlpha (0.0f), r.getX(), y1, false });
        g.fillRect (fromTop ? r.withHeight (depth)
                            : r.withTop (r.getBottom() - depth));
    }

    /** §1's header block: the material every other header element sits on, and the ground §6's
        header contrast figures are quoted against. */
    inline void paintHeaderBlock (juce::Graphics& g)
    {
        const auto r = nf::HeaderGeometry::block().toFloat();

        fillRoundedVertical (g, r, Layout::blockCornerRadius,
                             Colour::headerBlockTop, Colour::headerBlockBottom);

        insetEdgeShade (g, r, Layout::blockCornerRadius, Colour::headerBlockFootShade, 8.0f, false);

        // inset 0 1px 0 rgba(255,255,255,.07) — a hairline, not a ramp.
        g.setColour (Colour::headerBlockTopLight);
        g.fillRect (r.withTrimmedLeft (Layout::blockCornerRadius)
                     .withTrimmedRight (Layout::blockCornerRadius).withHeight (1.0f));

        g.setColour (Colour::headerBlockRing);
        g.drawRoundedRectangle (r.reduced (0.5f), Layout::blockCornerRadius, 1.0f);
    }

    /** A display well — the LCD and both meters. `#07090a` glass in a `#363c41` ring, recessed. */
    inline void paintDisplayWell (juce::Graphics& g, juce::Rectangle<float> r)
    {
        g.setColour (Colour::ledWindowBg);
        g.fillRoundedRectangle (r, Layout::wellCornerRadius);

        insetEdgeShade (g, r, Layout::wellCornerRadius, Colour::ledWindowRecess, 6.0f, true);

        g.setColour (Colour::ledWindowBorder);
        g.drawRoundedRectangle (r.reduced (0.5f), Layout::wellCornerRadius, 1.0f);
    }

    /** A Program button's face. Dark, because the legends are backlit — see `legendLit`. */
    inline void paintProgramButtonFace (juce::Graphics& g, juce::Rectangle<float> r)
    {
        fillRoundedVertical (g, r, Layout::buttonCornerRadius,
                             Colour::buttonCapTop, Colour::buttonCapBottom);

        insetEdgeShade (g, r, Layout::buttonCornerRadius, Colour::buttonCapFootShade, 5.0f, false);

        g.setColour (Colour::buttonCapTopLight);
        g.fillRect (r.withTrimmedLeft (Layout::buttonCornerRadius)
                     .withTrimmedRight (Layout::buttonCornerRadius).withHeight (1.0f));

        g.setColour (Colour::buttonCapRing);
        g.drawRoundedRectangle (r.reduced (0.5f), Layout::buttonCornerRadius, 1.0f);
    }

    /*  **§5's IMAGE row is a flex centring, so its x is MEASURED at draw time.**

        The delivered prototype's row is `width:128; justify-content:center; gap:12` holding a
        34 x 68 sprite and a legend column. The pair is centred on the cell, so where the sprite
        starts depends on how wide `STEREO` renders in the shipping build — which is a property of
        the font binary, not of the design.

        **Storing an x would freeze one font's metrics into the layout**, which is the same defect
        as a stored rotation fraction freezing one taper: correct until the thing it was derived
        from moves, and silent when it does. This casting has both faces calibrated at runtime
        already (`labelFontHeightForCssPx` fits a reference string to a reference width), so the
        measurement is the cheaper of the two. */
    inline float switchLegendColumnW()
    {
        const auto font = labelFont (labelFontHeightForCssPx (Layout::switchLegendCssPx));
        const float tracking = trackingPxForEm (Layout::switchLegendTrackingEm,
                                                Layout::switchLegendCssPx);
        return juce::jmax (trackedTextWidth (Layout::switchLegendStereo, font, tracking),
                           trackedTextWidth (Layout::switchLegendMono, font, tracking));
    }

    inline float switchSpriteX()
    {
        const float total = Layout::switchSpriteW + Layout::switchLegendGap + switchLegendColumnW();
        return Layout::switchCellX + (Layout::switchCellW - total) * 0.5f;
    }

    inline float switchLegendX()
    {
        return switchSpriteX() + Layout::switchSpriteW + Layout::switchLegendGap;
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
