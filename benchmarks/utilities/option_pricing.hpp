#ifndef OPTION_PRICING_H
#define OPTION_PRICING_H
#define _USE_MATH_DEFINES

#include <bits/stdc++.h>
#include <random>
#include <time.h>
#include <cmath>

#define NUM_STEPS 10

struct OptionPricing {
    double spotPrice;
    double strikePrice;
    double timeToMaturity;
    double riskFreeRate;
    double volatility;
};

double SND(double n) {
    return 0.5 * (1 + erf(n / sqrt(2)));
}

double blackScholesEuropean(const OptionPricing priceData) {
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

double binomialEuropean(const OptionPricing priceData) {


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

    for (int i = 0; i <= NUM_STEPS; i++) {
        double spotPriceAtExpiration = spotPrice * 
        		pow(upFactor, NUM_STEPS - i) * pow(downFactor, i);
        optionTree[i] = std::max(0.0, spotPriceAtExpiration - strikePrice);
    }

    for (int i = NUM_STEPS - 1; i >= 0; i--) {
        for (int j = 0; j <= i; j++) {
            optionTree[j] = (optionTree[j] * (1 - riskFreeRate * deltaT) + 
            	optionTree[j + 1] * riskFreeRate * deltaT) / discountFactor;
        }
    }
    return optionTree[0];
}

void generateRandomOptionInputs(OptionPricing& priceData) {
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

int generatePoissonRandomVariable(double lambda) {
    int x = 0;
    double p = exp(-lambda);
    double s = p;
    double u = rand() / (double)RAND_MAX;
    while (u > s) {
        x++;
        p *= lambda / x;
        s += p;
    }
    return x;
}

double generateStandardNormalRandomVariable() {
    double u1 = rand() / (double)RAND_MAX;
    double u2 = rand() / (double)RAND_MAX;
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}


double hestonCallOptionPrice(const OptionPricing priceData) {

    double kappa = 2.0;
    double theta = 0.04;
    double sigma = 0.3;
    double rho = -0.5;
    double v0 = priceData.volatility * priceData.volatility;
    double lambda = 0.0;
    double muJ = 0.0;
    double sigmaJ = 0.0;
    int numSteps = 100;
    double dt = priceData.timeToMaturity / numSteps;
    double drift = priceData.riskFreeRate - 0.5 * v0;
    double vol = priceData.volatility;
    double spot = priceData.spotPrice;
    double variance = v0;

    for (int i = 0; i < numSteps; ++i) {
        double dW1 = sqrt(dt) * generateStandardNormalRandomVariable();
        double dW2 = rho * dW1 + sqrt(1.0 - rho * rho) * generateStandardNormalRandomVariable();
        double dS = spot * (drift * dt + vol * sqrt(variance * dt) * dW1);
        double dV = kappa * (theta - variance) * dt + sigma * sqrt(variance * dt) * dW2;
        spot += dS;
        variance += dV;
        if (variance < 0.0) {
            variance = -variance;
        }
    }
    double callPrice = std::max(spot - priceData.strikePrice, 0.0);
    callPrice *= exp(-priceData.riskFreeRate * priceData.timeToMaturity);
    return callPrice;
}



double batesCallOptionPrice(const OptionPricing priceData) {
    double kappa = 2.0;
    double theta = 0.04;
    double sigma = 0.3;
    double rho = -0.5;
    double v0 = priceData.volatility * priceData.volatility;
    double lambda = 0.5;
    double muJ = -0.1;
    double sigmaJ = 0.25;
    int numSteps = 100;
    double dt = priceData.timeToMaturity / numSteps;
    double drift = priceData.riskFreeRate - 0.5 * v0 - lambda * (exp(muJ + 0.5 * sigmaJ * sigmaJ) - 1.0);
    double vol = priceData.volatility;
    double spot = priceData.spotPrice;
    double variance = v0;
    double jump = 0.0;

    for (int i = 0; i < numSteps; ++i) {
        double dW1 = sqrt(dt) * generateStandardNormalRandomVariable();
        double dW2 = rho * dW1 + sqrt(1.0 - rho * rho) * generateStandardNormalRandomVariable();
        double dN = generatePoissonRandomVariable(lambda * dt);
        double dS = spot * (drift * dt + vol * sqrt(variance * dt) * dW1);
        double dV = kappa * (theta - variance) * dt + sigma * sqrt(variance * dt) * dW2;
        double dJ = jump * dN;
        spot += dS;
        variance += dV;
        jump += dJ;
        if (variance < 0.0) {
            variance = -variance;
        }
    }
    double callPrice = std::max(spot + jump - priceData.strikePrice, 0.0);
    callPrice *= exp(-priceData.riskFreeRate * priceData.timeToMaturity);
    return callPrice;
}


#endif