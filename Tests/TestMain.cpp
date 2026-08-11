#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

int main()
{
    // **Without a MessageManager, AsyncUpdater::triggerAsyncUpdate() silently clears its own
    // pending flag** (juce_AsyncUpdater.cpp:80-83), so every deferred Program change never happens
    // and the tests that exercise it pass while proving nothing.
    juce::ScopedJuceInitialiser_GUI juceInit;

    juce::UnitTestRunner runner;
    runner.runAllTests();

    for (int i = 0; i < runner.getNumResults(); ++i)
        if (runner.getResult(i)->failures > 0)
            return 1;

    return 0;
}
