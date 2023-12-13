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

	uint32_t sum_of_parts_numbers = 0;

	constexpr uint32_t BUFFER_SIZE = 256u;
	int8_t buffer[BUFFER_SIZE];

	struct Row {
		int8_t data[BUFFER_SIZE];
		uint32_t length;
		uint32_t id;
	};
	constexpr uint32_t NUM_ROWS = 3;
	Row rows[NUM_ROWS] = {};
	uint32_t row_counter = 0;
	uint32_t num_processed_rows = 0;
	uint32_t current_row_pos = 0;

	struct PartReader {
		uint32_t row_id;
		uint32_t part_start_offset;
		uint32_t part_end_offset;
		uint32_t part_number;
		int32_t status; // returns: -2 (need more data), -1 (not valid part), 0 (none), 1 (valid part), 2 (evaluating)
	};

	PartReader part_reader = {};

	uint32_t buffer_offset = 0u;
	uint32_t buffer_read_bytes = BUFFER_SIZE;
	while (true) {
		const uint32_t bytes_read = (uint32_t)fread(buffer + buffer_offset, sizeof(int8_t), buffer_read_bytes, input_file);
		const uint32_t bytes_available = buffer_offset + bytes_read;
		if (bytes_available == 0u)
			break;

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

			const bool more_data_in_buffer = end_pos + 1 < bytes_available;

			// add new row
			const uint32_t row_index = row_counter % NUM_ROWS;
			rows[row_index].length = end_pos - current_pos;
			rows[row_index].id = row_counter++;
			memcpy(rows[row_index].data, buffer + current_pos, rows[row_index].length);

			// continue evaluating part number after new row was added
			if (part_reader.status == -2)
				part_reader.status = 2;

			// process rows
			while (true) {
				if (part_reader.status == 0) {
					// find part number to evaluate
					uint32_t row_index_to_process = num_processed_rows % NUM_ROWS;
					for (uint32_t pos = current_row_pos; pos < rows[row_index_to_process].length; ++pos) {
						if (rows[row_index_to_process].data[pos] == '.')
							continue;

						if (rows[row_index_to_process].data[pos] == '-' || rows[row_index_to_process].data[pos] == '+')
							continue;

						char* part_end_ptr = 0;
						uint32_t part_number = (uint32_t)strtol((char*)rows[row_index_to_process].data + pos, &part_end_ptr, 10);
						if (part_number == 0)
							continue;

						// found part number to evaluate
						part_reader.part_start_offset = pos;
						part_reader.part_end_offset = pos + (uint32_t)(uintptr_t)(part_end_ptr - ((char*)rows[row_index_to_process].data + pos));
						part_reader.part_number = part_number;
						part_reader.row_id = rows[row_index_to_process].id;
						part_reader.status = 2;

						current_row_pos = pos;
						break;
					}
					// go to next row
					if (part_reader.status == 0) {
						current_row_pos = 0;
						num_processed_rows++;
					}
				}

				// evaluate part number
				while (part_reader.status == 2) {
					const uint32_t row_index = part_reader.row_id % NUM_ROWS;
					bool found_adjacent_symbol = false;

					// look for symbol before part number
					if (part_reader.part_start_offset > 0 && rows[row_index].data[part_reader.part_start_offset - 1] != '.') 
						found_adjacent_symbol = true;

					// look for symbol after part number
					if (part_reader.part_end_offset < rows[row_index].length && rows[row_index].data[part_reader.part_end_offset] != '.') {
						found_adjacent_symbol = true;
					}

					// look for symbol in above and below rows
					const int32_t part_number_length = part_reader.part_end_offset - part_reader.part_start_offset;
					for (uint32_t i = 0; i < 2 && !found_adjacent_symbol; ++i) {
						const uint32_t current_row_id = i == 0 ? (part_reader.row_id - 1) : (part_reader.row_id + 1);
						const bool is_row_valid = current_row_id < row_counter;
						if (!is_row_valid)
							continue;

						const uint32_t current_row_index = current_row_id % NUM_ROWS;
						for (int32_t j = -1; j < part_number_length + 1; ++j) {
							int32_t offset = part_reader.part_start_offset + j;
							if (offset == rows[current_row_index].length)
								break;
							if (offset < 0)
								continue;
							if (rows[current_row_index].data[offset] != '.') {
								found_adjacent_symbol = true;
								break;
							}
						}
					}

					const bool need_more_row_data = (part_reader.row_id + 1) == row_counter;
					if (found_adjacent_symbol) {
						part_reader.status = 1;
					} else if(need_more_row_data && more_data_in_buffer)
						part_reader.status = -2;
					else
						part_reader.status = -1;
				}

				// handle result after evaluating
				if (part_reader.status == 1) {
					sum_of_parts_numbers += part_reader.part_number;
					current_row_pos = part_reader.part_end_offset;
					part_reader.status = 0;
				} else if (part_reader.status == -1) {
					current_row_pos = part_reader.part_end_offset;
					part_reader.status = 0;
				} else if (part_reader.status == -2)
					break;

				// get more rows from buffer
				if (num_processed_rows == row_counter)
					break;
			}

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

	printf("Q: What is the sum of all of the part numbers in the engine schematic?\nA: %d", sum_of_parts_numbers);

	return 0;
}
