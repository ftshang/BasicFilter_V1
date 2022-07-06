/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicFilter_V1AudioProcessor::BasicFilter_V1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("cutoff", this);
    treeState.addParameterListener("resonance", this);
}

BasicFilter_V1AudioProcessor::~BasicFilter_V1AudioProcessor()
{
    treeState.removeParameterListener("cutoff", this);
    treeState.removeParameterListener("resonance", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout BasicFilter_V1AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto twelveDB = 1.0 / juce::MathConstants<float>::sqrt2;
    auto twentyFourDB = twelveDB * 2.0f;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 500.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", juce::NormalisableRange<float>(twelveDB, twelveDB * 4.0f, twelveDB), twentyFourDB));

    return { params.begin(), params.end() };
}

void BasicFilter_V1AudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    updateParameters();
}

void BasicFilter_V1AudioProcessor::updateParameters()
{
    DBG("Cutoff Value: " << treeState.getRawParameterValue("cutoff")->load());
    DBG("Resonance Value: " << treeState.getRawParameterValue("resonance")->load());
    filterModule.setCutoffFrequency(treeState.getRawParameterValue("cutoff")->load());
    filterModule.setResonance(treeState.getRawParameterValue("resonance")->load());
}

//==============================================================================
const juce::String BasicFilter_V1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicFilter_V1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicFilter_V1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicFilter_V1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicFilter_V1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicFilter_V1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicFilter_V1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicFilter_V1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicFilter_V1AudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicFilter_V1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicFilter_V1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;

    filterModule.prepare(spec);
    filterModule.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    updateParameters();
}

void BasicFilter_V1AudioProcessor::releaseResources()
{
    filterModule.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicFilter_V1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BasicFilter_V1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    filterModule.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

//==============================================================================
bool BasicFilter_V1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicFilter_V1AudioProcessor::createEditor()
{
    //return new BasicFilter_V1AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BasicFilter_V1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void BasicFilter_V1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));

    if (tree.isValid())
    {
        treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicFilter_V1AudioProcessor();
}
