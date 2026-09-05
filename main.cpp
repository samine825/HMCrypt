#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iomanip>
#include <format>
#include <cstdint>
#include <string_view>
#include <clocale>


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
std::vector<std::wstring> generate_alphabets(std::vector<u64> seeds, int &lenseeds, std::wstring abc, int &lenabc) {
	std::vector<std::wstring> abcs(lenseeds);
    for (int i = 0; i<lenseeds; i++) {
    	abcs[i] = randomize_alphabet(seeds[i], abc);
    	//std::wcout << randomize_alphabet(seeds[i], abc) << std::endl;
    }
    return abcs;
}



// запросы ввода
std::vector<std::wstring> seeds_req() {
	std::wstring raw_seeds;
	std::wcout << L"enter seeds: ";
	std::wcin.ignore();
	std::getline(std::wcin, raw_seeds);
	
	std::vector<u64> seeds;
    
    std::wstringstream ss(raw_seeds);
    u64 temporary_seed;
    
    while (ss >> temporary_seed) {
        seeds.push_back(temporary_seed);
    }

	int lenseeds = seeds.size();
	
	std::wstring mainabc = L"\nabcdefghijklmnopqrstuvwxyzабвгдежзийклмнопрстуфхцчшщьыъэюя1234567890 !?,.-äüöß<>()'\\/\"*:;[]#€₽@_~=";
	int lenmainabc = mainabc.size();
	
	std::vector<std::wstring> abcs = generate_alphabets(seeds, lenseeds, mainabc, lenmainabc);
	return abcs;
}
std::wstring text_req() {
    std::wstring text;
    std::wcout << L"enter text: ";
	//std::wcin.ignore();
    std::getline(std::wcin, text);
    return text;
}



//кодеры
std::wstring encode() {
	std::vector<std::wstring> abcs = seeds_req();
	std::wstring text = text_req();
	
	std::vector<wchar_t> arr(text.begin(), text.end());
	std::wstring result = L"";
	for (size_t i = 0; i<arr.size(); i++) {
		result += std::format(L"{:02}", abcs[i % abcs.size()].find(arr[i]));
    }
    return result;
}
std::wstring decode() {
	std::vector<std::wstring> abcs = seeds_req();
	std::wstring text = text_req();
	
	std::vector<wchar_t> arr(text.begin(), text.end());
	std::wstring result = L"";
	std::wstring buffer = L"";
	for (size_t i = 0; i<arr.size(); i++) {
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
	OptGenCrt,
	OptExit
};

int main(int argc, char* argv[]) {
	std::setlocale(LC_ALL, "");
    std::wcout << std::format(L"HMcrypt {}\n", VERSION_STR/*при сборке, симейк ставит эту переменную*/) << std::endl;
    
    
    std::string arg1 = argv[1]; 
    //std::string arg1 = argv[1]; 
    //std::string arg1 = argv[1]; 
    
    int selected_option = -1;
    if (arg1 == "encode") {
    	selected_option = Options::OptEncode;
    }
    else if (arg1 == "decode") {
    	selected_option = Options::OptDecode;
    }
    else if (arg1 == "gencrt") {
    	selected_option = Options::OptGenCrt;
    } else {
    	selected_option = Options::OptExit;
    }
    
    
    
    
    // основной цикл
    bool running = true; 
    while (running) {
	    if (selected_option == -1) {
		    std::wcout << L"0. encode text\n1. decode text\n2. generate certificate\n3. exit" << std::endl;
	        std::wcout << L"option: ";
	        std::wcin >> selected_option;
	    }
	    switch (selected_option) {
	    	case Options::OptEncode: {
	            std::wstring encoded = encode();
	            std::wcout << L"encoded text: " << encoded << std::endl;
	            break;
	        }
	    	case Options::OptDecode: {
	            std::wstring decoded = decode();
	            std::wcout << L"decoded text: " << decoded << std::endl;
	            break;
	        }
	        case Options::OptExit:
            	running = false;
	            break;
	        default:
            	running = false;
	            std::wcout << L"invalid option" << std::endl;
	            break;
	    }
    }
    return 0;
}


/*
TODO: сделать обработку нечетных вводов (декод)
сделать обработку неизвестных символов (енкод)
сделать обработку неизвестных номеров (декод)

сделать аргументы

        hmcrypt -------- cli mode
		   |
   ----------------------------
encode  encode              gencrt
   --------                   |
      |                       |
   ------------------         |
 text   textfile   file   "f.825crt"
   |      |         |         |
 "abc"  "f.txt"   "f.txt"    1000
   |      |         |         |
   |    "f.txt"   "f.825"    1000
   ------------------
      |
   ---------
  seed    crt
   |       |
"123 12" "f.825crt"

*/