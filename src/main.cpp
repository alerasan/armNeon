#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "argparse/argparse.hpp"
#include "KwsAgc/KwsAgc.hpp"

void parseInt16DataFile(std::string filename, int16_t** out_data, uint32_t* out_len)
{
    FILE* fp;
    std::cout << "reading: " << filename << std::endl;
    fp = fopen(filename.c_str(), "rb");

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        *out_len = ftell(fp) / sizeof(int16_t);
        std::cout << "file size (samples): " << *out_len << std::endl;
        fseek(fp, 0, SEEK_SET);
        *out_data = (int16_t*)malloc(*out_len * sizeof(int16_t));
        if (*out_data)
        {
            fread(*out_data, sizeof(int16_t), *out_len, fp);
            std::cout << "read file - OK" << std::endl;
        }
        else
        {
            std::cout << "cannot allocate memory, quitting" << std::endl;
        }
        fclose(fp);
    }
    else
    {
        std::cout << "cannot open output file" << std::endl;
    }
}
std::array<std::string, 2> parse_arguments(int argc, char *argv[])
{
  argparse::ArgumentParser program("program_name");

  program.add_argument("--if")
  .help("specify input file name (basic directory is ../data)");
  
  program.add_argument("--of")
  .help("specify output file name (basic directory is ../data)");
  
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

  for(uint32_t it = 0; it < outp_len; it++)
  {
    if(outp_data[it] != golden_data[it])
    {
      if (abs(outp_data[it] - golden_data[it]) > 1.1f)
      {
        std::cout << "iteration: " << it << " expected: " << golden_data[it] << " received: " << outp_data[it]<<std::endl;
      }
      else
      {
        std::cout << "quant diff: " << abs(outp_data[it] - golden_data[it]) << std::endl;
      }
      return false;
    }
  }
  return true;
}
int main(int argc, char *argv[]) {

  std::array<std::string, 2> filepaths;
  filepaths = parse_arguments(argc, argv);
  std::cout << "input file is: " << filepaths[0] << std::endl;
  std::cout << "output file is: " << filepaths[1] << std::endl;
  std::string filepath = "../data/";
  int16_t* inp_data = nullptr;
  int16_t* golden_data = nullptr;
  int16_t* outp_data = nullptr;
  uint32_t inp_len;
  uint32_t golden_len;
  uint32_t outp_len;

  parseInt16DataFile(filepath + filepaths[0], &inp_data, &inp_len);
  parseInt16DataFile(filepath + filepaths[1], &golden_data, &golden_len);

  KwsAgc agc{ new KwsAgcParams() };

  agc.Call(inp_data, inp_len, &outp_data, &outp_len);

  //printOutput(outp_data, outp_len);
  
  std::cout <<  (assertOutput(outp_data, outp_len, golden_data, golden_len) ? "PASSED": "FAILED") << std::endl;
  
  free(inp_data);
  free(outp_data);
  free(golden_data);
  return 0;
}