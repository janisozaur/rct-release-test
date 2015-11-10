/*****************************************************************************
 * Copyright (c) 2014 Ted John
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "../addresses.h"
#include "../platform/platform.h"
#include "sawyercoding.h"

static size_t decode_chunk_rle(const uint8* src_buffer, uint8* dst_buffer, size_t length);
static size_t decode_chunk_repeat(uint8 *buffer, size_t length);
static void decode_chunk_rotate(uint8 *buffer, size_t length);

static size_t encode_chunk_rle(const uint8 *src_buffer, uint8 *dst_buffer, size_t length);
static size_t encode_chunk_repeat(const uint8 *src_buffer, uint8 *dst_buffer, size_t length);
static void encode_chunk_rotate(uint8 *buffer, size_t length);

uint32 sawyercoding_calculate_checksum(const uint8* buffer, size_t length)
{
	size_t i;
	uint32 checksum = 0;
	for (i = 0; i < length; i++)
		checksum += buffer[i];

	return checksum;
}

/**
 *
 *  rct2: 0x00676FD2
 */
int sawyercoding_validate_checksum(SDL_RWops* rw)
{
	size_t i, dataSize, bufferSize;
	uint32 checksum, fileChecksum;
	uint8 buffer[1024];

	// Get data size
	SDL_RWseek(rw, 0, RW_SEEK_END);
	// for whatever the reason, SDL_RWtell return Sint64 instead of size_t.
	dataSize = (size_t)SDL_RWtell(rw);
	if (dataSize < 8)
		return 0;
	dataSize -= 4;

	// Calculate checksum
	SDL_RWseek(rw, 0, RW_SEEK_SET);
	checksum = 0;
	do {
		bufferSize = min(dataSize, 1024);
		if (SDL_RWread(rw, buffer, bufferSize, 1) != 1)
			return 0;

		for (i = 0; i < bufferSize; i++)
			checksum += buffer[i];
		dataSize -= bufferSize;
	} while (dataSize != 0);

	// Read file checksum
	if (SDL_RWread(rw, &fileChecksum, sizeof(fileChecksum), 1) != 1)
		return 0;

	// Reset file position
	SDL_RWseek(rw, 0, RW_SEEK_SET);

	// Validate
	return checksum == fileChecksum;
}

/**
 *
 *  rct2: 0x0067685F
 * buffer (esi)
 */
size_t sawyercoding_read_chunk(SDL_RWops* rw, uint8 *buffer)
{
	sawyercoding_chunk_header chunkHeader;

	// Read chunk header
	if (SDL_RWread(rw, &chunkHeader, sizeof(sawyercoding_chunk_header), 1) != 1) {
		log_error("Unable to read chunk header!");
		return -1;
	}

	uint8* src_buffer = malloc(chunkHeader.length);

	// Read chunk data
	if (SDL_RWread(rw, src_buffer, chunkHeader.length, 1) != 1) {
		free(src_buffer);
		log_error("Unable to read chunk data!");
		return -1;
	}

	// Decode chunk data
	switch (chunkHeader.encoding) {
	case CHUNK_ENCODING_NONE:
		memcpy(buffer, src_buffer, chunkHeader.length);
		break;
	case CHUNK_ENCODING_RLE:
		chunkHeader.length = decode_chunk_rle(src_buffer, buffer, chunkHeader.length);
		break;
	case CHUNK_ENCODING_RLECOMPRESSED:
		chunkHeader.length = decode_chunk_rle(src_buffer, buffer, chunkHeader.length);
		chunkHeader.length = decode_chunk_repeat(buffer, chunkHeader.length);
		break;
	case CHUNK_ENCODING_ROTATE:
		memcpy(buffer, src_buffer, chunkHeader.length);
		decode_chunk_rotate(buffer, chunkHeader.length);
		break;
	}
	free(src_buffer);
	// Set length
	RCT2_GLOBAL(0x009E3828, uint32) = chunkHeader.length;
	return chunkHeader.length;
}

