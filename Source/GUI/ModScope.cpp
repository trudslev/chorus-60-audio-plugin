#include "ModScope.h"
#include "Chorus60Theme.h"
#include "../Parameters.h"
#include <cmath>

ModScope::ModScope(Chorus60AudioProcessor& processor) : processorRef(processor)
{
    noiseRaw = processorRef.apvts.getRawParameterValue(ParamIDs::noise);

    setInterceptsMouseClicks(false, false);
    startTimerHz(60);
}

ModScope::~ModScope()
{
    stopTimer();
}

void ModScope::timerCallback()
{
    using namespace Chorus60Theme::Layout;

    // The real modulator signal: sum of the (already engine-gated) modulation offset and the
    // ever-moving drift offset - see this class's own header comment and the getters' own
    // documentation in PluginProcessor.h.
    const float traceMs = processorRef.getModulationOffsetMs() + processorRef.getDriftOffsetMs();

    // Dry input-underlay amplitude: real signal level (getInputMeterDb(), mapped like a standard
    // -60..0dB meter) modulated by Noise per the spec's own underlay formula
    // ("0.20 x h x env x (0.35 + 0.65*noise)"), plus a little per-sample texture jitter so it reads
    // as grain rather than a smooth band - PluginProcessor doesn't expose a raw per-sample envelope,
    // so getInputMeterDb() is the closest real (not decorative) substitute for the mockup's "env".
    const float inputDb = processorRef.getInputMeterDb();
    const float inputNorm = juce::jlimit(0.0f, 1.0f, (inputDb + 60.0f) / 60.0f);
    const float noise01 = noiseRaw != nullptr ? juce::jlimit(0.0f, 1.0f, noiseRaw->load(std::memory_order_relaxed) * 0.01f) : 0.0f;
    const float baseHalfPx = scopeWellH * 0.20f * inputNorm * (0.35f + 0.65f * noise01);
    const float jitter = 0.6f + 0.4f * random.nextFloat();

    history[(size_t) writeIndex] = { traceMs, baseHalfPx * jitter };
    writeIndex = (writeIndex + 1) % historySize;

    const float pixelsPerFrame = scopeWellW / (scopeHistorySeconds * scopeFps);
    const float gridSpacing = scopeWellW / (float) scopeNumDivisions;
    gridScrollPhase = std::fmod(gridScrollPhase + pixelsPerFrame, gridSpacing);

    repaint();
}

