#include <Windows.h>
#include <Console.h>
#include <Utils.h>
#include <vector>

constexpr const wchar_t* kInputFile = L"input.bmp";
constexpr const wchar_t* kOutputFile = L"output.bmp";
constexpr const char* kAlphabet = "abcdefghijklmnopqrstuvwxyz ";
constexpr const char* kMessage = "never gonna give you up";

#pragma pack(push, 1)
struct BMPFileHeader {
	char signature[2];
	uint32_t size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BMPInfoHeader {
	uint32_t headerSize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t depth;
	uint32_t compression;
	uint32_t imageSize;
	int32_t xResolution;
	int32_t yResolution;
	uint32_t colorsUsed;
	uint32_t colorsImportant;
};
#pragma pack(pop)

void BmpHideMessage(const wchar_t* input, const wchar_t* output, const char* message, const char* alphabet) {
	std::vector<uint8_t> data = ReadFile<uint8_t>(input);
	if (data.size() < sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)) {
		std::runtime_error("Format is not supported!");
	}

	auto fileHeader = reinterpret_cast<BMPFileHeader*>(data.data());
	auto infoHeader = reinterpret_cast<BMPInfoHeader*>(data.data() + sizeof(BMPFileHeader));
	size_t alphabetSize = strlen(alphabet);
	size_t colorsUsed = infoHeader->colorsUsed;
	if (!strncmp("BM", fileHeader->signature, 2)
		|| infoHeader->headerSize != 40
		|| infoHeader->depth != 8
		|| infoHeader->compression != 0
		|| infoHeader->colorsUsed == 0
		|| infoHeader->colorsUsed * alphabetSize > UINT8_MAX) { // > 256 in general, but one additional palette is used for \0
		std::runtime_error("Format is not supported!");
	}

	// Add space for new palette colors and update pointers
	data.resize(data.size() + alphabetSize * infoHeader->colorsUsed * sizeof(uint32_t));
	fileHeader = reinterpret_cast<BMPFileHeader*>(data.data());
	infoHeader = reinterpret_cast<BMPInfoHeader*>(data.data() + sizeof(BMPFileHeader));

	// Duplicate palette for each letter
	auto paletteOffset = data.begin() + sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
	for (int i = 0; i < alphabetSize; ++i) {
		data.insert(data.begin() + fileHeader->offset,
			paletteOffset,
			paletteOffset + infoHeader->colorsUsed * sizeof(uint32_t));
	}

	fileHeader->size += infoHeader->colorsUsed * alphabetSize * sizeof(uint32_t);
	fileHeader->offset += infoHeader->colorsUsed * alphabetSize * sizeof(uint32_t);
	infoHeader->colorsUsed *= alphabetSize + 1;
	infoHeader->colorsImportant = infoHeader->colorsUsed;

	// Hide message
	auto rowBegin = data.begin() + fileHeader->offset;
	auto iter = data.begin() + fileHeader->offset;
	for (size_t i = 0; i < strlen(message); ++i) {
		*iter = *iter + colorsUsed * (uint8_t)(strchr(alphabet, message[i]) - alphabet + 1); // Index of the letter + 1
		++iter;
		// Program handles only 8-bit depth so that will go
		if (iter - rowBegin >= infoHeader->width) {
			iter += 2;
			rowBegin = iter;
		}
	}

	// Save data
	WriteFile(output, data);
}

std::string BmpReadMessage(const wchar_t* input, const char* alphabet) {
	std::vector<uint8_t> data = ReadFile<uint8_t>(input);
	if (data.size() < sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)) {
		std::runtime_error("Format is not supported!");
	}

	auto fileHeader = reinterpret_cast<BMPFileHeader*>(data.data());
	auto infoHeader = reinterpret_cast<BMPInfoHeader*>(data.data() + sizeof(BMPFileHeader));
	size_t alphabetSize = strlen(alphabet);
	size_t paletteSize = (infoHeader->colorsUsed - 1) / alphabetSize;
	if (!strncmp("BM", fileHeader->signature, 2)
		|| infoHeader->headerSize != 40
		|| infoHeader->depth != 8
		|| infoHeader->compression != 0
		|| infoHeader->colorsUsed == 0
		|| infoHeader->colorsUsed * alphabetSize > UINT8_MAX) { // > 256 in general, but one additional palette is used for \0
		std::runtime_error("Format is not supported!");
	}

	// Get message
	auto rowBegin = data.begin() + fileHeader->offset;
	auto iter = data.begin() + fileHeader->offset;
	std::string message;

	while (iter - data.begin() < data.size()) {
		// Decode till data is present
		size_t paletteIndex = *iter / paletteSize;
		if (paletteIndex == 0) {
			break;
		}

		message.append(1, alphabet[paletteIndex - 1]);
		++iter;
		// Program handles only 8-bit depth so that will go
		if (iter - rowBegin >= infoHeader->width) {
			iter += 2;
			rowBegin = iter;
		}
	}

	return message;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	// Hide message
	BmpHideMessage(kInputFile, kOutputFile, kMessage, kAlphabet);
	// Read message
	std::string message = BmpReadMessage(kOutputFile, kAlphabet);
	// Print data
	Console::GetInstance()->PrintF("Message: %s\n", message.c_str());

	Console::GetInstance()->Pause();
	return 0;
}
