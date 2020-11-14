#include <iostream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <wait.h>
#include <map>

const size_t MAX_ARGS_AMOUNT = 100;
const size_t MAX_ENV_VARS_AMOUNT = 100;
std::string userName = "user"; 
bool running = true;
std::map<std::string, std::string> env_vars;

std::vector<std::string> split(const std::string& s, char delimiter){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

void execute(char* argv[]);

char* strToChar(std::string& s){
	return const_cast<char*>(s.data());
}

void parseCommandLine(const std::string& s) {
	if(s.size() == 0){
		return;
	}

	auto tokens = split(s, ' ');

	char *argv[MAX_ARGS_AMOUNT];
	for (int i = 0 ; i < tokens.size() ; i++) {
		argv[i] = strToChar(tokens[i]);
	}
	argv[tokens.size()] = nullptr;

	execute(argv);
}

void closeShell() {
	running = false;
}

std::vector<std::string> getEnvVars(){
	std::vector<std::string> ans(env_vars.size());
	int pos = 0;
	for (auto it : env_vars) {
		ans[pos] = (it.first + "=" + it.second);
		pos++;
	}
	return ans;
}

void execute(char* argv[]){
	switch(pid_t pid = fork()){
		case -1:
			//error
			std::cout << "cant't fork\n";
			break;
		case 0:{
			//child
			auto env_vars_vec = getEnvVars();
			char* env[MAX_ENV_VARS_AMOUNT];
			
			for (size_t i = 0; i < env_vars_vec.size(); i++) {
				env[i] = strToChar(env_vars_vec[i]);
			}
			env[env_vars_vec.size()] = nullptr;
			
			if (execve(argv[0], argv, env) == -1) {
				std::cout << "can't execute";
				exit(-1);
			}
			break;
		}
		default:
			//parent
			int status;
			if (waitpid(pid, &status, 0) == -1) {
				std::cout << "can't execute\n";
			} else {
				std::cout << "\nreturn code: " << WEXITSTATUS(status) << '\n';
			}
			break;
	}
}

void printShit(){
	std::cout << userName + "@kidshell: ";
}

void showEnvVars(){
	for (auto it : env_vars) {
		std::cout << it.first << " = " << it.second << '\n';
	}
}

void showHelp(){
	std::cout << "----------------------------------------\n"
	             "Simple and useless shell:\n"
	             "Usage <flag> | <program name> <arguments ...>\n"
	             "flags:\n"
	             ":q - quit \n"
	             ":h - help \n"
	             ":set <var_name> <var_value> - set environment var \n"
	             ":set - show all environment variables\n"
	             ":unset <var name> - unset environment var \n"
	             ":unsetAll - unset all environment vars \n"
	             ":userName <userName> - change user name (this option is completely useless and meaningless)\n"
	             "----------------------------------------\n";

}

void parseShellCommand(const std::string& s) {
	if (std::cin.eof()) {
		std::cout << '\n';
		closeShell();
	}

	if (s[0] == ':') {
		if (s == ":q") {
			closeShell();
		}
		if (s == ":h"){
			showHelp();
		} 
		auto tokens = split(s, ' ');
		if (tokens[0] == ":userName") {
			if (tokens.size() == 2) {
				userName = tokens[1];
			}
		}
		if (tokens[0] == ":set") {
			switch (tokens.size()) {
				case 1:
					showEnvVars();
					break;
				case 2:
					env_vars[tokens[1]] = "";
					break;
				case 3:
					env_vars[tokens[1]] = tokens[2];
					break;
			}
		}
		if (tokens[0] == ":unset" && tokens.size() == 2) {
				env_vars.erase(tokens[1]);
		}
		if(tokens[0] == ":unsetAll"){
			env_vars.clear();
		}
	} else {
		parseCommandLine(s);
	}
}

int main(){
	showHelp();
	while(running) {
		printShit();
		std::string commandLine;
		std::getline(std::cin, commandLine, '\n');
		parseShellCommand(commandLine);
	}
}