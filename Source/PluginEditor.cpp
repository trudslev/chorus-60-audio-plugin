#include "PluginEditor.h"

namespace
{
    // The exported plate's own size, 1 px outer border included.
    constexpr int referenceWidth = (int) Chorus60Theme::Layout::canvasWidth;
    constexpr int referenceHeight = (int) Chorus60Theme::Layout::canvasHeight;
}

Chorus60AudioProcessorEditor::Chorus60AudioProcessorEditor(Chorus60AudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), content(p)
{
    // The plate goes here rather than inside the content, for two reasons. It has to sit behind the
    // three DimmableGroups, each of which paints its own crop of the same plate so the OFF state can
    // multiply those regions; and drawing it at full canvas size lets the content sit inset by the
    // border, which is what keeps every Layout constant a literal inside-border spec value instead
    // of one with a +1 hanging off it.
    addAndMakeVisible(panelBackground);
    addAndMakeVisible(content);

    setResizable(true, true);
    if (auto* constrainer = getConstrainer())
    {
        constrainer->setFixedAspectRatio((double) referenceWidth / (double) referenceHeight);
        constrainer->setSizeLimits(referenceWidth / 2, referenceHeight / 2,
                                    referenceWidth * 2, referenceHeight * 2);
    }

    setSize(referenceWidth, referenceHeight);
}

Chorus60AudioProcessorEditor::~Chorus60AudioProcessorEditor() = default;

void Chorus60AudioProcessorEditor::resized()
{
    const float scale = (float) getWidth() / (float) referenceWidth;

    panelBackground.setTransform(juce::AffineTransform::scale(scale));
    panelBackground.setBounds(0, 0, referenceWidth, referenceHeight);

    // **Content sits at the canvas origin now.** It used to be offset by the plate's 1 px border so
    // that every Layout constant could be an inside-border literal; the shared header part states
    // its coordinates in canvas space, so that offset would have to be carried through every shared
    // figure as a −1. The border is the plate's, which is what it always was.
    content.setTransform(juce::AffineTransform::scale(scale));
    content.setBounds(0, 0, referenceWidth, referenceHeight);
}