/**
*
*  rct2: 0x006762E1
*
*/
size_t sawyercoding_write_chunk_buffer(uint8 *dst_file, uint8* buffer, sawyercoding_chunk_header chunkHeader){
	uint8 *encode_buffer, *encode_buffer2;

	switch (chunkHeader.encoding){
	case CHUNK_ENCODING_NONE:
		memcpy(dst_file, &chunkHeader, sizeof(sawyercoding_chunk_header));
		dst_file += sizeof(sawyercoding_chunk_header);
		memcpy(dst_file, buffer, chunkHeader.length);
		//fwrite(&chunkHeader, sizeof(sawyercoding_chunk_header), 1, file);
		//fwrite(buffer, 1, chunkHeader.length, file);
		break;
	case CHUNK_ENCODING_RLE:
		encode_buffer = malloc(0x600000);
		chunkHeader.length = encode_chunk_rle(buffer, encode_buffer, chunkHeader.length);
		memcpy(dst_file, &chunkHeader, sizeof(sawyercoding_chunk_header));
		dst_file += sizeof(sawyercoding_chunk_header);
		memcpy(dst_file, encode_buffer, chunkHeader.length);

		free(encode_buffer);
		break;
	case CHUNK_ENCODING_RLECOMPRESSED:
		encode_buffer = malloc(chunkHeader.length * 2);
		encode_buffer2 = malloc(0x600000);
		chunkHeader.length = encode_chunk_repeat(buffer, encode_buffer, chunkHeader.length);
		chunkHeader.length = encode_chunk_rle(encode_buffer, encode_buffer2, chunkHeader.length);
		memcpy(dst_file, &chunkHeader, sizeof(sawyercoding_chunk_header));
		dst_file += sizeof(sawyercoding_chunk_header);
		memcpy(dst_file, encode_buffer2, chunkHeader.length);

		free(encode_buffer2);
		free(encode_buffer);
		break;
	case CHUNK_ENCODING_ROTATE:
		encode_buffer = malloc(chunkHeader.length);
		memcpy(encode_buffer, buffer, chunkHeader.length);
		encode_chunk_rotate(encode_buffer, chunkHeader.length);
		memcpy(dst_file, &chunkHeader, sizeof(sawyercoding_chunk_header));
		dst_file += sizeof(sawyercoding_chunk_header);
		memcpy(dst_file, encode_buffer, chunkHeader.length);

		free(encode_buffer);
		break;
	}

	return chunkHeader.length + sizeof(sawyercoding_chunk_header);
}

size_t sawyercoding_decode_sv4(const uint8 *src, uint8 *dst, size_t length)
{
	// (0 to length - 4): RLE chunk
	// (length - 4 to length): checksum
	return decode_chunk_rle(src, dst, length - 4);
}

size_t sawyercoding_decode_sc4(const uint8 *src, uint8 *dst, size_t length)
{
	size_t decodedLength, i;
	uint32 *code;

	// Uncompress
	decodedLength = decode_chunk_rle(src, dst, length - 4);

	// Decode
	for (i = 0x60018; i <= min(decodedLength - 1, 0x1F8353); i++)
		dst[i] = dst[i] ^ 0x9C;

	for (i = 0x60018; i <= min(decodedLength - 1, 0x1F8350); i += 4) {
		dst[i + 1] = ror8(dst[i + 1], 3);

		code = (uint32*)&dst[i];
		*code = rol32(*code, 9);
	}

	return decodedLength;
}

size_t sawyercoding_encode_sv4(const uint8 *src, uint8 *dst, size_t length)
{
	size_t encodedLength;
	uint32 checksum;

	// Encode
	encodedLength = encode_chunk_rle(src, dst, length);

	// Append checksum
	checksum = sawyercoding_calculate_checksum(dst, encodedLength);
	*((uint32*)&dst[encodedLength]) = checksum;

	return encodedLength + 4;
}

