#!/bin/bash
echo "STARTING INTEGRATION TESTS"
# Running the server with a static config file in the background
../../build/bin/server ../config_parser/basic_config 2> integration.log &
# Storing the pid so we can kill the process later
server_proc=$(pgrep server)
server=$(pgrep server)
##### TEST 1: curl GET request #####
# Sending a request to the server and storing the response
echo "STARTING TEST 1"
curl localhost:8080 -o temp_response.txt -s -S
# Diffing the response file with the correct response for test 1
diff temp_response.txt correct_integration_test_1.txt

# Getting the exit code to check if the diff passed
passed=$?

# Checking the exit code and exiting if necessary
if [ $passed -ne 0 ]
then
    echo "FAILED TEST 1"
    rm temp_response.txt
    kill -9 $server_proc
    kill -9 $server
    exit 1
fi
echo "PASSED TEST 1"
##### TEST 2: Random nc that fails #####
# Specifying the test string that will be echoed
echo "STARTING TEST 2"
test_string="please fail" 

# Getting the response and diffing it
echo "$test_string" | nc -q 1 localhost 8080 > temp_response.txt
diff temp_response.txt correct_integration_test_2.txt

passed=$?

if [ $passed -ne 0 ]
then
    echo "FAILED TEST 2"
    rm temp_response.txt
    kill -9 $server_proc
    kill -9 $server
    exit 1
fi
echo "PASSED TEST 2"

##### TEST 3: Random nc that should pass #####
echo "STARTING TEST 3"
# Creating the expected response
response_headers_prefix="HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: "
# Specifying the test string that will be echoed
test_string_len=`cat valid_request.txt | wc -c`
printf "$response_headers_prefix$test_string_len\n\n" > correct_integration_test_3.txt
cat valid_request.txt >> correct_integration_test_3.txt

# Getting the response and diffing it
cat valid_request.txt | nc -q 1 localhost 8080 > temp_response.txt

diff -b temp_response.txt correct_integration_test_3.txt

passed=$?

if [ $passed -ne 0 ]
then
    echo "FAILED TEST 3"
    rm temp_response.txt
    kill -9 $server_proc
    kill -9 $server
    exit 1
fi
echo "PASSED TEST 3"

# Killing the process with the stored process ID
kill -9 $server
rm temp_response.txt

info_log_count=$(cat integration.log | grep "\[info\]" | wc -l)

# Remove all generated log files
rm *.log

if [ $info_log_count -ne 1 ]
then
    echo "Invalid info log count"
    exit 1
fi

echo "Passed all integration tests"
exit 0