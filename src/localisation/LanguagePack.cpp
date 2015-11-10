extern "C" {
	#include "../common.h"
	#include "../util/util.h"
	#include "localisation.h"
	#include "../platform/platform.h"
}

#include "../core/FileStream.hpp"
#include "../core/Memory.hpp"
#include "../core/StringBuilder.hpp"
#include "LanguagePack.h"
#include <SDL.h>

constexpr rct_string_id ObjectOverrideBase = 0x6000;
constexpr int ObjectOverrideMaxStringCount = 4;

constexpr rct_string_id ScenarioOverrideBase = 0x7000;
constexpr int ScenarioOverrideMaxStringCount = 3;

LanguagePack *LanguagePack::FromFile(int id, const utf8 *path)
{
	assert(path != nullptr);

	uint32 fileLength;
	utf8 *fileData = nullptr;

	// Load file directly into memory
	try {
		FileStream fs = FileStream(path, FILE_MODE_OPEN);

		fileLength = (uint32)fs.GetLength();
		fileData = Memory::Allocate<utf8>(fileLength + 1);
		fs.Read(fileData, fileLength);
		fileData[fileLength] = '\0';

		fs.Dispose();
	} catch (Exception ex) {
		Memory::Free(fileData);
		log_error("Unable to open %s: %s", path, ex.GetMessage());
		return nullptr;
	}

	// Parse the memory as text
	LanguagePack *result = FromText(id, fileData);

	Memory::Free(fileData);
	return result;
}

LanguagePack *LanguagePack::FromText(int id, const utf8 *text)
{
	return new LanguagePack(id, text);
}

LanguagePack::LanguagePack(int id, const utf8 *text)
{
	assert(text != nullptr);

	_id = id;
	_stringData = nullptr;
	_currentGroup = nullptr;
	_currentObjectOverride = nullptr;
	_currentScenarioOverride = nullptr;

	auto reader = UTF8StringReader(text);
	while (reader.CanRead()) {
		ParseLine(&reader);
	}

	_stringData = _stringDataSB.GetString();

	size_t stringDataBaseAddress = (size_t)_stringData;
	for (size_t i = 0; i < _strings.size(); i++) {
		_strings[i] = (utf8*)(stringDataBaseAddress + (size_t)_strings[i]);
	}
	for (size_t i = 0; i < _objectOverrides.size(); i++) {
		for (int j = 0; j < ObjectOverrideMaxStringCount; j++) {
			const utf8 **strPtr = &(_objectOverrides[i].strings[j]);
			if (*strPtr != nullptr) {
				*strPtr = (utf8*)(stringDataBaseAddress + (size_t)*strPtr);
			}
		}
	}
	for (size_t i = 0; i < _scenarioOverrides.size(); i++) {
		for (int j = 0; j < ScenarioOverrideMaxStringCount; j++) {
			const utf8 **strPtr = &(_scenarioOverrides[i].strings[j]);
			if (*strPtr != nullptr) {
				*strPtr = (utf8*)(stringDataBaseAddress + (size_t)*strPtr);
			}
		}
	}

	// Destruct the string builder to free memory
	_stringDataSB = StringBuilder();
}

LanguagePack::~LanguagePack()
{
	SafeFree(_stringData);
	SafeFree(_currentGroup);
}

const utf8 *LanguagePack::GetString(int stringId) const {
	if (stringId >= ScenarioOverrideBase) {
		int offset = stringId - ScenarioOverrideBase;
		int ooIndex = offset / ScenarioOverrideMaxStringCount;
		int ooStringIndex = offset % ScenarioOverrideMaxStringCount;

		if (_scenarioOverrides.size() > (size_t)ooIndex) {
			return _scenarioOverrides[ooIndex].strings[ooStringIndex];
		} else {
			return nullptr;
		}
	}else if (stringId >= ObjectOverrideBase) {
		int offset = stringId - ObjectOverrideBase;
		int ooIndex = offset / ObjectOverrideMaxStringCount;
		int ooStringIndex = offset % ObjectOverrideMaxStringCount;

		if (_objectOverrides.size() > (size_t)ooIndex) {
			return _objectOverrides[ooIndex].strings[ooStringIndex];
		} else {
			return nullptr;
		}
	} else {
		if (_strings.size() > (size_t)stringId) {
			return _strings[stringId];
		} else {
			return nullptr;
		}
	}
}

rct_string_id LanguagePack::GetObjectOverrideStringId(const char *objectIdentifier, int index)
{
	assert(objectIdentifier != nullptr);
	assert(index < ObjectOverrideMaxStringCount);

	int ooIndex = 0;
	for (const ObjectOverride &objectOverride : _objectOverrides) {
		if (strncmp(objectOverride.name, objectIdentifier, 8) == 0) {
			if (objectOverride.strings[index] == nullptr) {
				return STR_NONE;
			}
			return ObjectOverrideBase + (ooIndex * ObjectOverrideMaxStringCount) + index;
		}
		ooIndex++;
	}

	return STR_NONE;
}

