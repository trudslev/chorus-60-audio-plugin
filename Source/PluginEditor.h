#pragma once

#include "PluginProcessor.h"
#include "GUI/Chorus60PanelBackground.h"
#include "GUI/Chorus60EditorContent.h"
#include <juce_audio_processors/juce_audio_processors.h>

class Chorus60AudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit Chorus60AudioProcessorEditor(Chorus60AudioProcessor&);
    ~Chorus60AudioProcessorEditor() override;

    void resized() override;

private:
    Chorus60AudioProcessor& processorRef;
    Chorus60PanelBackground panelBackground;
    Chorus60EditorContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chorus60AudioProcessorEditor)
};
