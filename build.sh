rm -rf build/
mkdir build/
cd build/
echo "### Starting CMake... ###"
cmake ..
echo "### CMake OK. ###"
echo "### Starting make... ###"
make > qq
echo "### make OK. ###"
echo "### executing regular ###"
./agc_test --if agc_test.raw --of agc_test_float.res.raw --repeats 100 -v 1
echo "### executing neon ###"
./agc_test_neon --if agc_test.raw --of agc_test_float.res.raw --repeats 100 -v 0