void ModScope::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;
    using namespace Chorus60Theme::Layout;

    // Same resolver the audio thread uses, so the trace can never disagree with what is being
    // heard about which configuration is engaged.
    const auto active = processorRef.resolveActiveConfiguration();

    // --- Caption row -----------------------------------------------------------------------
    //
    /*  **"DELAY MODULATION" IS STILL NOT DRAWN HERE, AND THAT IS NOW A GAP RATHER THAN A RULE.**

        The reason it was left out was sound: the old handoff §1.1 listed it as baked, and drawing
        it as well double-printed it at a one-pixel offset — the failure that manifest existed to
        prevent. The revision-4 plate does not carry it. Measured, not assumed: a brightness sweep
        of the title's own line box on `chorus60-background-plate@3x.png` peaks at 20.7 against
        every panel ink being 110 upward, which is bare ground.

        **The failure mode inverted with the plate and the surviving comment kept arguing the old
        one.** It is left standing, corrected, rather than deleted: this casting's CLAUDE.md now
        carries the title as an enumerated absent row, and the sentence that used to justify the
        omission is the clearest possible statement of why the enumeration has to be checked against
        the asset rather than against the last manifest anyone read.
    */
    //
    // Only the status row is live, and section 1.2 gives it exactly two fields: the engine state and
    // the time division. The "DEPTH n%" that used to sit between them is gone with the standing
    // readouts - section 4 of the spec is explicit that the LCD is now the only numeric display on
    // the panel.
    using Configuration = Chorus60AudioProcessor::Configuration;
    const juce::String stateText = active.which == Configuration::both ? "ENGINE I + II"
                                  : active.which == Configuration::one ? "ENGINE I"
                                  : active.which == Configuration::two ? "ENGINE II"
                                  : "ENGINE BYPASS";

    const juce::String divText = "250 ms / DIV";

    // Section 3: Share Tech Mono 11 px, .06em, in the caption grey.
    const auto readoutFont = monoFont(monoFontHeightForCssPx(11.0f));
    const float readoutTracking = trackingPxForEm(0.06f, 11.0f);

    // The caption row is §1's: the well's own x and width, on the row above it at y 136.
    float cursorRight = scopeWellX + scopeWellW;
    const float readoutGap = 26.0f;
    for (const auto& text : { divText, stateText })
    {
        const float w = trackedTextWidth(text, readoutFont, readoutTracking);
        const juce::Rectangle<float> r(cursorRight - w, scopeCaptionRowY, w, scopeCaptionRowH);
        drawTrackedText(g, text, readoutFont, readoutTracking, r, juce::Justification::left,
                         Colour::captionTertiary);
        cursorRight = r.getX() - readoutGap;
    }

    // --- Scope rect (section 5: "same construction as Gatecrasher's envelope scope") ---
    const juce::Rectangle<float> outerRect(scopeWellX, scopeWellY, scopeWellW, scopeWellH);
    const auto innerRect = outerRect.reduced(scopeInnerInset);

    juce::ColourGradient bgGradient(Colour::scopeBgTop, innerRect.getCentreX(), innerRect.getY(),
                                     Colour::scopeBgBottom, innerRect.getCentreX(), innerRect.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(innerRect);
    g.setColour(Colour::scopeBorder);
    g.drawRect(outerRect, 1.0f);

    g.saveState();
    g.reduceClipRegion(innerRect.getSmallestIntegerContainer());

    const float pixelsPerFrame = scopeWellW / (scopeHistorySeconds * scopeFps);
    const float gridSpacing = scopeWellW / (float) scopeNumDivisions;

    // Scrolling vertical grid, moving in lockstep with the trace's own scroll (section 5: "8
    // vertical divisions scroll with the signal").
    g.setColour(Colour::scopeGrid);
    for (float x = innerRect.getRight() - gridScrollPhase; x >= innerRect.getX(); x -= gridSpacing)
        g.drawVerticalLine((int) x, innerRect.getY(), innerRect.getBottom());

    // Vertical centre offset by the engaged configuration's Delay Center (section 5's formula,
    // against Parameters.h's [2,14]ms range - see Chorus60Theme::Layout::delayCenterRangeMid/Half).
    const float delayCenterNorm =
        juce::jlimit(-1.0f, 1.0f, (active.centreMs - delayCenterRangeMid) / delayCenterRangeHalf);
    const float centreY = innerRect.getCentreY() + scopeCentreOffsetFraction * innerRect.getHeight() * delayCenterNorm;

    g.setColour(Colour::scopeCentreLine);
    g.drawHorizontalLine((int) centreY, innerRect.getX(), innerRect.getRight());

    const float amplitudePx = scopeAmplitudeFraction * innerRect.getHeight();

    const int visibleColumns = juce::jmin(historySize, (int) std::ceil(innerRect.getWidth() / pixelsPerFrame) + 1);

    juce::Path tracePath;
    bool firstPoint = true;
    juce::Point<float> lastPoint(innerRect.getRight(), centreY);

    for (int col = 0; col < visibleColumns; ++col)
    {
        const int age = visibleColumns - 1 - col; // 0 = newest (rightmost)
        const int idx = ((writeIndex - 1 - age) % historySize + historySize) % historySize;
        const float x = innerRect.getRight() - (float) age * pixelsPerFrame;
        if (x < innerRect.getX() - pixelsPerFrame)
            continue;

        const auto& sample = history[(size_t) idx];
        const float excursion01 = juce::jlimit(-1.0f, 1.0f, sample.traceMs / scopeReferenceExcursionMs);
        const float y = centreY - excursion01 * amplitudePx;

        // Dry input underlay - grey, never red, always behind the trace (section 5).
        g.setColour(Colour::scopeInputUnderlay);
        g.drawVerticalLine((int) x, y - sample.greyHalfPx, y + sample.greyHalfPx);

        if (firstPoint)
        {
            tracePath.startNewSubPath(x, y);
            firstPoint = false;
        }
        else
        {
            tracePath.lineTo(x, y);
        }
        lastPoint = {x, y};
    }

    if (!firstPoint)
    {
        // Glow pass: soft 20px shadow first, then the 7px-wide stroke itself on top (section 5:
        // "glow pass underneath at 7px rgba(255,43,28,.45) with a 20px shadow in rgba(255,43,28,.80)").
        juce::Path glowOutline;
        juce::PathStrokeType(7.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt)
            .createStrokedPath(glowOutline, tracePath);
        juce::DropShadow glowShadow(Colour::chorusAccent.withAlpha(0.80f), 20, {0, 0});
        glowShadow.drawForPath(g, glowOutline);
        g.setColour(Colour::chorusAccent.withAlpha(0.45f));
        g.fillPath(glowOutline);

        // Core pass: 10px shadow behind the actual 3px hard-mitred trace (section 5: "then the core
        // pass with a 10px shadow... Mitre joins... do not smooth it").
        juce::Path coreOutline;
        juce::PathStrokeType(3.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt)
            .createStrokedPath(coreOutline, tracePath);
        juce::DropShadow coreShadow(Colour::chorusAccent.withAlpha(0.70f), 10, {0, 0});
        coreShadow.drawForPath(g, coreOutline);

        g.setColour(Colour::chorusAccent);
        g.strokePath(tracePath, juce::PathStrokeType(3.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));

        // 4px filled dot at the right edge, on the current sample (section 5).
        g.setColour(Colour::chorusAccent.withAlpha(0.90f));
        g.fillEllipse(lastPoint.x - 2.0f, lastPoint.y - 2.0f, 4.0f, 4.0f);
    }

    g.restoreState();

    // Annotations, Share Tech Mono 9px (section 5).
    g.setColour(Colour::scopeAnnotation);
    g.setFont(monoFont(9.0f));
    g.drawText("DLY MOD", juce::Rectangle<float>(innerRect.getX() + 4.0f, innerRect.getY() + 2.0f, 80.0f, 12.0f),
               juce::Justification::centredLeft, false);
    g.drawText("+ MAX", juce::Rectangle<float>(innerRect.getRight() - 50.0f, innerRect.getY() + 2.0f, 46.0f, 12.0f),
               juce::Justification::centredRight, false);
    g.drawText("- MAX", juce::Rectangle<float>(innerRect.getRight() - 50.0f, innerRect.getBottom() - 14.0f, 46.0f, 12.0f),
               juce::Justification::centredRight, false);
}
