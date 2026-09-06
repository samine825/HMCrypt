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
std::wstring encode(std::wstring text,std::vector<std::wstring> abcs) {
	std::vector<wchar_t> arr(text.begin(), text.end());
	std::wstring result = L"";
	for (size_t i = 0; i<arr.size(); i++) {
		result += std::format(L"{:02}", abcs[i % abcs.size()].find(arr[i]));
    }
    return result;
}
std::wstring decode(std::wstring text,std::vector<std::wstring> abcs) {
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

int check(int argc, char* argv[], std::string_view target) {
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == target) {
            return i;
        }
    }
    return 0;
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
    
    int selected_option = -1;
    try {
        // проверка на аргумент режима
        if (argc > 1) {
            std::string arg1 = argv[1]; 
            if (arg1 == "encode") {
                selected_option = Options::OptEncode;
            }
            else if (arg1 == "decode") {
                selected_option = Options::OptDecode;
            }
            else if (arg1 == "gencrt") {
                selected_option = Options::OptGenCrt;
            } 
            else if (arg1 == "--help" || arg1 == "-h") {
                std::wcout << "help menu" << std::endl;
                return 0;
            }
            else {
                throw std::invalid_argument("invalid first argument"); 
            }
        
            std::string_view seed = "--seed";
            std::string_view crt = "--crt";
            std::string_view text = "--text";
            std::string_view input = "--input";
            std::string_view output = "--output";
            std::string_view count = "--count";

            int check_seed = check(argc, argv, seed);
            int check_crt = check(argc, argv, crt);
            int check_text = check(argc, argv, text);
            int check_input = check(argc, argv, input);
            int check_output = check(argc, argv, output);
            int check_count = check(argc, argv, count);

            if ((check_seed && check_crt) || (check_text && check_input) || (arg1 == "gencrt" && (check_crt || check_text || check_seed || check_input)) || ((arg1 == "decode" || arg1 == "encode") && check_count)) {
                throw std::invalid_argument("invalid arguments"); 
            }

            
            // сиды
            if (check_crt) {
                // сиды из сертификата
            }
            else if (check_seed) {
                // сиды текстом
            }
            else {
                //cli список сидов
            }

            // ввод
            if (check_text) {
                // текст
            }
            else if (check_input) {
                // файл
            }
            else {
                // cli ввод текста
            }

            // вывод (для всех режимов)
            if (check_output) {
                // вывод в файл
            }
            else {
                // вывод к консоль
            }


            if (check_count) {
                // количество сидов сертификата
            }
            else {
                // базовое число, к примеру 1000
            }
        }
        
        //source aibrine-venv/bin/ctivate && python aibrine.py
        // qs -p /home/samine/.config/quickshell/825UI/Shell.qml
        
        // основной цикл
        bool running = true;
        while (running) {
            if (argc == 1) {
                std::wcout << L"0. encode text\n1. decode text\n2. generate certificate\n3. exit" << std::endl;
                std::wcout << L"option: ";
                std::wstring temp_option;
                std::getline(std::wcin, temp_option);
                selected_option = std::stoi(temp_option);
            }
            switch (selected_option) {
                case Options::OptEncode: {
                    std::vector<std::wstring> abcs = seeds_req();
                    std::wstring text = text_req();
                    std::wstring encoded = encode(text, abcs);
                    std::wcout << L"encoded text: " << encoded << std::endl;
                    break;
                }
                case Options::OptDecode: {
                    std::vector<std::wstring> abcs = seeds_req();
                    std::wstring text = text_req();
                    std::wstring decoded = decode(text, abcs);
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
    }
    catch (const std::invalid_argument& e) { 
        std::cerr << "Error: " << e.what() << "\n";
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
      |
   ---------
  seed    crt
   |       |
"123 12" "f.825crt"
   ---------
      |                       |
   ------------------         |
 text   textfile   file   "f.825crt"
   |      |         |         |
 "abc"  "f.txt"   "f.any"    1000
   ||      ||         |       |
 "f.txt" "f.txt"  "f.825"    1000



 hmcrypt [encode|decode] [seed|crt] [<seed>] [text|textfile|file] [<input>] [<output>]
	
 hmcrypt gencrt [<seeds count>] [<seeds range>]


hmcrypt encode  --seed "123 12"          --text "hello world"
hmcrypt encode  --crt  cert.crt          --input  data.bin   --output data.bin.enc
hmcrypt encode  --crt  cert.crt          --input  notes.txt  --output notes.txt.enc
hmcrypt decode  --seed "123 12"          --text "0a1b2c3d"
hmcrypt decode  --crt  cert.crt          --input  data.bin.enc

hmcrypt gencrt  --count 1000             [--range 0-999]     [--output cert.crt]


*/