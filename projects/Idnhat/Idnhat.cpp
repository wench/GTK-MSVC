// Idnhat.cpp : I do not have Auto tools 
// Parses configure.ac for macro and variable definitions and preprocess .in 
// files. I wrote this so I could compile GTK2 in Visual Studio preserving the 
// correct version info without needing to use Mingw to run autogen.sh. 
// As such the functionality is really only what was needed to preprocess the 
// few win32 files in the GTK2 source that needed to be preprocessed. It probably
// wont well well for other projects
// 
// Copyright (c) 2026 Alexis Ryan
// 
// Distributed under the terms of the GNU General Public license 2.0
// 
// To compile only C++/20 is needed

#include <exception>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <span>

std::unordered_map<std::string, std::string> defines;
std::string toLower(std::string in) {
    for (char& c : in) {
        c = std::tolower(c);
    }
    return in;
}
std::string m4_eval(std::string in) {
    std::string copy = in;
    in = in.substr(7);
    if (in.starts_with('('))in = in.substr(1);
    if (in.ends_with(')')) in.resize(in.size()-1);
    std::regex re("(\\d*)\\s*(\\S*)\\s*");
    long result=0;
    

    std::string::size_type pos = 0, next = 0;

    for (auto& p : defines) {
        std::string::size_type pos = 0;

        while ((pos = toLower(in).find(p.first, 0)) != std::string::npos) {
            std::string value = p.second;
            if (value.starts_with("m4_eval"))value = m4_eval(value);
            in.replace(pos, p.first.size(), " " + value + " ");
        }
    }
    std::list<std::string> tokens;
    std::smatch m;
    tokens.emplace_back("+");
    while (!in.empty() && std::regex_search(in, m, re))
    {

        if(m.size()>1 && m[1].length())
        tokens.push_back(m[1]);
        if(m.size()>2 && m[2].length())
            tokens.push_back(m[2]);
        in = m.suffix();
    }
    

    while (tokens.size()>=2) {
             auto token = tokens.front();
             tokens.pop_front();
             long value = std::atoi(tokens.front().c_str());
             tokens.pop_front();

        if (token == "*") result *=value;
        else if (token == "+") result +=value;
        else if (token == "-") result -=value;
        else if (token == "/") result /= value;
        else if (token == "^") result = result^value;
        else if (token == "%") result = result%value;
        else {
            static char msg[] = "Unknown Operator ?";
            msg[17] = token[0];
            throw std::exception(msg);
        }
        
        
    }
    
        return std::to_string(result);
}


void ReadConfigure(std::string path) {
    std::cout << "Reading configure.ac from: \"" << path << "\""<<std::endl;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::exception("Falied to open configure.ac");
        }

    std::string prev_line = {}, line;
    std::regex re("\\s*m4_define\\(\\[(.*)\\],\\s*\\[(.*)\\]\\)\\s*");
    std::regex re2("(\\w*)=(.*?)");

    while (file.good()) {
        std::getline(file, line);
        std::smatch m;
        if (!prev_line.empty()) {
            line = prev_line + "\n" + line;
        }
        if (std::regex_match(line, m, re))
        {
            prev_line.clear();

            std::string name = m[1];
            std::string value = m[2];
            if (value.starts_with("m4_eval"))
            {
                value = m4_eval(value);
            }
            defines[toLower(name)] = value;
            //std::cout << "Found" << std::endl;
            //}
            //else {
            //  std::cout << "Ignoring" << std::endl;
            //}
            //std::cout << name << " = " << value << std::endl;
        }
        else if (std::regex_match(line, m, re2))
        {
            prev_line.clear();

            std::string name = toLower(m[1]);
            std::string value = m[2];

            if (name != value) {
                if (value.starts_with("m4_eval"))
                {
                    value = m4_eval(value);
                }
                defines[name] = value;
                //std::cout << "Found" << std::endl;
            }
            else {
                //std::cout << "Ignoring" << std::endl;
            }

            //std::cout << m[1] << " = " << value << std::endl;
        }
        
        else if (line.starts_with("m4_define")) {
            prev_line = line;
        }
    }

}
void PreprocessFile(std::string path) {
    std::cout << "Processing file: \"" << path << "\""<<std::endl;
    if (!path.ends_with(".in")) {
        throw std::exception("File does not end with .in");
    }
    std::ifstream ifile(path);
    if (!ifile.is_open()) {
        throw std::exception("Falied to open file");
    }
    std::string opath = path.substr(0,path.size() - 3);
    std::cout << "Writing to: \"" << opath << "\""<<std::endl;
    std::ofstream ofile(opath);
    if (!ofile.is_open()) {
        throw std::exception("Failed to open file for writing");
    }
    std::string line;
    std::regex re("^\\s*m4_define\\(\\[(.*)\\],\\s* \\[(.*)\\]\\)\\s*$");

    while (ifile.good()) {
        std::getline(ifile, line);

        for (auto &p : defines) {
                std::string::size_type pos = 0;
               
                while ((pos = toLower(line).find("@"+p.first+"@", 0)) != std::string::npos) {
                    line.replace(pos, p.first.size()+2, p.second);
                }

            }


        ofile << line << std::endl;
    }

}

int main(int argc, char* argv[])
{
    std::cout << "I do not have Auto tools!" <<std::endl;

    if (argc < 2)
    {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "idnhat [configue.ac] <list of in files to process>" << std::endl;
        return -1;
    }
    try{
        ReadConfigure(argv[1]);

        std::regex re("(\\W*)(\\w*)(\\W*)");
        std::vector<std::string> tokens;

        for (auto& p : defines) {
            std::smatch m;
            
            std::string tosplit = p.second;

            tokens.clear();
            while (!tosplit.empty()&& std::regex_search(tosplit, m, re)) {
           
                tokens.push_back(m[1]);
                tokens.push_back(m[2]);
                tokens.push_back(m[3]);
                tosplit = m.suffix();
        }
            bool changed = false;
            std::string value = {};
            for (auto& t:tokens) {
                if (t.size()) {
                    auto it = defines.find(toLower(t));
                    if (it != defines.end()) {
                        changed = true;
                        t = it->second;
                    }
                }
                value += t;
            }
            p.second = value;
        }
        defines["srcdir"] = ".";
        for (int i = 2; i < argc;i++) {
            PreprocessFile(argv[i]);

        }

        
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -2;
    }
    return 0;
}
