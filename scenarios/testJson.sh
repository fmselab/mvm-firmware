#!/bin/bash

# Check the correctness of the parameters

if [ "$#" -eq  0 ] || [ "$#" -gt 2 ] 
then
	echo "Usage of the command: testJson.sh [--force] <path of the JSON file>"
	echo ""
	echo "use --force to force overwriting of the .out file"
	exit
else
	if [ "$#" -eq 1 ] 
	then
		jsonFile=$1
		force="0"
	else
		jsonFile=$2
		if [ $1  == "--force" ]
		then
			force="1"
		else
			echo "Usage of the command: testJson.sh [--force] <path of the JSON file>"
			echo ""
			echo "use --force to force overwriting of the .out file"
			exit
		fi
	fi
fi

# If the out file does not exist or we need to force the overwriting

outputFile=$(grep "SerialTTY" $jsonFile)
separator=':' 
read -ra ADDR <<< "$outputFile"
outputFile=${ADDR[1]}
outputFile=$(tr -d '"' <<< $outputFile)
outputFile=$(tr -d ',' <<< $outputFile)

if [ ! -f "$outputFile" ]; then
	force="1"
fi

# Test execution

if [ -f "$jsonFile" ]
then

	if [ $force == "1" ]; then
		echo "Executing test over scenario " $jsonFile 

		./../MVMFirmwareUnitTests/mvm_fw_unit_test_run $jsonFile
	fi

	echo "Evaluation of the oracolo"

	ruby ./../MVMFirmwareUnitTests/mvm_fw_test_oracle.rb $jsonFile
else
	echo "File " $jsonFile " does not exist or it is an invalid file"
fi
