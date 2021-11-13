#include <cassert>
#include <iostream>

const unsigned int size = 1024 * 1024;
double x[size];
double y[size];
double z[size];
double xx[size];
double yy[size];
double zz[size];

const unsigned int block_size = 64;
const unsigned int address_offset = 6; // log(block_size)

const unsigned int L1_assoc = 4;
const unsigned int L1_size = 32 * 1024;

const unsigned int L2_assoc = 8;
const unsigned int L2_size = 1024 * 1024;

// wrapper from real address of arrays (x, y, z, ...) to their
// imaginary address from the statements (considering arrays follow each other)
unsigned int get_imag_address(double *arr, unsigned int index) {
	if (arr == x) {
		return index * 8;
	}
	if (arr == y) {
		return (size + index) * 8;
	}
	if (arr == z) {
		return (size * 2 + index) * 8;
	}
	if (arr == xx) {
		return (size * 3 + index) * 8;
	}
	if (arr == yy) {
		return (size * 4 + index) * 8;
	}
	if (arr == zz) {
		return (size * 5 + index) * 8;
	}
	assert(0);
}

struct Cache {
	struct Cache_line {
		int address, used;

		Cache_line() {
			used = -1; address = -1;
		}
	};

	unsigned int assoc, size, bank_size;
	unsigned int cache_hit, cache_miss, timer;
	Cache_line **lines;
	Cache *next_device; // pointer to device on next level (for example,
	                    // pointer to L2 for L1 or pointer to RAM for L2)

	Cache(unsigned int _assoc, unsigned int _size, Cache *next_dev = nullptr) {
		assoc = _assoc; size = _size; bank_size = size / block_size / assoc;
		cache_hit = 0; cache_miss = 0; timer = 0;
		lines = new Cache_line*[assoc];
		for (unsigned int i = 0; i < assoc; i++) {
			lines[i] = new Cache_line[bank_size];
		}
		next_device = next_dev;
	}

	~Cache() {
		for (unsigned int i = 0; i < assoc; i++) {
			delete[] lines[i];
		}
		delete[] lines;
	}

	// checks if address is already stored in cache;
	// if stored returns its position,
	// otherwise returns -1
	int find_address(unsigned int address) {
		for (unsigned int i = 0; i < assoc; i++) {
			if (lines[i][address % bank_size].address == address) {
				return i;
			}
		}
		return -1;
	}

	// loads address to cache using LRU replacement policy
	int load_address(unsigned int address) {
		unsigned int bank_num = 0;
		for (unsigned int i = 1; i < assoc; i++) {
			if (lines[i][address % bank_size].used < lines[bank_num][address % bank_size].used) {
				bank_num = i;
			}
		}
		// if it's needed to replace element in cache,
		// element is going to next level device
		// (from L1 to L2, from L2 to RAM)
		if (next_device != nullptr && lines[bank_num][address % bank_size].address != -1) {
			next_device->put_address(lines[bank_num][address % bank_size].address);
		}
		lines[bank_num][address % bank_size].address = address;
		return bank_num;
	}

	// function to emulate access to specified address;
	// checks if address is already stored in cache;
	// if not, loads this address to cache;
	// registers cache hits and misses
	bool put_address(unsigned int address) {
		address = (address >> address_offset) << address_offset;
		int bank_num = find_address(address);
		if (bank_num != -1) {
			cache_hit++;
			lines[bank_num][address % bank_size].used = timer++;
			return true;
		}
		cache_miss++;
		bank_num = load_address(address);
		lines[bank_num][address % bank_size].used = timer++;
		return false;
	}
};

Cache l2(L2_assoc, L2_size, nullptr); // there must be a pointer to RAM here
Cache l1(L1_assoc, L1_size, &l2);

struct Cache_controller {
	void push_address_to_cache(unsigned int address) {
		if (l1.put_address(address)) {
			return;
		}
		l2.put_address(address);
	}
} controller;

// file with function
#include "func.h"

int main() {
	f(1.0);

	std::cerr << l1.cache_hit << " " << l1.cache_miss << std::endl;
	std::cerr << l2.cache_hit << " " << l2.cache_miss << std::endl;

	std::cout << l1.cache_hit / (double)(l1.cache_hit + l1.cache_miss) * 100 << std::endl;
	std::cout << l2.cache_hit / (double)(l2.cache_hit + l2.cache_miss) * 100 << std::endl;

	return 0;
}