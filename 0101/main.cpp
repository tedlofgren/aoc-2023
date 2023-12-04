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

	constexpr uint32_t DIGITS_STORAGE_SIZE = 8u;
	int8_t digits_storage[DIGITS_STORAGE_SIZE];

	uint32_t sum_of_calibration_values = 0u;

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
		// Each line contains a string with some random digits
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
			// failed to find string, buffer might be too short
			if (current_pos == 0u && end_pos == 0u)
				return -2;
			// no new line found in buffer
			if (current_pos == end_pos)
				break;

			// store digits from current string

			uint32_t num_digits = 0u;
			for (uint32_t i = current_pos; i < end_pos; ++i) {
				const int8_t& item = buffer[i];
				if (item < 48l || item > 57l)
					continue;

				if (num_digits == DIGITS_STORAGE_SIZE - 1u)
					return -3;

				digits_storage[num_digits++] = item;
			}

			if (num_digits == 0u)
				return -4;

			if (num_digits == 1u)
				digits_storage[num_digits++] = digits_storage[0];

			char calibration_string[3];
			calibration_string[0] = digits_storage[0];
			calibration_string[1] = digits_storage[num_digits - 1u];
			calibration_string[2] = 0;

			sum_of_calibration_values += (uint32_t)strtol(calibration_string, nullptr, 10l);

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
