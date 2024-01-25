/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioRecordingComponent
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Records audio to a file.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioRecordingComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

//==============================================================================
/** A simple class that acts as an AudioIODeviceCallback and writes the
    incoming audio data to a WAV file.
*/
class AudioRecorder final : public juce::AudioIODeviceCallback {
public:
    AudioRecorder(juce::AudioThumbnail& thumbnailToUpdate)
        : thumbnail(thumbnailToUpdate) {
        backgroundThread.startThread();
    }

    ~AudioRecorder() override {
        stop();
    }

    //==============================================================================
    void startRecording(const juce::File& file) {
        stop();

        if (sampleRate > 0) {
            // Create an OutputStream to write to our destination file...
            file.deleteFile();

            if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
                // Now create a WAV writer object that writes to our output stream...
                juce::WavAudioFormat wavFormat;

                if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 2, 32, {}, 0)) {
                    fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                    // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                    // write the data to disk on our background thread.
                    threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));

                    // Reset our recording thumbnail
                    thumbnail.reset(writer->getNumChannels(), writer->getSampleRate());
                    nextSampleNum = 0;

                    // And now, swap over our active writer pointer so that the audio callback will start using it..
                    const juce::ScopedLock sl(writerLock);
                    activeWriter = threadedWriter.get();
                }
            }
        }
    }

    void stop() {
        // First, clear this pointer to stop the audio callback from using our writer object..
        {
            const juce::ScopedLock sl(writerLock);
            activeWriter = nullptr;
        }

        // Now we can delete the writer object. It's done in this order because the deletion could
        // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
        // the audio callback while this happens.
        threadedWriter.reset();
    }

    bool isRecording() const {
        return activeWriter.load() != nullptr;
    }

    //==============================================================================
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
        sampleRate = device->getCurrentSampleRate();
    }

    void audioDeviceStopped() override {
        sampleRate = 0;
    }

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext& context) override {
        juce::ignoreUnused(context);

        const juce::ScopedLock sl(writerLock);

        if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels()) {
            activeWriter.load()->write(inputChannelData, numSamples);

            // Create an AudioBuffer to wrap our incoming data, note that this does no allocations or copies, it simply references our input data
            juce::AudioBuffer<float> buffer(const_cast<float**> (inputChannelData), thumbnail.getNumChannels(), numSamples);
            thumbnail.addBlock(nextSampleNum, buffer, 0, numSamples);
            nextSampleNum += numSamples;
        }

        // We need to clear the output buffers, in case they're full of junk..
        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != nullptr)
                juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
    }

private:
    juce::AudioThumbnail& thumbnail;
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" }; // the thread that will write our audio data to disk
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    double sampleRate = 0.0;
    juce::int64 nextSampleNum = 0;

    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
};

//==============================================================================
class RecordingThumbnail final : public juce::Component,
    private juce::ChangeListener {
public:
    RecordingThumbnail() {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener(this);
    }

    ~RecordingThumbnail() override {
        thumbnail.removeChangeListener(this);
    }

    juce::AudioThumbnail& getAudioThumbnail() { return thumbnail; }

    void setDisplayFullThumbnail(bool displayFull) {
        displayFullThumb = displayFull;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::white);

        if (thumbnail.getTotalLength() > 0.0) {
            auto endTime = displayFullThumb ? thumbnail.getTotalLength()
                : juce::jmax(30.0, thumbnail.getTotalLength());

            auto thumbArea = getLocalBounds();
            thumbnail.drawChannels(g, thumbArea.reduced(2), 0.0, endTime, 2.0f);
        } else {
            g.setFont(14.0f);
            g.drawFittedText("(No file recorded)", getLocalBounds(), juce::Justification::centred, 2);
        }
    }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 10 };
    juce::AudioThumbnail thumbnail { 128, formatManager, thumbnailCache };

    bool displayFullThumb = false;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        if (source == &thumbnail)
            repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordingThumbnail)
};

//==============================================================================
class AudioRecordingComponent final : public juce::Component {
public:
    AudioRecordingComponent() {
        addAndMakeVisible(recordButton);

        recordButton.onClick = [this] {
            if (recorder.isRecording())
                stopRecording();
            else
                startRecording();
        };

        addAndMakeVisible(recordingThumbnail);
        recordingThumbnail.setDisplayFullThumbnail(true);


        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [this](bool granted) {
                int numInputChannels = granted ? 2 : 0;
                audioDeviceManager.initialise(numInputChannels, 2, nullptr, true, {}, nullptr);
            });

        audioDeviceManager.addAudioCallback(&recorder);
    }

    ~AudioRecordingComponent() override {
        audioDeviceManager.removeAudioCallback(&recorder);
    }

    void resized() override {
        auto area = getLocalBounds();
        recordButton.setBounds(area.removeFromLeft(80));
        area.removeFromLeft(5);
        recordingThumbnail.setBounds(area);
    }

private:
    juce::AudioDeviceManager audioDeviceManager;

    RecordingThumbnail recordingThumbnail;
    AudioRecorder recorder{ recordingThumbnail.getAudioThumbnail() };

    juce::TextButton recordButton{ "Record" };
    juce::File lastRecording;
    juce::FileChooser chooser { "Output file...", juce::File::getCurrentWorkingDirectory().getChildFile("recording.wav"), "*.wav" };

    void startRecording() {
        if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage)) {
            SafePointer<AudioRecordingComponent> safeThis(this);

            juce::RuntimePermissions::request(juce::RuntimePermissions::writeExternalStorage,
                [safeThis](bool granted) mutable {
                    if (granted)
                        safeThis->startRecording();
        });
            return;
    }

        auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);

        lastRecording = parentDir.getNonexistentChildFile("osci-render-recording", ".wav");
        recorder.startRecording(lastRecording);
        recordButton.setButtonText("Stop");

        recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        recordButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    }

    void stopRecording() {
        recorder.stop();

        recordButton.setButtonText("Record");

        recordButton.setColour(juce::TextButton::buttonColourId, findColour(juce::TextButton::buttonColourId));
        recordButton.setColour(juce::TextButton::textColourOnId, findColour(juce::TextButton::textColourOnId));

        chooser.launchAsync(juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& c) {
                if (juce::FileInputStream inputStream(lastRecording); inputStream.openedOk())
                    if (const auto outputStream = makeOutputStream(c.getURLResult()))
                        outputStream->writeFromInputStream(inputStream, -1);

                lastRecording.deleteFile();
            });
    }

    inline juce::Colour getUIColourIfAvailable(juce::LookAndFeel_V4::ColourScheme::UIColour uiColour, juce::Colour fallback = juce::Colour(0xff4d4d4d)) noexcept {
        if (auto* v4 = dynamic_cast<juce::LookAndFeel_V4*> (&juce::LookAndFeel::getDefaultLookAndFeel()))
            return v4->getCurrentColourScheme().getUIColour(uiColour);

        return fallback;
    }

    inline std::unique_ptr<juce::OutputStream> makeOutputStream(const juce::URL& url) {
        if (const auto doc = juce::AndroidDocument::fromDocument(url))
            return doc.createOutputStream();

#if ! JUCE_IOS
        if (url.isLocalFile())
            return url.getLocalFile().createOutputStream();
#endif

        return url.createOutputStream();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecordingComponent)
};
