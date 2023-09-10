#include "custom_sounds.hpp"

__forceinline void setup_sounds()
{
	CreateDirectory("csgo\\sound", nullptr);
	FILE* file = nullptr;

	file = fopen(crypt_str("csgo\\sound\\metallic.wav"), crypt_str("wb"));
	fwrite(metallic, sizeof(unsigned char), 64700, file); //-V575
	fclose(file);
}