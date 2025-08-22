#include <cstdint>
#include <tuple>
#include <filesystem>
#include <print>
#include <fstream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Node
{
	char data;
	Node* left;
	Node* right;
	Node(char d) : data(d), left(nullptr), right(nullptr) {}
};

struct HuffmanCode
{
	uint32_t bits;
	uint8_t length;
};

bool encode(std::filesystem::path path)
{
	// Read given file as binary to buffer
	std::ifstream file(path, std::ios::binary);
    if (!std::filesystem::exists(path)) {
		std::println("File doesnt exist, {}", path.c_str());
		return false;
	}
    file.seekg(0, std::ios::end);
    uint64_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
	if (buffer.empty())
		return false;


	// Get most used chars
	std::unordered_map<char, uint64_t> frequencies;
	for (const auto& byte : buffer)
	{
		frequencies[byte]++;
	}

	// Sort the most used chars
	auto cmp = [](const std::pair<Node*, uint64_t>& a, const std::pair<Node*, uint64_t>& b)
	{
		return a.second > b.second;
	};
	std::priority_queue<std::pair<Node*, uint64_t>, std::vector<std::pair<Node*, uint64_t>>, decltype(cmp)> min_heap;

	for (const auto& [ch, freq] : frequencies)
	{
		min_heap.push({new Node(ch), freq});
	}

	// Create huffman tree
	while (min_heap.size() > 1)
	{
		auto left = min_heap.top();
		min_heap.pop();
		auto right = min_heap.top();
		min_heap.pop();
		Node* parent = new Node('\0');
		parent->left = left.first;
		parent->right = right.first;
		min_heap.push({parent, left.second + right.second});
	}
	Node* root = min_heap.top().first;

	// Assign codes
	std::unordered_map<unsigned char, HuffmanCode> codes;
	std::stack<std::pair<Node*, HuffmanCode>> stk;
	stk.push({root, HuffmanCode{0,0}});

	while (!stk.empty())
	{
		auto [node, path] = stk.top();
		stk.pop();
		if (!node->left && !node->right) {
			codes[node->data] = path;
		} else {
			if (node->right)
			{
				HuffmanCode next = path;
				next.bits = (next.bits << 1) | 1; // append 1
				next.length++;
				stk.push({node->right, next});
			}
			if (node->left)
			{
				HuffmanCode next = path;
				next.bits = (next.bits << 1) | 0; // append 0
				next.length++;
				stk.push({node->left, next});
			}
		}
	}

	// Pack bits into a bitstream
	std::vector<uint8_t> bitstream;
	bitstream.reserve(buffer.size());
	uint64_t bit_buffer;
	uint8_t current_byte = 0;
	int bit_count = 0;
	size_t percent;
	for (const unsigned char& sym : buffer)
	{
		auto code = codes[sym];
		for (int i = code.length - 1; i >= 0; --i)
		{
			current_byte = (current_byte << 1) | (code.bits >> i) & 1;
			bit_count++;
			if (bit_count == 8)
			{
				bitstream.push_back(current_byte);
				current_byte = 0;
				bit_count = 0;
			}
		}

		if (percent % (buffer.size() / 100) == 0) {
           std::print("\rProgress: {}%", (percent * 100) / buffer.size());
           std::fflush(stdout);
		}
		percent++;
	}
	if (bit_count > 0)
	{
		current_byte <<= (8 - bit_count);
		bitstream.push_back(current_byte);
	}

	// Save data to file
	path += ".hff";
	std::ofstream compressed_file(path, std::ios::binary);
	uint64_t freq_size = frequencies.size();
	compressed_file.write(reinterpret_cast<const char*>(&freq_size), sizeof(freq_size));
	compressed_file.write(reinterpret_cast<const char*>(&size), sizeof(size));
	for (const auto& [char_, freq] : frequencies)
	{
		uint64_t f = freq;
		compressed_file.write(&char_, sizeof(char_));
		compressed_file.write(reinterpret_cast<const char*>(&f), sizeof(f));
	}
	compressed_file.write(reinterpret_cast<const char*>(bitstream.data()), bitstream.size());
	compressed_file.close();

	return true;
}

