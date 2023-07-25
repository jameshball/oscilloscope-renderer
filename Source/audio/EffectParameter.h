#pragma once
#include "../shape/Vector2.h"
#include <JuceHeader.h>

class FloatParameter : public juce::AudioProcessorParameterWithID {
public:
	std::atomic<float> min = 0.0;
	std::atomic<float> max = 1.0;
	std::atomic<float> step = 0.001;

	FloatParameter(juce::String name, juce::String id, float value, float min, float max, float step = 0.001, juce::String label = "") : juce::AudioProcessorParameterWithID(id, name), value(value), min(min), max(max), step(step), label(label) {}

	juce::String getName(int maximumStringLength) const override {
		return name.substring(0, maximumStringLength);
	}

	juce::String getLabel() const override {
		return label;
	}

	// returns value in range [0, 1]
	float getNormalisedValue(float value) const {
		// clip value to valid range
		auto min = this->min.load();
		auto max = this->max.load();
		value = juce::jlimit(min, max, value);
		// normalize value to range [0, 1]
		return (value - min) / (max - min);
	}

	float getUnnormalisedValue(float value) const {
		value = juce::jlimit(0.0f, 1.0f, value);
		auto min = this->min.load();
		auto max = this->max.load();
		return min + value * (max - min);
	}

	float getValue() const override {
		return getNormalisedValue(value.load());
	}

	float getValueUnnormalised() const {
		return value.load();
	}

	void setValue(float newValue) override {
		value = getUnnormalisedValue(newValue);
	}

	void setValueUnnormalised(float newValue) {
		value = newValue;
	}

	void setUnnormalisedValueNotifyingHost(float newValue) {
		setValueNotifyingHost(getNormalisedValue(newValue));
	}

	float getDefaultValue() const override {
		return 0.0f;
	}

	int getNumSteps() const override {
		return (max.load() - min.load()) / step.load();
	}

	bool isDiscrete() const override {
		return false;
	}

	bool isBoolean() const override {
		return false;
	}

	bool isOrientationInverted() const override {
		return false;
	}

	juce::String getText(float value, int maximumStringLength) const override {
		auto string = juce::String(getUnnormalisedValue(value), 3);
		return string.substring(0, maximumStringLength);
	}

	float getValueForText(const juce::String& text) const override {
		return getNormalisedValue(text.getFloatValue());
	}

	bool isAutomatable() const override {
		return true;
	}

	bool isMetaParameter() const override {
		return false;
	}

	juce::AudioProcessorParameter::Category getCategory() const override {
		return juce::AudioProcessorParameter::genericParameter;
	}

	void save(juce::XmlElement* xml) {
		xml->setAttribute("id", paramID);
		xml->setAttribute("value", value.load());
		xml->setAttribute("min", min.load());
		xml->setAttribute("max", max.load());
		xml->setAttribute("step", step.load());
	}

private:
	// value is not necessarily in the range [min, max] so effect applications may need to clip to a valid range
	std::atomic<float> value = 0.0;
	juce::String label;
};

class IntParameter : public juce::AudioProcessorParameterWithID {
public:
	std::atomic<int> min = 0;
	std::atomic<int> max = 10;

	IntParameter(juce::String name, juce::String id, int value, int min, int max) : AudioProcessorParameterWithID(id, name), value(value), min(min), max(max) {}

	juce::String getName(int maximumStringLength) const override {
		return name.substring(0, maximumStringLength);
	}

	juce::String getLabel() const override {
		return juce::String();
	}

	// returns value in range [0, 1]
	float getNormalisedValue(float value) const {
		// clip value to valid range
		auto min = this->min.load();
		auto max = this->max.load();
		value = juce::jlimit(min, max, (int) value);
		// normalize value to range [0, 1]
		return (value - min) / (max - min);
	}

	float getUnnormalisedValue(float value) const {
		value = juce::jlimit(0.0f, 1.0f, value);
		auto min = this->min.load();
		auto max = this->max.load();
		return min + value * (max - min);
	}

	float getValue() const override {
		return getNormalisedValue(value.load());
	}

	float getValueUnnormalised() const {
		return value.load();
	}

	void setValue(float newValue) override {
		value = getUnnormalisedValue(newValue);
	}

	void setValueUnnormalised(float newValue) {
		value = newValue;
	}

	void setUnnormalisedValueNotifyingHost(float newValue) {
		setValueNotifyingHost(getNormalisedValue(newValue));
	}

