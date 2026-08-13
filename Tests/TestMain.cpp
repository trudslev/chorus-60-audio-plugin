#include <juce_audio_processors/juce_audio_processors.h>
#include <nf/UserProgramDirectory.h>
#include <juce_gui_basics/juce_gui_basics.h>

int main()
{
    // **Without a MessageManager, AsyncUpdater::triggerAsyncUpdate() silently clears its own
    // pending flag** (juce_AsyncUpdater.cpp:80-83), so every deferred Program change never happens
    // and the tests that exercise it pass while proving nothing.
    juce::ScopedJuceInitialiser_GUI juceInit;

    // **Every suite in this process resolves User Programs under a scratch directory, not the
    // user's real one.** The test harness compiles the shipping AudioProcessor, which builds its
    // ProgramManager from the real per-OS path because that is its job — so without this, any test
    // constructing the processor can reach
    // ~/Library/Application Support/<Company>/<Product>/Programs.
    //
    // A comment saying "do not write there" is a convention, and a convention gets broken silently.
    // It is also the one most likely to be broken by someone doing the right thing: verifying the
    // Program list needs several saved Programs, and building that state by hand is the obvious way
    // to get it. A cleanup glob has already destroyed a Program a user had just saved.
    //
    // Installed before the runner for the same reason ScopedJuceInitialiser_GUI is: it has to be in
    // force before the first line of the first test. See nf/UserProgramDirectory.h.
    const nf::ScopedUserProgramDirectoryOverride programRedirect {
        juce::File::getSpecialLocation (juce::File::tempDirectory)
            .getChildFile ("NeonFoundryTestPrograms")
    };

    juce::UnitTestRunner runner;
    runner.runAllTests();

    for (int i = 0; i < runner.getNumResults(); ++i)
        if (runner.getResult(i)->failures > 0)
            return 1;

    return 0;
}
