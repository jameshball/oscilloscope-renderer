#include "VisualiserSettings.h"
#include "VisualiserComponent.h"
#include "../PluginEditor.h"

VisualiserSettings::VisualiserSettings(OscirenderAudioProcessor& p, VisualiserComponent& visualiser) : audioProcessor(p), visualiser(visualiser) {
	addAndMakeVisible(intensity);
	addAndMakeVisible(persistence);
    addAndMakeVisible(hue);
    addAndMakeVisible(graticuleToggle);
    addAndMakeVisible(smudgeToggle);
    
    intensity.setSliderOnValueChange();
    persistence.setSliderOnValueChange();
    hue.setSliderOnValueChange();
}

VisualiserSettings::~VisualiserSettings() {}

void VisualiserSettings::resized() {
	auto area = getLocalBounds().reduced(20);
	double rowHeight = 30;
    intensity.setBounds(area.removeFromTop(rowHeight));
    persistence.setBounds(area.removeFromTop(rowHeight));
    hue.setBounds(area.removeFromTop(rowHeight));
    graticuleToggle.setBounds(area.removeFromTop(rowHeight));
    smudgeToggle.setBounds(area.removeFromTop(rowHeight));
}

juce::var VisualiserSettings::getSettings() {
    auto settings = new juce::DynamicObject();
    settings->setProperty("intensity", audioProcessor.intensityEffect->getActualValue());
    settings->setProperty("persistence", audioProcessor.persistenceEffect->getActualValue());
    settings->setProperty("hue", audioProcessor.hueEffect->getActualValue());
    settings->setProperty("graticule", audioProcessor.graticuleEnabled->getBoolValue());
    settings->setProperty("smudges", audioProcessor.smudgesEnabled->getBoolValue());
    return juce::var(settings);
}
