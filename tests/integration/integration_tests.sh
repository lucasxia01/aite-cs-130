#!/bin/bash
echo "STARTING INTEGRATION TESTS"
mkdir test_output
# Running the server with a static config file in the background
../../build/bin/server ../config_parser/basic_config 2>test_output/integration.log &
# Storing the pid so we can kill the process later
server_proc=$(pgrep server)
server=$(pgrep server)
##### TEST 1: curl GET request #####
# Sending a request to the server and storing the response
echo "STARTING TEST 1"
curl localhost:8080/echo -o test_output/temp_response.txt -s -S
# Diffing the response file with the correct response for test 1
diff test_output/temp_response.txt correct_integration_test_1.txt

# Getting the exit code to check if the diff passed
passed=$?

# Checking the exit code and exiting if necessary
if [ $passed -ne 0 ]; then
    echo "FAILED TEST 1"
    rm test_output/temp_response.txt
    kill -9 $server_proc
    kill -9 $server
    rm -rf test_output
    exit 1
fi
echo "PASSED TEST 1"
##### TEST 2: Random nc that fails #####
# Specifying the test string that will be echoed
echo "STARTING TEST 2"
test_string="please fail"

# Getting the response and diffing it
echo "$test_string" | nc -q 1 localhost 8080 >test_output/temp_response.txt
diff test_output/temp_response.txt correct_integration_test_2.txt

passed=$?

if [ $passed -ne 0 ]; then
    echo "FAILED TEST 2"
    rm test_output/temp_response.txt
    kill -9 $server_proc
    kill -9 $server
    rm -rf test_output
    exit 1
fi
echo "PASSED TEST 2"

##### TEST 3: Random nc that should pass #####
echo "STARTING TEST 3"
# Creating the expected response
response_headers_prefix="HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: "
# Specifying the test string that will be echoed
test_string_len=$(cat valid_request.txt | wc -c)
printf "$response_headers_prefix$test_string_len\n\n" >correct_integration_test_3.txt
cat valid_request.txt >>correct_integration_test_3.txt

# Getting the response and diffing it
cat valid_request.txt | nc -q 1 localhost 8080 >test_output/temp_response.txt

diff -b test_output/temp_response.txt correct_integration_test_3.txt

passed=$?

if [ $passed -ne 0 ]; then
    echo "FAILED TEST 3"
    rm test_output/temp_response.txt
    kill -9 $server_proc
    kill -9 $server
    rm -rf test_output
    exit 1
fi
echo "PASSED TEST 3"

echo "TESTING STATIC FILES"
mkdir static_test_output && cd static_test_output
curl -v localhost:8080/static/tests/static_test_files/static_image.png --output png.png
curl -v localhost:8080/static/tests/static_test_files/static_index.html --output html.html
curl -v localhost:8080/static/tests/static_test_files/static_image.jpeg --output jpeg.jpeg
curl -v localhost:8080/static/tests/static_test_files/static_compressed.zip --output zip.zip
curl -v localhost:8080/static/tests/static_test_files/static_file.txt --output txt.txt
png_exists=$(ls | grep "png.png" | wc -l)
html_exists=$(ls | grep "html.html" | wc -l)
jpeg_exists=$(ls | grep "jpeg.jpeg" | wc -l)
zip_exists=$(ls | grep "zip.zip" | wc -l)
txt_exists=$(ls | grep "txt.txt" | wc -l)
cd .. && rm -rf static_test_output
if [[ $png_exists -eq 0 || $html_exists -eq 0 || $jpeg_exists -eq 0 || $zip_exists -eq 0 || $txt_exists -eq 0 ]]; then
    echo "Invalid static file request"
    kill -9 $server_proc
    kill -9 $server
    rm -rf test_output
    exit 1
fi
echo "PASSED STATIC FILES TEST"
##### Cleanup #####
# Killing the process with the stored process ID
kill -9 $server
rm test_output/temp_response.txt

info_logs_count=$(cat test_output/integration.log | grep "\[info\]" | wc -l)
debug_logs_count=$(cat test_output/integration.log | grep "\[debug\]" | wc -l)

# Remove all generated output
rm -rf test_output
rm -rf logs

if [[ $info_logs_count -eq 0 || $debug_logs_count -eq 0 ]]; then
    echo "Invalid log count"
    exit 1
fi

echo "Passed all integration tests"
exit 0
