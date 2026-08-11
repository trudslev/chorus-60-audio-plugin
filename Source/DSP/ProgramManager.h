#pragma once

#include "FactoryPrograms.h"
#include <functional>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

// Owns factory + user program bookkeeping and file I/O - reused directly from Gatecrasher's own
// ProgramManager (same class shape, same semantics), per explicit instruction rather than
// redesigned. See gatecrasher/Source/DSP/ProgramManager.h for the original.
//
// Async-safe the same way both siblings' preset/program switching is: a host can call
// requestProgramChange from a non-message thread (VST3 delivers program-change as an automatable
// parameter, which can arrive during audio-thread automation), so the actual apply is deferred
// through AsyncUpdater. requestProgramChange is safe from any thread; handleAsyncUpdate always
// runs on the message thread.
//
// Save always creates a new named user program and never overwrites an existing one, even the
// currently-loaded one - there is no "overwrite" or "New Program" method anywhere in this
// interface. Starting fresh is just loading any program, tweaking APVTS parameters, and calling
// saveNewUserProgram.
class ProgramManager : private juce::AsyncUpdater
{
public:
    explicit ProgramManager(juce::AudioProcessorValueTreeState& stateToControl);
    ~ProgramManager() override;

    // Call once from PluginProcessor's constructor, after the APVTS/parameters exist.
    void initialise();

    /** The Factory bank's size - what the host is told, and it never changes. */
    int getNumPrograms() const noexcept { return kNumFactoryPrograms; }

    ProgramId getCurrentProgramId() const;
    static ProgramId factoryIdAt(int factoryPosition);
    static ProgramId initId();
    static int factoryPositionOf(const juce::String& slug);

    /** The Factory position of the current Program, or 0 when it is INIT, a User Program or
        unresolved - none of which the host's list contains. */
    int getCurrentFactoryPosition() const;

    ProgramId resolve(ProgramBank bank, const juce::String& id, const juce::String& displayName) const;
    std::vector<ProgramId> listPrograms() const;

    /** **What the LCD and the dropdown print - a label, not a key.** Only Factory Programs get the
        two-digit number, computed from their bank position at paint time. */
    juce::String displayLabelFor(const ProgramId& id) const;

    /** User Program files rejected on the last scan because their schema could not be read, with
        the reason. **Surfaced rather than dropped**: a Program vanishing from the dropdown with no
        explanation is indistinguishable from data loss. */
    const juce::StringArray& getUnloadableProgramReports() const noexcept { return unloadableReports; }
    // Safe to call from any thread (see class comment) - actual application happens async.
    void requestProgramChange(const ProgramId& id);
    /** Raw, unnumbered - what the HOST's list wants, since a host renders its own numbering. */
    juce::String getProgramName(int factoryPosition) const;




    // Always creates a new file and switches to it - never overwrites. Name is defensively
    // uppercased and capped at 22 characters here (not just enforced by the GUI's name-entry
    // field), falling back to "NEW PROGRAM" if empty, per CHORUS60-GUI-SPEC.md section 6.
    void saveNewUserProgram(const juce::String& requestedName);

    // No-op for factory indices. Falls back to defaultFactoryProgramIndex if the deleted program
    // was the currently loaded one.
    void deleteUserProgram(const ProgramId& id);

    // Called by PluginProcessor's setStateInformation after apvts.replaceState() restores a saved
    // session - keeps the FACT/USER header tag in sync with whatever program index the session
    // remembers, without re-applying its parameters (they just came from the session state itself).
    // True once the APVTS parameters differ from the currently-loaded program's own values - i.e.
    // the user has actually moved something and there is a change worth saving. The GUI uses this to
    // disable SAVE on an untouched program (see ProgramHeader), so "Save" always means "store the
    // edits I just made as a new program" rather than "duplicate this program unchanged". Matches
    // Gatecrasher's behaviour exactly - the two plugins share one program paradigm.
    bool isModifiedFromLoadedProgram() const;

    void setCurrentProgramWithoutApplying(const ProgramId& id);

    // Called by PluginProcessor's setStateInformation before restoring a full session: drops any
    // requestProgramChange that arrived just before the restore but hasn't been applied yet (its
    // handleAsyncUpdate dispatch may otherwise land after the restore and silently clobber the
    // just-restored parameter values with a stale program).
    void cancelPendingChange() noexcept;

    // PluginProcessor wires this to updateHostDisplay(...withProgramChanged(true)) - kept as a
    // callback rather than a base-class call so ProgramManager doesn't need to know about
    // juce::AudioProcessor at all.
    std::function<void()> onProgramListChanged;

private:
    void handleAsyncUpdate() override;
    void applyProgram(const ProgramId& id);
    void setCurrentId(const ProgramId& id);
    juce::File userProgramFile(const juce::String& stem) const;
    void applyFactoryProgram(const FactoryProgram& program);
    void refreshUserProgramList();
    void captureCleanSnapshot();
    static juce::File getUserProgramDirectory();

    juce::AudioProcessorValueTreeState& apvts;

    mutable juce::SpinLock currentIdLock;
    ProgramId currentId;
    juce::StringArray unloadableReports;
    juce::SpinLock pendingLock;
    bool hasPendingProgram = false;
    ProgramId pendingProgram;

    // Sorted alphabetically by filename (stable across relaunches, unlike mtime-sort). Index i in
    // this array is program index kNumFactoryPrograms + i.
    juce::Array<juce::File> userProgramFiles;

    // Normalised parameter values as of the last program load, in getParameters() order - what
    // isModifiedFromLoadedProgram compares against. Message-thread only (every writer runs there:
    // initialise at construction, handleAsyncUpdate, saveNewUserProgram, and the session-restore
    // path), so it needs no synchronisation of its own.
    std::vector<float> cleanSnapshot;

    // **31, recomputed now that the "NN " prefix is gone from the displayed string.**
    //
    // The theme declares a 36-character budget, but the paint path trims lcdNameRightPadding (26px)
    // off the cell, so the string is actually drawn into 326px - 33 characters at 9.6px each. The
    // dirty marker " *" takes 2 of those. The naming field uses a different inset (reduced(12,0),
    // 328px = 34) and draws a block cursor, so 31 + 1 clears that too.
    //
    // The old 24 came from the spec rather than from either measurement. Chorus60Theme.h's
    // lcdCharacterBudget still says 36; that figure describes the cell, not the drawn run, and the
    // comment there now says so.
    static constexpr int maxProgramNameLength = 31;   // mirrored by Layout::maxProgramNameLength
};