size_t sawyercoding_decode_td6(const uint8 *src, uint8 *dst, size_t length)
{
	return decode_chunk_rle(src, dst, length - 4);
}

size_t sawyercoding_encode_td6(const uint8* src, uint8* dst, size_t length){
	size_t output_length = encode_chunk_rle(src, dst, length);

	uint32 checksum = 0;
	for (size_t i = 0; i < output_length; i++){
		uint8 new_byte = ((checksum & 0xFF) + dst[i]) & 0xFF;
		checksum = (checksum & 0xFFFFFF00) + new_byte;
		checksum = rol32(checksum, 3);
	}
	checksum -= 0x1D4C1;

	*((uint32*)&dst[output_length]) = checksum;
	output_length += 4;
	return output_length;
}

/* Based off of rct2: 0x006770C1 */
int sawyercoding_validate_track_checksum(const uint8* src, size_t length){
	uint32 file_checksum = *((uint32*)&src[length - 4]);

	uint32 checksum = 0;
	for (size_t i = 0; i < length - 4; i++){
		uint8 new_byte = ((checksum & 0xFF) + src[i]) & 0xFF;
		checksum = (checksum & 0xFFFFFF00) + new_byte;
		checksum = rol32(checksum, 3);
	}

	if (checksum - 0x1D4C1 == file_checksum)
		return 1;	// .TD6
	else if (checksum - 0x1A67C == file_checksum)
		return 1;	// .TD4
	else if (checksum - 0x1A650 == file_checksum)
		return 1;	// .TD4
	else
		return 0;
}

#pragma region Decoding

/**
 *
 *  rct2: 0x0067693A
 */
static size_t decode_chunk_rle(const uint8* src_buffer, uint8* dst_buffer, size_t length)
{
	size_t i, j, count;
	uint8 *dst, rleCodeByte;

	dst = dst_buffer;

	for (i = 0; i < length; i++) {
		rleCodeByte = src_buffer[i];
		if (rleCodeByte & 128) {
			i++;
			count = 257 - rleCodeByte;
			for (j = 0; j < count; j++)
				*dst++ = src_buffer[i];
		} else {
			for (j = 0; j <= rleCodeByte; j++)
				*dst++ = src_buffer[++i];
		}
	}

	// Return final size
	return dst - dst_buffer;
}

/**
 *
 *  rct2: 0x006769F1
 */
static size_t decode_chunk_repeat(uint8 *buffer, size_t length)
{
	size_t i, j, count;
	uint8 *src, *dst, *copyOffset;

	// Backup buffer
	src = malloc(length);
	memcpy(src, buffer, length);
	dst = buffer;

	for (i = 0; i < length; i++) {
		if (src[i] == 0xFF) {
			*dst++ = src[++i];
		} else {
			count = (src[i] & 7) + 1;
			copyOffset = dst + (int)(src[i] >> 3) - 32;
			for (j = 0; j < count; j++)
				*dst++ = *copyOffset++;
		}
	}

	// Free backup buffer
	free(src);

	// Return final size
	return dst - buffer;
}

/**
 *
 *  rct2: 0x006768F4
 */
static void decode_chunk_rotate(uint8 *buffer, size_t length)
{
	size_t i;
	uint8 code = 1;
	for (i = 0; i < length; i++) {
		buffer[i] = ror8(buffer[i], code);
		code = (code + 2) % 8;
	}
}

#pragma endregion

#pragma region Encoding

/**
 * Ensure dst_buffer is bigger than src_buffer then resize afterwards
 * returns length of dst_buffer
 */
