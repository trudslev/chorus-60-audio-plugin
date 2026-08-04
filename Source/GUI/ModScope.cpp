#include "ModScope.h"
#include "Chorus60Theme.h"
#include "../Parameters.h"
#include <cmath>

ModScope::ModScope(Chorus60AudioProcessor& processor) : processorRef(processor)
{
    engine1Raw = processorRef.apvts.getRawParameterValue(ParamIDs::engine1);
    engine2Raw = processorRef.apvts.getRawParameterValue(ParamIDs::engine2);
    depth1Raw = processorRef.apvts.getRawParameterValue(ParamIDs::depth1);
    depth2Raw = processorRef.apvts.getRawParameterValue(ParamIDs::depth2);
    delayCenterRaw = processorRef.apvts.getRawParameterValue(ParamIDs::delayCenter);
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
    const float baseHalfPx = scopeH * 0.20f * inputNorm * (0.35f + 0.65f * noise01);
    const float jitter = 0.6f + 0.4f * random.nextFloat();

    history[(size_t) writeIndex] = { traceMs, baseHalfPx * jitter };
    writeIndex = (writeIndex + 1) % historySize;

    const float pixelsPerFrame = scopeW / (scopeHistorySeconds * scopeFps);
    const float gridSpacing = scopeW / (float) scopeNumDivisions;
    gridScrollPhase = std::fmod(gridScrollPhase + pixelsPerFrame, gridSpacing);

    repaint();
}

void ModScope::paint(juce::Graphics& g)
{
    using namespace Chorus60Theme;
    using namespace Chorus60Theme::Layout;

    const bool engine1On = engine1Raw != nullptr && engine1Raw->load(std::memory_order_relaxed) > 0.5f;
    const bool engine2On = engine2Raw != nullptr && engine2Raw->load(std::memory_order_relaxed) > 0.5f;
    const bool engaged = engine1On || engine2On;

    // --- Caption row: "DELAY MODULATION" + right-aligned state/depth/division readouts (section 5) ---
    const juce::Rectangle<float> captionRect(captionRowX, captionRowY, captionRowW, captionRowH);
    drawTrackedText(g, "DELAY MODULATION", labelFontBold(11.0f), 11.0f * 0.28f, captionRect,
                     juce::Justification::centredLeft, engaged ? Colour::engravedHeadingText : Colour::captionTertiary);

    const juce::String stateText = (engine1On && engine2On) ? "ENGINE I + II"
                                  : engine1On ? "ENGINE I"
                                  : engine2On ? "ENGINE II"
                                  : "ENGINE BYPASS";
    const float depthSum = (engine1On && depth1Raw != nullptr ? depth1Raw->load(std::memory_order_relaxed) : 0.0f)
                          + (engine2On && depth2Raw != nullptr ? depth2Raw->load(std::memory_order_relaxed) : 0.0f);
    const juce::String depthText = "DEPTH " + juce::String((int) std::round(depthSum)) + "%";
    const juce::String divText = "250 ms / DIV";

    const auto readoutFont = monoFont(11.0f);
    g.setFont(readoutFont);
    g.setColour(Colour::captionTertiary);

    float cursorRight = captionRowX + captionRowW;
    const float readoutGap = 26.0f;
    for (const auto& text : { divText, depthText, stateText })
    {
        const float w = juce::GlyphArrangement::getStringWidth(readoutFont, text);
        const juce::Rectangle<float> r(cursorRight - w, captionRowY, w, captionRowH);
        g.drawText(text, r, juce::Justification::centredLeft, false);
        cursorRight = r.getX() - readoutGap;
    }

    // --- Scope rect (section 5: "same construction as Gatecrasher's envelope scope") ---
    const juce::Rectangle<float> outerRect(scopeX, scopeY, scopeW, scopeH);
    const auto innerRect = outerRect.reduced(scopeInnerInset);

    juce::ColourGradient bgGradient(Colour::scopeBgTop, innerRect.getCentreX(), innerRect.getY(),
                                     Colour::scopeBgBottom, innerRect.getCentreX(), innerRect.getBottom(), false);
    g.setGradientFill(bgGradient);
    g.fillRect(innerRect);
    g.setColour(Colour::scopeBorder);
    g.drawRect(outerRect, 1.0f);

    g.saveState();
    g.reduceClipRegion(innerRect.getSmallestIntegerContainer());

    const float pixelsPerFrame = scopeW / (scopeHistorySeconds * scopeFps);
    const float gridSpacing = scopeW / (float) scopeNumDivisions;

    // Scrolling vertical grid, moving in lockstep with the trace's own scroll (section 5: "8
    // vertical divisions scroll with the signal").
    g.setColour(Colour::scopeGrid);
    for (float x = innerRect.getRight() - gridScrollPhase; x >= innerRect.getX(); x -= gridSpacing)
        g.drawVerticalLine((int) x, innerRect.getY(), innerRect.getBottom());

    // Vertical centre offset by Delay Center (section 5's formula, adapted to Parameters.h's actual
    // [5,15]ms range - see Chorus60Theme::Layout::delayCenterRangeMid/Half's comment).
    const float delayCenterNorm = delayCenterRaw != nullptr
        ? juce::jlimit(-1.0f, 1.0f, (delayCenterRaw->load(std::memory_order_relaxed) - delayCenterRangeMid) / delayCenterRangeHalf)
        : 0.0f;
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
