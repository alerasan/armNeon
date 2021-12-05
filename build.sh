rm -rf build/
mkdir build/
cd build/
echo "### Starting CMake... ###"
cmake ..
echo "### CMake OK. ###"
echo "### Starting make... ###"
make > qq
echo "### make OK. ###"
echo "### executing program ###"
./agc_test --if agc_test.raw --of agc_test_float.res.raw