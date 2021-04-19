#!/bin/bash

# Running the server with a static config file in the background
../../build/bin/server ../config_parser/basic_config &
# Storing the pid so we can kill the process later
pid=$!

##### TEST 1: curl GET request #####
# Sending a request to the server and storing the response
curl localhost:8080 -v -o temp_response.txt -s -S

# Diffing the response file with the correct response for test 1
diff temp_response.txt correct_integration_test_1.txt

# Getting the exit code to check if the diff passed
passed=$?

# Checking the exit code and exiting if necessary
if [ $passed -ne 0 ]
then
    echo "Failed test 1"
    rm temp_response.txt
    kill -9 $pid
    exit 1
fi

##### TEST 2: Random nc that fails #####
# Specifying the test string that will be echoed
test_string="please fail" 

# Getting the response and diffing it
echo "$test_string" | nc -q 1 localhost 8080 > temp_response.txt
diff temp_response.txt correct_integration_test_2.txt

passed=$?

if [ $passed -ne 0 ]
then
    echo "Failed test 2"
    rm temp_response.txt
    kill -9 $pid
    exit 1
fi

##### TEST 3: Random nc that should pass #####
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
    echo "Failed test 3"
    rm temp_response.txt
    kill -9 $pid
    exit 1
fi

# TODO: better format of test cases


# Killing the process with the stored process ID
kill -9 $pid
rm temp_response.txt

echo "Passed all integration tests"