#pragma once

#include <cstring>
#include <cmath>

#include "KwsAgcParams.hpp"
#include "../util/util.hpp"

class KwsAgc
{
public:
    KwsAgcParams* params = nullptr;
    KwsAgc()
    {

    }
    KwsAgc(KwsAgcParams *_params)
    {
        this->params = _params;

    }

    void Call(int16_t * inp_data, uint32_t inp_len, int16_t ** outp_data, uint32_t * outp_len, uint32_t rate=16000)
    {
        float * inp_flp;
        uint32_t inp_flp_len = inp_len;
        *outp_len = inp_len;
        inp_flp = (float*)malloc(inp_len * sizeof(float));
        *outp_data = (int16_t*)malloc(inp_len * sizeof(int16_t));
        if(!inp_flp && !(*outp_data))
        {
            std::cout << "couldn't allocate memory, aborting..." << std::endl;
            return;
        }
        for(uint32_t it = 0; it < inp_len; it++)
        {
            inp_flp[it] = (float)inp_data[it] / (float)INT16_MAX;
        }
        uint32_t win_size = (uint32_t)((float)rate / 1000.0f * this->params->win_size);
        float mu_s = this->params->mu_s;
        float mu_b = this->params->mu_b;
        float sigma_s = this->params->sigma_s;
        float sigma_b = this->params->sigma_b;
        for(uint32_t r = win_size; r < inp_len; r+=win_size)
        {
            uint32_t e = r;
            uint32_t s = r - win_size;
            std::vector<float> w;
            w.resize(win_size);
            for (uint32_t it = 0; it < win_size; it++)
            {
                w[it] = inp_flp[s + it];
            }
            float l = 0;
            for (uint32_t it = 0; it < win_size; it++)
            {
                l = max(abs(w[it]), l);
            }
            float mu_s_n = this->params->k_mu * l + (1 - this->params->k_mu) * mu_s;
            float mu_b_n = this->params->k_mu * l + (1 - this->params->k_mu) * mu_b;
            float sigma_s_n = this->params->k_sigma * (l - mu_s_n) * (l - mu_s_n) + \
                         (1 - this->params->k_sigma) * sigma_s * sigma_s;
            float sigma_b_n = this->params->k_sigma * (l - mu_b_n) * (l - mu_b_n) + \
                         (1 - this->params->k_sigma) * sigma_b * sigma_b;
            
            float z_s = (l - mu_s_n) / std::sqrt(sigma_s_n);
            float z_b = (l - mu_b_n) / std::sqrt(sigma_b_n);


            bool speech = z_s * z_s < z_b * z_b;
            float tau = 0.5 * l;
            float gain = 0;
            if (speech)
            {
                mu_s = mu_s_n;
                if (sigma_s_n < tau * tau)
                {
                    sigma_s_n = sigma_s_n + (sigma_s_n + sigma_b_n) / (2.0f * this->params->delta);
                }
                sigma_s = std::sqrt(sigma_s_n);
                if((mu_s - mu_b) > (sigma_s + sigma_b))
                {
                    gain = this->params->theta / (mu_s + sigma_s);
                }
                else
                {

                    gain = this->params->theta_s / min((mu_s + sigma_s),(mu_b + sigma_b));
                }                            
            }
            else
            {
                mu_b = mu_b_n;
                if (sigma_b_n < tau * tau)
                {
                    sigma_b_n = sigma_b_n + (sigma_b_n + sigma_s_n) / (2 * this->params->delta);
                }
                sigma_b = std::sqrt(sigma_b_n);
                gain = 1;
            }
            for (uint32_t it = s; it < e; it++)
            {
                inp_flp[it] *= gain;
            }
        }

        for(uint32_t it = 0; it < *outp_len; it++)
        {
            (*outp_data)[it] = (int16_t)(inp_flp[it] * (float)INT16_MAX);
        }
        free(inp_flp);
    }
};