bool decode(std::filesystem::path path)
{
	// Read given file as binary to buffer
	std::ifstream file(path, std::ios::binary);
    if (!std::filesystem::exists(path)) {
		std::println("File doesnt exist, {}", path.c_str());
		return false;
	}
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
	if (buffer.empty())
		return false;

	// Get frequency table and total char size
	uint64_t freq_size;
	uint64_t og_size;
	std::unordered_map<char, uint64_t> frequencies;
    std::memcpy(&freq_size, buffer.data(), sizeof(freq_size));
    std::memcpy(&og_size, buffer.data() + sizeof(freq_size), sizeof(og_size));
	int offset = sizeof(freq_size) + sizeof(og_size);
	for (uint64_t i = 0; i < freq_size; i++)
	{
		char symbol = buffer[offset];
		uint64_t freq;
        std::memcpy(&freq, &buffer[offset + 1], sizeof(freq));
        frequencies[symbol] = freq;
        offset += 1 + sizeof(freq);
	}

	// Get Huffman data
    std::vector<unsigned char> bitstream(buffer.begin() + offset, buffer.end());

	// Sort the most used chars
	auto cmp = [](const std::pair<Node*, uint64_t>& a, const std::pair<Node*, uint64_t>& b)
	{
		return a.second > b.second;
	};
	std::priority_queue<std::pair<Node*, uint64_t>, std::vector<std::pair<Node*, uint64_t>>, decltype(cmp)> min_heap;

	for (const auto& [ch, freq] : frequencies)
	{
		min_heap.push({new Node(ch), freq});
	}

	// Rebuild huffman tree
	while (min_heap.size() > 1)
	{
		auto left = min_heap.top();
		min_heap.pop();
		auto right = min_heap.top();
		min_heap.pop();
		Node* parent = new Node('\0');
		parent->left = left.first;
		parent->right = right.first;
		min_heap.push({parent, left.second + right.second});
	}
	Node* root = min_heap.top().first;

	// Decode the compressed data
	std::string result;
	result.reserve(og_size);
	Node* current = root;
	bool done = false;
	size_t percent;
	for (const auto& byte : bitstream)
	{
		for (int i = 7; i >= 0; i--)
		{
			auto bit = (byte >> i) & 1;

			if (!bit)
				current = current->left;
			else
				current = current->right;
			if (!current->left && !current->right)
			{
				result.push_back(current->data);
				if (result.size() == og_size)
				{
					done = true;
					break;
				}
				current = root;
			}
		}
		if (done)
			break;

		if (percent % (buffer.size() / 100) == 0) {
           std::print("\rProgress: {}%", (percent * 100) / buffer.size());
           std::fflush(stdout);
		}
		percent++;
	}

	// Save decoded data
	path = path.replace_extension("");
	std::ofstream raw_file(path, std::ios::binary);
	raw_file.write(result.c_str(), result.size());
	raw_file.close();

	return true;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::println("Error : Incorrect number of parameters received.");
		std::println("Correct usage : {} -[e,d] <file>", argv[0]);
		std::println("Encoding : ./huffman-encoder -e example.txt");
		std::println("Decoding : ./huffman-encoder -d example.hff");
	}

	std::string mode = argv[1];
	std::filesystem::path file = argv[2];

	if (mode == "-e")
	{
		if (encode(file))
			std::println("\nFile compressed successfully.");
		else
			 std::println("Error during compression, exiting.");
	} else if (mode == "-d") {
		 if(decode(file))
			 std::println("\nFile decompressed successfully.");
		 else
			 std::println("Error during decompression, exiting.");
	} else {
		std::println("Unknown mode: {}", mode);
        std::println("Usage: {} -[e|d] <file>", argv[0]);
		return 1;
	}
	return 0;
}
