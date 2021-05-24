#!/bin/bash
echo "####### STARTING INTEGRATION TESTS #######"
mkdir test_output
mkdir static_test_output
# Running the server with a static config file in the background
../../build/bin/server 5 ../config_parser/basic_config 2>test_output/integration.log &
# Wait for server start up in the background
sleep 1
# Storing the pid so we can kill the process later
server=$(pgrep server)
passed_all_tests=true

#########################################################################################
# test_valid_echo_curl: curl echo request passes if path matches /echo(\/.+)?
#########################################################################################
echo "--------- STARTING test_valid_echo_curl ---------"
curl localhost:8080/echo/anypath -o test_output/temp_response.txt -s -S

cmp -s test_output/temp_response.txt correct_test_valid_echo_curl.txt
result=$?

if [ $result -ne 0 ]; then
    echo "FAILED test_valid_echo_curl"
    passed_all_tests=false
else
    echo "PASSED test_valid_echo_curl"
fi

#########################################################################################
# test_invalid_echo_nc: Random nc that fails
#########################################################################################
echo "--------- STARTING test_invalid_echo_nc ---------"
test_string="please fail"

echo "$test_string" | nc -q 1 localhost 8080 >test_output/temp_response.txt
cmp -s test_output/temp_response.txt correct_bad_request.txt

result=$?

if [ $result -ne 0 ]; then
    echo "FAILED test_invalid_echo_nc"
    passed_all_tests=false
else
    echo "PASSED test_invalid_echo_nc"
fi

#########################################################################################
# test_valid_echo_nc: Random nc that passes
#########################################################################################
echo "--------- STARTING test_valid_echo_nc ---------"
# Creating the expected response
response_headers_prefix="HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: "
# Specifying the test string that will be echoed
test_string_len=$(cat valid_echo_request.txt | wc -c)
printf "$response_headers_prefix$test_string_len\n\n" >correct_test_valid_echo_nc.txt
cat valid_echo_request.txt >>correct_test_valid_echo_nc.txt

cat valid_echo_request.txt | nc -q 1 localhost 8080 >test_output/temp_response.txt

diff -b test_output/temp_response.txt correct_test_valid_echo_nc.txt

result=$?

if [ $result -ne 0 ]; then
    echo "FAILED test_valid_echo_nc"
    passed_all_tests=false
else
    echo "PASSED test_valid_echo_nc"
fi

#########################################################################################
# test_invalid_path_nc: nc with a path not defined in the config should fail
#########################################################################################
echo "--------- STARTING test_invalid_path_nc ---------"

cat invalid_path_nc_request.txt | nc -q 1 localhost 8080 >test_output/temp_response.txt
cmp -s test_output/temp_response.txt correct_bad_request.txt

result=$?

if [ $result -ne 0 ]; then
    echo "FAILED test_invalid_path_nc"
    passed_all_tests=false
else
    echo "PASSED test_invalid_path_nc"
fi

#########################################################################################
# test_invalid_path_curl: curl with a path not defined in the config should fail
#########################################################################################
echo "--------- STARTING test_invalid_path_curl ---------"

curl localhost:8080/invalidpath -o test_output/temp_response.txt -s -S
cmp -s test_output/temp_response.txt <(echo -n '<!DOCTYPE html><html><head><title>Error</title></head><body><h1>Not Found Error</h1><p>Description: Uri path /invalidpath not found</p></body></html>')

result=$?

if [ $result -ne 0 ]; then
    echo "FAILED test_invalid_path_curl"
    passed_all_tests=false
else
    echo "PASSED test_invalid_path_curl"
fi
#########################################################################################
# test_valid_static_files: Static files found in directory should be served
#########################################################################################
echo "--------- STARTING test_valid_static_files ---------"
cd static_test_output
curl localhost:8080/static/tests/static_test_files/static_image.png -o png.png -s -S
curl localhost:8080/static/tests/static_test_files/static_index.html -o html.html -s -S
curl localhost:8080/static/tests/static_test_files/static_image.jpeg -o jpeg.jpeg -s -S
curl localhost:8080/static/tests/static_test_files/static_compressed.zip -o zip.zip -s -S
curl localhost:8080/static/tests/static_test_files/static_file.txt -o txt.txt -s -S

cmp png.png ../../static_test_files/static_image.png
png_cmp=$?
cmp html.html ../../static_test_files/static_index.html
html_cmp=$?
cmp jpeg.jpeg ../../static_test_files/static_image.jpeg
jpeg_cmp=$?
cmp zip.zip ../../static_test_files/static_compressed.zip
zip_cmp=$?
cmp txt.txt ../../static_test_files/static_file.txt
txt_cmp=$?
cd ..

if [[ $png_cmp -ne 0 || $html_cmp -ne 0 || $jpeg_cmp -ne 0 || $zip_cmp -ne 0 || $txt_cmp -ne 0 ]]; then
    echo "FAILED test_valid_static_files"
    passed_all_tests=false
