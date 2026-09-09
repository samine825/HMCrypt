#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <format>
#include <string_view>
#include <clocale>


// РАНДОМ
using u64 = unsigned long long;
using u32 = unsigned int;
class Random {
    private: 
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
    public:
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
};

#include <filesystem>
#include <fstream>
class FileAccess {
    private: 
        // инкапсуляция по ооп
        std::wstring path;
        std::wfstream file;

    public:
        enum OpenMode {
            Read,
            Write,
            ReadBin,
            WriteBin
        };
        // деструктор
        ~FileAccess() {
            close();
        }

        int open(std::wstring hPath, OpenMode mode) {
            path = hPath;
            file.open(std::filesystem::path(path), 
                (OpenMode::Read == mode) ? std::ios::in : 
                (OpenMode::Write == mode) ? std::ios::out | std::ios::trunc :
                (OpenMode::ReadBin == mode) ? std::ios::in | std::ios::binary :
                std::ios::out | std::ios::binary | std::ios::trunc
            );
            if (!file.is_open()) {
                return 1;
            }
            return 0;
        }
        void close() {
            file.close();
        }
        void write(std::wstring data) {
            file << data;
            file.flush(); 
        }
        std::wstring read() {
            std::wstringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
};

class Coder {
    private: 
        std::wstring text;
        std::vector<std::wstring> abcs;

    public:
        Coder(std::wstring hText, std::vector<u64> hSeeds) {
            abcs = generate_alphabets(hSeeds);
            text = hText;
        }
        //работа с генерируемыми алфавитами
        std::wstring randomize_alphabet(u64 &seed, std::wstring &abc) {
            std::vector<wchar_t> arr(abc.begin(), abc.end());
            Random r;
            r.shuffle_wchar(seed, arr);
            std::wstring result(arr.begin(), arr.end());
            return result;
        }
        std::vector<std::wstring> generate_alphabets(std::vector<u64> seeds) {
            int lenseeds = seeds.size();
            
            std::wstring abc = L"\nabcdefghijklmnopqrstuvwxyzабвгдежзийклмнопрстуфхцчшщьыъэюя1234567890 !?,.-äüöß<>()'\\/\"*:;[]#€₽@_~=";

            std::vector<std::wstring> abcs(lenseeds);
            for (int i = 0; i<lenseeds; i++) {
                abcs[i] = randomize_alphabet(seeds[i], abc);
            }
            return abcs;
        }

