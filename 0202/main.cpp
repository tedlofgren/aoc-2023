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

	uint32_t sum_power_of_largest_sets = 0;

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
		// Each line contains a string information about game out comes
		// Each string starts with Game ID: followed by comma separated list of outcomes
		// Sum game ids that are possible (based on some requirements)

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

			// parse string

			constexpr uint32_t GAME_ID_OFFSET = 5u; // "Game "
			char* comma_separated_list = nullptr;
			uint32_t game_id = (uint32_t)strtol((char*)buffer + current_pos + GAME_ID_OFFSET, &comma_separated_list, 10l);
			if (comma_separated_list == nullptr)
				return -3;
			comma_separated_list += 2; // skip ": "

			uint32_t red = 0u;
			uint32_t green = 0u;
			uint32_t blue = 0u;
			while (true) {
				uint32_t number = (uint32_t)strtol(comma_separated_list, &comma_separated_list, 10l);
				if (number == 0u)
					return -4;

				// skip whitespace
				comma_separated_list += 1;

				if (strncmp(comma_separated_list, "red", 3) == 0) {
					if (number > red)
						red = number;

					comma_separated_list += 3;
				} else if (strncmp(comma_separated_list, "green", 5) == 0) {
					if (number > green)
						green = number;

					comma_separated_list += 5;
				} else if (strncmp(comma_separated_list, "blue", 4) == 0) {
					if (number > blue)
						blue = number;

					comma_separated_list += 4;
				} else {
					return -5;
				}

				if (comma_separated_list[0] == ',' || comma_separated_list[0] == ';')
					comma_separated_list += 2;

				if ((uintptr_t)comma_separated_list == (uintptr_t)(buffer + end_pos))
					break;
			}

			sum_power_of_largest_sets += (red * green * blue);

			// continue parsing buffer
			current_pos = end_pos + 1u;
		}

		// ensure valid pos
		if (current_pos > bytes_available)
			return -6;

		const uint32_t buffer_left = bytes_available - current_pos;
		memmove(buffer, buffer + current_pos, buffer_left);
		buffer_offset = buffer_left;
		buffer_read_bytes = BUFFER_SIZE - buffer_left;
	}

	fclose(input_file);

	printf("Q: What is the sum of the power of these sets?\nA: %d", sum_power_of_largest_sets);

	return 0;
}
