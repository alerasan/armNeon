cd build
echo "### executing regular ###"
./agc_test --if agc_test.raw --of agc_test_float.res.raw --repeats 100 -v 0
echo "### executing neon ###"
./agc_test_neon --if agc_test.raw --of agc_test_float.res.raw --repeats 100 -v 1