#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iomanip>
#include <format>
#include <cstdint>
#include <string_view>

namespace Config {
    constexpr std::string_view VERSION = "1.0.2";
}


// РАНДОМ
using u64 = unsigned long long;
using u32 = unsigned int;

struct PCG_State {
    u64 state;
    u64 increment;
};

static constexpr u64 PCG_MULT = 6364136223846793005ULL;

static u32 next_pcg32(PCG_State* p) {
    u64 prev = p->state;
    p->state = prev * PCG_MULT + p->increment;
    
    u32 mixed = (u32)(((prev >> 18u) ^ prev) >> 27u);
    u32 shift = (u32)(prev >> 59u);
    
    return (mixed >> shift) | (mixed << ((~shift + 1u) & 31u));
}

static void init_pcg(PCG_State* p, u64 seed_value, u64 sequence) {
    p->state = 0;
    p->increment = (sequence << 1u) | 1u;
    
    next_pcg32(p);
    p->state += seed_value;
    next_pcg32(p);
}

void shuffle_wchar(u64& seed_ref, std::vector<wchar_t>& items) {
    PCG_State gen;
    init_pcg(&gen, seed_ref, 1442695040888963407ULL);
    
    long count = (long)items.size();
    if (count < 2) return;

    for (long idx = count - 1; idx > 0; --idx) {
        u32 rnd = next_pcg32(&gen);
        long other = (long)(rnd % (idx + 1));

        wchar_t tmp = items[(size_t)other];
        items[(size_t)other] = items[(size_t)idx];
        items[(size_t)idx] = tmp;
    }
}








//работа с генерируемыми алфавитами
std::wstring randomize_alphabet(u64 &seed, std::wstring &abc) {
	std::vector<wchar_t> arr(abc.begin(), abc.end());
	shuffle_wchar(seed, arr);
	std::wstring result(arr.begin(), arr.end());
	return result;
}
std::vector<std::wstring> generate_alphabets(u64 *seeds, int &lenseeds, std::wstring abc, int &lenabc) {
	std::vector<std::wstring> abcs(lenseeds);
    for (int i = 0; i<lenseeds; i++) {
    	abcs[i] = randomize_alphabet(seeds[i], abc);
    	std::wcout << randomize_alphabet(seeds[i], abc) << std::endl;
    }
    return abcs;
}


// запросы ввода
std::vector<std::wstring> seeds_req() {
	u64 seeds[] = {714, 825, 285};
	int lenseeds = 3;
	
	std::wstring mainabc = L"\nabcdefghijklmnopqrstuvwxyzабвгдежзийклмнопрстуфхцчшщьыъэюя1234567890 !?,.-äüöß<>()'\\/\"*:;[]#€₽@_~=";
	int lenmainabc = mainabc.size();
	
	std::vector<std::wstring> abcs = generate_alphabets(seeds, lenseeds, mainabc, lenmainabc);
	return abcs;
}
std::wstring text_req() {
	return L"<3";
}


//вспомогательное
std::wstring pad2(int number) {
    std::wstringstream wss;
    wss << std::setfill(L'0') << std::setw(2) << number;    
    return wss.str();
}


//кодеры
std::wstring encode() {
	std::vector<std::wstring> abcs = seeds_req();
	std::wstring text = text_req();
	
	std::vector<wchar_t> arr(text.begin(), text.end());
	std::wstring result = L"";
	for (int i = 0; i<arr.size(); i++) {
		result += pad2(abcs[i % abcs.size()].find(arr[i]));
    }
    return result;
}

std::wstring decode() {
	std::vector<std::wstring> abcs = seeds_req();
	std::wstring text = text_req();
	
	std::vector<wchar_t> arr(text.begin(), text.end());
	std::wstring result = L"";
	std::wstring buffer = L"";
	for (int i = 0; i<arr.size(); i++) {
		buffer += text[i];
		if (i%2) {
			result += abcs[(i/2) % abcs.size()][std::stoi(buffer)];
			buffer = L"";
        }
    }
    return result;
}

enum Options {
	OptEncode,
	OptDecode,
	OptExit
};
int main() {
	//float VERSION = 0.5;
    std::cout << std::format("HMcoder {}\n", Config::VERSION) << std::endl;
    
    //std::wcout << "HMcoder {}\n" << Config::VERSION << std::endl;
    
    // основной цикл
    bool running = true; 
    while (running) {
	    int selected_option;
	    std::cout << "0. encode text\n1. decode text\n2. exit" << std::endl;
	    std::cout << "option: ";
	    std::cin >> selected_option;
	    switch (selected_option) {
	    	case Options::OptEncode: {
	            std::wstring encoded = encode();
	            std::wcout << "encoded text: " << encoded << std::endl;
	            break;
	        }
	    	case Options::OptDecode: {
	            std::wstring decoded = decode();
	            std::wcout << "decoded text: " << decoded << std::endl;
	            break;
	        }
	        case Options::OptExit:
            	running = false;
	            break;
	        default:
	            std::cout << "invalid option" << std::endl;
	            break;
	    }
    }
    return 0;
}