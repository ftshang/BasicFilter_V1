/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BasicFilter_V1AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BasicFilter_V1AudioProcessorEditor (BasicFilter_V1AudioProcessor&);
    ~BasicFilter_V1AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BasicFilter_V1AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicFilter_V1AudioProcessorEditor)
};