	float getDefaultValue() const override {
		return 0;
	}

	int getNumSteps() const override {
		return max.load() - min.load() + 1;
	}

	bool isDiscrete() const override {
		return true;
	}

	bool isBoolean() const override {
		return false;
	}

	bool isOrientationInverted() const override {
		return false;
	}

	juce::String getText(float value, int maximumStringLength) const override {
		auto string = juce::String((int) getUnnormalisedValue(value));
		return string.substring(0, maximumStringLength);
	}

	float getValueForText(const juce::String& text) const override {
		return getNormalisedValue(text.getIntValue());
	}

	bool isAutomatable() const override {
		return true;
	}

	bool isMetaParameter() const override {
		return false;
	}

	juce::AudioProcessorParameter::Category getCategory() const override {
		return juce::AudioProcessorParameter::genericParameter;
	}

private:
	// value is not necessarily in the range [min, max] so effect applications may need to clip to a valid range
	std::atomic<int> value = 0;
};

enum class LfoType : int {
	Static = 1,
	Sine = 2,
	Square = 3,
	Seesaw = 4,
	Triangle = 5,
	Sawtooth = 6,
	ReverseSawtooth = 7,
	Noise = 8
};

class LfoTypeParameter : public IntParameter {
public:
	LfoTypeParameter(juce::String name, juce::String id, int value) : IntParameter(name, id, value, 1, 8) {}

	juce::String getText(float value, int maximumStringLength) const override {
		switch ((LfoType)(int)getUnnormalisedValue(value)) {
            case LfoType::Static:
                return "Static";
            case LfoType::Sine:
                return "Sine";
            case LfoType::Square:
                return "Square";
            case LfoType::Seesaw:
                return "Seesaw";
            case LfoType::Triangle:
                return "Triangle";
            case LfoType::Sawtooth:
                return "Sawtooth";
            case LfoType::ReverseSawtooth:
                return "Reverse Sawtooth";
            case LfoType::Noise:
                return "Noise";
            default:
                return "Unknown";
        }
	}

	float getValueForText(const juce::String& text) const override {
		if (text == "Static") {
            return (int)LfoType::Static;
		} else if (text == "Sine") {
            return (int)LfoType::Sine;
		} else if (text == "Square") {
            return (int)LfoType::Square;
		} else if (text == "Seesaw") {
            return (int)LfoType::Seesaw;
		} else if (text == "Triangle") {
            return (int)LfoType::Triangle;
		} else if (text == "Sawtooth") {
            return (int)LfoType::Sawtooth;
		} else if (text == "Reverse Sawtooth") {
            return (int)LfoType::ReverseSawtooth;
		} else if (text == "Noise") {
            return (int)LfoType::Noise;
		} else {
            return (int)LfoType::Static;
        }
	}

	void save(juce::XmlElement* xml) {
        xml->setAttribute("lfo", getText(getValue(), 100));
    }
};

class EffectParameter : public FloatParameter {
public:
	std::atomic<bool> smoothValueChange = true;
	LfoTypeParameter* lfo = new LfoTypeParameter(name + " LFO", paramID + "Lfo", 1);
	FloatParameter* lfoRate = new FloatParameter(name + " LFO Rate", paramID + "LfoRate", 1.0f, 0.0f, 100.0f, 0.1f, "Hz");
	std::atomic<float> phase = 0.0f;

	std::vector<juce::AudioProcessorParameter*> getParameters() {
		std::vector<juce::AudioProcessorParameter*> parameters;
		parameters.push_back(this);
		if (lfo != nullptr) {
			parameters.push_back(lfo);
		}
		if (lfoRate != nullptr) {
			parameters.push_back(lfoRate);
		}
		return parameters;
    }

	void disableLfo() {
		delete lfo;
		delete lfoRate;
		lfo = nullptr;
		lfoRate = nullptr;
	}

	void save(juce::XmlElement* xml) {
		FloatParameter::save(xml);

		if (lfo != nullptr && lfoRate != nullptr) {
			auto lfoXml = xml->createNewChildElement("lfo");
			lfo->save(lfoXml);
			lfoRate->save(lfoXml);
		}
    }

	EffectParameter(juce::String name, juce::String id, float value, float min, float max, float step = 0.001, bool smoothValueChange = true) : FloatParameter(name, id, value, min, max, step), smoothValueChange(smoothValueChange) {}
};