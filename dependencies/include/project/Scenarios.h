#ifndef SCENARIOS_H
#define SCENARIOS_H

#include <vector>
#include <memory>
#include <string>
#include "Object.h"

enum class ScenarioType {
	SolarSystem,
	BinaryStars,
	EarthMoonSun,
	Symmetric3BodyV1,
	Symmetric3BodyV2,
	Symmetric4BodyV1,
	Symmetric4BodyV2,
	Symmetric5Body,
	Count
};

const char* scenarioName(ScenarioType type);

void loadScenario(ScenarioType type,
	std::vector<std::unique_ptr<Object>>& objects);

#endif
