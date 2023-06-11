#ifndef OPTION_PRICING_H
#define OPTION_PRICING_H
#define _USE_MATH_DEFINES

#include <bits/stdc++.h>
#include <random>
#include <time.h>
#include <cmath>

#define NUM_STEPS 10

struct OptionPricing
{
	double spotPrice;
    double strikePrice;
    double timeToMaturity;
    double riskFreeRate;
    double volatility;
    double callPrice;
};

double SND(double n) 
{
    return 0.5 * (1 + erf(n / sqrt(2)));
}


double blackScholesEuropean(const OptionPricing& priceData)
{
	double spotPrice = priceData.spotPrice;
	double strikePrice = priceData.strikePrice;
	double timeToMaturity = priceData.timeToMaturity;
	double riskFreeRate = priceData.riskFreeRate;
	double volatility = volatility;

	double d1 = (log(spotPrice / strikePrice) + 
				(riskFreeRate + pow(volatility, 2) / 2) * timeToMaturity) 
				/ (volatility * sqrt(timeToMaturity));
	double d2 = d1 - volatility * sqrt(timeToMaturity);
	double callOptionPrice = spotPrice * SND(d1) - strikePrice * 
		 		exp(-riskFreeRate * timeToMaturity) * SND(d2);
	return callOptionPrice;
}

double binomialEuropean(const OptionPricing& priceData)
{

	double spotPrice = priceData.spotPrice;
	double strikePrice = priceData.strikePrice;
	double timeToMaturity = priceData.timeToMaturity;
	double riskFreeRate = priceData.riskFreeRate;
	double volatility = volatility;

    double deltaT = timeToMaturity / NUM_STEPS; 
    double discountFactor = exp(-riskFreeRate * deltaT); 
    double upFactor = exp(volatility * sqrt(deltaT));
    double downFactor = 1.0 / upFactor;
    double probabilityUp = (exp(riskFreeRate * deltaT) - downFactor) 
    			/ (upFactor - downFactor);
    double optionTree[NUM_STEPS + 1];

    for (int i = 0; i <= NUM_STEPS; i++) 
    {
        double spotPriceAtExpiration = spotPrice * 
        		pow(upFactor, NUM_STEPS - i) * pow(downFactor, i);
        optionTree[i] = std::max(0.0, spotPriceAtExpiration - strikePrice);
    }

    for (int i = NUM_STEPS - 1; i >= 0; i--) 
    {
        for (int j = 0; j <= i; j++) 
        {
            optionTree[j] = (optionTree[j] * (1 - riskFreeRate * deltaT) + 
            	optionTree[j + 1] * riskFreeRate * deltaT) / discountFactor;
        }
    }
    return optionTree[0];
}

void generateRandomOptionInputs(OptionPricing& priceData) 
{
    double minSpotPrice = 50.0;
    double maxSpotPrice = 200.0;
    double minStrikePrice = 50.0;
    double maxStrikePrice = 200.0;
    double minTimeToMaturity = 0.25;
    double maxTimeToMaturity = 2.0;
    double minRiskFreeRate = 0.01;
    double maxRiskFreeRate = 0.1;
    double minVolatility = 0.1;
    double maxVolatility = 0.5;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> spotPriceDist(minSpotPrice, maxSpotPrice);
    std::uniform_real_distribution<> strikePriceDist(minStrikePrice, maxStrikePrice);
    std::uniform_real_distribution<> timeToMaturityDist(minTimeToMaturity, maxTimeToMaturity);
    std::uniform_real_distribution<> riskFreeRateDist(minRiskFreeRate, maxRiskFreeRate);
    std::uniform_real_distribution<> volatilityDist(minVolatility, maxVolatility);

    priceData.spotPrice = spotPriceDist(gen);
    priceData.strikePrice = strikePriceDist(gen);
    priceData.timeToMaturity = timeToMaturityDist(gen);
    priceData.riskFreeRate = riskFreeRateDist(gen);
    priceData.volatility = volatilityDist(gen);
}



#endif