rct_string_id LanguagePack::GetScenarioOverrideStringId(const utf8 *scenarioFilename, int index)
{
	assert(scenarioFilename != nullptr);
	assert(index < ScenarioOverrideMaxStringCount);

	int ooIndex = 0;
	for (const ScenarioOverride &scenarioOverride : _scenarioOverrides) {
		if (_stricmp(scenarioOverride.filename, scenarioFilename) == 0) {
			if (scenarioOverride.strings[index] == nullptr) {
				return STR_NONE;
			}
			return ScenarioOverrideBase + (ooIndex * ScenarioOverrideMaxStringCount) + index;
		}
		ooIndex++;
	}

	return STR_NONE;
}

LanguagePack::ObjectOverride *LanguagePack::GetObjectOverride(const char *objectIdentifier)
{
	assert(objectIdentifier != nullptr);

	for (size_t i = 0; i < _objectOverrides.size(); i++) {
		ObjectOverride *oo = &_objectOverrides[i];
		if (strncmp(oo->name, objectIdentifier, 8) == 0) {
			return oo;
		}
	}
	return nullptr;
}

LanguagePack::ScenarioOverride *LanguagePack::GetScenarioOverride(const utf8 *scenarioIdentifier)
{
	assert(scenarioIdentifier != nullptr);

	for (size_t i = 0; i < _scenarioOverrides.size(); i++) {
		ScenarioOverride *so = &_scenarioOverrides[i];
		// At this point ScenarioOverrides were not yet rewritten to point at
		// strings, but rather still hold offsets from base.
		const utf8 *name = _stringDataSB.GetBuffer() + (size_t)so->name;
		if (_stricmp(name, scenarioIdentifier) == 0) {
			return so;
		}
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Partial support to open a uncompiled language file which parses tokens and converts them to the corresponding character
// code. Due to resource strings (strings in scenarios and objects) being written to the original game's string table,
// get_string will use those if the same entry in the loaded language is empty.
//
// Unsure at how the original game decides which entries to write resource strings to, but this could affect adding new
// strings for the time being. Further investigation is required.
//
// When reading the language files, the STR_XXXX part is read and XXXX becomes the string id number. Everything after the colon
// and before the new line will be saved as the string. Tokens are written with inside curly braces {TOKEN}.
// Use # at the beginning of a line to leave a comment.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool IsWhitespace(int codepoint)
{
	return codepoint == '\t' || codepoint == ' ' || codepoint == '\r' || codepoint == '\n';
}

static bool IsNewLine(int codepoint)
{
	return codepoint == '\r' || codepoint == '\n';
}

static void SkipWhitespace(IStringReader *reader)
{
	int codepoint;
	while (reader->TryPeek(&codepoint)) {
		if (IsWhitespace(codepoint)) {
			reader->Skip();
		} else {
			break;
		}
	}
}

static void SkipNewLine(IStringReader *reader)
{
	int codepoint;
	while (reader->TryPeek(&codepoint)) {
		if (IsNewLine(codepoint)) {
			reader->Skip();
		} else {
			break;
		}
	}
}

static void SkipToEndOfLine(IStringReader *reader)
{
	int codepoint;
	while (reader->TryPeek(&codepoint)) {
		if (codepoint != '\r' && codepoint != '\n') {
			reader->Skip();
		} else {
			break;
		}
	}
}

void LanguagePack::ParseLine(IStringReader *reader)
{
	SkipWhitespace(reader);

	int codepoint;
	if (reader->TryPeek(&codepoint)) {
		switch (codepoint) {
		case '#':
			SkipToEndOfLine(reader);
			break;
		case '[':
			ParseGroupObject(reader);
			break;
		case '<':
			ParseGroupScenario(reader);
			break;
		case '\r':
		case '\n':
			break;
		default:
			ParseString(reader);
			break;
		}
		SkipToEndOfLine(reader);
		SkipNewLine(reader);
	}
}

void LanguagePack::ParseGroupObject(IStringReader *reader)
{
	auto sb = StringBuilder();
	int codepoint;

	// Should have already deduced that the next codepoint is a [
	reader->Skip();

	// Read string up to ] or line end
	bool closedCorrectly = false;
	while (reader->TryPeek(&codepoint)) {
		if (IsNewLine(codepoint)) break;

		reader->Skip();
		if (codepoint == ']') {
			closedCorrectly = true;
			break;
		}
		sb.Append(codepoint);
	}

	if (closedCorrectly) {
		SafeFree(_currentGroup);

		while (sb.GetLength() < 8) {
			sb.Append(' ');
		}
		if (sb.GetLength() == 8) {
			_currentGroup = sb.GetString();
			_currentObjectOverride = GetObjectOverride(_currentGroup);
			_currentScenarioOverride = nullptr;
			if (_currentObjectOverride == nullptr) {
				_objectOverrides.push_back(ObjectOverride());
				_currentObjectOverride = &_objectOverrides[_objectOverrides.size() - 1];
				memset(_currentObjectOverride, 0, sizeof(ObjectOverride));
				memcpy(_currentObjectOverride->name, _currentGroup, 8);
			}
		}
	}
}

void LanguagePack::ParseGroupScenario(IStringReader *reader)
{
	auto sb = StringBuilder();
	int codepoint;

	// Should have already deduced that the next codepoint is a <
	reader->Skip();

	// Read string up to > or line end
	bool closedCorrectly = false;
	while (reader->TryPeek(&codepoint)) {
		if (IsNewLine(codepoint)) break;

		reader->Skip();
		if (codepoint == '>') {
			closedCorrectly = true;
			break;
		}
		sb.Append(codepoint);
	}

	if (closedCorrectly) {
		SafeFree(_currentGroup);

		_currentGroup = sb.GetString();
		_currentObjectOverride = nullptr;
		_currentScenarioOverride = GetScenarioOverride(_currentGroup);
		if (_currentScenarioOverride == nullptr) {
			_scenarioOverrides.push_back(ScenarioOverride());
			_currentScenarioOverride = &_scenarioOverrides[_scenarioOverrides.size() - 1];
			memset(_currentScenarioOverride, 0, sizeof(ScenarioOverride));
			_currentScenarioOverride->filename = sb.GetString();
		}
	}
}

void LanguagePack::ParseString(IStringReader *reader)
{
	auto sb = StringBuilder();
	int codepoint;

	// Parse string identifier
	while (reader->TryPeek(&codepoint)) {
		if (IsNewLine(codepoint)) {
			// Unexpected new line, ignore line entirely
			return;
		} else if (!IsWhitespace(codepoint) && codepoint != ':') {
			reader->Skip();
			sb.Append(codepoint);
		} else {
			break;
		}
	}

	SkipWhitespace(reader);

	// Parse a colon
	if (!reader->TryPeek(&codepoint) || codepoint != ':') {
		// Expected a colon, ignore line entirely
		return;
	}
	reader->Skip();

	// Validate identifier
	const utf8 *identifier = sb.GetBuffer();

	int stringId;
	if (_currentGroup == nullptr) {
		if (sscanf(identifier, "STR_%4d", &stringId) != 1) {
			// Ignore line entirely
			return;
		}
	} else {
		if (strcmp(identifier, "STR_NAME") == 0) { stringId = 0; }
		else if (strcmp(identifier, "STR_DESC") == 0) { stringId = 1; }
		else if (strcmp(identifier, "STR_CPTY") == 0) { stringId = 2; }

		else if (strcmp(identifier, "STR_SCNR") == 0) { stringId = 0; }
		else if (strcmp(identifier, "STR_PARK") == 0) { stringId = 1; }
		else if (strcmp(identifier, "STR_DTLS") == 0) { stringId = 2; }
		else {
			// Ignore line entirely
			return;
		}
	}

	// Rest of the line is the actual string
	sb.Clear();
	while (reader->TryPeek(&codepoint) && !IsNewLine(codepoint)) {
		if (codepoint == '{') {
			uint32 token;
			bool isByte;
			if (ParseToken(reader, &token, &isByte)) {
				if (isByte) {
					sb.Append((const utf8*)&token, 1);
				} else {
					sb.Append((int)token);
				}
			} else {
				// Syntax error or unknown token, ignore line entirely
				return;
			}
		} else {
			reader->Skip();
			sb.Append(codepoint);
		}
	}

	// Append a null terminator for the benefit of the last string
	_stringDataSB.Append(0);

	// Get the relative offset to the string (add the base offset when we extract the string properly)
	utf8 *relativeOffset = (utf8*)_stringDataSB.GetLength();

	if (_currentGroup == nullptr) {
		// Make sure the list is big enough to contain this string id
		while (_strings.size() <= (size_t)stringId) {
			_strings.push_back(nullptr);
		}

		_strings[stringId] = relativeOffset;
	} else {
		if (_currentObjectOverride != nullptr) {
			_currentObjectOverride->strings[stringId] = relativeOffset;
		} else {
			_currentScenarioOverride->strings[stringId] = relativeOffset;
		}
	}

	_stringDataSB.Append(&sb);
}

bool LanguagePack::ParseToken(IStringReader *reader, uint32 *token, bool *isByte)
{
	auto sb = StringBuilder();
	int codepoint;

	// Skip open brace
	reader->Skip();

	while (reader->TryPeek(&codepoint)) {
		if (IsNewLine(codepoint)) return false;
		if (IsWhitespace(codepoint)) return false;

		reader->Skip();

		if (codepoint == '}') break;

		sb.Append(codepoint);
	}

	const utf8 *tokenName = sb.GetBuffer();
	*token = format_get_code(tokenName);
	*isByte = false;

	// Handle explicit byte values
	if (*token == 0) {
		int number;
		if (sscanf(tokenName, "%d", &number) == 1) {
			*token = Math::Clamp(0, number, 255);
			*isByte = true;
		}
	}

	return true;
}