        //кодеры
        std::wstring encode() {
            std::vector<wchar_t> arr(text.begin(), text.end());
            std::wstring result = L"";
            for (size_t i = 0; i<arr.size(); i++) {
                result += std::format(L"{:02}", abcs[i % abcs.size()].find(arr[i]));
            }
            return result;
        }
        std::wstring decode() {
            std::vector<wchar_t> arr(text.begin(), text.end());
            std::wstring result = L"";
            std::wstring buffer = L"";
            for (size_t i = 0; i<arr.size(); i++) {
                buffer += text[i];
                if (i%2) {
                    result += abcs[(i/2) % abcs.size()][std::stoul(buffer)];
                    buffer = L"";
                }
            }
            return result;
        }
};

// запросы ввода
std::vector<u64> seeds_req() {
	std::wstring raw_seeds;
	std::wcout << L"enter seeds: ";
	std::getline(std::wcin, raw_seeds);
	
	std::vector<u64> seeds;
    
    std::wstringstream ss(raw_seeds);
    u64 temporary_seed;
    
    while (ss >> temporary_seed) {
        seeds.push_back(temporary_seed);
    }

	return seeds;
}
std::wstring text_req() {
    std::wstring text;
    std::wcout << L"enter text: ";
	//std::wcin.ignore();
    std::getline(std::wcin, text);
    return text;
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

    #ifndef VERSION_STR
    #define VERSION_STR "dev (unversioned build)"
    #endif

	std::setlocale(LC_ALL, "");
    std::wcout << std::format(L"HMcrypt {}\n", VERSION_STR/*при сборке, симейк ставит эту переменную*/) << std::endl;
    
    

    std::vector<std::wstring> wargs;

    // Конвертируем каждый аргумент через std::filesystem::path
    for (int i = 1; i < argc; ++i) {
        wargs.push_back(std::filesystem::path(argv[i]).wstring());
    }

    int opt = -1;
    bool cycle = false;

    std::string_view seed_arg = "--seed";
    std::string_view crt_arg = "--crt";
    std::string_view text_arg = "--text";
    std::string_view input_arg = "--input";
    std::string_view output_arg = "--output";
    std::string_view count_arg = "--count";

    std::string_view cycle_arg = "--cycle";

    int check_seed = check(argc, argv, seed_arg);
    int check_crt = check(argc, argv, crt_arg);
    int check_text = check(argc, argv, text_arg);
    int check_input = check(argc, argv, input_arg);
    int check_output = check(argc, argv, output_arg);
    int check_count = check(argc, argv, count_arg);

    cycle = check(argc, argv, cycle_arg);

    try {
        // проверка на аргумент режима
        if (argc > 1) {
            std::string arg1 = argv[1]; 
            if (arg1 == "encode") {
                opt = Options::OptEncode;
            }
            else if (arg1 == "decode") {
                opt = Options::OptDecode;
            }
            else if (arg1 == "gencrt") {
                opt = Options::OptGenCrt;
            } 
            else if (arg1 == "--help" || arg1 == "-h") {
                std::wcout << "help menu" << std::endl;
                return 0;
            }


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

            


            if (check_count) {
                // количество сидов сертификата
            }
            else {
                // базовое число, к примеру 1000
            }
        }
        
        // основной цикл
        int selected_option = -1;
        do {
            if (opt < 0) {
                std::wcout << L"0. encode text\n1. decode text\n2. generate certificate\n3. exit" << std::endl;
                std::wcout << L"option: ";
                std::wstring temp_option;
                std::getline(std::wcin, temp_option);
                selected_option = std::stoi(temp_option);
            }
            else {
                selected_option = opt;
            }


            if (selected_option == Options::OptEncode || selected_option == Options::OptDecode) {
                FileAccess file;
                
                // получение сидов
                std::vector<u64> seeds;
                if (check_crt) {
                    std::wstring crt_path = wargs[check_crt];
                    std::wcout << L"типо открытие файла сертификата" << std::endl;
                    seeds = {714, 825, 285};
                    // seeds = read_file(crt_path) -- псевдокод
                }
                else if (check_seed) {
                    std::wstringstream ss(wargs[check_seed]);
                    u64 temporary_seed;

                    while (ss >> temporary_seed) {
                        seeds.push_back(temporary_seed);
                    }
                }
                else {
                    seeds = seeds_req();
                }
                // получение текста
                std::wstring text;
                if (check_input) {
                    std::wstring input_path = wargs[check_input];
                    std::wcout << L"открытие файла инпута: " << input_path << std::endl;
                    text = L"вирко";
                    
                    file.open(input_path, FileAccess::OpenMode::Read);
                    text = (selected_option == Options::OptEncode) ? file.read() : file.read();
                    //                                             ^ bytes
                    file.close();
                }
                else if (check_text) {
                    text = wargs[check_text];
                }
                else {
                    text = text_req();
                }

                // кодирование текста
                Coder coder(text, seeds);
                std::wstring result = (selected_option == Options::OptEncode) ? coder.encode() : coder.decode();

                // вывод
                if (check_output) {
                    std::wstring output_path = wargs[check_output];
                    std::wcout << L"запись файла: " << output_path << std::endl;

                    file.open(output_path, FileAccess::OpenMode::Write);
                    (selected_option == Options::OptEncode) ? file.write(result) : file.write(result);
                    //                                                                        ^ bytes
                    file.close();
                }
                else {
                    std::wcout << result << std::endl;
                }
            }
            else if (selected_option == Options::OptGenCrt) {
                std::wcout << L"not implemented" << std::endl;
                cycle = false;
                return 0;
            }
            else if (selected_option == Options::OptExit) {
                cycle = false;
                return 0;
            }
            else {
                cycle = false;
                std::wcout << L"invalid option" << std::endl;
            }
        } while (cycle);
    }
    catch (const std::invalid_argument& e) { 
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}

//source aibrine-venv/bin/ctivate && python aibrine.py
// qs -p /home/samine/.config/quickshell/825UI/Shell.qml
/*
TODO: сделать обработку нечетных вводов (декод)
сделать обработку неизвестных символов (енкод)
сделать обработку неизвестных номеров (декод)

hmcrypt encode  --seed "123 12"          --text "hello world"
hmcrypt encode  --crt  cert.crt          --input  data.bin   --output data.bin.enc
hmcrypt encode  --crt  cert.crt          --input  notes.txt  --output notes.txt.enc
hmcrypt decode  --seed "123 12"          --text "0a1b2c3d"
hmcrypt decode  --crt  cert.crt          --input  data.bin.enc

hmcrypt gencrt  --count 1000             [--range 0-999]     [--output cert.crt]


*/