#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int num_args, char** args)
{
	FILE* input_file = nullptr;
	errno_t err = fopen_s(&input_file, "./input.txt", "rb");
	if (err != 0l)
		return -1;

	uint32_t sum_of_calibration_values = 0u;

	constexpr const char* LETTER_DIGITS[] = { "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
	constexpr const uint32_t LETTER_DIGIT_LENGTHS[] = { 3u, 3u, 5u, 4u, 4u, 3u, 5u, 5u, 4u };
	constexpr uint32_t NUM_LETTER_DIGITS = sizeof(LETTER_DIGITS) / sizeof(char*);

	constexpr uint32_t DIGITS_STORAGE_SIZE = 16u;
	int8_t digits_storage[DIGITS_STORAGE_SIZE];

	constexpr uint32_t BUFFER_SIZE = 256u;
	int8_t buffer[BUFFER_SIZE];

	uint32_t buffer_offset = 0u;
	uint32_t buffer_read_bytes = BUFFER_SIZE;
	while (true) {
		const uint32_t bytes_read = (uint32_t)fread(buffer + buffer_offset, sizeof(int8_t), buffer_read_bytes, input_file);
		if (bytes_read == 0u)
			break;

		const uint32_t bytes_available = buffer_offset + bytes_read;

		// Scan buffer for new lines (\n)
		// Each line contains a string with some random digits and some digitis are spelled out with letters
		// Letter digits one -> nine
		// Sum the first and last digit in the string.
		// Note: If only one digit in the string sum it twice.

		uint32_t current_pos = 0u;
		while (true) {
			uint32_t end_pos = current_pos;
			for (uint32_t i = current_pos; i < bytes_available; ++i) {
				if (buffer[i] == '\n') {
					end_pos = i;
					break;
				}
			}
			// failed to find string, buffer might be too small?
			if (current_pos == 0u && end_pos == 0u)
				return -2;
			// no new line found in buffer
			if (current_pos == end_pos)
				break;

			// store digits from current string

			uint32_t num_digits = 0u;
			for (uint32_t i = current_pos; i < end_pos;) {
				const int8_t& item = buffer[i];

				int8_t digit = 0l;
				const bool is_ascii_digit = item >= 48l && item <= 57l;
				if (is_ascii_digit) {
					digit = item;
					++i;
				} else {
					// find letter digit in current string
					for (uint32_t letter_index = 0u; letter_index < NUM_LETTER_DIGITS; ++letter_index) {
						// ensure not reading outside of buffer
						if (LETTER_DIGIT_LENGTHS[letter_index] > (end_pos - i))
							continue;
						const bool found_letter_digit = strncmp((const char*)buffer + i, LETTER_DIGITS[letter_index], LETTER_DIGIT_LENGTHS[letter_index]) == 0l;
						if (found_letter_digit) {
							digit = (int8_t)48l + (int8_t)(letter_index + 1u);
							// can't increment by digit length. consider: eightwothree
							//i += LETTER_DIGIT_LENGTHS[letter_index];
							++i;
							break;
						}
					}
					// no letter digit found, continue parsing string
					if (digit == 0l) {
						++i;
						continue;
					}
				}

				// `digits_storage` is full, increase storage size
				if (num_digits == DIGITS_STORAGE_SIZE - 1u)
					return -3;

				digits_storage[num_digits++] = digit;
			}

			// failed read any digit from string, is input corrupt?
			if (num_digits == 0u)
				return -4;

			if (num_digits == 1u)
				digits_storage[num_digits++] = digits_storage[0];

			char calibration_string[3];
			calibration_string[0] = digits_storage[0];
			calibration_string[1] = digits_storage[num_digits - 1u];
			calibration_string[2] = 0;

			sum_of_calibration_values += (uint32_t)strtol(calibration_string, nullptr, 10l);

			// continue parsing buffer
			current_pos = end_pos + 1u;
		}

		// ensure valid pos
		if (current_pos > bytes_available)
			return -5;

		const uint32_t buffer_left = bytes_available - current_pos;
		memmove(buffer, buffer + current_pos, buffer_left);
		buffer_offset = buffer_left;
		buffer_read_bytes = BUFFER_SIZE - buffer_left;
	}

	fclose(input_file);

	printf("Q: What is the sum of all of the calibration values?\nA: %d", sum_of_calibration_values);

	return 0;
}
