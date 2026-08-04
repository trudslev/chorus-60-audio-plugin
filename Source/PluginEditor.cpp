#include "PluginEditor.h"

namespace
{
    constexpr int referenceWidth = (int) Chorus60Theme::Layout::canvasWidth;
    constexpr int referenceHeight = (int) Chorus60Theme::Layout::canvasHeight;
}

Chorus60AudioProcessorEditor::Chorus60AudioProcessorEditor(Chorus60AudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), content(p)
{
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
    content.setTransform(juce::AffineTransform::scale(scale));
    content.setBounds(0, 0, referenceWidth, referenceHeight);
}
