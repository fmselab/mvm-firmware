#define CATCH_CONFIG_MAIN

//using namespace std;

#include "mvm_fw_unit_test_config.h"

//#include "Wire.h"

mvm_fw_unit_test_config    FW_TEST_main_config;

int main(int argc, char **argv) {
	const std::string &json_conf= "prova.json";
	  if (!FW_TEST_main_config.load_config(json_conf))
	   {
	    std::cerr << argv[0] << ":"
	              <<  FW_TEST_main_config.get_error_string()
	              << std::endl;
	    return 2;
	   }

}