else
    echo "PASSED test_valid_static_files"
fi

#########################################################################################
# test_static_file_not_found_curl: Static files not found in directory should 404
#########################################################################################
echo "--------- STARTING test_static_file_not_found_curl ---------"
cd static_test_output
curl localhost:8080/static/tests/static_test_files/not_bubujingxin.png -o not.png -s -S
cmp -s not.png <(echo -n '<!DOCTYPE html><html><head><title>Error</title></head><body><h1>Not Found Error</h1><p>Description: File /static/tests/static_test_files/not_bubujingxin.png not found</p></body></html>')
result=$?
cd ..
if [ $result -ne 0 ]; then
    echo "FAILED test_static_file_not_found_curl"
    passed_all_tests=false
else
    echo "PASSED test_static_file_not_found_curl"
fi

#########################################################################################
# test_default_not_found: Root should default to not found if unspecified
#########################################################################################
echo "--------- STARTING test_default_not_found ---------"
curl localhost:8080/ -o test_output/temp_response.txt -s -S
cmp -s test_output/temp_response.txt <(echo -n '<!DOCTYPE html><html><head><title>Error</title></head><body><h1>Not Found Error</h1><p>Description: Uri path / not found</p></body></html>')
result=$?
if [ $result -ne 0 ]; then
    echo "FAILED test_default_not_found"
    passed_all_tests=false
else
    echo "PASSED test_default_not_found"
fi

#########################################################################################
# test_status_curl: curl to status 
#########################################################################################
echo "--------- STARTING test_status_curl ---------"
cd test_output
curl localhost:8080/status -o status_output.txt -s -S
grep -q "<b>Total Number of Requests Served : </b>" status_output.txt
result=$?
cd ..
if [ $result -ne 0 ]; then
    echo "FAILED test_status_curl"
    passed_all_tests=false
else
    echo "PASSED test_status_curl"
fi

#########################################################################################
# test_health_curl: curl to status 
#########################################################################################
echo "--------- STARTING test_health_curl ---------"
cd test_output
curl localhost:8080/health -o health.txt -s -S
grep -q "OK" health.txt
result=$?
cd ..
if [ $result -ne 0 ]; then
    echo "FAILED test_health_curl"
    passed_all_tests=false
else
    echo "PASSED test_health_curl"
fi


#########################################################################################
# test_proxy_curl: curl to status 
#########################################################################################
echo "--------- STARTING test_proxy_curl ---------"
cd test_output
curl -vs localhost:8080/nyu >test_proxy_curl_output.txt 2>&1
grep -q "</body>" test_proxy_curl_output.txt
result=$?
cd ..
if [ $result -ne 0 ]; then
    echo "FAILED test_proxy_curl"
    passed_all_tests=false
else
    echo "PASSED test_proxy_curl"
fi

#########################################################################################
# test_proxy_redirect_curl: curl to status 
#########################################################################################
echo "--------- STARTING test_proxy_redirect_curl ---------"
cd test_output
curl -vs localhost:8080/bitly >test_proxy_redirect_curl_output.txt 2>&1
grep -q "Location: https://bitly.com/" test_proxy_redirect_curl_output.txt
result=$?
cd ..
if [ $result -ne 0 ]; then
    echo "FAILED test_proxy_redirect_curl"
    passed_all_tests=false
else
    echo "PASSED test_proxy_redirect_curl"
fi

#########################################################################################
# test_multithreading: sleep request + echo request
#########################################################################################
echo "--------- STARTING test_multithreading ---------"
cd test_output

curl -s -S localhost:8080/sleep >/dev/null &
curl localhost:8080/echo/anypath -o temp_response.txt -s -S
cmp -s temp_response.txt ../correct_test_valid_echo_curl.txt
result=$?
sleep 5

cd ..
if [ $result -ne 0 ]; then
    echo "FAILED test_multithreading"
    passed_all_tests=false
else
    echo "PASSED test_multithreading"
fi


#########################################################################################
# CLEANUP
#########################################################################################
# Killing the process with the stored process ID
kill -9 $server
wait $server 2>/dev/null
info_logs_count=$(cat test_output/integration.log | grep "\[info\]" | wc -l)
debug_logs_count=$(cat test_output/integration.log | grep "\[debug\]" | wc -l)

# Remove all generated output
rm -rf static_test_output
rm -rf test_output
rm -rf logs

if [[ $info_logs_count -eq 0 || $debug_logs_count -eq 0 ]]; then
    echo "Invalid log count"
    passed_all_tests=false
fi

if [ $passed_all_tests = true ]; then
    echo "####### PASSED INTEGRATION TESTS #######"
    exit 0
else
    echo "####### FAILED INTEGRATION TESTS #######"
    exit 1
fi
