#pragma once

class KwsAgcParams
{
private:

public:
    float delta = 16;
    float theta = 0.5;
    float theta_s = 0.1;
    float sigma_s = 0.003;
    float mu_s = 0.0025;
    float sigma_b = 0.003;
    float mu_b = 0.0;
    uint32_t win_size = 100;
    float k_mu = 0.4;
    float k_sigma = 0.33;
    KwsAgcParams()
    {

    }
};