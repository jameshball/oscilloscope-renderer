#include "FileParser.h"
#include "../shape/Line.h"
#include "../shape/CircleArc.h"
#include <numbers>

FileParser::FileParser() {}

void FileParser::parse(juce::String extension, std::unique_ptr<juce::InputStream> stream) {
	juce::SpinLock::ScopedLockType scope(lock);

	object = nullptr;
	camera = nullptr;
	svg = nullptr;
	text = nullptr;
	lua = nullptr;
	
	if (extension == ".obj") {
		object = std::make_shared<WorldObject>(stream->readEntireStreamAsString().toStdString());
		camera = std::make_shared<Camera>(1.0, 0, 0, 0.0);
		camera->findZPos(*object);
	} else if (extension == ".svg") {
		svg = std::make_shared<SvgParser>(stream->readEntireStreamAsString());
	} else if (extension == ".txt") {
		text = std::make_shared<TextParser>(stream->readEntireStreamAsString(), juce::Font(1.0f));
	} else if (extension == ".lua") {
		lua = std::make_shared<LuaParser>(stream->readEntireStreamAsString());
	}

	sampleSource = lua != nullptr;
}

std::vector<std::unique_ptr<Shape>> FileParser::nextFrame() {
	juce::SpinLock::ScopedLockType scope(lock);

	if (object != nullptr && camera != nullptr) {
		return camera->draw(*object);
	} else if (svg != nullptr) {
		return svg->draw();
	} else if (text != nullptr) {
		return text->draw();
	}
	auto tempShapes = std::vector<std::unique_ptr<Shape>>();
	tempShapes.push_back(std::make_unique<CircleArc>(0, 0, 0.5, 0.5, std::numbers::pi / 4.0, 2 * std::numbers::pi));
	return tempShapes;
}

Vector2 FileParser::nextSample() {
	juce::SpinLock::ScopedLockType scope(lock);

	if (lua != nullptr) {
		auto values = lua->run();
		if (values.size() < 2) {
            return Vector2();
        }
		return Vector2(values[0], values[1]);
	}
}

bool FileParser::isSample() {
	return sampleSource;
}

bool FileParser::isActive() {
	return active;
}

void FileParser::disable() {
	active = false;
}

void FileParser::enable() {
	active = true;
}

std::shared_ptr<WorldObject> FileParser::getObject() {
	return object;
}

std::shared_ptr<Camera> FileParser::getCamera() {
	return camera;
}

std::shared_ptr<SvgParser> FileParser::getSvg() {
	return svg;
}

std::shared_ptr<TextParser> FileParser::getText() {
	return text;
}

std::shared_ptr<LuaParser> FileParser::getLua() {
	return lua;
}