static size_t encode_chunk_rle(const uint8 *src_buffer, uint8 *dst_buffer, size_t length)
{
	const uint8* src = src_buffer;
	uint8* dst = dst_buffer;
	const uint8* end_src = src + length;
	uint8 count = 0;
	const uint8* src_norm_start = src;

	while (src < end_src - 1){

		if ((count && *src == src[1]) || count > 125){
			*dst++ = count - 1;
			for (; count != 0; --count){
				*dst++ = *src_norm_start++;
			}
		}
		if (*src == src[1]){
			for (; (count < 125) && ((src + count) < end_src); count++){
				if (*src != src[count]) break;
			}
			*dst++ = 257 - count;
			*dst++ = *src;
			src += count;
			src_norm_start = src;
			count = 0;
		}
		else{
			count++;
			src++;
		}
	}
	if (src == end_src - 1)count++;
	if (count){
		*dst++ = count - 1;
		for (; count != 0; --count){
			*dst++ = *src_norm_start++;
		}
	}
	return dst - dst_buffer;
}

static size_t encode_chunk_repeat(const uint8 *src_buffer, uint8 *dst_buffer, size_t length)
{
	size_t i, j, outLength;
	size_t searchIndex, searchEnd, maxRepeatCount;
	size_t bestRepeatIndex, bestRepeatCount, repeatIndex, repeatCount;

	if (length == 0)
		return 0;

	outLength = 0;

	// Need to emit at least one byte, otherwise there is nothing to repeat
	*dst_buffer++ = 255;
	*dst_buffer++ = src_buffer[0];
	outLength += 2;

	// Iterate through remainder of the source buffer
	for (i = 1; i < length; ) {
		searchIndex = max(0, i - 32);
		searchEnd = i - 1;

		bestRepeatCount = 0;
		for (repeatIndex = searchIndex; repeatIndex <= searchEnd; repeatIndex++) {
			repeatCount = 0;
			maxRepeatCount = min(7, searchEnd - repeatIndex);
			for (j = 0; j <= maxRepeatCount; j++) {
				if (src_buffer[repeatIndex + j] == src_buffer[i + j]) {
					repeatCount++;
				} else {
					break;
				}
			}
			if (repeatCount > bestRepeatCount) {
				bestRepeatIndex = repeatIndex;
				bestRepeatCount = repeatCount;

				// Maximum repeat count is 8
				if (repeatCount == 8)
					break;
			}
		}

		if (bestRepeatCount == 0) {
			*dst_buffer++ = 255;
			*dst_buffer++ = src_buffer[i];
			outLength += 2;
			i++;
		} else {
			*dst_buffer++ = (uint8)((bestRepeatCount - 1) | ((32 - (i - bestRepeatIndex)) << 3));
			outLength++;
			i += bestRepeatCount;
		}
	}

	return outLength;
}

static void encode_chunk_rotate(uint8 *buffer, size_t length)
{
	size_t i;
	uint8 code = 1;
	for (i = 0; i < length; i++) {
		buffer[i] = rol8(buffer[i], code);
		code = (code + 2) % 8;
	}
}

#pragma endregion

int sawyercoding_detect_file_type(const uint8 *src, size_t length)
{
	size_t i;

	// Currently can't detect TD4, as the checksum is the same as SC4 (need alternative method)

	uint32 checksum = *((uint32*)&src[length - 4]);
	uint32 actualChecksum = 0;
	for (i = 0; i < length - 4; i++) {
		actualChecksum = (actualChecksum & 0xFFFFFF00) | (((actualChecksum & 0xFF) + (uint8)src[i]) & 0xFF);
		actualChecksum = rol32(actualChecksum, 3);
	}

	switch (checksum - actualChecksum) {
	case +108156:	return FILE_VERSION_RCT1 | FILE_TYPE_SV4;
	case -108156:	return FILE_VERSION_RCT1 | FILE_TYPE_SC4;
	case +110001:	return FILE_VERSION_RCT1_AA | FILE_TYPE_SV4;
	case -110001:	return FILE_VERSION_RCT1_AA | FILE_TYPE_SC4;
	case +120001:	return FILE_VERSION_RCT1_LL | FILE_TYPE_SV4;
	case -120001:	return FILE_VERSION_RCT1_LL | FILE_TYPE_SC4;
	}

	return -1;
}
