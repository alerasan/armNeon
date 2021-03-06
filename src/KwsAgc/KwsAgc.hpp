#pragma once

#include <cstring>
#include <cmath>

#include "arm_neon.h"

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
        float * outp_flp;
        uint32_t inp_flp_len = inp_len;
        *outp_len = inp_len;
        uint32_t win_size = (uint32_t)((float)rate / 1000.0f * this->params->win_size);
        float * w;
        w = (float*)malloc(win_size * sizeof(float));
        outp_flp = (float*)malloc(inp_len * sizeof(float));
        *outp_data = (int16_t*)malloc(inp_len * sizeof(int16_t));
        if(!outp_flp && !(*outp_data) && !w)
        {
            std::cout << "couldn't allocate memory, aborting..." << std::endl;
            return;
        }
#ifdef BUILD_WITH_NEON

        float int16_max_float[4] = {32767.0f, 32767.0f, 32767.0f, 32767.0f};
        uint32_t iter;
        for (iter = 0; iter < (inp_len - 3); iter+=4)
        {
            float tmp[4] = {(float)inp_data[iter], (float)inp_data[iter + 1], (float)inp_data[iter + 2], (float)inp_data[iter + 3]};
            float32x4_t __a;
            float32x4_t __b;
            float32x4_t __r;
            __a = vld1q_f32(tmp);
            __b = vld1q_f32(int16_max_float);
            __r = vdivq_f32(__a, __b);
            vst1q_f32(&outp_flp[iter], __r);
        }
        for (uint32_t it = iter; it < inp_len; it++)
        {
            outp_flp[it] = (float)inp_data[it] / (float )INT16_MAX;
        }
#else
        for(uint32_t it = 0; it < inp_len; it++)
        {
            outp_flp[it] = (float)inp_data[it] / (float )INT16_MAX;
        }
#endif
        float mu_s = this->params->mu_s;
        float mu_b = this->params->mu_b;
        float sigma_s = this->params->sigma_s;
        float sigma_b = this->params->sigma_b;

        

        for(uint32_t r = win_size; r < inp_len; r+=win_size)
        {
            uint32_t e = r;
            uint32_t s = r - win_size;
            for (uint32_t it = 0; it < win_size; it++)
            {
                w[it] = outp_flp[s + it];
            }
            float l = 0;
#ifdef BUILD_WITH_NEON
            float32x4_t __a;
            float32x4_t __b;
            __a = vld1q_f32(&w[0]);
            __a = vabsq_f32(__a);
            for (iter = 4; iter < (win_size - 3); iter+=4)
            {
                __b = vld1q_f32(&w[iter]);                
                __b = vabsq_f32(__b);
                __a = vmaxq_f32(__a, __b);
            }
            float tmp[4] = {0};
            vst1q_f32(tmp, __a);
            for(uint8_t it = 0; it < 4; it ++)
            {
                l = max(tmp[it], l);
            }
            for (uint32_t it = iter; it < win_size; it++)
            {
                l = max(abs(w[it]), l);
            }
#else
            for (uint32_t it = 0; it < win_size; it++)
            {
                l = max(abs(w[it]), l);
            }
#endif
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
#if BUILD_WITH_NEON
            for (iter = s; iter < (e - 15); iter+=16)
            {
                float32x4_t __c;
                float32x4_t __d;
                __a = vld1q_f32(&outp_flp[iter]);
                __b = vld1q_f32(&outp_flp[iter+4]);
                __c = vld1q_f32(&outp_flp[iter+8]);
                __d = vld1q_f32(&outp_flp[iter+12]);
                __a = vmulq_n_f32(__a, gain);
                __b = vmulq_n_f32(__b, gain);
                __c = vmulq_n_f32(__c, gain);
                __d = vmulq_n_f32(__d, gain);
                vst1q_f32(&outp_flp[iter], __a);
                vst1q_f32(&outp_flp[iter+4], __b);
                vst1q_f32(&outp_flp[iter+8], __c);
                vst1q_f32(&outp_flp[iter+12], __d);
            }
            for (uint32_t it = iter; it < e; it++)
            {
                outp_flp[it] *= gain;
            }
#else
            for (uint32_t it = s; it < e; it++)
            {
                outp_flp[it] *= gain;
            }
#endif
        }
#if BUILD_WITH_NEON
        for (iter = 0; iter < (*outp_len - 15); iter+=16)
        {
            float32x4_t __a;
            float32x4_t __b;
            float32x4_t __c;
            float32x4_t __d;
            int32x4_t __ai;
            int32x4_t __bi;
            int32x4_t __ci;
            int32x4_t __di;
            int32_t tmp[16];
            __a = vld1q_f32(&outp_flp[iter]);
            __b = vld1q_f32(&outp_flp[iter+4]);
            __c = vld1q_f32(&outp_flp[iter+8]);
            __d = vld1q_f32(&outp_flp[iter+12]);
            __a = vmulq_n_f32(__a, 32767.0f);
            __b = vmulq_n_f32(__b, 32767.0f);
            __c = vmulq_n_f32(__c, 32767.0f);
            __d = vmulq_n_f32(__d, 32767.0f);
            __ai = vcvtq_s32_f32(__a);
            __bi = vcvtq_s32_f32(__b);
            __ci = vcvtq_s32_f32(__c);
            __di = vcvtq_s32_f32(__d);
            vst1q_s32(&tmp[0], __ai);
            vst1q_s32(&tmp[4], __bi);
            vst1q_s32(&tmp[8], __ci);
            vst1q_s32(&tmp[12], __di);
            for(uint32_t it = 0; it < 16; it++)
            {
                (*outp_data)[iter + it] = (int16_t)tmp[it];
            }
        }
        for (uint32_t it = iter; it < *outp_len; it++)
        {
            (*outp_data)[it] = (int16_t)(outp_flp[it] * (float)INT16_MAX);
        }
#else
        for(uint32_t it = 0; it < *outp_len; it++)
        {
            (*outp_data)[it] = (int16_t)(outp_flp[it] * (float)INT16_MAX);
        }
#endif
        free(outp_flp);
        free(w);
    }
};