#include <Windows.h>
#include <Console.h>
#include <vector>
#include <bitset>

#include "Utils.h"

constexpr const wchar_t* kInputFile = L"input.txt";
constexpr const wchar_t* kKeyFile = L"key.txt";
constexpr const wchar_t* kEncryptedFile = L"encrypted.txt";
constexpr const wchar_t* kDecryptedFile = L"decrypted.txt";

class A5Encoder
{
public:
	explicit A5Encoder(std::bitset<64> key)
	: _key(key) {
		Initialize();
	}

	bool Tick() {
		int x = _register0[8];
		int y = _register1[10];
		int z = _register2[10];
		int f = x && y || x && z || y && z;
		bool result = false;
		if (x == f) {
			bool value = _register0[13] != _register0[16] != _register0[17] != _register0[18] != true;
			result = result != value;
			_register0 <<= 1;
			_register0[0] = value;
		}

		if (y == f) {
			bool value = _register1[20] != _register1[21] != true;
			result = result != value;
			_register1 <<= 1;
			_register1[0] = value;
		}

		if (z == f) {
			bool value = _register2[7] != _register2[20] != _register2[21] != _register2[22] != true;
			result = result != value;
			_register2 <<= 1;
			_register2[0] = value;
		}

		return result;
	}

	uint8_t Encode(uint8_t byte) {
		for (int i = 0; i < 8; ++i) {
			if (_tick % 114) { // Using 1-way encoding, so only 1 frame
				Initialize();
				++_frame;
				_tick = 0;
			}

			byte ^= (Tick() ? 1 : 0) << i;

			++_tick;
		}

		return byte;
	}

	std::vector<uint8_t> Encode(const std::vector<uint8_t>& data) {
		std::vector<uint8_t> output(data.size());

		for (size_t i = 0; i < data.size(); ++i) {
			output[i] = Encode(data[i]);
		}

		return output;
	}

	void Reset() {
		_register0.reset();
		_register1.reset();
		_register2.reset();
		Initialize();
		_frame = 0;
		_tick = 0;
	}

private:
	std::bitset<19> _register0;
	std::bitset<22> _register1;
	std::bitset<23> _register2;
	std::bitset<64> _key;
	uint64_t _frame = 0;
	uint8_t _tick = 0;

	void Initialize() {
		for (int i = 0; i < _key.size(); ++i) {
			_register0[0] = _register0[0] != _key[i];
			_register0 <<= 1;
			_register1[0] = _register1[0] != _key[i];
			_register1 <<= 1;
			_register2[0] = _register2[0] != _key[i];
			_register2 <<= 1;
		}

		std::bitset<22> frame(_frame);
		for (int i = 0; i < frame.size(); ++i) {
			_register0[0] = _register0[0] != frame[i];
			_register0 <<= 1;
			_register1[0] = _register1[0] != frame[i];
			_register1 <<= 1;
			_register2[0] = _register2[0] != frame[i];
			_register2 <<= 1;
		}

		for (int i = 0; i < 100; ++i) {
			Tick();
		}
	}
};

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	std::bitset<64> key(VecConvert<uint32_t, uint8_t>(ReadFile(kKeyFile))[0]);
	A5Encoder encoder(key);
	std::vector<uint8_t> input = ReadFile(kInputFile);
	std::vector<uint8_t> encrypted = encoder.Encode(input);
	WriteFile(kEncryptedFile, encrypted);
	encoder.Reset();
	std::vector<uint8_t> decrypted = encoder.Encode(encrypted);
	WriteFile(kDecryptedFile, decrypted);
	return 0;
}