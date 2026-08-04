#include "ProgramManager.h"
#include "../Parameters.h"
#include <algorithm>

namespace
{
    // Deliberately not JucePlugin_Manufacturer/JucePlugin_Name: those macros only exist in the
    // real plugin target's JUCE-generated headers, and ProgramManager.cpp is also compiled
    // directly into the Tests console-app target (see Tests/CMakeLists.txt) so
    // ProgramManagerTests.cpp can exercise it without linking the whole plugin. Matches
    // CMakeLists.txt's COMPANY_NAME/PRODUCT_NAME - keep in sync if those change.
    constexpr const char* pluginCompanyName = "Chorus-60";
    constexpr const char* pluginProductName = "Chorus-60";
}

ProgramManager::ProgramManager(juce::AudioProcessorValueTreeState& stateToControl)
    : apvts(stateToControl)
{
}

ProgramManager::~ProgramManager()
{
    cancelPendingUpdate();
}

void ProgramManager::initialise()
{
    refreshUserProgramList();
    applyFactoryProgram(kFactoryPrograms[(size_t) defaultFactoryProgramIndex]);
    currentProgramIndex.store(defaultFactoryProgramIndex, std::memory_order_relaxed);
}

int ProgramManager::getNumPrograms() const noexcept
{
    return kNumFactoryPrograms + userProgramFiles.size();
}

void ProgramManager::requestProgramChange(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;
    pendingProgramIndex.store(index, std::memory_order_relaxed);
    triggerAsyncUpdate();
}

void ProgramManager::handleAsyncUpdate()
{
    const int index = pendingProgramIndex.exchange(-1, std::memory_order_relaxed);
    if (index >= 0)
        applyProgramByIndex(index);
}

juce::String ProgramManager::getProgramName(int index) const
{
    if (isFactoryProgram(index))
        return kFactoryPrograms[(size_t) index].name;

    const int userIndex = index - kNumFactoryPrograms;
    if (userIndex >= 0 && userIndex < userProgramFiles.size())
        return userProgramFiles.getReference(userIndex).getFileNameWithoutExtension();
    return {};
}

juce::File ProgramManager::getUserProgramDirectory()
{
   #if JUCE_WINDOWS || JUCE_LINUX
    // Windows: %APPDATA%\<Manufacturer>\<Plugin>\Programs. Linux: ~/.config/<Manufacturer>/<Plugin>/Programs
    // (JUCE's userApplicationDataDirectory resolves to the right per-OS location on each).
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(pluginCompanyName)
        .getChildFile(pluginProductName)
        .getChildFile("Programs");
   #else
    // "Presets" here is Apple/AU's own special-location folder name that Logic and other hosts
    // scan (~/Library/Audio/Presets/<Manufacturer>/<Plugin>/) - not a lapse into "Preset"
    // terminology, just the OS convention this path has to match to be discoverable.
    return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
        .getChildFile("Library")
        .getChildFile("Audio")
        .getChildFile("Presets")
        .getChildFile(pluginCompanyName)
        .getChildFile(pluginProductName);
   #endif
}

void ProgramManager::refreshUserProgramList()
{
    userProgramFiles.clear();
    const auto dir = getUserProgramDirectory();
    if (! dir.isDirectory())
        return;

    for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*.chorus60program"))
        userProgramFiles.add(entry.getFile());

    std::sort(userProgramFiles.begin(), userProgramFiles.end(),
               [] (const juce::File& a, const juce::File& b) { return a.getFileName() < b.getFileName(); });
}

void ProgramManager::applyProgramByIndex(int index)
{
    if (isFactoryProgram(index))
    {
        applyFactoryProgram(kFactoryPrograms[(size_t) index]);
    }
    else
    {
        const int userIndex = index - kNumFactoryPrograms;
        if (userIndex < 0 || userIndex >= userProgramFiles.size())
            return;

        std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(userProgramFiles.getReference(userIndex)));
        if (xml == nullptr || ! xml->hasTagName(apvts.state.getType()))
            return;

        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }

    currentProgramIndex.store(index, std::memory_order_relaxed);
    if (onProgramListChanged)
        onProgramListChanged();
}

void ProgramManager::applyFactoryProgram(const FactoryProgram& program)
{
    *dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(ParamIDs::engine1)) = program.engine1;
    *dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(ParamIDs::engine2)) = program.engine2;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::rate1)) = program.rate1Hz;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::depth1)) = program.depth1Percent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::rate2)) = program.rate2Hz;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::depth2)) = program.depth2Percent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::delayCenter)) = program.delayCenterMs;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::decorrelation)) = program.decorrelationPercent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::drift)) = program.driftPercent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::saturation)) = program.saturationPercent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::noise)) = program.noisePercent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::mix)) = program.mixPercent;
    *dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(ParamIDs::trim)) = program.trimDb;
}

void ProgramManager::saveNewUserProgram(const juce::String& requestedName)
{
    juce::String name = requestedName.trim().toUpperCase();
    if (name.isEmpty())
        name = "NEW PROGRAM";
    if (name.length() > maxProgramNameLength)
        name = name.substring(0, maxProgramNameLength);

    const auto dir = getUserProgramDirectory();
    if (! dir.isDirectory())
        dir.createDirectory();

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute(LegacyMigration::stateSchemaVersionAttribute, LegacyMigration::currentStateSchemaVersion);

    const juce::File file = dir.getChildFile(juce::File::createLegalFileName(name) + ".chorus60program");
    xml->writeTo(file);

    refreshUserProgramList();
    const int newIndex = kNumFactoryPrograms + userProgramFiles.indexOf(file);
    currentProgramIndex.store(newIndex, std::memory_order_relaxed);
    if (onProgramListChanged)
        onProgramListChanged();
}

void ProgramManager::deleteUserProgram(int index)
{
    if (isFactoryProgram(index))
        return;

    const int userIndex = index - kNumFactoryPrograms;
    if (userIndex < 0 || userIndex >= userProgramFiles.size())
        return;

    const bool wasCurrent = currentProgramIndex.load(std::memory_order_relaxed) == index;
    userProgramFiles.getReference(userIndex).deleteFile();
    refreshUserProgramList();

    if (wasCurrent)
        requestProgramChange(defaultFactoryProgramIndex);
    else if (onProgramListChanged)
        onProgramListChanged();
}

void ProgramManager::setCurrentProgramIndexWithoutApplying(int index) noexcept
{
    currentProgramIndex.store(index, std::memory_order_relaxed);
}

void ProgramManager::cancelPendingChange() noexcept
{
    pendingProgramIndex.store(-1, std::memory_order_relaxed);
    cancelPendingUpdate();
}
