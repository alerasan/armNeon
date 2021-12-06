#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <chrono>

#include "argparse/argparse.hpp"
#include "KwsAgc/KwsAgc.hpp"


int verbose = 0;

void parseInt16DataFile(std::string filename, int16_t** out_data, uint32_t* out_len)
{
    FILE* fp;
    if (verbose)
      std::cout << "reading: " << filename << std::endl;
    fp = fopen(filename.c_str(), "rb");

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        *out_len = ftell(fp) / sizeof(int16_t);
        if (verbose)
          std::cout << "file size (samples): " << *out_len << std::endl;
        fseek(fp, 0, SEEK_SET);
        *out_data = (int16_t*)malloc(*out_len * sizeof(int16_t));
        if (*out_data)
        {
            size_t dummy = fread(*out_data, sizeof(int16_t), *out_len, fp);
            if (verbose)
              std::cout << "read file - OK" << std::endl;
        }
        else
        {
          if (verbose)
            std::cout << "cannot allocate memory, quitting" << std::endl;
        }
        fclose(fp);
    }
    else
    {
      if (verbose)
        std::cout << "cannot open output file" << std::endl;
    }
}
std::array<std::string, 2> parse_arguments(int argc, char *argv[], int &repeats)
{
  argparse::ArgumentParser program("program_name");

  program.add_argument("--if")
  .help("specify input file name (basic directory is ../data)");
  
  program.add_argument("--of")
  .help("specify output file name (basic directory is ../data)");
  
  program.add_argument("--repeats")
  .help("specify number of repeats for Call function")
  .scan<'i', int>();

  program.add_argument("-v")
  .help("select 1 for verbose")
  .scan<'i', int>();

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
  
  auto input_file = program.get<std::string>("--if");
  auto output_file = program.get<std::string>("--of");
  repeats = program.get<int>("--repeats");
  verbose = program.get<int>("-v");

  std::array<std::string, 2> ret {input_file, output_file};
  return ret;
}
void printOutput(int16_t *outp_data, uint32_t outp_len)
{
  std::cout << "output data: " << std::endl;

  for(uint32_t it = 0; it < outp_len; it++)
  {
    std::cout << "out["<< it << "] = " << outp_data[it] << std::endl;
  }

}
bool assertOutput(int16_t *outp_data, uint32_t outp_len, int16_t *golden_data, uint32_t golden_len)
{
bool ret = true;
  for(uint32_t it = 0; it < outp_len; it++)
  {
    if(outp_data[it] != golden_data[it])
    {
      if (verbose)
      std::cout << "iteration: " << it << " expected: " << golden_data[it] << " received: " << outp_data[it]<<std::endl;
     // return false;
     ret = false;
    }
  }
  return ret;
}
void printBuildType()
{
#ifdef BUILD_WITH_NEON
  std::cout << "I was built with NEON usage" << std::endl;
#else
  std::cout << "I don't use NEON" << std::endl;
#endif
}
int main(int argc, char *argv[]) {
  using std::chrono::high_resolution_clock;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using std::chrono::milliseconds;
  std::array<std::string, 2> filepaths;
  int repeats;
  filepaths = parse_arguments(argc, argv, repeats);
  if(verbose)
  {
    std::cout << "input file is: " << filepaths[0] << std::endl;
    std::cout << "output file is: " << filepaths[1] << std::endl;
    std::cout << "repeats: " << repeats << std::endl;
  }
  std::string filepath = "../data/";
  int16_t* inp_data = nullptr;
  int16_t* golden_data = nullptr;
  int16_t* outp_data = nullptr;
  uint32_t inp_len;
  uint32_t golden_len;
  uint32_t outp_len;

  printBuildType();

  parseInt16DataFile(filepath + filepaths[0], &inp_data, &inp_len);
  parseInt16DataFile(filepath + filepaths[1], &golden_data, &golden_len);

  KwsAgc agc{ new KwsAgcParams() };

  auto t1 = high_resolution_clock::now();

  for(uint32_t it = 0; it < repeats; it++)
  {
    agc.Call(inp_data, inp_len, &outp_data, &outp_len);
    if (it < (repeats - 1))
      free(outp_data);
  }
  auto t2 = high_resolution_clock::now();
  duration<double, std::milli> ms_double = t2 - t1;

  //printOutput(outp_data, outp_len);
  
  std::cout <<  (assertOutput(outp_data, outp_len, golden_data, golden_len) ? "PASSED": "FAILED") << std::endl;
  
  std::cout <<"time taken for " << repeats << " repeats: "<< ms_double.count() << "ms\n";

  free(inp_data);
  free(outp_data);
  free(golden_data);
  return 0;
}