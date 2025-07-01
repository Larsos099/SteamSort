#include "process.hpp"

Process::Process() {};

void Process::Exec(const std::string &cmd) {
        std::cout << "Executing following command: " << cmd << std::endl;
        FILE* pipe = popen(cmd.c_str(), "r");
        if(!pipe){
            throw std::system_error(std::make_error_code(std::errc::permission_denied));
        }
        
        std::ostringstream cmdOut;

        std::cout << "Reading command output..." << std::endl;
        int c;
        while ((c = fgetc(pipe)) != EOF) {
            cmdOut << static_cast<char>(c);
        }
        std::cout << "Finished reading command output" << std::endl;
        if((pclose(pipe))){
            throw std::system_error(std::make_error_code(std::errc::permission_denied));
        }
        output = cmdOut.str();
        return;

}
